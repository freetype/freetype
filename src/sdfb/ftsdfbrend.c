
#include "ftsdfbrend.h"

#include "ftsdfberrs.h"

  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
  #undef  FT_COMPONENT
  #define FT_COMPONENT  sdfb

  

  FT_DEFINE_RENDERER(
    ft_sdfb_renderer_class,

    FT_MODULE_RENDERER,
    sizeof( SDFB_Renderer_Module ),

    "sdfb",
    0x10000L,
    0x20000L,

    NULL,

    (FT_Module_Constructor) NULL,
    (FT_Module_Destructor)  NULL,
    (FT_Module_Requester)   NULL,

    FT_GLYPH_FORMAT_BITMAP,

    (FT_Renderer_RenderFunc)    NULL,  /* render_glyph    */
    (FT_Renderer_TransformFunc) NULL,  /* transform_glyph */
    (FT_Renderer_GetCBoxFunc)   NULL,  /* get_glyph_cbox  */
    (FT_Renderer_SetModeFunc)   NULL,  /* set_mode        */

    (FT_Raster_Funcs*) NULL            /* raster_class    */
  )

/* END */
