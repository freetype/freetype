
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/ftoutln.h>
#include "ftsdfrend.h"
#include "ftsdf.h"

#include "ftsdferrs.h"


  /* generate signed distance field from a glyph's slot image */
  static FT_Error
  ft_sdf_render( FT_Renderer       render,
                 FT_GlyphSlot      slot,
                 FT_Render_Mode    mode,
                 const FT_Vector*  origin )
  {
    FT_Error     error   = FT_Err_Ok;
    FT_Outline*  outline = &slot->outline;
    FT_Bitmap*   bitmap  = &slot->bitmap;
    FT_Memory    memory  = render->root.memory;
    FT_Pos       x_shift = 0;
    FT_Pos       y_shift = 0;
    
    /* use hardcoded padding for now */
    FT_UInt      x_pad   = 10;
    FT_UInt      y_pad   = 10;

    FT_Raster_Params  params;

    /* check if slot format is correct before rendering */
    if ( slot->format != render->glyph_format )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* check if render mode is correct */
    if ( mode != FT_RENDER_MODE_SDF )
    {
      FT_ERROR(( "ft_sdf_render: sdf module only"
                 "render when using `FT_RENDER_MODE_SDF'" ));
      error = FT_THROW( Cannot_Render_Glyph );
      goto Exit;
    }

    /* deallocate the previously allocated bitmap */
    if ( slot->internal->flags & FT_GLYPH_OWN_BITMAP )
    {
      FT_FREE( bitmap->buffer );
      slot->internal->flags &= ~FT_GLYPH_OWN_BITMAP;
    }

    /* preset the bitmap using the glyph's outline;        */
    /* the sdf bitmap is similar to a antialiased bitmap   */
    /* with a slighty bigger size and different pixel mode */
    if ( ft_glyphslot_preset_bitmap( slot, FT_RENDER_MODE_NORMAL, origin ) )
    {
      error = FT_THROW( Raster_Overflow );
      goto Exit;
    }

    if ( !bitmap->rows || !bitmap->pitch )
      goto Exit;

    /* apply the padding */
    bitmap->rows  += y_pad;
    bitmap->width += x_pad;

    /* ignore the pitch, pixel mode and set custom */
    bitmap->pixel_mode = FT_PIXEL_MODE_GRAY16;
    bitmap->pitch = bitmap->width * 2;
    bitmap->num_grays = 65536;

    /* allocate new buffer */
    if ( FT_ALLOC_MULT( bitmap->buffer, bitmap->rows, bitmap->pitch ) )
      goto Exit;

    slot->internal->flags |= FT_GLYPH_OWN_BITMAP;

    x_shift  = 64 * -( slot->bitmap_left + x_pad );
    y_shift  = 64 * -( slot->bitmap_top + y_pad );
    y_shift += 64 * (FT_Int)bitmap->rows;

    if ( origin )
    {
      x_shift += origin->x;
      y_shift += origin->y;
    }

    /* translate outline to render it into the bitmap */
    if ( x_shift || y_shift )
      FT_Outline_Translate( outline, x_shift, y_shift );

    /* set up parameters */
    params.target = bitmap;
    params.source = outline;
    params.flags  = 0;

    /* render the outline */
    error = render->raster_render( render->raster, &params );

  Exit:
    if ( !error )
    {
      /* the glyph is successfully rendered to a bitmap */
      slot->format = FT_GLYPH_FORMAT_BITMAP;
    }
    else if ( slot->internal->flags & FT_GLYPH_OWN_BITMAP )
    {
      FT_FREE( bitmap->buffer );
      slot->internal->flags &= ~FT_GLYPH_OWN_BITMAP;
    }

    if ( x_shift || y_shift )
      FT_Outline_Translate( outline, -x_shift, -y_shift );

    return error;
  }

  /* transform the glyph using matrix and/or delta */
  static FT_Error
  ft_sdf_transform( FT_Renderer       render,
                    FT_GlyphSlot      slot,
                    const FT_Matrix*  matrix,
                    const FT_Vector*  delta )
  {
    FT_Error  error = FT_Err_Ok;


    if ( slot->format != render->glyph_format )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    if ( matrix )
      FT_Outline_Transform( &slot->outline, matrix );

    if ( delta )
      FT_Outline_Translate( &slot->outline, delta->x, delta->y );

  Exit:
    return error;
  }

  /* returns the control box of glyph's outline */
  static void
  ft_sdf_get_cbox( FT_Renderer   render,
                   FT_GlyphSlot  slot,
                   FT_BBox*      cbox )
  {
    FT_ZERO( cbox );

    if ( slot->format == render->glyph_format )
      FT_Outline_Get_CBox( &slot->outline, cbox );
  }

  /* set render specific modes or attributes */
  static FT_Error
  ft_sdf_set_mode( FT_Renderer  render,
                   FT_ULong     mode_tag,
                   FT_Pointer   data )
  {
    /* pass it to the rasterizer */
    return render->clazz->raster_class->raster_set_mode( render->raster,
                                                         mode_tag,
                                                         data );
  }

  FT_DEFINE_RENDERER(
    ft_sdf_renderer_class,

    FT_MODULE_RENDERER,
    sizeof( FT_RendererRec ),

    "sdf",
    0x10000L,
    0x20000L,

    NULL,

    (FT_Module_Constructor) NULL,
    (FT_Module_Destructor)  NULL,
    (FT_Module_Requester)   NULL,

    FT_GLYPH_FORMAT_OUTLINE,

    (FT_Renderer_RenderFunc)   ft_sdf_render,     /* render_glyph    */
    (FT_Renderer_TransformFunc)ft_sdf_transform,  /* transform_glyph */
    (FT_Renderer_GetCBoxFunc)  ft_sdf_get_cbox,   /* get_glyph_cbox  */
    (FT_Renderer_SetModeFunc)  ft_sdf_set_mode,   /* set_mode        */

    (FT_Raster_Funcs*)&ft_sdf_raster              /* raster_class    */
  )

/* END */
