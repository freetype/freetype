
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

  /* TEMPORARY */
  typedef enum Optimizations_ {
    OPTIMIZATION_NONE = 0,  /* default: check all points against all edges  */
    OPTIMIZATION_BB   = 1,  /* use bounding box to check nearby grid points */
    OPTIMIZATION_SUB  = 2,  /* subdivide then use bounding box              */
    OPTIMIZATION_CG   = 3   /* use coarse grid to only check relevant edges */

  } Optimizations;
  /* --------- */

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
    FT_Bool           flip_sign;
    FT_Bool           flip_y;

    /* TEMPORARY */
    FT_Int            optimization;

  } SDF_Raster_Params;

  /* rasterizer to convert outline to SDF */
  FT_EXPORT_VAR( const FT_Raster_Funcs ) ft_sdf_raster;

  /* rasterizer to convert bitmap to SDF */
  FT_EXPORT_VAR( const FT_Raster_Funcs ) ft_bitmap_sdf_raster;

FT_END_HEADER

#endif /* FTSDF_H_ */

/* END */
