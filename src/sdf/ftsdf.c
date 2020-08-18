
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/fttrigon.h>
#include "ftsdf.h"

#include "ftsdferrs.h"


  /**************************************************************************
   *
   * for tracking used memory
   *
   */

  /* The memory tracker only works when `FT_DEBUG_MEMORY` is defined; */
  /* we need some variables such as `_ft_debug_file`, which aren't    */
  /* available otherwise.                                             */
#if defined( FT_DEBUG_LEVEL_TRACE ) && defined( FT_DEBUG_MEMORY )


#undef FT_DEBUG_INNER
#undef FT_ASSIGNP_INNER

#define FT_DEBUG_INNER( exp )  ( _ft_debug_file   = __FILE__, \
                                 _ft_debug_lineno = line,     \
                                 (exp) )
#define FT_ASSIGNP_INNER( p, exp )  ( _ft_debug_file   = __FILE__, \
                                      _ft_debug_lineno = line,     \
                                      FT_ASSIGNP( p, exp ) )


  /* To be used with `FT_Memory::user' in order to track */
  /* memory allocations.                                 */
  typedef struct  SDF_MemoryUser_
  {
    void*    prev_user;
    FT_Long  total_usage;

  } SDF_MemoryUser;


  /*
   * These functions are used while allocating and deallocating memory.
   * They restore the previous user pointer before calling the allocation
   * functions.
   */

  static FT_Pointer
  sdf_alloc( FT_Memory  memory,
             FT_Long    size,
             FT_Error*  err,
             FT_Int     line )
  {
    SDF_MemoryUser*  current_user;
    FT_Pointer       ptr;
    FT_Error         error;


    current_user = (SDF_MemoryUser*)memory->user;
    memory->user = current_user->prev_user;

    if ( !FT_QALLOC( ptr, size ) )
      current_user->total_usage += size;

    memory->user = (void*)current_user;
    *err = error;

    return ptr;
  }


  static void
  sdf_free( FT_Memory    memory,
             FT_Pointer  ptr,
             FT_Int      line )
  {
    SDF_MemoryUser*  current_user;


    current_user = (SDF_MemoryUser*)memory->user;
    memory->user = current_user->prev_user;

    FT_FREE( ptr );

    memory->user = (void*)current_user;
  }


#define SDF_ALLOC( ptr, size )                   \
          ( ptr = sdf_alloc( memory, size,       \
                             &error, __LINE__ ), \
            error != 0 )

#define SDF_FREE( ptr )                     \
          sdf_free( memory, ptr, __LINE__ )

#define SDF_MEMORY_TRACKER_DECLARE()  SDF_MemoryUser  sdf_memory_user

#define SDF_MEMORY_TRACKER_SETUP()                       \
          sdf_memory_user.prev_user   = memory->user;    \
          sdf_memory_user.total_usage = 0;               \
          memory->user                = &sdf_memory_user

#define SDF_MEMORY_TRACKER_DONE()                    \
          memory->user = sdf_memory_user.prev_user;  \
                                                     \
          FT_TRACE0(( "[sdf] sdf_raster_render:"     \
                      " Total memory used = %ld\n",  \
                      sdf_memory_user.total_usage ))


#else /* !FT_DEBUG_LEVEL_TRACE */


#define SDF_ALLOC  FT_QALLOC
#define SDF_FREE   FT_FREE

#define SDF_MEMORY_TRACKER_DECLARE()  FT_DUMMY_STMNT
#define SDF_MEMORY_TRACKER_SETUP()    FT_DUMMY_STMNT
#define SDF_MEMORY_TRACKER_DONE()     FT_DUMMY_STMNT


#endif /* !FT_DEBUG_LEVEL_TRACE */


  /**************************************************************************
   *
   * definitions
   *
   */

  /*
   * If set to 1, the rasterizer uses Newton-Raphson's method for finding
   * the shortest distance from a point to a conic curve.
   *
   * If set to 0, an analytical method gets used instead, which computes the
   * roots of a cubic polynomial to find the shortest distance.  However,
   * the analytical method can currently underflow; we thus use Newton's
   * method by default.
   */
#ifndef USE_NEWTON_FOR_CONIC
#define USE_NEWTON_FOR_CONIC  1
#endif

  /*
   * The number of intervals a Bezier curve gets sampled and checked to find
   * the shortest distance.
   */
#define MAX_NEWTON_DIVISIONS  4

  /*
   * The number of steps of Newton's iterations in each interval of the
   * Bezier curve.  Basically, we run Newton's approximation
   *
   *   x -= Q(t) / Q'(t)
   *
   * for each division to get the shortest distance.
   */
#define MAX_NEWTON_STEPS  4

  /*
   * The epsilon distance (in 16.16 fractional units) used for corner
   * resolving.  If the difference of two distances is less than this value
   * they will be checked for a corner if they are ambiguous.
   */
#define CORNER_CHECK_EPSILON  32

#if 0
  /*
   * Coarse grid dimension.  Will probably be removed in the future because
   * coarse grid optimization is the slowest algorithm.
   */
#define CG_DIMEN  8
#endif


  /**************************************************************************
   *
   * macros
   *
   */

