/* Dense rasterizer header*/

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

#ifdef __cplusplus
extern "C"
{
#endif


  typedef signed long FT26D6;            /* 26.6 fixed-point representation  */
  typedef signed int FT20D12;            /* 20.12 fixed-point representation  */

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

  void dense_render_line( dense_worker* worker, FT_Pos to_x, FT_Pos to_y );
  void dense_render_quadratic( dense_worker* worker,
                               FT_Vector* control,
                               FT_Vector* to );
  void dense_render_cubic( dense_worker* worker,
                           FT_Vector*    control_1,
                           FT_Vector*    control_2,
                           FT_Vector*    to );

#ifdef __cplusplus
}  // extern "C"
#endif

FT_END_HEADER

#endif /* FTDENSE_H_ */

/* END */
