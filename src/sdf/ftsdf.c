
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

/* END */