#define MUL_26D6( a, b )  ( ( ( a ) * ( b ) ) / 64 )
#define VEC_26D6_DOT( p, q )  ( MUL_26D6( p.x, q.x ) + \
                                MUL_26D6( p.y, q.y ) )


  /**************************************************************************
   *
   * structures and enums
   *
   */

  /**************************************************************************
   *
   * @Struct:
   *   SDF_TRaster
   *
   * @Description:
   *   This struct is used in place of @FT_Raster and is stored within the
   *   internal FreeType renderer struct.  While rasterizing it is passed to
   *   the @FT_Raster_RenderFunc function, which then can be used however we
   *   want.
   *
   * @Fields:
   *   memory ::
   *     Used internally to allocate intermediate memory while raterizing.
   *
   */
  typedef struct  SDF_TRaster_
  {
    FT_Memory  memory;

  } SDF_TRaster;


  /**************************************************************************
   *
   * @Enum:
   *   SDF_Edge_Type
   *
   * @Description:
   *   Enumeration of all curve types present in fonts.
   *
   * @Fields:
   *   SDF_EDGE_UNDEFINED ::
   *     Undefined edge, simply used to initialize and detect errors.
   *
   *   SDF_EDGE_LINE ::
   *     Line segment with start and end point.
   *
   *   SDF_EDGE_CONIC ::
   *     A conic/quadratic Bezier curve with start, end, and one control
   *     point.
   *
   *   SDF_EDGE_CUBIC ::
   *     A cubic Bezier curve with start, end, and two control points.
   *
   */
  typedef enum  SDF_Edge_Type_
  {
    SDF_EDGE_UNDEFINED = 0,
    SDF_EDGE_LINE      = 1,
    SDF_EDGE_CONIC     = 2,
    SDF_EDGE_CUBIC     = 3

  } SDF_Edge_Type;


  /**************************************************************************
   *
   * @Enum:
   *   SDF_Contour_Orientation
   *
   * @Description:
   *   Enumeration of all orientation values of a contour.  We determine the
   *   orientation by calculating the area covered by a contour.  Contrary
   *   to values returned by @FT_Outline_Get_Orientation,
   *   `SDF_Contour_Orientation` is independent of the fill rule, which can
   *   be different for different font formats.
   *
   * @Fields:
   *   SDF_ORIENTATION_NONE ::
   *     Undefined orientation, used for initialization and error detection.
   *
   *   SDF_ORIENTATION_CW ::
   *     Clockwise orientation (positive area covered).
   *
   *   SDF_ORIENTATION_ACW ::
   *     Anti-clockwise orientation (negative area covered).
   *
   * @Note:
   *   See @FT_Outline_Get_Orientation for more details.
   *
   */
  typedef enum  SDF_Contour_Orientation_
  {
    SDF_ORIENTATION_NONE = 0,
    SDF_ORIENTATION_CW   = 1,
    SDF_ORIENTATION_ACW  = 2

  } SDF_Contour_Orientation;


  /**************************************************************************
   *
   * @Struct:
   *   SDF_Edge
   *
   * @Description:
   *   Represent an edge of a contour.
   *
   * @Fields:
   *   start_pos ::
   *     Start position of an edge.  Valid for all types of edges.
   *
   *   end_pos ::
   *     Etart position of an edge.  Valid for all types of edges.
   *
   *   control_a ::
   *     A control point of the edge.  Valid only for `SDF_EDGE_CONIC`
   *     and `SDF_EDGE_CUBIC`.
   *
   *   control_b ::
   *     Another control point of the edge.  Valid only for
   *     `SDF_EDGE_CONIC`.
   *
   *   edge_type ::
   *     Type of the edge, see @SDF_Edge_Type for all possible edge types.
   *
   *   next ::
   *     Used to create a singly linked list, which can be interpreted
   *     as a contour.
   *
   */
  typedef struct  SDF_Edge_
  {
    FT_26D6_Vec  start_pos;
    FT_26D6_Vec  end_pos;
    FT_26D6_Vec  control_a;
    FT_26D6_Vec  control_b;

    SDF_Edge_Type  edge_type;

    struct SDF_Edge_*  next;

  } SDF_Edge;


  /**************************************************************************
   *
   * @Struct:
   *   SDF_Contour
   *
   * @Description:
   *   Represent a complete contour, which contains a list of edges.
   *
   * @Fields:
   *   last_pos ::
   *     Contains the value of `end_pos' of the last edge in the list of
   *     edges.  Useful while decomposing the outline with
   *     @FT_Outline_Decompose.
   *
   *   edges ::
   *     Linked list of all the edges that make the contour.
   *
   *   next ::
   *     Used to create a singly linked list, which can be interpreted as a
   *     complete shape or @FT_Outline.
   *
   */
  typedef struct  SDF_Contour_
  {
    FT_26D6_Vec  last_pos;
    SDF_Edge*    edges;

    struct SDF_Contour_*  next;

  } SDF_Contour;


  /**************************************************************************
   *
   * @Struct:
   *   SDF_Shape
   *
   * @Description:
   *   Represent a complete shape, which is the decomposition of
   *   @FT_Outline.
   *
   * @Fields:
   *   memory ::
   *     Used internally to allocate memory.
   *
   *   contours ::
   *     Linked list of all the contours that make the shape.
   *
   */
  typedef struct  SDF_Shape_
  {
    FT_Memory     memory;
    SDF_Contour*  contours;

  } SDF_Shape;


  /**************************************************************************
   *
   * @Struct:
   *   SDF_Signed_Distance
   *
   * @Description:
   *   Represent signed distance of a point, i.e., the distance of the edge
   *   nearest to the point.
   *
   * @Fields:
   *   distance ::
   *     Distance of the point from the nearest edge.  Can be squared or
   *     absolute depending on the `USE_SQUARED_DISTANCES` macro defined in
   *     file `ftsdfcommon.h`.
   *
   *   cross ::
   *     Cross product of the shortest distance vector (i.e., the vector
   *     from the point to the nearest edge) and the direction of the edge
   *     at the nearest point.  This is used to resolve ambiguities of
   *     `sign`.
   *
   *   sign ::
   *     A value used to indicate whether the distance vector is outside or
   *     inside the contour corresponding to the edge.
   *
   * @Note:
   *   `sign` may or may not be correct, therefore it must be checked
   *   properly in case there is an ambiguity.
   *
   */
  typedef struct SDF_Signed_Distance_
  {
    FT_16D16  distance;
    FT_16D16  cross;
    FT_Char   sign;

  } SDF_Signed_Distance;


  /**************************************************************************
   *
   * @Struct:
   *   SDF_Params
   *
   * @Description:
   *   Yet another internal parameters required by the rasterizer.
   *
   * @Fields:
   *   orientation ::
   *     This is not the @SDF_Contour_Orientation value but @FT_Orientation,
   *     which determines whether clockwise-oriented outlines are to be
   *     filled or anti-clockwise-oriented ones.
   *
   *   flip_sign ::
   *     If set to true, flip the sign.  By default the points filled by the
   *     outline are positive.
   *
   *   flip_y ::
   *     If set to true the output bitmap is upside-down.  Can be useful
   *     because OpenGL and DirectX use different coordinate systems for
   *     textures.
   *
   *   overload_sign ::
   *     In the subdivision and bounding box optimization, the default
   *     outside sign is taken as -1.  This parameter can be used to modify
   *     that behaviour.  For example, while generating SDF for a single
   *     counter-clockwise contour, the outside sign should be 1.
   *
   */
  typedef struct SDF_Params_
  {
    FT_Orientation  orientation;
    FT_Bool         flip_sign;
    FT_Bool         flip_y;

    FT_Int  overload_sign;

  } SDF_Params;


  /**************************************************************************
   *
   * constants, initializer, and destructor
   *
   */

  static
  const FT_Vector  zero_vector = { 0, 0 };

  static
  const SDF_Edge  null_edge = { { 0, 0 }, { 0, 0 },
                                { 0, 0 }, { 0, 0 },
                                SDF_EDGE_UNDEFINED, NULL };

  static
  const SDF_Contour  null_contour = { { 0, 0 }, NULL, NULL };

  static
  const SDF_Shape  null_shape = { NULL, NULL };

  static
  const SDF_Signed_Distance  max_sdf = { INT_MAX, 0, 0 };


  /* Create a new @SDF_Edge on the heap and assigns the `edge` */
  /* pointer to the newly allocated memory.                    */
  static FT_Error
  sdf_edge_new( FT_Memory   memory,
                SDF_Edge**  edge )
  {
    FT_Error   error = FT_Err_Ok;
    SDF_Edge*  ptr   = NULL;


    if ( !memory || !edge )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    if ( !SDF_ALLOC( ptr, sizeof ( *ptr ) ) )
    {
      *ptr = null_edge;
      *edge = ptr;
    }

  Exit:
    return error;
  }


  /* Free the allocated `edge` variable. */
  static void
  sdf_edge_done( FT_Memory   memory,
                 SDF_Edge**  edge )
  {
    if ( !memory || !edge || !*edge )
      return;

    SDF_FREE( *edge );
  }


  /* Create a new @SDF_Contour on the heap and assign     */
  /* the `contour` pointer to the newly allocated memory. */
  static FT_Error
  sdf_contour_new( FT_Memory      memory,
                   SDF_Contour**  contour )
  {
    FT_Error      error = FT_Err_Ok;
    SDF_Contour*  ptr   = NULL;


    if ( !memory || !contour )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    if ( !SDF_ALLOC( ptr, sizeof ( *ptr ) ) )
    {
      *ptr     = null_contour;
      *contour = ptr;
    }

  Exit:
    return error;
  }


  /* Free the allocated `contour` variable. */
  /* Also free the list of edges.           */
  static void
  sdf_contour_done( FT_Memory      memory,
                    SDF_Contour**  contour )
  {
    SDF_Edge*  edges;
    SDF_Edge*  temp;


    if ( !memory || !contour || !*contour )
      return;

    edges = (*contour)->edges;

    /* release all edges */
    while ( edges )
    {
      temp  = edges;
      edges = edges->next;

      sdf_edge_done( memory, &temp );
    }

    SDF_FREE( *contour );
  }


  /* Create a new @SDF_Shape on the heap and assign     */
  /* the `shape` pointer to the newly allocated memory. */
  static FT_Error
  sdf_shape_new( FT_Memory    memory,
                 SDF_Shape**  shape )
  {
    FT_Error    error = FT_Err_Ok;
    SDF_Shape*  ptr   = NULL;


    if ( !memory || !shape )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    if ( !SDF_ALLOC( ptr, sizeof ( *ptr ) ) )
    {
      *ptr        = null_shape;
      ptr->memory = memory;
      *shape      = ptr;
    }

  Exit:
    return error;
  }


  /* Free the allocated `shape` variable. */
  /* Also free the list of contours.      */
  static void
  sdf_shape_done( SDF_Shape**  shape )
  {
    FT_Memory     memory;
    SDF_Contour*  contours;
    SDF_Contour*  temp;


    if ( !shape || !*shape )
      return;

    memory   = (*shape)->memory;
    contours = (*shape)->contours;

    if ( !memory )
      return;

    /* release all contours */
    while ( contours )
    {
      temp     = contours;
      contours = contours->next;

      sdf_contour_done( memory, &temp );
    }

    /* release the allocated shape struct  */
    SDF_FREE( *shape );
  }


  /**************************************************************************
   *
   * shape decomposition functions
   *
   */

  /* This function is called when starting a new contour at `to`, */
  /* which gets added to the shape's list.                        */
  static FT_Error
  sdf_move_to( const FT_26D6_Vec* to,
               void*              user )
  {
    SDF_Shape*    shape   = ( SDF_Shape* )user;
    SDF_Contour*  contour = NULL;

    FT_Error   error  = FT_Err_Ok;
    FT_Memory  memory = shape->memory;


    if ( !to || !user )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    FT_CALL( sdf_contour_new( memory, &contour ) );

    contour->last_pos = *to;
    contour->next     = shape->contours;
    shape->contours   = contour;

  Exit:
    return error;
  }


  /* This function is called when there is a line in the      */
  /* contour.  The line starts at the previous edge point and */
  /* stops at `to`.                                           */
  static FT_Error
  sdf_line_to( const FT_26D6_Vec*  to,
               void*               user )
  {
    SDF_Shape*    shape    = ( SDF_Shape* )user;
    SDF_Edge*     edge     = NULL;
    SDF_Contour*  contour  = NULL;

    FT_Error      error    = FT_Err_Ok;
    FT_Memory     memory   = shape->memory;


    if ( !to || !user )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    contour = shape->contours;

    if ( contour->last_pos.x == to->x &&
         contour->last_pos.y == to->y )
      goto Exit;

    FT_CALL( sdf_edge_new( memory, &edge ) );

    edge->edge_type = SDF_EDGE_LINE;
    edge->start_pos = contour->last_pos;
    edge->end_pos   = *to;

    edge->next        = contour->edges;
    contour->edges    = edge;
    contour->last_pos = *to;

  Exit:
    return error;
  }


  /* This function is called when there is a conic Bezier curve   */
  /* in the contour.  The curve starts at the previous edge point */
  /* and stops at `to`, with control point `control_1`.           */
  static FT_Error
  sdf_conic_to( const FT_26D6_Vec*  control_1,
                const FT_26D6_Vec*  to,
                void*               user )
  {
    SDF_Shape*    shape    = ( SDF_Shape* )user;
    SDF_Edge*     edge     = NULL;
    SDF_Contour*  contour  = NULL;

    FT_Error   error  = FT_Err_Ok;
    FT_Memory  memory = shape->memory;


    if ( !control_1 || !to || !user )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    contour = shape->contours;

    FT_CALL( sdf_edge_new( memory, &edge ) );

    edge->edge_type = SDF_EDGE_CONIC;
    edge->start_pos = contour->last_pos;
    edge->control_a = *control_1;
    edge->end_pos   = *to;

    edge->next        = contour->edges;
    contour->edges    = edge;
    contour->last_pos = *to;

  Exit:
    return error;
  }


  /* This function is called when there is a cubic Bezier curve   */
  /* in the contour.  The curve starts at the previous edge point */
  /* and stops at `to`, with two control points `control_1` and   */
  /* `control_2`.                                                 */
  static FT_Error
  sdf_cubic_to( const FT_26D6_Vec*  control_1,
                const FT_26D6_Vec*  control_2,
                const FT_26D6_Vec*  to,
                void*               user )
  {
    SDF_Shape*    shape    = ( SDF_Shape* )user;
    SDF_Edge*     edge     = NULL;
    SDF_Contour*  contour  = NULL;

    FT_Error   error  = FT_Err_Ok;
    FT_Memory  memory = shape->memory;


    if ( !control_2 || !control_1 || !to || !user )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    contour = shape->contours;

    FT_CALL( sdf_edge_new( memory, &edge ) );

    edge->edge_type = SDF_EDGE_CUBIC;
    edge->start_pos = contour->last_pos;
    edge->control_a = *control_1;
    edge->control_b = *control_2;
    edge->end_pos   = *to;

    edge->next        = contour->edges;
    contour->edges    = edge;
    contour->last_pos = *to;

  Exit:
    return error;
  }


  /* Construct the structure to hold all four outline */
  /* decomposition functions.                         */
  FT_DEFINE_OUTLINE_FUNCS(
    sdf_decompose_funcs,

    (FT_Outline_MoveTo_Func) sdf_move_to,   /* move_to  */
    (FT_Outline_LineTo_Func) sdf_line_to,   /* line_to  */
    (FT_Outline_ConicTo_Func)sdf_conic_to,  /* conic_to */
    (FT_Outline_CubicTo_Func)sdf_cubic_to,  /* cubic_to */

    0,                                      /* shift    */
    0                                       /* delta    */
  )


  /* Decompose `outline` and put it into the `shape` structure.  */
  static FT_Error
  sdf_outline_decompose( FT_Outline*  outline,
                         SDF_Shape*   shape )
  {
    FT_Error  error = FT_Err_Ok;


    if ( !outline || !shape )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    error = FT_Outline_Decompose( outline,
                                  &sdf_decompose_funcs,
                                  (void*)shape );

  Exit:
    return error;
  }


  /**************************************************************************
   *
   * utility functions
   *
   */

  /* Return the control box of a edge.  The control box is a rectangle */
  /* in which all the control points can fit tightly.                  */
  static FT_CBox
  get_control_box( SDF_Edge  edge )
  {
    FT_CBox  cbox;
    FT_Bool  is_set = 0;


    switch ( edge.edge_type )
    {
    case SDF_EDGE_CUBIC:
      cbox.xMin = edge.control_b.x;
      cbox.xMax = edge.control_b.x;
      cbox.yMin = edge.control_b.y;
      cbox.yMax = edge.control_b.y;

      is_set = 1;
      /* fall through */

    case SDF_EDGE_CONIC:
      if ( is_set )
      {
        cbox.xMin = edge.control_a.x < cbox.xMin
                    ? edge.control_a.x
                    : cbox.xMin;
        cbox.xMax = edge.control_a.x > cbox.xMax
                    ? edge.control_a.x
                    : cbox.xMax;

        cbox.yMin = edge.control_a.y < cbox.yMin
                    ? edge.control_a.y
                    : cbox.yMin;
        cbox.yMax = edge.control_a.y > cbox.yMax
                    ? edge.control_a.y
                    : cbox.yMax;
      }
      else
      {
        cbox.xMin = edge.control_a.x;
        cbox.xMax = edge.control_a.x;
        cbox.yMin = edge.control_a.y;
        cbox.yMax = edge.control_a.y;

        is_set = 1;
      }
      /* fall through */

    case SDF_EDGE_LINE:
      if ( is_set )
      {
        cbox.xMin = edge.start_pos.x < cbox.xMin
                    ? edge.start_pos.x
                    : cbox.xMin;
        cbox.xMax = edge.start_pos.x > cbox.xMax
                    ? edge.start_pos.x
                    : cbox.xMax;

        cbox.yMin = edge.start_pos.y < cbox.yMin
                    ? edge.start_pos.y
                    : cbox.yMin;
        cbox.yMax = edge.start_pos.y > cbox.yMax
                    ? edge.start_pos.y
                    : cbox.yMax;
      }
      else
      {
        cbox.xMin = edge.start_pos.x;
        cbox.xMax = edge.start_pos.x;
        cbox.yMin = edge.start_pos.y;
        cbox.yMax = edge.start_pos.y;
      }

      cbox.xMin = edge.end_pos.x < cbox.xMin
                  ? edge.end_pos.x
                  : cbox.xMin;
      cbox.xMax = edge.end_pos.x > cbox.xMax
                  ? edge.end_pos.x
                  : cbox.xMax;

      cbox.yMin = edge.end_pos.y < cbox.yMin
                  ? edge.end_pos.y
                  : cbox.yMin;
      cbox.yMax = edge.end_pos.y > cbox.yMax
                  ? edge.end_pos.y
                  : cbox.yMax;

      break;

    default:
      break;
    }

    return cbox;
  }


  /* Return orientation of a single contour.                    */
  /* Note that the orientation is independent of the fill rule! */
  /* So, for TTF a clockwise-oriented contour has to be filled  */
  /* and the opposite for OTF fonts.                            */
  static SDF_Contour_Orientation
  get_contour_orientation ( SDF_Contour*  contour )
  {
    SDF_Edge*  head = NULL;
    FT_26D6    area = 0;


    /* return none if invalid parameters */
    if ( !contour || !contour->edges )
      return SDF_ORIENTATION_NONE;

    head = contour->edges;

    /* Calculate the area of the control box for all edges. */
    while ( head )
    {
      switch ( head->edge_type )
      {
      case SDF_EDGE_LINE:
        area += MUL_26D6( ( head->end_pos.x - head->start_pos.x ),
                          ( head->end_pos.y + head->start_pos.y ) );
        break;

      case SDF_EDGE_CONIC:
        area += MUL_26D6( head->control_a.x - head->start_pos.x,
                          head->control_a.y + head->start_pos.y );
        area += MUL_26D6( head->end_pos.x - head->control_a.x,
                          head->end_pos.y + head->control_a.y );
        break;

      case SDF_EDGE_CUBIC:
        area += MUL_26D6( head->control_a.x - head->start_pos.x,
                          head->control_a.y + head->start_pos.y );
        area += MUL_26D6( head->control_b.x - head->control_a.x,
                          head->control_b.y + head->control_a.y );
        area += MUL_26D6( head->end_pos.x - head->control_b.x,
                          head->end_pos.y + head->control_b.y );
        break;

      default:
        return SDF_ORIENTATION_NONE;
      }

      head = head->next;
    }

    /* Clockwise contours cover a positive area, and anti-clockwise */
    /* contours cover a negative area.                              */
    if ( area > 0 )
      return SDF_ORIENTATION_CW;
    else
      return SDF_ORIENTATION_ACW;
  }


  /* This function is exactly the same as the one */
  /* in the smooth renderer.  It splits a conic   */
  /* into two conics exactly half way at t = 0.5. */
  static void
  split_conic( FT_26D6_Vec*  base )
  {
    FT_26D6  a, b;


    base[4].x = base[2].x;
    a         = base[0].x + base[1].x;
    b         = base[1].x + base[2].x;
    base[3].x = b / 2;
    base[2].x = ( a + b ) / 4;
    base[1].x = a / 2;

    base[4].y = base[2].y;
    a         = base[0].y + base[1].y;
    b         = base[1].y + base[2].y;
    base[3].y = b / 2;
    base[2].y = ( a + b ) / 4;
    base[1].y = a / 2;
  }


  /* This function is exactly the same as the one */
  /* in the smooth renderer.  It splits a cubic   */
  /* into two cubics exactly half way at t = 0.5. */
  static void
  split_cubic( FT_26D6_Vec*  base )
  {
    FT_26D6  a, b, c;


    base[6].x = base[3].x;
    a         = base[0].x + base[1].x;
    b         = base[1].x + base[2].x;
    c         = base[2].x + base[3].x;
    base[5].x = c / 2;
    c        += b;
    base[4].x = c / 4;
    base[1].x = a / 2;
    a        += b;
    base[2].x = a / 4;
    base[3].x = ( a + c ) / 8;

    base[6].y = base[3].y;
    a         = base[0].y + base[1].y;
    b         = base[1].y + base[2].y;
    c         = base[2].y + base[3].y;
    base[5].y = c / 2;
    c        += b;
    base[4].y = c / 4;
    base[1].y = a / 2;
    a        += b;
    base[2].y = a / 4;
    base[3].y = ( a + c ) / 8;
  }


  /* Split a conic Bezier curve into a number of lines */
  /* and add them to `out'.                            */
  /*                                                   */
  /* This function uses recursion; we thus need        */
  /* parameter `max_splits' for stopping.              */
  static FT_Error
  split_sdf_conic( FT_Memory     memory,
                   FT_26D6_Vec*  control_points,
                   FT_Int        max_splits,
                   SDF_Edge**    out )
  {
    FT_Error     error = FT_Err_Ok;
    FT_26D6_Vec  cpos[5];
    SDF_Edge*    left,*  right;


    if ( !memory || !out  )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* split conic outline */
    cpos[0] = control_points[0];
    cpos[1] = control_points[1];
    cpos[2] = control_points[2];

    split_conic( cpos );

    /* If max number of splits is done */
    /* then stop and add the lines to  */
    /* the list.                       */
    if ( max_splits <= 2 )
      goto Append;

    /* Otherwise keep splitting. */
    FT_CALL( split_sdf_conic( memory, &cpos[0], max_splits / 2, out ) );
    FT_CALL( split_sdf_conic( memory, &cpos[2], max_splits / 2, out ) );

    /* [NOTE]: This is not an efficient way of   */
    /* splitting the curve.  Check the deviation */
    /* instead and stop if the deviation is less */
    /* than a pixel.                             */

    goto Exit;

  Append:
    /* Do allocation and add the lines to the list. */

    FT_CALL( sdf_edge_new( memory, &left ) );
    FT_CALL( sdf_edge_new( memory, &right ) );

    left->start_pos  = cpos[0];
    left->end_pos    = cpos[2];
    left->edge_type  = SDF_EDGE_LINE;

    right->start_pos = cpos[2];
    right->end_pos   = cpos[4];
    right->edge_type = SDF_EDGE_LINE;

    left->next  = right;
    right->next = (*out);
    *out        = left;

  Exit:
    return error;
  }


  /* Split a cubic Bezier curve into a number of lines */
  /* and add them to `out`.                            */
  /*                                                   */
  /* This function uses recursion; we thus need        */
  /* parameter `max_splits' for stopping.              */
  static FT_Error
  split_sdf_cubic( FT_Memory     memory,
                   FT_26D6_Vec*  control_points,
                   FT_Int        max_splits,
                   SDF_Edge**    out )
  {
    FT_Error     error = FT_Err_Ok;
    FT_26D6_Vec  cpos[7];
    SDF_Edge*    left,*  right;


    if ( !memory || !out  )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* split the conic */
    cpos[0] = control_points[0];
    cpos[1] = control_points[1];
    cpos[2] = control_points[2];
    cpos[3] = control_points[3];

    split_cubic( cpos );

    /* If max number of splits is done */
    /* then stop and add the lines to  */
    /* the list.                       */
    if ( max_splits <= 2 )
      goto Append;

    /* Otherwise keep splitting. */
    FT_CALL( split_sdf_cubic( memory, &cpos[0], max_splits / 2, out ) );
    FT_CALL( split_sdf_cubic( memory, &cpos[3], max_splits / 2, out ) );

    /* [NOTE]: This is not an efficient way of   */
    /* splitting the curve.  Check the deviation */
    /* instead and stop if the deviation is less */
    /* than a pixel.                             */

    goto Exit;

  Append:
    /* Do allocation and add the lines to the list. */

    FT_CALL( sdf_edge_new( memory, &left) );
    FT_CALL( sdf_edge_new( memory, &right) );

    left->start_pos  = cpos[0];
    left->end_pos    = cpos[3];
    left->edge_type  = SDF_EDGE_LINE;

    right->start_pos = cpos[3];
    right->end_pos   = cpos[6];
    right->edge_type = SDF_EDGE_LINE;

    left->next  = right;
    right->next = (*out);
    *out        = left;

  Exit:
    return error;
  }


  /* Subdivide an entire shape into line segments */
  /* such that it doesn't look visually different */
  /* from the original curve.                     */
  static FT_Error
  split_sdf_shape( SDF_Shape*  shape )
  {
    FT_Error   error = FT_Err_Ok;
    FT_Memory  memory;

    SDF_Contour*  contours;
    SDF_Contour*  new_contours = NULL;


    if ( !shape || !shape->memory )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    contours = shape->contours;
    memory   = shape->memory;

    /* for each contour */
    while ( contours )
    {
      SDF_Edge*  edges     = contours->edges;
      SDF_Edge*  new_edges = NULL;

      SDF_Contour*  tempc;


      /* for each edge */
      while ( edges )
      {
        SDF_Edge*  edge = edges;
        SDF_Edge*  temp;

        switch ( edge->edge_type )
        {
        case SDF_EDGE_LINE:
          /* Just create a duplicate edge in case     */
          /* it is a line.  We can use the same edge. */
          FT_CALL( sdf_edge_new( memory, &temp ) );

          ft_memcpy( temp, edge, sizeof ( *edge ) );

          temp->next = new_edges;
          new_edges  = temp;
          break;

        case SDF_EDGE_CONIC:
          /* Subdivide the curve and add it to the list. */
          {
            FT_26D6_Vec  ctrls[3];


            ctrls[0] = edge->start_pos;
            ctrls[1] = edge->control_a;
            ctrls[2] = edge->end_pos;

            error = split_sdf_conic( memory, ctrls, 32, &new_edges );
          }
          break;

        case SDF_EDGE_CUBIC:
          /* Subdivide the curve and add it to the list. */
          {
            FT_26D6_Vec  ctrls[4];


            ctrls[0] = edge->start_pos;
            ctrls[1] = edge->control_a;
            ctrls[2] = edge->control_b;
            ctrls[3] = edge->end_pos;

            error = split_sdf_cubic( memory, ctrls, 32, &new_edges );
          }
          break;

        default:
          error = FT_THROW( Invalid_Argument );
          goto Exit;
        }

        edges = edges->next;
      }

      /* add to the contours list */
      FT_CALL( sdf_contour_new( memory, &tempc ) );

      tempc->next  = new_contours;
      tempc->edges = new_edges;
      new_contours = tempc;
      new_edges    = NULL;

      /* deallocate the contour */
      tempc    = contours;
      contours = contours->next;

      sdf_contour_done( memory, &tempc );
    }

    shape->contours = new_contours;

  Exit:
    return error;
  }


  /**************************************************************************
   *
   * math functions
   *
   */

