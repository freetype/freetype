/****************************************************************************
 *
 * ftsvg.c
 *
 *   The FreeType svg renderer interface (body).
 *
 * Copyright (C) 1996-2019 by
 * David Turner, Robert Wilhelm, Werner Lemberg and Moazin Khatti.
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
#include FT_BBOX_H

#include <stdio.h>

#include "ftsvg.h"

  /* ft_svg_init */
  static FT_Error
  ft_svg_init( SVG_Renderer svg_module )
  {
    FT_Error    error = FT_Err_Ok;
    svg_module->loaded = FALSE;
    return error;
  }

  static void
  ft_svg_done( SVG_Renderer svg_module )
  {
    FT_Library  library = svg_module->root.root.library;
    FT_Memory   memory  = library->memory;
    if ( svg_module->loaded == TRUE )
      svg_module->hooks.svg_lib_free( library );
      FT_FREE( library->svg_renderer_state );
    svg_module->loaded = FALSE;
  }

  static FT_Error
  ft_svg_render( FT_Renderer       renderer,
                 FT_GlyphSlot      slot,
                 FT_Render_Mode    mode,
                 const FT_Vector*  origin )
  {
    SVG_Renderer  svg_renderer = (SVG_Renderer)renderer;
    FT_Library    library      = renderer->root.library;
    FT_Memory     memory       = library->memory;
    FT_BBox       outline_bbox;
    FT_Error      error;

    SVG_RendererHooks hooks = svg_renderer->hooks;

    unsigned long  size_image_buffer;

    if ( svg_renderer->loaded == FALSE )
    {
      FT_MEM_ALLOC( library->svg_renderer_state,
                    hooks.svg_lib_get_state_size() );
      if ( error )
        return error;
      error = hooks.svg_lib_init( library );
      svg_renderer->loaded = TRUE;
    }

    /* Let's calculate the bounding box in font units here */
    error = FT_Outline_Get_BBox( &slot->outline, &outline_bbox );
    if( error != FT_Err_Ok )
      return error;

    size_image_buffer = hooks.svg_lib_get_buffer_size( slot, outline_bbox );

    FT_MEM_ALLOC( slot->bitmap.buffer, size_image_buffer );
    if ( error )
      return error;

    return hooks.svg_lib_render( slot, outline_bbox );
  }

  static FT_Error
  ft_svg_set_hooks( FT_Module                module,
                    SVG_Lib_Init             init_hook,
                    SVG_Lib_Free             free_hook,
                    SVG_Lib_Render           render_hook,
                    SVG_Lib_Get_State_Size   get_state_size,
                    SVG_Lib_Get_Buffer_Size  get_buffer_size )
  {
    SVG_Renderer  renderer;

    renderer = (SVG_Renderer)module;
    renderer->hooks.svg_lib_init   = init_hook;
    renderer->hooks.svg_lib_free   = free_hook;
    renderer->hooks.svg_lib_render = render_hook;

    renderer->hooks.svg_lib_get_state_size  = get_state_size;
    renderer->hooks.svg_lib_get_buffer_size = get_buffer_size;

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
      (FT_Module_Destructor)ft_svg_done,      /* module_done */
      NULL,
      FT_GLYPH_FORMAT_SVG,
      (FT_Renderer_RenderFunc)ft_svg_render,
      NULL,
      NULL,
      NULL,
      NULL
  )
