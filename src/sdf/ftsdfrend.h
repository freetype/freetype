
#ifndef FTSDFREND_H_
#define FTSDFREND_H_

#include <freetype/ftrender.h>
#include <freetype/ftmodapi.h>
#include <freetype/internal/ftobjs.h>

FT_BEGIN_HEADER


  /**************************************************************************
   *
   * @struct:
   *   SDF_Renderer_Module
   *
   * @description:
   *   This struct extends the native renderer struct `FT_RendererRec'.
   *   It is basically used to store various parameters required by the
   *   renderer and some additional parameters which can be used to
   *   tweak the output of the renderer.
   *
   * @fields:
   *   root ::
   *     The native rendere struct.
   *
   *   spread ::
   *     This is an essential parameter/property required by the
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
   *     Setting this parameter to true makes the output image flipped
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
   *   All properties except `overlaps' is valid for both `sdf' and
   *   `bsdf' renderer.
   *
   */
  typedef struct  SDF_Renderer_Module_
  {
    FT_RendererRec  root;
    FT_UInt         spread;
    FT_Bool         flip_sign;
    FT_Bool         flip_y;
    FT_Bool         overlaps;

  } SDF_Renderer_Module, *SDF_Renderer;


  /**************************************************************************
   *
   * @renderer:
   *   ft_sdf_renderer_class
   *
   * @description:
   *   Renderer to convert `FT_Outline' to signed distance fields.
   *
   */
  FT_DECLARE_RENDERER( ft_sdf_renderer_class )


  /**************************************************************************
   *
   * @renderer:
   *   ft_bitmap_sdf_renderer_class
   *
   * @description:
   *   This is not exactly a renderer, it's just a converter which
   *   convert bitmaps to signed distance fields.
   *
   * @note:
   *   This is not a separate module, it is a part of the `sdf' module.
   *
   */
  FT_DECLARE_RENDERER( ft_bitmap_sdf_renderer_class )


FT_END_HEADER

#endif /* FTSDFREND_H_ */

/* END */
