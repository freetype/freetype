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
#include FT_INTERNAL_DEBUG_H
#include FT_SERVICE_PROPERTIES_H
#include FT_SVG_RENDER_H
#include FT_BBOX_H

#ifdef FT_CONFIG_OPTION_DEFAULT_SVG
#include <rsvg_port.h>
#endif
#include <stdio.h>

#include "ftsvg.h"

  /* ft_svg_init */
  static FT_Error
  ft_svg_init( SVG_Renderer svg_module )
  {
    FT_Error    error = FT_Err_Ok;
    svg_module->loaded = FALSE;
#ifdef FT_CONFIG_OPTION_DEFAULT_SVG
    svg_module->hooks.init_svg = (SVG_Lib_Init_Func)rsvg_port_init;
    svg_module->hooks.free_svg = (SVG_Lib_Free_Func)rsvg_port_free;
    svg_module->hooks.render_svg = (SVG_Lib_Render_Func)rsvg_port_render;
    svg_module->hooks.get_buffer_size = (SVG_Lib_Get_Buffer_Size_Func)rsvg_port_get_buffer_size;
    svg_module->hooks_set = TRUE;
#else
    svg_module->hooks_set = FALSE;
#endif
    return error;
  }

  static void
  ft_svg_done( SVG_Renderer svg_module )
  {
    FT_Library  library = svg_module->root.root.library;
    if ( svg_module->loaded    == TRUE &&
         svg_module->hooks_set == TRUE )
      svg_module->hooks.free_svg( library );
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
    FT_ULong      size_image_buffer;

    SVG_RendererHooks hooks = svg_renderer->hooks;

    if ( svg_renderer->hooks_set == FALSE )
    {
      return FT_THROW( Missing_SVG_Hooks );
    }

    if ( svg_renderer->loaded == FALSE )
    {
      error = hooks.init_svg( library );
      svg_renderer->loaded = TRUE;
    }

    /* Let's calculate the bounding box in font units here */
    error = FT_Outline_Get_BBox( &slot->outline, &outline_bbox );
    if( error != FT_Err_Ok )
      return error;

    size_image_buffer = hooks.get_buffer_size( slot, outline_bbox );

    FT_MEM_ALLOC( slot->bitmap.buffer, size_image_buffer );
    if ( error )
      return error;

    return hooks.render_svg( slot, outline_bbox );
  }

  static FT_Error
  ft_svg_property_set( FT_Module    module,
                       const char*  property_name,
                       const void*  value,
                       FT_Bool      value_is_string )
  {
    FT_Error      error    = FT_Err_Ok;
    SVG_Renderer  renderer = (SVG_Renderer)module;

    if ( !ft_strcmp( property_name, "svg_hooks" ) )
    {
      SVG_RendererHooks*  hooks = (SVG_RendererHooks*)value;
      renderer->hooks     = *hooks;
      renderer->hooks_set = TRUE;
    }
    else
    {
      error = FT_THROW( Missing_Property );
    }
    return error;
  }

  static FT_Error
  ft_svg_property_get( FT_Module    module,
                       const char*  property_name,
                       const void*  value )
  {
    FT_Error      error    = FT_Err_Ok;
    SVG_Renderer  renderer = (SVG_Renderer)module;

    if ( !ft_strcmp( property_name, "svg_hooks" ) )
    {
      SVG_RendererHooks*  hooks = (SVG_RendererHooks*)value;
      *hooks = renderer->hooks;
    }
    else
    {
      error = FT_THROW( Missing_Property );
    }
    return error;
  }

  FT_DEFINE_SERVICE_PROPERTIESREC(
    ft_svg_service_properties,

    (FT_Properties_SetFunc)ft_svg_property_set, /* set_property */
    (FT_Properties_GetFunc)ft_svg_property_get  /* get_property */
  )

  FT_DEFINE_SERVICEDESCREC1(
    ft_svg_services,
    FT_SERVICE_ID_PROPERTIES,  &ft_svg_service_properties )


  FT_CALLBACK_DEF( FT_Module_Interface )
  ft_svg_get_interface( FT_Module    module,
                        const char*  ft_svg_interface )
  {
    FT_Module_Interface  result;


    result = ft_service_list_lookup( ft_svg_services, ft_svg_interface );
    if ( result )
      return result;
    return 0;
  }

  FT_DEFINE_RENDERER(
    ft_svg_renderer_class,

      FT_MODULE_RENDERER,
      sizeof( SVG_RendererRec ),

      "ot-svg",
      0x10000L,
      0x20000L,
      NULL,                                /* module specific interface */
      (FT_Module_Constructor)ft_svg_init,  /* module_init */
      (FT_Module_Destructor)ft_svg_done,   /* module_done */
      ft_svg_get_interface,                /* get_interface */
      FT_GLYPH_FORMAT_SVG,
      (FT_Renderer_RenderFunc)ft_svg_render,
      NULL,
      NULL,
      NULL,
      NULL
  )
