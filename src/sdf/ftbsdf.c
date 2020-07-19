
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>

#include "ftsdf.h"
#include "ftsdferrs.h"

  /**************************************************************************
   *
   * interface functions
   *
   */

  static FT_Error
  bsdf_raster_new( FT_Memory   memory,
                   FT_Raster*  araster)
  {
    FT_UNUSED( memory );
    FT_UNUSED( araster );

    return FT_Err_Ok;
  }

  static void
  bsdf_raster_reset( FT_Raster       raster,
                     unsigned char*  pool_base,
                     unsigned long   pool_size )
  {
    /* no use of this function */
    FT_UNUSED( raster );
    FT_UNUSED( pool_base );
    FT_UNUSED( pool_size );
  }

  static FT_Error
  bsdf_raster_set_mode( FT_Raster      raster,
                        unsigned long  mode,
                        void*          args )
  {
    FT_UNUSED( raster );
    FT_UNUSED( mode );
    FT_UNUSED( args );


    return FT_Err_Ok;
  }

  static FT_Error
  bsdf_raster_render( FT_Raster                raster,
                      const FT_Raster_Params*  params )
  {
    FT_UNUSED( raster );
    FT_UNUSED( params );

    return FT_THROW( Unimplemented_Feature ) ;
  }

  static void
  bsdf_raster_done( FT_Raster  raster )
  {
    FT_UNUSED( raster );
  }

  FT_DEFINE_RASTER_FUNCS(
    ft_bitmap_sdf_raster,

    FT_GLYPH_FORMAT_BITMAP,

    (FT_Raster_New_Func)      bsdf_raster_new,       /* raster_new      */
    (FT_Raster_Reset_Func)    bsdf_raster_reset,     /* raster_reset    */
    (FT_Raster_Set_Mode_Func) bsdf_raster_set_mode,  /* raster_set_mode */
    (FT_Raster_Render_Func)   bsdf_raster_render,    /* raster_render   */
    (FT_Raster_Done_Func)     bsdf_raster_done       /* raster_done     */
  )

/* END */
