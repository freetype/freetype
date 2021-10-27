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
 * macros and default property values
 *
 */
#define DENSE_RENDERER( rend ) ( (DENSE_Renderer)rend )

/**************************************************************************
 *
 * for setting properties
 *
 */

/* property setter function */
static FT_Error
dense_property_set( FT_Module   module,
                    const char* property_name,
                    const void* value,
                    FT_Bool     value_is_string )
{
  FT_Error       error  = FT_Err_Ok;
  DENSE_Renderer render = DENSE_RENDERER( FT_RENDERER( module ) );

  FT_UNUSED( value_is_string );

  if ( ft_strcmp( property_name, "spread" ) == 0 )
  {
    // FT_Int val = *(const FT_Int*)value;
    // if ( val > MAX_SPREAD || val < MIN_SPREAD )
    // {
    //   FT_TRACE0(
    //       ( "[sdf] dense_property_set:"
    //         " the `spread' property can have a value\n" ) );
    //   FT_TRACE0(
    //       ( "                       "
    //         " within range [%d, %d] (value provided: %d)\n",
    //         MIN_SPREAD, MAX_SPREAD, val ) );
    //
    //   error = FT_THROW( Invalid_Argument );
    //   goto Exit;
    // }
    //
    // render->spread = (FT_UInt)val;
    // FT_TRACE7(
    //     ( "[sdf] dense_property_set:"
    //       " updated property `spread' to %d\n",
    //       val ) );
  }

  else if ( ft_strcmp( property_name, "flip_sign" ) == 0 )
  {
    ;
  }

  else
  {
    FT_TRACE0(
        ( "[dense] dense_property_set:"
          " missing property `%s'\n",
          property_name ) );
    error = FT_THROW( Missing_Property );
  }

Exit:
  return error;
}

/* property getter function */
static FT_Error
dense_property_get( FT_Module module, const char* property_name, void* value )
{
  FT_Error       error  = FT_Err_Ok;
  DENSE_Renderer render = DENSE_RENDERER( FT_RENDERER( module ) );

  if ( ft_strcmp( property_name, "spread" ) == 0 )
  {
    // FT_Int* val = (FT_Int*)value;

    // *val = render->spread;
  }

  else if ( ft_strcmp( property_name, "flip_sign" ) == 0 )
  {
    ;
  }

  else
  {
    FT_TRACE0(
        ( "[dense] dense_property_get:"
          " missing property `%s'\n",
          property_name ) );
    error = FT_THROW( Missing_Property );
  }

  return error;
}

FT_DEFINE_SERVICE_PROPERTIESREC(
    dense_service_properties,

    (FT_Properties_SetFunc)dense_property_set,  /* set_property */
    (FT_Properties_GetFunc)dense_property_get ) /* get_property */

FT_DEFINE_SERVICEDESCREC1( dense_services,

                           FT_SERVICE_ID_PROPERTIES,
                           &dense_service_properties )

static FT_Module_Interface
ft_dense_requester( FT_Renderer render, const char* module_interface )
{
  FT_UNUSED( render );

  return ft_service_list_lookup( dense_services, module_interface );
}

/**************************************************************************
 *
 * interface functions
 *
 */

static FT_Error
ft_dense_init( FT_Renderer render )
{
  DENSE_Renderer dense_render = DENSE_RENDERER( render );

  //   dense_render->spread    = 0;
  //   dense_render->flip_sign = 0;
  //   dense_render->flip_y    = 0;
  //   dense_render->overlaps  = 0;

  return FT_Err_Ok;
}

static void
ft_dense_done( FT_Renderer render )
{
  FT_UNUSED( render );
}

