
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftobjs.h>
#include "ftsdfrend.h"
#include "ftsdf.h"

#include "ftsdferrs.h"


  static FT_Error
  ft_sdf_render( FT_Renderer       render,
                 FT_GlyphSlot      slot,
                 FT_Render_Mode    mode,
                 const FT_Vector*  origin )
  {
    FT_Error  error = FT_THROW( Unimplemented_Feature );


    FT_UNUSED( render );
    FT_UNUSED( slot );
    FT_UNUSED( mode );
    FT_UNUSED( origin );

    return error;
  }

  static FT_Error
  ft_sdf_transform( FT_Renderer       render,
                    FT_GlyphSlot      slot,
                    const FT_Matrix*  matrix,
                    const FT_Vector*  delta )
  {
    FT_Error  error = FT_THROW( Unimplemented_Feature );


    FT_UNUSED( render );
    FT_UNUSED( slot );
    FT_UNUSED( matrix );
    FT_UNUSED( delta );

    return error;
  }

  static void
  ft_sdf_get_cbox( FT_Renderer   render,
                   FT_GlyphSlot  slot,
                   FT_BBox*      cbox )
  {
    FT_UNUSED( render );
    FT_UNUSED( slot );
    FT_UNUSED( cbox );
  }

  static FT_Error
  ft_sdf_set_mode( FT_Renderer  render,
                   FT_ULong     mode_tag,
                   FT_Pointer   data )
  {
    FT_Error  error = FT_THROW( Unimplemented_Feature );


    FT_UNUSED( render );
    FT_UNUSED( mode_tag );
    FT_UNUSED( data );

    return error;
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
