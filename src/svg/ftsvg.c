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
#include <stdio.h>

#include "ftsvg.h"

  /* tmp hook injection */
  FT_Error 
  tmp_svg_lib_init()
  {
    FT_Error error = FT_Err_Ok;
    printf("Init svg\n");
    return error;
  }

  /* ft_svg_init */
  static FT_Error
  ft_svg_init( SVG_Renderer svg_module )
  {
    FT_Error           error = FT_Err_Ok;
    SVG_RendererHooks  hooks;

    hooks.svg_lib_init = tmp_svg_lib_init;
    svg_module->hooks  = hooks;
    svg_module->loaded = FALSE;

    return error; 
  }

  static FT_Error
  ft_svg_render( FT_Renderer       renderer,
                 FT_GlyphSlot      slot,
                 FT_Render_Mode    mode,
                 const FT_Vector*  origin )
  {
    SVG_Renderer renderer_ = (SVG_Renderer)renderer;
    if( renderer_->loaded == FALSE )
      renderer_->loaded = TRUE;
    renderer_->hooks.svg_lib_init();
  }



  FT_DEFINE_RENDERER(
    ft_svg_renderer_class,

      FT_MODULE_RENDERER,
      sizeof( SVG_RendererRec ),

      "ot-svg",
      0x10000L,
      0x20000L,
      NULL,     /* module specific interface */
      (FT_Module_Constructor)ft_svg_init,     /* module_init */
      NULL,
      NULL,
      FT_GLYPH_FORMAT_SVG,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL
  )
