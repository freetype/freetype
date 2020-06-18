
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>
#include "ftsdf.h"

#include "ftsdferrs.h"

  static int
  sdf_raster_new( FT_Memory   memory,
                  FT_Raster*  araster)
  {
    FT_Error  error = FT_THROW( Unimplemented_Feature );


    FT_UNUSED( memory );
    FT_UNUSED( araster );

    return error;
  }

  static void
  sdf_raster_reset( FT_Raster       raster,
                    unsigned char*  pool_base,
                    unsigned long   pool_size )
  {
    FT_UNUSED( raster );
    FT_UNUSED( pool_base );
    FT_UNUSED( pool_size );
  }

  static int
  sdf_raster_set_mode( FT_Raster      raster,
                       unsigned long  mode,
                       void*          args )
  {
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
    FT_UNUSED( raster );
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