/* generate signed distance field from a glyph's slot image */
static FT_Error
ft_dense_render( FT_Renderer      module,
                 FT_GlyphSlot     slot,
                 FT_Render_Mode   mode,
                 const FT_Vector* origin )
{
  FT_Error    error   = FT_Err_Ok;
  FT_Outline* outline = &slot->outline;
  FT_Bitmap*  bitmap  = &slot->bitmap;
  FT_Memory   memory  = NULL;
  FT_Renderer render  = NULL;

  FT_Pos x_shift = 0;
  FT_Pos y_shift = 0;

  FT_Pos x_pad = 0;
  FT_Pos y_pad = 0;

  DENSE_Raster_Params params;
  DENSE_Renderer      dense_module = DENSE_RENDERER( module );

  render = &dense_module->root;
  memory = render->root.memory;

  /* check whether slot format is correct before rendering */
  if ( slot->format != render->glyph_format )
  {
    error = FT_THROW( Invalid_Glyph_Format );
    goto Exit;
  }

  /* check whether render mode is correct */
  if ( mode != FT_RENDER_MODE_NORMAL )
  {
    FT_ERROR(
        ( "[dense] ft_dense_render:"
          " sdf module only render when"
          " using `FT_RENDER_MODE_NORMAL'\n" ) );
    error = FT_THROW( Cannot_Render_Glyph );
    goto Exit;
  }

  /* deallocate the previously allocated bitmap */
  if ( slot->internal->flags & FT_GLYPH_OWN_BITMAP )
  {
    FT_FREE( bitmap->buffer );
    slot->internal->flags &= ~FT_GLYPH_OWN_BITMAP;
  }

  /* preset the bitmap using the glyph's outline;         */
  /* the sdf bitmap is similar to an antialiased bitmap   */
  /* with a slightly bigger size and different pixel mode */
  if ( ft_glyphslot_preset_bitmap( slot, FT_RENDER_MODE_NORMAL, origin ) )
  {
    error = FT_THROW( Raster_Overflow );
    goto Exit;
  }

  if ( !bitmap->rows || !bitmap->pitch )
    goto Exit;

  /* the padding will simply be equal to the `spread' */
  x_pad = dense_module->spread;
  y_pad = dense_module->spread;

  /* apply the padding; will be in all the directions */
  bitmap->rows += y_pad * 2;
  bitmap->width += x_pad * 2;

  /* ignore the pitch, pixel mode and set custom */
  bitmap->pixel_mode = FT_PIXEL_MODE_GRAY;
  bitmap->pitch      = bitmap->width;
  bitmap->num_grays  = 255;

  /* allocate new buffer */
  if ( FT_ALLOC_MULT( bitmap->buffer, bitmap->rows, bitmap->pitch ) )
    goto Exit;

  slot->internal->flags |= FT_GLYPH_OWN_BITMAP;

  x_shift = 64 * -( slot->bitmap_left - x_pad );
  y_shift = 64 * -( slot->bitmap_top + y_pad );
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
  params.root.target = bitmap;
  params.root.source = outline;
  params.root.flags  = FT_RASTER_FLAG_DEFAULT;
  params.spread      = dense_module->spread;
  params.flip_sign   = dense_module->flip_sign;
  params.flip_y      = dense_module->flip_y;
  params.overlaps    = dense_module->overlaps;

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
static FT_Error
ft_dense_transform( FT_Renderer      render,
                    FT_GlyphSlot     slot,
                    const FT_Matrix* matrix,
                    const FT_Vector* delta )
{
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
static void
ft_dense_get_cbox( FT_Renderer render, FT_GlyphSlot slot, FT_BBox* cbox )
{
  FT_ZERO( cbox );

  if ( slot->format == render->glyph_format )
    FT_Outline_Get_CBox( &slot->outline, cbox );
}

/* set render specific modes or attributes */
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
    sizeof( DENSE_Renderer_Module ),

    "dense",
    0x10000L,
    0x20000L,

    NULL,

    (FT_Module_Constructor)ft_dense_init,
    (FT_Module_Destructor)ft_dense_done,
    (FT_Module_Requester)ft_dense_requester,

    FT_GLYPH_FORMAT_OUTLINE,

    (FT_Renderer_RenderFunc)ft_dense_render,       /* render_glyph    */
    (FT_Renderer_TransformFunc)ft_dense_transform, /* transform_glyph */
    (FT_Renderer_GetCBoxFunc)ft_dense_get_cbox,    /* get_glyph_cbox  */
    (FT_Renderer_SetModeFunc)ft_dense_set_mode,    /* set_mode        */

    (FT_Raster_Funcs*)&ft_dense_raster /* raster_class    */
)

/* END */