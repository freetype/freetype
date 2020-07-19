
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
   *   [TODO]
   *
   * @fields:
   *   [TODO]
   *
   */
  typedef struct  SDF_Renderer_Module_
  {
    FT_RendererRec  root;
    FT_UInt         spread;
    FT_Bool         flip_sign;
    FT_Bool         flip_y;

    /* TEMPORARY */
    FT_Int          optimization;

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
