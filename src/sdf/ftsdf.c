
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>
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
    FT_List      edges      /* list of all edges in the contour */

  } SDF_Contour;

  /* Represent a set a contours which makes up a complete */
  /* glyph outline.                                       */
  typedef struct  SDF_Shape_
  {
    FT_Memory  memory;    /* used internally to allocate memory  */
    FT_List    contours;  /* list of all contours in the outline */

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
  const SDF_Contour  null_contour = { { 0, 0 }, NULL };

  static
  const SDF_Shape    null_shape   = { NULL, NULL };

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
    if ( error != FT_Err_Ok )
      *edge = NULL;
    else
    {
      *ptr = null_edge;
      *edge = ptr;
    }

  Exit:
    return error;
  }

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
    if ( error != FT_Err_Ok )
      *contour = NULL;
    else
    {
      *ptr = null_contour;
      FT_QNEW( ptr->edges );
      *contour = ptr;
    }

  Exit:
    return error;
  }

  static void
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
    if ( error != FT_Err_Ok )
      *shape = NULL;
    else
    {
      *ptr = null_shape;
      FT_QNEW( ptr->contours );
      *shape = ptr;
    }

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
