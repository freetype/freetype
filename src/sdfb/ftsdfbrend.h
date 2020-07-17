
#ifndef FTSDFBREND_H_
#define FTSDFBREND_H_


#include <freetype/ftrender.h>
#include <freetype/ftmodapi.h>
#include <freetype/internal/ftobjs.h>


FT_BEGIN_HEADER

  /**************************************************************************
   *
   * @struct:
   *   SDFB_Renderer_Module
   *
   * @description:
   *   [TODO]
   *
   * @fields:
   *   [TODO]
   *
   */
  typedef struct  SDFB_Renderer_Module_
  {
    FT_RendererRec  root;
    FT_UInt         spread;
    FT_Bool         flip_sign;
    FT_Bool         flip_y;

  } SDFB_Renderer_Module, *SDFB_Renderer;


  FT_DECLARE_RENDERER( ft_sdfb_renderer_class )


FT_END_HEADER

#endif /* FTSDFBREND_H_ */


/* END */
