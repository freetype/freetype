
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/ftlist.h>
#include "ftsdf.h"

#include "ftsdferrs.h"

  /**************************************************************************
   *
   * typedefs
   *
   */

  typedef  FT_Vector FT_26D6_Vec;   /* with 26.6 fixed point components  */
  typedef  FT_Vector FT_16D16_Vec;  /* with 16.16 fixed point components */

  /**************************************************************************
   *
   * structures and enums
   *
   */

  typedef struct  SDF_TRaster_
  {
    FT_Memory  memory; /* used internally to allocate memory */

  } SDF_TRaster;

  /* enumeration of all the types of curve present in vector fonts */
  typedef enum  SDF_Edge_Type_
  {
    SDF_EDGE_UNDEFINED  = 0,  /* undefined, used to initialize */
    SDF_EDGE_LINE       = 1,  /* straight line segment         */
    SDF_EDGE_CONIC      = 2,  /* second order bezier curve     */
    SDF_EDGE_CUBIC      = 3   /* third order bezier curve      */

  } SDF_Edge_Type;

  /* represent a single edge in a contour */
  typedef struct  SDF_Edge_
  {
    FT_26D6_Vec    start_pos;   /* start position of the edge             */
    FT_26D6_Vec    end_pos;     /* end position of the edge               */
    FT_26D6_Vec    control_a;   /* first control point of a bezier curve  */
    FT_26D6_Vec    control_b;   /* second control point of a bezier curve */

    SDF_Edge_Type  edge_type;   /* edge identifier                        */

  } SDF_Edge;

  /* A contour represent a set of edges which make a closed */
  /* loop.                                                  */
  typedef struct  SDF_Contour_
  {
    FT_26D6_Vec  last_pos;  /* end position of the last edge    */
    FT_ListRec   edges;     /* list of all edges in the contour */

  } SDF_Contour;

  /* Represent a set a contours which makes up a complete */
  /* glyph outline.                                       */
  typedef struct  SDF_Shape_
  {
    FT_Memory   memory;    /* used internally to allocate memory  */
    FT_ListRec  contours;  /* list of all contours in the outline */

  } SDF_Shape;

  /**************************************************************************
   *
   * constants, initializer and destructor
   *
   */

  static
  const FT_Vector    zero_vector  = { 0, 0 };

  static
  const SDF_Edge     null_edge    = { { 0, 0 }, { 0, 0 },
                                      { 0, 0 }, { 0, 0 },
                                      SDF_EDGE_UNDEFINED };

  static
  const SDF_Contour  null_contour = { { 0, 0 }, { NULL, NULL } };

  static
  const SDF_Shape    null_shape   = { NULL, { NULL, NULL } };

  /* Creates a new `SDF_Edge' on the heap and assigns the `edge' */
  /* pointer to the newly allocated memory.                      */
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

    FT_QNEW( ptr );
    if ( error == FT_Err_Ok )
    {
      *ptr = null_edge;
      *edge = ptr;
    }

  Exit:
    return error;
  }

  /* Frees the allocated `edge' variable. */
  static void
  sdf_edge_done( FT_Memory   memory,
                 SDF_Edge**  edge )
  {
    if ( !memory || !edge || !*edge )
      return;

    FT_FREE( *edge );
  }

  /* Used in `FT_List_Finalize' */
  static void
  sdf_edge_destructor( FT_Memory  memory,
                       void*      data,
                       void*      user )
  {
    SDF_Edge*  edge = (SDF_Edge*)data;


    sdf_edge_done( memory, &edge );
  }

  /* Creates a new `SDF_Contour' on the heap and assigns the `contour'  */
  /* pointer to the newly allocated memory. Note that the function also */
  /* allocate the `contour.edges' list variable and sets to empty list. */
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

    FT_QNEW( ptr );
    if ( error == FT_Err_Ok )
    {
      *ptr = null_contour;
      *contour = ptr;
    }

  Exit:
    return error;
  }

  /* Frees the allocated `contour' variable and also frees */
  /* the list of edges.                                    */
  static void
  sdf_contour_done( FT_Memory      memory,
                    SDF_Contour**  contour )
  {
    if ( !memory || !contour || !*contour )
      return;

    /*  */
    FT_List_Finalize( &(*contour)->edges, sdf_edge_destructor,
                      memory, NULL );

    FT_FREE( *contour );
  }

  /* Used in `FT_List_Finalize' */
  static void
  sdf_contour_destructor( FT_Memory  memory,
                          void*      data,
                          void*      user )
  {
    SDF_Contour*  contour = (SDF_Contour*)data;


    sdf_contour_done( memory, &contour );
  }

  /* Creates a new `SDF_Shape' on the heap and assigns the `shape'       */
  /* pointer to the newly allocated memory. Note that the function also  */
  /* allocate the `shape.contours' list variable and sets to empty list. */
  static FT_Error
  sdf_shape_new( FT_Memory    memory,
                 SDF_Shape**  shape )
  {
    FT_Error      error = FT_Err_Ok;
    SDF_Shape*    ptr   = NULL;


    if ( !memory || !shape )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    FT_QNEW( ptr );
    if ( error == FT_Err_Ok )
    {
      *ptr = null_shape;
      ptr->memory = memory;
      *shape = ptr;
    }

  Exit:
    return error;
  }

  /* Frees the allocated `shape' variable and also frees */
  /* the list of contours.                               */
  static void
  sdf_shape_done( FT_Memory    memory,
                  SDF_Shape**  shape )
  {
    if ( !memory || !shape || !*shape )
      return;

    /* release the list of contours */
    FT_List_Finalize( &(*shape)->contours, sdf_contour_destructor, 
                       memory, NULL );

    /* release the allocated shape struct  */
    FT_FREE( *shape );
  }
    

  /**************************************************************************
   *
   * shape decomposition functions
   *
   */

  static FT_Error
  sdf_move_to( const FT_26D6_Vec* to,
               void*              user )
  {
    /* This function is called when walking along a new contour */
    /* so add a new contour to the shape's list.                */
    SDF_Shape*    shape    = ( SDF_Shape* )user;
    SDF_Contour*  contour  = NULL;
    FT_ListNode   node     = NULL;

    FT_Error      error    = FT_Err_Ok;
    FT_Memory     memory   = shape->memory;


    if ( !to || !user )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    error = sdf_contour_new( memory, &contour );
    if ( error != FT_Err_Ok )
      goto Exit;

    FT_QNEW( node );
    if ( error != FT_Err_Ok )
      goto Exit;

    contour->last_pos = *to;

    node->data = contour;
    FT_List_Add( &shape->contours, node );

  Exit:
    return error;
  }

  static FT_Error
  sdf_line_to( const FT_26D6_Vec*  to,
               void*               user )
  {
    SDF_Shape*    shape    = ( SDF_Shape* )user;
    SDF_Edge*     edge     = NULL;
    SDF_Contour*  contour  = NULL;
    FT_ListNode   node     = NULL;

    FT_Error      error    = FT_Err_Ok;
    FT_Memory     memory   = shape->memory;


    if ( !to || !user )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    contour = ( SDF_Contour* )shape->contours.tail->data;

    if ( contour->last_pos.x == to->x && 
         contour->last_pos.y == to->y )
      goto Exit;

    error = sdf_edge_new( memory, &edge );
    if ( error != FT_Err_Ok )
      goto Exit;

    FT_QNEW( node );
    if ( error != FT_Err_Ok )
      goto Exit;

    edge->edge_type = SDF_EDGE_LINE;
    edge->start_pos = contour->last_pos;
    edge->end_pos   = *to;

    contour->last_pos = *to;

    node->data = edge;
    FT_List_Add( &contour->edges, node );

  Exit:
    return error;
  }

  static FT_Error
  sdf_conic_to( const FT_26D6_Vec*  control_1,
                const FT_26D6_Vec*  to,
                void*               user )
  {
    SDF_Shape*    shape    = ( SDF_Shape* )user;
    SDF_Edge*     edge     = NULL;
    SDF_Contour*  contour  = NULL;
    FT_ListNode   node     = NULL;

    FT_Error      error    = FT_Err_Ok;
    FT_Memory     memory   = shape->memory;


    if ( !control_1 || !to || !user )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    contour = ( SDF_Contour* )shape->contours.tail->data;

    error = sdf_edge_new( memory, &edge );
    if ( error != FT_Err_Ok )
      goto Exit;

    FT_QNEW( node );
    if ( error != FT_Err_Ok )
      goto Exit;

    edge->edge_type = SDF_EDGE_CONIC;
    edge->start_pos = contour->last_pos;
    edge->control_a = *control_1;
    edge->end_pos   = *to;

    contour->last_pos = *to;

    node->data = edge;
    FT_List_Add( &contour->edges, node );

  Exit:
    return error;
  }

  static FT_Error
  sdf_cubic_to( const FT_26D6_Vec*  control_1,
                const FT_26D6_Vec*  control_2,
                const FT_26D6_Vec*  to,
                void*               user )
  {
    SDF_Shape*    shape    = ( SDF_Shape* )user;
    SDF_Edge*     edge     = NULL;
    SDF_Contour*  contour  = NULL;
    FT_ListNode   node     = NULL;

    FT_Error      error    = FT_Err_Ok;
    FT_Memory     memory   = shape->memory;


    if ( !control_2 || !control_1 || !to || !user )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    contour = ( SDF_Contour* )shape->contours.tail->data;

    error = sdf_edge_new( memory, &edge );
    if ( error != FT_Err_Ok )
      goto Exit;

    FT_QNEW( node );
    if ( error != FT_Err_Ok )
      goto Exit;

    edge->edge_type = SDF_EDGE_CUBIC;
    edge->start_pos = contour->last_pos;
    edge->control_a = *control_1;
    edge->control_b = *control_2;
    edge->end_pos   = *to;

    contour->last_pos = *to;

    node->data = edge;
    FT_List_Add( &contour->edges, node );

  Exit:
    return error;
  }

  FT_DEFINE_OUTLINE_FUNCS(
      sdf_decompose_funcs,

      (FT_Outline_MoveTo_Func)  sdf_move_to,   /* move_to  */
      (FT_Outline_LineTo_Func)  sdf_line_to,   /* line_to  */
      (FT_Outline_ConicTo_Func) sdf_conic_to,  /* conic_to */
      (FT_Outline_CubicTo_Func) sdf_cubic_to,  /* cubic_to */

      0,                                       /* shift    */
      0                                        /* delta    */
  )

  /* function decomposes the outline and puts it into the `shape' struct */
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

    error = FT_Outline_Decompose( outline, &sdf_decompose_funcs, (void*)shape );

  Exit:
    return error;
  }

  /**************************************************************************
   *
   * interface functions
   *
   */

  static FT_Error
  sdf_raster_new( FT_Memory   memory,
                  FT_Raster*  araster)
  {
    FT_Error      error  = FT_Err_Ok;
    SDF_TRaster*  raster = NULL;


    *araster = 0;
    if ( !FT_ALLOC( raster, sizeof( SDF_TRaster ) ) )
    {
      raster->memory = memory;
      *araster = (FT_Raster)raster;
    }

    return error;
  }

  static void
  sdf_raster_reset( FT_Raster       raster,
                    unsigned char*  pool_base,
                    unsigned long   pool_size )
  {
    /* no use of this function */
    FT_UNUSED( raster );
    FT_UNUSED( pool_base );
    FT_UNUSED( pool_size );
  }

  static FT_Error
  sdf_raster_set_mode( FT_Raster      raster,
                       unsigned long  mode,
                       void*          args )
  {
    FT_UNUSED( raster );
    FT_UNUSED( mode );
    FT_UNUSED( args );


    return FT_Err_Ok;
  }

  static FT_Error
  sdf_raster_render( FT_Raster                raster,
                     const FT_Raster_Params*  params )
  {
    FT_UNUSED( raster );
    FT_UNUSED( params );

    FT_Memory  memory = (FT_Memory)((SDF_TRaster*)raster)->memory;

    SDF_Shape * shape = NULL;
    sdf_shape_new( memory, &shape );

    sdf_outline_decompose( params->source, shape );

    sdf_shape_done( memory, &shape );

    return FT_THROW( Unimplemented_Feature );
  }

  static void
  sdf_raster_done( FT_Raster  raster )
  {
    FT_Memory  memory = (FT_Memory)((SDF_TRaster*)raster)->memory;


    FT_FREE( raster );
  }

  FT_DEFINE_RASTER_FUNCS(
    ft_sdf_raster,

    FT_GLYPH_FORMAT_OUTLINE,

    (FT_Raster_New_Func)      sdf_raster_new,       /* raster_new      */
    (FT_Raster_Reset_Func)    sdf_raster_reset,     /* raster_reset    */
    (FT_Raster_Set_Mode_Func) sdf_raster_set_mode,  /* raster_set_mode */
    (FT_Raster_Render_Func)   sdf_raster_render,    /* raster_render   */
    (FT_Raster_Done_Func)     sdf_raster_done       /* raster_done     */
  )

/* END */
