/****************************************************************************
 *
 * ftsvg.c
 *
 *   The FreeType svg renderer interface (body).
 *
 * Copyright (C) 1996-2019 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

#include <ft2build.h>
#include FT_SVG_RENDERER_H

#include <stdio.h>

#include "ftsvg.h"

  /* tmp hook injection */
  FT_Error
  tmp_svg_lib_init()
  {
    FT_Error  error;

    error = FT_Err_Ok;
    printf("Init svg\n");
    return error;
  }

  /* ft_svg_init */
  static FT_Error
  ft_svg_init( SVG_Renderer svg_module )
  {
    FT_Error           error = FT_Err_Ok;
    svg_module->loaded = FALSE;
    return error;
  }

  static FT_Error
  ft_svg_render( FT_Renderer       renderer,
                 FT_GlyphSlot      slot,
                 FT_Render_Mode    mode,
                 const FT_Vector*  origin )
  {
    SVG_Renderer  renderer_ = (SVG_Renderer)renderer;

    if( renderer_->loaded == FALSE )
    {
      renderer_->loaded = TRUE;
      renderer_->hooks.svg_lib_init();
    }

    return renderer_->hooks.svg_lib_render( slot );
  }

  static FT_Error
  ft_svg_set_hooks( FT_Module       renderer_,
                    SVG_Lib_Init    init_hook,
                    SVG_Lib_Free    free_hook,
                    SVG_Lib_Render  render_hook )
  {
    SVG_Renderer  renderer;

    renderer = (SVG_Renderer)renderer_;
    renderer->hooks.svg_lib_init   = init_hook;
    renderer->hooks.svg_lib_free   = free_hook;
    renderer->hooks.svg_lib_render = render_hook;

    return FT_Err_Ok;
  }


  static const SVG_Renderer_Interface svg_renderer_interface =
  {
    (SVG_Set_Hooks)ft_svg_set_hooks
  };


  FT_DEFINE_RENDERER(
    ft_svg_renderer_class,

      FT_MODULE_RENDERER,
      sizeof( SVG_RendererRec ),

      "ot-svg",
      0x10000L,
      0x20000L,
      (const void*)&svg_renderer_interface,   /* module specific interface */
      (FT_Module_Constructor)ft_svg_init,     /* module_init */
      NULL,
      NULL,
      FT_GLYPH_FORMAT_SVG,
      (FT_Renderer_RenderFunc)ft_svg_render,
      NULL,
      NULL,
      NULL,
      NULL
  )
