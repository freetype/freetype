
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>

#include "ftsdf.h"
#include "ftsdferrs.h"

  /**************************************************************************
   *
   * useful macros
   *
   */

  /* Convenient macro which calls the function */
  /* and returns if any error occurs.          */
  #define FT_CALL( x ) do                          \
                       {                           \
                         error = ( x );            \
                         if ( error != FT_Err_Ok ) \
                           goto Exit;              \
                       } while ( 0 )

  /**************************************************************************
   *
   * rasterizer functions
   *
   */

  static FT_Error
  bsdf_copy_source_to_target( const FT_Bitmap*  source,
                              FT_Bitmap*        target )
  {
    FT_Error  error         = FT_Err_Ok;
    FT_Bool   is_monochrome = 0;

    FT_Int    x_diff, y_diff;
    FT_Int    num_channels;
    FT_Byte*  t, s;

    /* again check the parameters (probably unnecessary) */
    if ( !source || !target )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* Because of the way we convert bitmap to SDF     */
    /* i.e. aligning the source to the center of the   */
    /* target, the target's width/rows must be checked */
    /* before copying.                                 */
    if ( target->width < source->width ||
         target->rows  < source->rows )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* check pixel mode */
    if ( source->pixel_mode == FT_PIXEL_MODE_NONE )
    {
      FT_ERROR(( "[bsdf] bsdf_copy_source_to_target: "
                 "Invalid pixel mode of source bitmap" ));
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* make sure target bitmap is 16bpp */
    if ( target->pixel_mode != FT_PIXEL_MODE_GRAY16 )
    {
      FT_ERROR(( "[bsdf] bsdf_copy_source_to_target: "
                 "Invalid pixel mode of target bitmap" ));
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

  #ifdef FT_DEBUG_LEVEL_TRACE
    if ( source->pixel_mode == FT_PIXEL_MODE_MONO )
    {
      FT_TRACE0(( "[bsdf] bsdf_copy_source_to_target:\n"
                  "The `bsdf' renderer can convert monochrome bitmap\n"
                  "to SDF, but the results are not perfect because there\n"
                  "is no way to approximate actual outline from monochrome\n"
                  "bitmap. Consider using anti-aliased bitmap instead.\n" ));
    }
  #endif

    /* Calculate the difference in width and rows */
    /* of the target and source.                  */
    x_diff = target->width - source->width;
    y_diff = target->rows - source->rows;

    x_diff /= 2;
    y_diff /= 2;

    t = target->buffer;
    s = source->buffer;

    /* For now we only support pixel mode `FT_PIXEL_MODE_MONO'  */
    /* and `FT_PIXEL_MODE_GRAY'. More will be added later.      */
    /* [NOTE]: We can also use `FT_Bitmap_Convert' to convert   */
    /*         bitmap to 8bpp. To avoid extra allocation and    */
    /*         since the target bitmap can be 16bpp we manually */
    /*         convert the source bitmap to desired bpp.        */
    switch ( source->pixel_mode ) {
    case FT_PIXEL_MODE_MONO:
      /* [TODO] */
      error = FT_THROW( Unimplemented_Feature );
      break;
    case FT_PIXEL_MODE_GRAY:
      /* [TODO] */
      error = FT_THROW( Unimplemented_Feature );
      break;
    default:
      FT_ERROR(( "[bsdf] bsdf_copy_source_to_target: "
                 "unsopported pixel mode of source bitmap\n" ));
      error = FT_THROW( Unimplemented_Feature );
      break;
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
    FT_Error    error  = FT_Err_Ok;
    FT_Bitmap*  source = NULL;
    FT_Bitmap*  target = NULL;

    const SDF_Raster_Params*  sdf_params = (const SDF_Raster_Params*)params;


    FT_UNUSED( raster );

    /* check for valid parameters */
    if ( !params )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* check if the flag is set */
    if ( sdf_params->root.flags != FT_RASTER_FLAG_SDF )
    {
      error = FT_THROW( Raster_Corrupted );
      goto Exit;
    }

    source = sdf_params->root.source;
    target = sdf_params->root.target;

    /* check the source and target bitmap */
    if ( !source || !target )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit; 
    }

    /* check if spread is set properly */
    if ( sdf_params->spread > MAX_SPREAD ||
         sdf_params->spread < MIN_SPREAD )
    {
      FT_TRACE0(( 
        "[bsdf] bsdf_raster_render:\n"
        "       The `spread' field of `SDF_Raster_Params' is invalid,\n"
        "       the value of this field must be within [%d, %d].\n"
        "       Also, you must pass `SDF_Raster_Params' instead of the\n"
        "       default `FT_Raster_Params' while calling this function\n"
        "       and set the fields properly.\n"
        , MIN_SPREAD, MAX_SPREAD) );
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    FT_CALL( bsdf_copy_source_to_target( source, target ) );

  Exit:
    return error;
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
