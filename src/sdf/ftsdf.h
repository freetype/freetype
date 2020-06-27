
#ifndef FTSDF_H_
#define FTSDF_H_

#include <ft2build.h>
#include FT_CONFIG_CONFIG_H
#include <freetype/ftimage.h>

FT_BEGIN_HEADER

  /* default spread value */
  #define DEFAULT_SPREAD  8

  /* minimum spread supported by the rasterizer. */
  #define MIN_SPREAD      2

  /* maximum spread supported by the rasterizer. */
  #define MAX_SPREAD      32

  /**************************************************************************
   *
   * @struct:
   *   SDF_Raster_Params
   *
   * @description:
   *   This struct must be used for the raster render function
   *   `FT_Raster_Render_Func' instead of `FT_Raster_Params' because
   *   the rasterizer require some addition information to render properly.
   *
   * @fields:
   *   [TODO]
   *
   */
  typedef struct  SDF_Raster_Params_
  {
    FT_Raster_Params  root;
    FT_UInt           spread;

  } SDF_Raster_Params;

  FT_EXPORT_VAR( const FT_Raster_Funcs ) ft_sdf_raster;

FT_END_HEADER

#endif /* FTSDF_H_ */

/* END */