#if !USE_NEWTON_FOR_CONIC

  /* [NOTE]: All the functions below down until rasterizer */
  /*         can be avoided if we decide to subdivide the  */
  /*         curve into lines.                             */

  /* This function uses Newton's iteration to find */
  /* the cube root of a fixed-point integer.       */
  static FT_16D16
  cube_root( FT_16D16  val )
  {
    /* [IMPORTANT]: This function is not good as it may */
    /* not break, so use a lookup table instead.  Or we */
    /* can use an algorithm similar to `square_root`.   */

    FT_Int  v, g, c;


    if ( val == 0                  ||
         val == -FT_INT_16D16( 1 ) ||
         val ==  FT_INT_16D16( 1 ) )
      return val;

    v = val < 0 ? -val : val;
    g = square_root( v );
    c = 0;

    while ( 1 )
    {
      c = FT_MulFix( FT_MulFix( g, g ), g ) - v;
      c = FT_DivFix( c, 3 * FT_MulFix( g, g ) );

      g -= c;

      if ( ( c < 0 ? -c : c ) < 30 )
        break;
    }

    return val < 0 ? -g : g;
  }


  /* Calculate the perpendicular by using '1 - base^2'. */
  /* Then use arctan to compute the angle.              */
  static FT_16D16
  arc_cos( FT_16D16  val )
  {
    FT_16D16  p;
    FT_16D16  b   = val;
    FT_16D16  one = FT_INT_16D16( 1 );


    if ( b > one )
      b = one;
    if ( b < -one )
      b = -one;

    p = one - FT_MulFix( b, b );
    p = square_root( p );

    return FT_Atan2( b, p );
  }


  /* Compute roots of a quadratic polynomial, assign them to `out`, */
  /* and return number of real roots.                               */
  /*                                                                */
  /* The procedure can be found at                                  */
  /*                                                                */
  /*   https://mathworld.wolfram.com/QuadraticFormula.html          */
  static FT_UShort
  solve_quadratic_equation( FT_26D6   a,
                            FT_26D6   b,
                            FT_26D6   c,
                            FT_16D16  out[2] )
  {
    FT_16D16  discriminant = 0;


    a = FT_26D6_16D16( a );
    b = FT_26D6_16D16( b );
    c = FT_26D6_16D16( c );

    if ( a == 0 )
    {
      if ( b == 0 )
        return 0;
      else
      {
        out[0] = FT_DivFix( -c, b );

        return 1;
      }
    }

    discriminant = FT_MulFix( b, b ) - 4 * FT_MulFix( a, c );

    if ( discriminant < 0 )
      return 0;
    else if ( discriminant == 0 )
    {
      out[0] = FT_DivFix( -b, 2 * a );

      return 1;
    }
    else
    {
      discriminant = square_root( discriminant );

      out[0] = FT_DivFix( -b + discriminant, 2 * a );
      out[1] = FT_DivFix( -b - discriminant, 2 * a );

      return 2;
    }
  }


  /* Compute roots of a cubic polynomial, assign them to `out`, */
  /* and return number of real roots.                           */
  /*                                                            */
  /* The procedure can be found at                              */
  /*                                                            */
  /*   https://mathworld.wolfram.com/CubicFormula.html          */
  static FT_UShort
  solve_cubic_equation( FT_26D6   a,
                        FT_26D6   b,
                        FT_26D6   c,
                        FT_26D6   d,
                        FT_16D16  out[3] )
  {
    FT_16D16  q = 0;      /* intermediate */
    FT_16D16  r = 0;      /* intermediate */

    FT_16D16  a2 = b;     /* x^2 coefficients */
    FT_16D16  a1 = c;     /* x coefficients   */
    FT_16D16  a0 = d;     /* constant         */

    FT_16D16  q3   = 0;
    FT_16D16  r2   = 0;
    FT_16D16  a23  = 0;
    FT_16D16  a22  = 0;
    FT_16D16  a1x2 = 0;


    /* cutoff value for `a` to be a cubic, otherwise solve quadratic */
    if ( a == 0 || FT_ABS( a ) < 16 )
      return solve_quadratic_equation( b, c, d, out );

    if ( d == 0 )
    {
      out[0] = 0;

      return solve_quadratic_equation( a, b, c, out + 1 ) + 1;
    }

    /* normalize the coefficients; this also makes them 16.16 */
    a2 = FT_DivFix( a2, a );
    a1 = FT_DivFix( a1, a );
    a0 = FT_DivFix( a0, a );

    /* compute intermediates */
    a1x2 = FT_MulFix( a1, a2 );
    a22  = FT_MulFix( a2, a2 );
    a23  = FT_MulFix( a22, a2 );

    q = ( 3 * a1 - a22 ) / 9;
    r = ( 9 * a1x2 - 27 * a0 - 2 * a23 ) / 54;

    /* [BUG]: `q3` and `r2` still cause underflow. */

    q3 = FT_MulFix( q, q );
    q3 = FT_MulFix( q3, q );

    r2 = FT_MulFix( r, r );

    if ( q3 < 0 && r2 < -q3 )
    {
      FT_16D16  t = 0;


      q3 = square_root( -q3 );
      t  = FT_DivFix( r, q3 );

      if ( t > ( 1 << 16 ) )
        t =  ( 1 << 16 );
      if ( t < -( 1 << 16 ) )
        t = -( 1 << 16 );

      t   = arc_cos( t );
      a2 /= 3;
      q   = 2 * square_root( -q );

      out[0] = FT_MulFix( q, FT_Cos( t / 3 ) ) - a2;
      out[1] = FT_MulFix( q, FT_Cos( ( t + FT_ANGLE_PI * 2 ) / 3 ) ) - a2;
      out[2] = FT_MulFix( q, FT_Cos( ( t + FT_ANGLE_PI * 4 ) / 3 ) ) - a2;

      return 3;
    }

    else if ( r2 == -q3 )
    {
      FT_16D16  s = 0;


      s   = cube_root( r );
      a2 /= -3;

      out[0] = a2 + ( 2 * s );
      out[1] = a2 - s;

      return 2;
    }

    else
    {
      FT_16D16  s    = 0;
      FT_16D16  t    = 0;
      FT_16D16  dis  = 0;


      if ( q3 == 0 )
        dis = FT_ABS( r );
      else
        dis = square_root( q3 + r2 );

      s = cube_root( r + dis );
      t = cube_root( r - dis );
      a2 /= -3;
      out[0] = ( a2 + ( s + t ) );

      return 1;
    }
  }

