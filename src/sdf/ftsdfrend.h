
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

  } SDF_Renderer_Module, *SDF_Renderer;


  FT_DECLARE_RENDERER( ft_sdf_renderer_class )


FT_END_HEADER

#endif /* FTSDFREND_H_ */


/* END */
