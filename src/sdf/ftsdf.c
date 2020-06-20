
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>
#include "ftsdf.h"

#include "ftsdferrs.h"

  /**************************************************************************
   *
   * structures and enums
   *
   */

  typedef struct  sdf_TRaster_
  {
    FT_Memory  memory; /* used internally to allocate memory */
  } sdf_TRaster;

  /**************************************************************************
   *
   * interface functions
   *
   */

  static int
  sdf_raster_new( FT_Memory   memory,
                  FT_Raster*  araster)
  {
    FT_Error      error  = FT_Err_Ok;
    sdf_TRaster*  raster = NULL;


    *araster = 0;
    if ( !FT_ALLOC( raster, sizeof( sdf_TRaster ) ) )
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

  static int
  sdf_raster_set_mode( FT_Raster      raster,
                       unsigned long  mode,
                       void*          args )
  {
    /* Currently there is no use for this function but later */
    /* it will be used to modify the `spread' parameter.     */
    FT_UNUSED( raster );
    FT_UNUSED( mode );
    FT_UNUSED( args );


    return 0;
  }

  static int
  sdf_raster_render( FT_Raster                raster,
                     const FT_Raster_Params*  params )
  {
    FT_UNUSED( raster );
    FT_UNUSED( params );


    return 0;
  }

  static void
  sdf_raster_done( FT_Raster  raster )
  {
    FT_Memory  memory = (FT_Memory)((sdf_TRaster*)raster)->memory;


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
