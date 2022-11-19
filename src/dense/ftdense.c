/** The rasterizer for the 'dense' renderer */

#undef FT_COMPONENT
#define FT_COMPONENT dense

#include <freetype/ftoutln.h>
#include <freetype/internal/ftcalc.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftobjs.h>
#include <math.h>

#include "ftdense.h"
#include "ftdenseerrs.h"

#define PIXEL_BITS 8

#define ONE_PIXEL  ( 1 << PIXEL_BITS )
#define TRUNC( x ) (int)( ( x ) >> PIXEL_BITS )

#define UPSCALE( x )   ( ( x ) * ( ONE_PIXEL >> 6 ) )
#define DOWNSCALE( x ) ( ( x ) >> ( PIXEL_BITS - 6 ) )

#define FT_SWAP(a, b)   ( (a) ^= (b) ^=(a) ^= (b))
#define FT_MIN( a, b )  ( (a) < (b) ? (a) : (b) )
#define FT_MAX( a, b )  ( (a) > (b) ? (a) : (b) )
#define FT_ABS( a )     ( (a) < 0 ? -(a) : (a) )


typedef struct dense_TRaster_
{
  void* memory;

} dense_TRaster, *dense_PRaster;


static int
dense_move_to( const FT_Vector* to, dense_worker* worker )
{
  FT_Pos x, y;

  x              = UPSCALE( to->x );
  y              = UPSCALE( to->y );
  worker->prev_x = x;
  worker->prev_y = y;
  return 0;
}

static int
dense_line_to( const FT_Vector* to, dense_worker* worker )
{
  dense_render_line( worker, UPSCALE( to->x ), UPSCALE( to->y ) );
  dense_move_to( to, worker );
  return 0;
}

void
dense_render_line( dense_worker* worker, TPos tox, TPos toy )
{
  return;
}

static int
dense_conic_to( const FT_Vector* control,
                const FT_Vector* to,
                dense_worker*    worker )
{
  dense_render_quadratic( worker, control, to );
  return 0;
}

void
dense_render_quadratic( dense_worker*    worker,
                        FT_Vector* control,
                        FT_Vector* to )
{
    return;
}

static int
dense_cubic_to( const FT_Vector* control1,
                const FT_Vector* control2,
                const FT_Vector* to,
                dense_worker*    worker )
{
  dense_render_cubic( worker, control1, control2, to );
  return 0;
}

void
dense_render_cubic( dense_worker* worker,
                    FT_Vector*    control_1,
                    FT_Vector*    control_2,
                    FT_Vector*    to )
{
  return;
}

static int
dense_raster_new( FT_Memory memory, dense_PRaster* araster )
{
  FT_Error      error;
  dense_PRaster raster;

  if ( !FT_NEW( raster ) )
    raster->memory = memory;

  *araster = raster;
  return error;
}

static void
dense_raster_done( FT_Raster raster )
{
  FT_Memory memory = (FT_Memory)( (dense_PRaster)raster )->memory;

  FT_FREE( raster );
}

static void
dense_raster_reset( FT_Raster      raster,
                    unsigned char* pool_base,
                    unsigned long  pool_size )
{
  FT_UNUSED( raster );
  FT_UNUSED( pool_base );
  FT_UNUSED( pool_size );
}

static int
dense_raster_set_mode( FT_Raster raster, unsigned long mode, void* args )
{
  FT_UNUSED( raster );
  FT_UNUSED( mode );
  FT_UNUSED( args );

  return 0; /* nothing to do */
}

FT_DEFINE_OUTLINE_FUNCS( dense_decompose_funcs,

                         (FT_Outline_MoveTo_Func)dense_move_to,   /* move_to  */
                         (FT_Outline_LineTo_Func)dense_line_to,   /* line_to  */
                         (FT_Outline_ConicTo_Func)dense_conic_to, /* conic_to */
                         (FT_Outline_CubicTo_Func)dense_cubic_to, /* cubic_to */

                         0, /* shift    */
                         0  /* delta    */
)

static int
dense_render_glyph( dense_worker* worker, const FT_Bitmap* target )
{
  FT_Error error = FT_Outline_Decompose( &( worker->outline ),
                                         &dense_decompose_funcs, worker );
  return error;
}

static int
dense_raster_render( FT_Raster raster, const FT_Raster_Params* params )
{
  return 0;
}

FT_DEFINE_RASTER_FUNCS(
    ft_dense_raster,

    FT_GLYPH_FORMAT_OUTLINE,

    (FT_Raster_New_Func)dense_raster_new,           /* raster_new      */
    (FT_Raster_Reset_Func)dense_raster_reset,       /* raster_reset    */
    (FT_Raster_Set_Mode_Func)dense_raster_set_mode, /* raster_set_mode */
    (FT_Raster_Render_Func)dense_raster_render,     /* raster_render   */
    (FT_Raster_Done_Func)dense_raster_done          /* raster_done     */
)

/* END */
