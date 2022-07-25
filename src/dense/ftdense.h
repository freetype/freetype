
#ifndef FTDENSE_H_
#define FTDENSE_H_

#include <ft2build.h>
#include FT_CONFIG_CONFIG_H
#include <freetype/ftimage.h>

FT_BEGIN_HEADER

#ifndef FT_EXPORT_VAR
#define FT_EXPORT_VAR( x ) extern x
#endif
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

  typedef long TPos;

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

    FT_Pos prev_x, prev_y;

    FT_Outline outline;
  } dense_worker;

  void dense_render_line( dense_worker* aRasterFP, TPos to_x, TPos to_y );
  void dense_render_quadratic( dense_worker*    aRasterFP,
                               const FT_Vector* control,
                               const FT_Vector* to );
  void dense_render_cubic( dense_worker* aRasterFP,
                           FT_Vector*    aP1,
                           FT_Vector*    aP2,
                           FT_Vector*    aP3 );

#ifdef __cplusplus
}  // extern "C"
#endif

FT_END_HEADER

#endif /* FTDENSE_H_ */

/* END */
