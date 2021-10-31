/** The 'dense' renderer */

#include "ftdenserend.h"
#include <freetype/ftbitmap.h>
#include <freetype/ftoutln.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/services/svprop.h>
#include "ftdense.h"

#include "ftdenseerrs.h"

/**************************************************************************
 *
 * The macro FT_COMPONENT is used in trace mode.  It is an implicit
 * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
 * messages during execution.
 */
#undef FT_COMPONENT
#define FT_COMPONENT dense



/**************************************************************************
 *
 * interface functions
 *
 */

/* @QUES: So, I think this is used for setting up the
initial properties of the renderer ??? */
static FT_Error
ft_dense_init( FT_Renderer render )
{
  printf( "ft_dense_init\n" );
  return FT_Err_Ok;
}

/* @QUES: A destructor probably. The smooth renderer doesn't have this
so, I guess this is unnecessary ??? */
static void
ft_dense_done( FT_Renderer render )
{
  printf( "ft_dense_done\n" );
  FT_UNUSED( render );
}

/* generate bitmap from a glyph's slot image */

/* @QUES: This method allocates the bitmap buffer, shifts the outlines
as required (Why exactly is shifting required?), and finally calls 
raster_render interface methods with correct params */
static FT_Error
ft_dense_render( FT_Renderer      render,
                 FT_GlyphSlot     slot,
                 FT_Render_Mode   mode,
                 const FT_Vector* origin )
{
  printf( "ft_dense_render\n" );
  FT_Error    error   = FT_Err_Ok;
  FT_Outline* outline = &slot->outline;
  FT_Bitmap*  bitmap  = &slot->bitmap;

/* @QUES: You know, it should be a bit more clear that FT_FREE and FT_ALLOC_MULT macros
take a variable named `memory`. It can only be known if you follow the macros 3 level deep.*/
  FT_Memory   memory  = render->root.memory;

  FT_Pos x_shift = 0;
  FT_Pos y_shift = 0;

  FT_Raster_Params params;

  /* check whether slot format is correct before rendering */
  if ( slot->format != render->glyph_format )
  {
    error = FT_THROW( Invalid_Glyph_Format );
    goto Exit;
  }


  /* deallocate the previously allocated bitmap */
  if ( slot->internal->flags & FT_GLYPH_OWN_BITMAP )
  {
    FT_FREE( bitmap->buffer );
    slot->internal->flags &= ~FT_GLYPH_OWN_BITMAP;
  }

  /* preset the bitmap using the glyph's outline;         */
  if ( ft_glyphslot_preset_bitmap( slot, mode, origin ) )
  {
    error = FT_THROW( Raster_Overflow );
    goto Exit;
  }

  if ( !bitmap->rows || !bitmap->pitch )
    goto Exit;


  /* allocate new one */
  if ( FT_ALLOC_MULT( bitmap->buffer, bitmap->rows, bitmap->pitch ) )
    goto Exit;

  /* @QUES: Where can I read more about why x and y shift are required */
  x_shift = 64 * -slot->bitmap_left;
  y_shift = 64 * -slot->bitmap_top;

  /* @QUES: What does this flag mean ?*/
  slot->internal->flags |= FT_GLYPH_OWN_BITMAP;
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

  /* @QUES: Why is my final bitmap upside down ??🤔*/

  /* render the outline */
  error =
      render->raster_render( render->raster, (const FT_Raster_Params*)&params );

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
/* @QUES: This mthod isn't called, atleast in normal ftlint execution.
What is it for ?*/
static FT_Error
ft_dense_transform( FT_Renderer      render,
                    FT_GlyphSlot     slot,
                    const FT_Matrix* matrix,
                    const FT_Vector* delta )
{
  printf( "ft_dense_transform\n" );

  FT_Error error = FT_Err_Ok;

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

/* return the control box of a glyph's outline */
/* @QUES: This method isn't called either in normal ftlint execution*/
static void
ft_dense_get_cbox( FT_Renderer render, FT_GlyphSlot slot, FT_BBox* cbox )
{
  printf( "ft_dense_get_cbox\n" );

  FT_ZERO( cbox );

  if ( slot->format == render->glyph_format )
    FT_Outline_Get_CBox( &slot->outline, cbox );
}

/* set render specific modes or attributes */
/* @QUES: Isn't called in normal ftlint execution*/
static FT_Error
ft_dense_set_mode( FT_Renderer render, FT_ULong mode_tag, FT_Pointer data )
{
  /* pass it to the rasterizer */
  return render->clazz->raster_class->raster_set_mode( render->raster, mode_tag,
                                                       data );
}

FT_DEFINE_RENDERER(
    ft_dense_renderer_class,

    FT_MODULE_RENDERER,
    sizeof( FT_RendererRec ),

    "dense",
    0x10000L,
    0x20000L,

    NULL,

    (FT_Module_Constructor)ft_dense_init,
    (FT_Module_Destructor)ft_dense_done,
    (FT_Module_Requester)NULL,

    FT_GLYPH_FORMAT_OUTLINE,

    (FT_Renderer_RenderFunc)ft_dense_render,       /* render_glyph    */
    (FT_Renderer_TransformFunc)ft_dense_transform, /* transform_glyph */
    (FT_Renderer_GetCBoxFunc)ft_dense_get_cbox,    /* get_glyph_cbox  */
    (FT_Renderer_SetModeFunc)ft_dense_set_mode,    /* set_mode        */

    (FT_Raster_Funcs*)&ft_dense_raster /* raster_class    */
)

/* END */