#endif /* !USE_NEWTON_FOR_CONIC */


  /*************************************************************************/
  /*************************************************************************/
  /**                                                                     **/
  /**  RASTERIZER                                                         **/
  /**                                                                     **/
  /*************************************************************************/
  /*************************************************************************/

  /**************************************************************************
   *
   * @Function:
   *   resolve_corner
   *
   * @Description:
   *   At some places on the grid two edges can give opposite directions;
   *   this happens when the closest point is on one of the endpoint.  In
   *   that case we need to check the proper sign.
   *
   *   This can be visualized by an example:
   *
   *   ```
   *                x
   *
   *                   o
   *                  ^ \
   *                 /   \
   *                /     \
   *           (a) /       \  (b)
   *              /         \
   *             /           \
   *            /             v
   *   ```
   *
   *   Suppose `x` is the point whose shortest distance from an arbitrary
   *   contour we want to find out.  It is clear that `o` is the nearest
   *   point on the contour.  Now to determine the sign we do a cross
   *   product of the shortest distance vector and the edge direction, i.e.,
   *
   *   ```
   *   => sign = cross(x - o, direction(a))
   *   ```
   *
   *   Using the right hand thumb rule we can see that the sign will be
   *   positive.
   *
   *   If we use `b', however, we have
   *
   *   ```
   *   => sign = cross(x - o, direction(b))
   *   ```
   *
   *   In this case the sign will be negative.  To determine the correct
   *   sign we thus divide the plane in two halves and check which plane the
   *   point lies in.
   *
   *   ```
   *                   |
   *                x  |
   *                   |
   *                   o
   *                  ^|\
   *                 / | \
   *                /  |  \
   *           (a) /   |   \  (b)
   *              /    |    \
   *             /           \
   *            /             v
   *   ```
   *
   *   We can see that `x` lies in the plane of `a`, so we take the sign
   *   determined by `a`.  This test can be easily done by calculating the
   *   orthogonality and taking the greater one.
   *
   *   The orthogonality is simply the sinus of the two vectors (i.e.,
   *   x - o) and the corresponding direction.  We efficiently pre-compute
   *   the orthogonality with the corresponding `get_min_distance_`
   *   functions.
   *
   * @Input:
   *   sdf1 ::
   *     First signed distance (can be any of `a` or `b`).
   *
   *   sdf1 ::
   *     Second signed distance (can be any of `a` or `b`).
   *
   * @Return:
   *   The correct signed distance, which is computed by using the above
   *   algorithm.
   *
   * @Note:
   *   The function does not care about the actual distance, it simply
   *   returns the signed distance which has a larger cross product.  As a
   *   consequence, this function should not be used if the two distances
   *   are fairly apart.  In that case simply use the signed distance with
   *   a shorter absolute distance.
   *
   */
  static SDF_Signed_Distance
  resolve_corner( SDF_Signed_Distance  sdf1,
                  SDF_Signed_Distance  sdf2 )
  {
    return FT_ABS( sdf1.cross ) > FT_ABS( sdf2.cross ) ? sdf1 : sdf2;
  }

/* END */
