#include <freetype/internal/ftobjs.h>

 /* sets render-specific mode */
  static  FT_Error  ft_renderer_set_mode( FT_Renderer  render,
                                          FT_ULong     mode_tag,
                                          FT_Pointer   data )
  {
    /* we simply pass it to the raster */
    return render->clazz->raster_class->raster_set_mode(
                    render->raster, mode_tag, data );
  }                                          


 /* convert a slot's glyph image into a bitmap */
  static  FT_Error  ft_renderer_render( FT_Renderer  render,
                                        FT_GlyphSlot slot,
                                        FT_UInt      mode )
  {
    FT_Error     error;
    FT_Outline*  outline;
    FT_BBox      cbox;
    FT_UInt      width, height, pitch;
    FT_Bitmap*   bitmap;
    FT_Memory    memory;
    
    FT_Raster_Params  params;
    
    /* first of all, transform the outline */
    if (slot->format != ft_glyph_format_outline)
    {
      error = FT_Err_Invalid_Argument;
      goto Exit;
    }
    
    outline = &slot->outline;
    
    FT_Outline_Transform( outline, &slot->transform_matrix );
    FT_Outline_Translate( outline, slot->transform_delta.x,
                                   slot->transform_delta.y );
    
    /* compute the control box, and grid fit it */
    FT_Outline_Get_CBox( outline, &cbox );
    
    cbox.xMin &= -64;
    cbox.yMin &= -64;
    cbox.xMax  = (cbox.xMax+63) & -64;
    cbox.yMax  = (cbox.yMax+63) & -64;

    width  = (cbox.xMax - cbox.xMin) >> 6;
    height = (cbox.yMax - cbox.yMin) >> 6;
    bitmap = &slot->bitmap;
    memory = slot->face->memory;
    
    /* release old bitmap buffer */
    if ((slot->flags & ft_glyph_own_bitmap))
      FREE(bitmap->buffer);
      
    /* allocate new one, depends on pixel format */
    if ( mode & ft_render_mode_antialias )
    {
      pitch = width;
      bitmap->pixel_mode = ft_pixel_mode_grays;
      bitmap->num_grays  = 256;
    }
    else
    {
      pitch  = (width+7) >> 3;
      bitmap->pixel_mode = ft_pixel_mode_mono;
    }

    bitmap->width = width;
    bitmap->rows  = height;
    bitmap->pitch = pitch;
    
    if (ALLOC( bitmap->buffer, (FT_ULong)pitch * height ))
      goto Exit;

    /* translate outline to render it into the bitmap */
    FT_Outline_Translate( outline, -cbox.xMin, -cbox.yMin );

    /* set up parameters */
    params.target = bitmap;
    params.source = outline;
    params.flags  = 0;

    if ( bitmap->pixel_mode == ft_pixel_mode_grays )
      params.flags |= ft_raster_flag_aa;

    /* render outline into the bitmap */
    error = render->render( render->raster, &params );
    if (error) goto Exit;
    
    slot->format = ft_glyph_format_bitmap;
    slot->bitmap_left = cbox.xMin >> 6;
    slot->bitmap_top  = cbox.yMax >> 6;
    
  Exit:
    return error;
  }

#ifndef  FT_CONFIG_OPTION_NO_STD_RASTER

#include <freetype/ftraster.h>

  const FT_Renderer_Class   ft_standard_renderer_class =
  {
    {
      ft_module_renderer,
      sizeof( FT_RendererRec ),
      
      "standard renderer",
      0x10000,
      0x20000,
      
      0,    /* module specific interface */
      
      (FT_Module_Constructor)  0,
      (FT_Module_Destructor)   0,
      (FT_Module_Requester)    0
    },
    
    ft_glyph_format_outline,
    
    (FTRenderer_render)       ft_renderer_render,
    (FTRenderer_setMode)      ft_renderer_set_mode,
    
    (FT_Raster_Funcs*)        &ft_standard_raster
  };
  
#endif /* !FT_CONFIG_OPTION_NO_STD_RASTER */

#ifndef FT_CONFIG_OPTION_NO_SMOOTH_RASTER

#include <freetype/ftgrays.h>

  const FT_Renderer_Class  ft_smooth_renderer_class =
  {
    {
      ft_module_renderer,
      sizeof( FT_RendererRec ),
      
      "smooth renderer",
      0x10000,
      0x20000,
      
      0,    /* module specific interface */
      
      (FT_Module_Constructor)  0,
      (FT_Module_Destructor)   0,
      (FT_Module_Requester)    0
    },

    ft_glyph_format_outline,
    
    (FTRenderer_render)       ft_renderer_render,
    (FTRenderer_setMode)      ft_renderer_set_mode,
    
    (FT_Raster_Funcs*)        &ft_grays_raster
  };

#endif /* !FT_CONFIG_OPTION_NO_SMOOTH_RASTER */
