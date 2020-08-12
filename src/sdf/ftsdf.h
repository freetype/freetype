
#ifndef FTSDF_H_
#define FTSDF_H_

#include <ft2build.h>
#include FT_CONFIG_CONFIG_H
#include <freetype/ftimage.h>

/* common properties and function */
#include "ftsdfcommon.h"

FT_BEGIN_HEADER

  /**************************************************************************
   *
   * @struct:
   *   SDF_Raster_Params
   *
   * @description:
   *   This struct must be used for the raster render function
   *   `FT_Raster_Render_Func' instead of `FT_Raster_Params' because
   *   the rasterizer require some addition information to render properly.
   *   So, this struct is used to pass additional parameter to the
   *   rasterizer.
   *
   * @fields:
   *   root ::
   *     The native raster params struct.
   *
   *   spread ::
   *     This is and essential parameter/property required by the
   *     rendere. `spread' defines the maximum unsigned value that
   *     will be present in the final SDF output. For default value
   *     check `ftsdfcommon.h'.
   *
   *   flip_sign ::
   *     By default the position values are inside the contours i.e.
   *     filled by a contour. If this property is true then that output
   *     will be opposite from the default i.e. negative will be filled
   *     by a contour.
   *
   *   flip_y ::
   *     Setting this parameter to true maked the output image flipped
   *     along the y-axis.
   *
   *   overlaps ::
   *     Set this to true to generate SDF for glyphs having overlapping
   *     contours. The overlapping support is limited to glyph which do
   *     not have self intersecting contours. Also, removing overlaps
   *     require a considerable amount of extra memory and this is not
   *     valid while generating SDF from bitmap.
   *
   * @note:
   *   It is valid for both `sdf' and `bsdf' renderer.
   *
   */
  typedef struct  SDF_Raster_Params_
  {
    FT_Raster_Params  root;
    FT_UInt           spread;
    FT_Bool           flip_sign;
    FT_Bool           flip_y;
    FT_Bool           overlaps;

  } SDF_Raster_Params;

  /* rasterizer to convert outline to SDF */
  FT_EXPORT_VAR( const FT_Raster_Funcs ) ft_sdf_raster;

  /* rasterizer to convert bitmap to SDF */
  FT_EXPORT_VAR( const FT_Raster_Funcs ) ft_bitmap_sdf_raster;

FT_END_HEADER

#endif /* FTSDF_H_ */

/* END */
