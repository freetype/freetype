
#ifndef FTDENSEREND_H_
#define FTDENSEREND_H_


#include <freetype/ftmodapi.h>
#include <freetype/ftrender.h>
#include <freetype/internal/ftobjs.h>

FT_BEGIN_HEADER

/**************************************************************************
 *
 * @struct:
 *   DENSE_Renderer_Module
 *
 * @description:
 *   This struct extends the native renderer struct `FT_RendererRec`.  It
 *   is basically used to store various parameters required by the
 *   renderer and some additional parameters that can be used to tweak the
 *   output of the renderer.
 *
 * @fields:
 *   root ::
 *     The native rendere struct.
 *
 *   spread ::
 *     This is an essential parameter/property required by the renderer.
 *     `spread` defines the maximum unsigned value that is present in the
 *     final SDF output.  For the default value check file
 *     `ftsdfcommon.h`.
 *
 *   flip_sign ::
 *     By default positive values indicate positions inside of contours,
 *     i.e., filled by a contour.  If this property is true then that
 *     output will be the opposite of the default, i.e., negative values
 *     indicate positions inside of contours.
 *
 *   flip_y ::
 *     Setting this parameter to true makes the output image flipped
 *     along the y-axis.
 *
 *   overlaps ::
 *     Set this to true to generate SDF for glyphs having overlapping
 *     contours.  The overlapping support is limited to glyphs that do not
 *     have self-intersecting contours.  Also, removing overlaps require a
 *     considerable amount of extra memory; additionally, it will not work
 *     if generating SDF from bitmap.
 *
 * @note:
 *   All properties except `overlaps` are valid for both the 'sdf' and
 *   'bsdf' renderers.
 *
 */
typedef struct DENSE_Renderer_Module_
{
  FT_RendererRec root;
  FT_UInt        spread;
  FT_Bool        flip_sign;
  FT_Bool        flip_y;
  FT_Bool        overlaps;

} DENSE_Renderer_Module, *DENSE_Renderer;

/**************************************************************************
 *
 * @renderer:
 *   ft_sdf_renderer_class
 *
 * @description:
 *   Renderer to convert @FT_Outline to signed distance fields.
 *
 */
FT_DECLARE_RENDERER( ft_dense_renderer_class )

FT_END_HEADER

#endif /* FTDENSEREND_H_ */

/* END */
