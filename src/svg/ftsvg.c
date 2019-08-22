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
#include FT_OTSVG_H
#include FT_INTERNAL_SVG_INTERFACE_H
#include FT_BBOX_H

#ifdef FT_CONFIG_OPTION_DEFAULT_SVG
#include <rsvg_port.h>
#endif
#include <stdio.h>

#include "ftsvg.h"

#undef  FT_COMPONENT
#define FT_COMPONENT otsvg

#ifdef FT_CONFIG_OPTION_SVG
  /* ft_svg_init */
  static FT_Error
  ft_svg_init( SVG_Renderer svg_module )
  {
    FT_Error    error = FT_Err_Ok;
    svg_module->loaded = FALSE;
#ifdef FT_CONFIG_OPTION_DEFAULT_SVG
    FT_TRACE3(( "ft_svg_init: Default hooks being set\n" ));
    svg_module->hooks.init_svg = (SVG_Lib_Init_Func)rsvg_port_init;
    svg_module->hooks.free_svg = (SVG_Lib_Free_Func)rsvg_port_free;
    svg_module->hooks.render_svg = (SVG_Lib_Render_Func)rsvg_port_render;
    svg_module->hooks.preset_slot = (SVG_Lib_Preset_Slot_Func)rsvg_port_preset_slot;
    svg_module->hooks_set = TRUE;
#else
    FT_TRACE3(( "ft_svg_init: No default hooks set\n" ));
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
  ft_svg_preset_slot( FT_Module     module,
                      FT_GlyphSlot  slot,
                      FT_Bool       cache )
  {
    SVG_Renderer      svg_renderer = (SVG_Renderer)module;
    SVG_RendererHooks hooks        = svg_renderer->hooks;

    if ( svg_renderer->hooks_set == FALSE )
    {
      FT_TRACE1(( "Hooks are NOT set. Can't render OT-SVG glyphs\n" ));
      return FT_THROW( Missing_SVG_Hooks );
    }

    return hooks.preset_slot( slot, cache );
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
    FT_Error      error;
    FT_ULong      size_image_buffer;

    SVG_RendererHooks hooks = svg_renderer->hooks;

    if ( svg_renderer->hooks_set == FALSE )
    {
      FT_TRACE1(( "Hooks are NOT set. Can't render OT-SVG glyphs\n" ));
      return FT_THROW( Missing_SVG_Hooks );
    }

    if ( svg_renderer->loaded == FALSE )
    {
      FT_TRACE3(( "ft_svg_render: first rendering, calling init hook\n" ));
      error = hooks.init_svg( library );
      svg_renderer->loaded = TRUE;
    }

    ft_svg_preset_slot( (FT_Module)renderer, slot, TRUE);
    size_image_buffer = slot->bitmap.pitch * slot->bitmap.rows;
    FT_MEM_ALLOC( slot->bitmap.buffer, size_image_buffer);
    if ( error )
      return error;

    return hooks.render_svg( slot );
  }

  static const SVG_Interface svg_interface = {
    (Preset_Bitmap_Func)ft_svg_preset_slot
  };

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
      SVG_RendererHooks*  hooks;


      if ( value_is_string == TRUE )
        return FT_THROW( Invalid_Argument );

      hooks = (SVG_RendererHooks*)value;
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

  static FT_Error
  ft_svg_transform( FT_Renderer       renderer,
                    FT_GlyphSlot      slot,
                    const FT_Matrix*  _matrix,
                    const FT_Vector*  _delta )
  {
    FT_SVG_Document  doc = (FT_SVG_Document)slot->other;

    FT_Matrix*  matrix = _matrix;
    FT_Vector*  delta  = _delta;
    FT_Matrix   tmp_matrix;
    FT_Vector   tmp_delta;
    FT_Matrix   a, b;
    FT_Pos      x, y;

    if ( !matrix )
    {
      tmp_matrix.xx = 0x10000;
      tmp_matrix.xy = 0;
      tmp_matrix.yx = 0;
      tmp_matrix.yy = 0x10000;
      matrix = &tmp_matrix;
    }

    if ( !delta )
    {
      tmp_delta.x = 0;
      tmp_delta.y = 0;
    }

    a = doc->transform;
    b = *matrix;
    FT_Matrix_Multiply( &b, &a );


    x = ADD_LONG(ADD_LONG(
        FT_MulFix(matrix->xx, doc->delta.x),
        FT_MulFix(matrix->xy, doc->delta.y)),
        delta->x);
    y = ADD_LONG(ADD_LONG(
        FT_MulFix(matrix->yx, doc->delta.x),
        FT_MulFix(matrix->yy, doc->delta.y)),
        delta->y);
    doc->delta.x = x;
    doc->delta.y = y;

    doc->transform = a;

    return FT_Err_Ok;
  }

#endif

#ifdef FT_CONFIG_OPTION_SVG
#define PUT_SVG_MODULE( a )  a
#define SVG_GLYPH_FORMAT FT_GLYPH_FORMAT_SVG
#else
#define PUT_SVG_MODULE( a )  NULL
#define SVG_GLYPH_FORMAT FT_GLYPH_FORMAT_NONE
#endif


  FT_DEFINE_RENDERER(
    ft_svg_renderer_class,

      FT_MODULE_RENDERER,
      sizeof( SVG_RendererRec ),

      "ot-svg",
      0x10000L,
      0x20000L,
      (const void*)&svg_interface,                           /* module specific interface */
      (FT_Module_Constructor)PUT_SVG_MODULE( ft_svg_init ),  /* module_init */
      (FT_Module_Destructor)PUT_SVG_MODULE( ft_svg_done ),   /* module_done */
      PUT_SVG_MODULE( ft_svg_get_interface ),                /* get_interface */
      SVG_GLYPH_FORMAT,
      (FT_Renderer_RenderFunc)PUT_SVG_MODULE( ft_svg_render ),
      (FT_Renderer_TransformFunc)PUT_SVG_MODULE( ft_svg_transform ),
      NULL,
      NULL,
      NULL
  )
