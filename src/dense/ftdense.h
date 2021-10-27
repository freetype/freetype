
#ifndef FTDENSE_H_
#define FTDENSE_H_


#include <ft2build.h>
#include FT_CONFIG_CONFIG_H
#include <freetype/ftimage.h>


FT_BEGIN_HEADER

/**************************************************************************
 *
 * @struct:
 *   SDF_Raster_Params
 *
 * @description:
 *   This struct must be passed to the raster render function
 *   @FT_Raster_RenderFunc instead of @FT_Raster_Params because the
 *   rasterizer requires some additional information to render properly.
 *
 * @fields:
 *   root ::
 *     The native raster parameters structure.
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
 *     Setting this parameter to true maked the output image flipped
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
 *   All properties are valid for both the 'sdf' and 'bsdf' renderers; the
 *   exception is `overlaps`, which gets ignored by the 'bsdf' renderer.
 *
 */
typedef struct DENSE_Raster_Params_
{
  FT_Raster_Params root;
  FT_UInt          spread;
  FT_Bool          flip_sign;
  FT_Bool          flip_y;
  FT_Bool          overlaps;

} DENSE_Raster_Params;

/* rasterizer to convert outline to SDF */
FT_EXPORT_VAR( const FT_Raster_Funcs ) ft_dense_raster;

/**
RASTER_FP

A floating-point anti-aliasing renderer.
Graham Asher, August 2016.

Most of this code is derived from Raph Levien's font-rs code in the Rust
language, which is licensed under the Apache License, version 2.0.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct
  {
    float m_x;
    float m_y;
  } RasterFP_Point;

  typedef struct
  {
    /** The array used to store signed area differences. */
    float* m_a;
    /** The number of elements in m_a. */
    int m_a_size;
    /** The width of the current raster in pixels. */
    int m_w;
    /** The height of the current raster in pixels. */
    int m_h;
    /** The x origin of the raster. */
    int m_origin_x;
    /** The y origin of the raster. */
    int m_origin_y;

    RasterFP_Point last_point;
  } RasterFP;

  void RasterFP_Create( RasterFP* aRasterFP );
  void RasterFP_StartRasterizing( RasterFP* aRasterFP,
                                  int       aOriginX,
                                  int       aOriginY,
                                  int       aWidth,
                                  int       aHeight );
  void RasterFP_Destroy( RasterFP* aRasterFP );
  void RasterFP_DrawLine( RasterFP*      aRasterFP,
                          RasterFP_Point aP0,
                          RasterFP_Point aP1 );
  void RasterFP_DrawQuadratic( RasterFP*      aRasterFP,
                               RasterFP_Point aP0,
                               RasterFP_Point aP1,
                               RasterFP_Point aP2 );
  void RasterFP_DrawCubic( RasterFP*      aRasterFP,
                           RasterFP_Point aP0,
                           RasterFP_Point aP1,
                           RasterFP_Point aP2,
                           RasterFP_Point aP3 );
  void RasterFP_GetBitmap( RasterFP* aRasterFP, unsigned char* aBitmap );

#ifdef __cplusplus
}  // extern "C"
#endif


FT_END_HEADER

#endif /* FTDENSE_H_ */

/* END */
