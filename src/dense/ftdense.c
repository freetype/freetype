/** The rasterizer for the 'dense' renderer */

#undef FT_COMPONENT
#define FT_COMPONENT dense

#include <freetype/ftoutln.h>
#include <freetype/internal/ftcalc.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftobjs.h>
#include "ftdense.h"

#include "ftdenseerrs.h"

typedef struct dense_TRaster_
{
  void* memory;

} dense_TRaster, *dense_PRaster;

static RasterFP_Point
Lerp( float aT, RasterFP_Point aP0, RasterFP_Point aP1 )
{
  RasterFP_Point p;
  p.m_x = aP0.m_x + aT * ( aP1.m_x - aP0.m_x );
  p.m_y = aP0.m_y + aT * ( aP1.m_y - aP0.m_y );
  return p;
}

static int
dense_move_to( const FT_Vector* to, RasterFP* aRasterFP )
{

  RasterFP_Point lp = {to->x, to->y};

  aRasterFP->last_point = lp;
  return 0;
}

static int
dense_line_to( const FT_Vector* to, RasterFP* aRasterFP )
{
  RasterFP_Point tp = {to->x, to->y};
  RasterFP_DrawLine(aRasterFP, aRasterFP->last_point, tp);
  return 0;
}

void
RasterFP_DrawLine( RasterFP* aRasterFP, RasterFP_Point aP0, RasterFP_Point aP1 )
{
  assert( aRasterFP );
  if ( aP0.m_y == aP1.m_y )
    return;

  aP0.m_x -= aRasterFP->m_origin_x;
  aP0.m_y -= aRasterFP->m_origin_y;
  aP1.m_x -= aRasterFP->m_origin_x;
  aP1.m_y -= aRasterFP->m_origin_y;

  float dir;
  if ( aP0.m_y < aP1.m_y )
    dir = 1;
  else
  {
    dir                 = -1;
    RasterFP_Point temp = aP0;
    aP0                 = aP1;
    aP1                 = temp;
  }

  // Clip to the height.
  if ( aP0.m_y >= aRasterFP->m_h || aP1.m_y <= 0 )
    return;

  float dxdy = ( aP1.m_x - aP0.m_x ) / ( aP1.m_y - aP0.m_y );
  if ( aP0.m_y < 0 )
  {
    aP0.m_x -= aP0.m_y * dxdy;
    aP0.m_y = 0;
  }
  if ( aP1.m_y > aRasterFP->m_h )
  {
    aP1.m_x -= ( aP1.m_y - aRasterFP->m_h ) * dxdy;
    aP1.m_y = (float)aRasterFP->m_h;
  }

  /**
  Handle parts of the line outside the clip rectangle by
  snapping them to the left or right edge, or, if they intersect the clip area,
  by recursive calls.
  */
  RasterFP_Point intersect = { 0, 0 };
  int            recursive = 0;
  if ( aP0.m_x >= aRasterFP->m_w && aP1.m_x >= aRasterFP->m_w )
  {
    aP0.m_x = aP1.m_x = (float)aRasterFP->m_w;
    dxdy              = 0;
  }
  else if ( aP0.m_x <= 0 && aP1.m_x <= 0 )
  {
    aP0.m_x = aP1.m_x = 0;
    dxdy              = 0;
  }
  else if ( ( aP0.m_x < 0 ) != ( aP1.m_x < 0 ) )
  {
    intersect.m_x = 0;
    intersect.m_y = aP1.m_y - aP1.m_x / dxdy;
    recursive     = 1;
  }
  else if ( ( aP0.m_x > aRasterFP->m_w ) != ( aP1.m_x > aRasterFP->m_w ) )
  {
    intersect.m_x = (float)aRasterFP->m_w;
    intersect.m_y = aP0.m_y + ( aRasterFP->m_w - aP0.m_x ) / dxdy;
    recursive     = 1;
  }
  if ( recursive )
  {
    aP0.m_x += aRasterFP->m_origin_x;
    aP0.m_y += aRasterFP->m_origin_y;
    aP1.m_x += aRasterFP->m_origin_x;
    aP1.m_y += aRasterFP->m_origin_y;
    intersect.m_x += aRasterFP->m_origin_x;
    intersect.m_y += aRasterFP->m_origin_y;
    if ( dir == 1 )
    {
      RasterFP_DrawLine( aRasterFP, aP0, intersect );
      RasterFP_DrawLine( aRasterFP, intersect, aP1 );
    }
    else
    {
      RasterFP_DrawLine( aRasterFP, aP1, intersect );
      RasterFP_DrawLine( aRasterFP, intersect, aP0 );
    }
    return;
  }

  float  x       = aP0.m_x;
  int    y0      = (int)aP0.m_y;
  int    y_limit = (int)ceil( aP1.m_y );
  float* m_a     = aRasterFP->m_a;
  for ( int y = y0; y < y_limit; y++ )
  {
    int   linestart = y * aRasterFP->m_w;
    float dy        = fmin( y + 1.0f, aP1.m_y ) - fmax( (float)y, aP0.m_y );
    float xnext     = x + dxdy * dy;
    float d         = dy * dir;

    float x0, x1;
    if ( x < xnext )
    {
      x0 = x;
      x1 = xnext;
    }
    else
    {
      x0 = xnext;
      x1 = x;
    }

    /*
    It's possible for x0 to be negative on the last scanline because of
    floating-point inaccuracy That would cause an out-of-bounds array access at
    index -1.
    */
    float x0floor = x0 <= 0.0f ? 0.0f : (float)floor( x0 );

    int   x0i    = (int)x0floor;
    float x1ceil = (float)ceil( x1 );
    int   x1i    = (int)x1ceil;
    if ( x1i <= x0i + 1 )
    {
      float xmf = 0.5f * ( x + xnext ) - x0floor;
      m_a[linestart + x0i] += d - d * xmf;
      m_a[linestart + ( x0i + 1 )] += d * xmf;
    }
    else
    {
      float s   = 1.0f / ( x1 - x0 );
      float x0f = x0 - x0floor;
      float a0  = 0.5f * s * ( 1.0f - x0f ) * ( 1.0f - x0f );
      float x1f = x1 - x1ceil + 1.0f;
      float am  = 0.5f * s * x1f * x1f;
      m_a[linestart + x0i] += d * a0;
      if ( x1i == x0i + 2 )
        m_a[linestart + ( x0i + 1 )] += d * ( 1.0f - a0 - am );
      else
      {
        float a1 = s * ( 1.5f - x0f );
        m_a[linestart + ( x0i + 1 )] += d * ( a1 - a0 );
        for ( int xi = x0i + 2; xi < x1i - 1; xi++ )
          m_a[linestart + xi] += d * s;
        float a2 = a1 + ( x1i - x0i - 3 ) * s;
        m_a[linestart + ( x1i - 1 )] += d * ( 1.0f - a2 - am );
      }
      m_a[linestart + x1i] += d * am;
    }
    x = xnext;
  }
}





static int
dense_conic_to( const FT_Vector* control,
               const FT_Vector* to,
               RasterFP* aRasterFP)
{
  RasterFP_Point controlP = {control->x, control->y};
  RasterFP_Point toP = {to->x, to->y};
  RasterFP_DrawQuadratic( aRasterFP, aRasterFP->last_point,  controlP, toP );
  return 0;
}

void
RasterFP_DrawQuadratic( RasterFP*      aRasterFP,
                        RasterFP_Point aP0,
                        RasterFP_Point aP1,
                        RasterFP_Point aP2 )
{
  assert( aRasterFP );

  /*
  Calculate devsq as the square of four times the
  distance from the control point to the midpoint of the curve.
  This is the place at which the curve is furthest from the
  line joining the control points.

  4 x point on curve = p0 + 2p1 + p2
  4 x midpoint = 4p1

  The division by four is omitted to save time.
  */
  float devx  = aP0.m_x - aP1.m_x - aP1.m_x + aP2.m_x;
  float devy  = aP0.m_y - aP1.m_y - aP1.m_y + aP2.m_y;
  float devsq = devx * devx + devy * devy;

  if ( devsq < 0.333f )
  {
    RasterFP_DrawLine( aRasterFP, aP0, aP2 );
    return;
  }

  /*
  According to Raph Levien, the reason for the subdivision by n (instead of
  recursive division by the Casteljau system) is that "I expect the flatness
  computation to be semi-expensive (it's done once rather than on each potential
  subdivision) and also because you'll often get fewer subdivisions. Taking a
  circular arc as a simplifying assumption, where I get n, a recursive approach
  would get 2^ceil(lg n), which, if I haven't made any horrible mistakes, is
  expected to be 33% more in the limit".
  */

  const float    tol    = 3.0f;
  int            n      = (int)floor( sqrt( sqrt( tol * devsq ) ) );
  RasterFP_Point p      = aP0;
  float          nrecip = 1.0f / ( n + 1.0f );
  float          t      = 0.0f;
  for ( int i = 0; i < n; i++ )
  {
    t += nrecip;
    RasterFP_Point next = Lerp( t, Lerp( t, aP0, aP1 ), Lerp( t, aP1, aP2 ) );
    RasterFP_DrawLine( aRasterFP, p, next );
    p = next;
  }

  RasterFP_DrawLine( aRasterFP, p, aP2 );
}





static int
dense_cubic_to( const FT_Vector* control1,
               const FT_Vector* control2,
               const FT_Vector* to,
               RasterFP* aRasterFP )
{

  RasterFP_Point ap1 = {control1->x, control1->y};
  RasterFP_Point ap2 = {control2->x, control2->y};
  RasterFP_Point ap3 = {to->x, to->y};


  RasterFP_DrawCubic( aRasterFP, aRasterFP->last_point, ap1, ap2, ap3 );
  return 0;
}

void
RasterFP_DrawCubic( RasterFP*      aRasterFP,
                    RasterFP_Point aP0,
                    RasterFP_Point aP1,
                    RasterFP_Point aP2,
                    RasterFP_Point aP3 )
{
  assert( aRasterFP );

  float devx   = aP0.m_x - aP1.m_x - aP1.m_x + aP2.m_x;
  float devy   = aP0.m_y - aP1.m_y - aP1.m_y + aP2.m_y;
  float devsq0 = devx * devx + devy * devy;
  devx         = aP1.m_x - aP2.m_x - aP2.m_x + aP3.m_x;
  devy         = aP1.m_y - aP2.m_y - aP2.m_y + aP3.m_y;
  float devsq1 = devx * devx + devy * devy;
  float devsq  = fmax( devsq0, devsq1 );

  if ( devsq < 0.333f )
  {
    RasterFP_DrawLine( aRasterFP, aP0, aP3 );
    return;
  }

  const float    tol    = 3.0f;
  int            n      = (int)floor( sqrt( sqrt( tol * devsq ) ) );
  RasterFP_Point p      = aP0;
  float          nrecip = 1.0f / ( n + 1.0f );
  float          t      = 0.0f;
  for ( int i = 0; i < n; i++ )
  {
    t += nrecip;
    RasterFP_Point a    = Lerp( t, Lerp( t, aP0, aP1 ), Lerp( t, aP1, aP2 ) );
    RasterFP_Point b    = Lerp( t, Lerp( t, aP1, aP2 ), Lerp( t, aP2, aP3 ) );
    RasterFP_Point next = Lerp( t, a, b );
    RasterFP_DrawLine( aRasterFP, p, next );
    p = next;
  }

  RasterFP_DrawLine( aRasterFP, p, aP3 );
}




static int
dense_raster_new( FT_Memory memory, dense_PRaster* araster )
{
  FT_Error     error;
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

FT_DEFINE_OUTLINE_FUNCS(
    func_interface,

    (FT_Outline_MoveTo_Func)dense_move_to,   /* move_to  */
    (FT_Outline_LineTo_Func)dense_line_to,   /* line_to  */
    (FT_Outline_ConicTo_Func)dense_conic_to, /* conic_to */
    (FT_Outline_CubicTo_Func)dense_cubic_to, /* cubic_to */

    0, /* shift    */
    0  /* delta    */
)








static int
gray_convert_glyph_inner( RAS_ARG, int continued )
{
  int error;

  if ( ft_setjmp( ras.jump_buffer ) == 0 )
  {
    if ( continued )
      FT_Trace_Disable();
    error = FT_Outline_Decompose( &ras.outline, &func_interface, &ras );
    if ( continued )
      FT_Trace_Enable();

    FT_TRACE7( ( "band [%d..%d]: %ld cell%s remaining/\n", ras.min_ey,
                 ras.max_ey, ras.cell_null - ras.cell_free,
                 ras.cell_null - ras.cell_free == 1 ? "" : "s" ) );
  }
  else
  {
    error = FT_THROW( Raster_Overflow );

    FT_TRACE7( ( "band [%d..%d]: to be bisected\n", ras.min_ey, ras.max_ey ) );
  }

  return error;
}

static int gray_convert_glyph( RAS_ARG )
{
  const TCoord yMin = ras.min_ey;
  const TCoord yMax = ras.max_ey;

  TCell   buffer[FT_MAX_GRAY_POOL];
  size_t  height = (size_t)( yMax - yMin );
  size_t  n      = FT_MAX_GRAY_POOL / 8;
  TCoord  y;
  TCoord  bands[32]; /* enough to accommodate bisections */
  TCoord* band;

  int continued = 0;

  /* Initialize the null cell at the end of the poll. */
  ras.cell_null        = buffer + FT_MAX_GRAY_POOL - 1;
  ras.cell_null->x     = CELL_MAX_X_VALUE;
  ras.cell_null->area  = 0;
  ras.cell_null->cover = 0;
  ras.cell_null->next  = NULL;

  /* set up vertical bands */
  ras.ycells = (PCell*)buffer;

  if ( height > n )
  {
    /* two divisions rounded up */
    n      = ( height + n - 1 ) / n;
    height = ( height + n - 1 ) / n;
  }

  for ( y = yMin; y < yMax; )
  {
    ras.min_ey = y;
    y += height;
    ras.max_ey = FT_MIN( y, yMax );

    band    = bands;
    band[1] = ras.min_ey;
    band[0] = ras.max_ey;

    do
    {
      TCoord width = band[0] - band[1];
      TCoord w;
      int    error;

      for ( w = 0; w < width; ++w )
        ras.ycells[w] = ras.cell_null;

      /* memory management: skip ycells */
      n = ( width * sizeof( PCell ) + sizeof( TCell ) - 1 ) / sizeof( TCell );

      ras.cell_free = buffer + n;
      ras.cell      = ras.cell_null;
      ras.min_ey    = band[1];
      ras.max_ey    = band[0];
      ras.count_ey  = width;

      error     = gray_convert_glyph_inner( RAS_VAR, continued );
      continued = 1;

      if ( !error )
      {
        if ( ras.render_span ) /* for FT_RASTER_FLAG_DIRECT only */
          gray_sweep_direct( RAS_VAR );
        else
          gray_sweep( RAS_VAR );
        band--;
        continue;
      }
      else if ( error != Smooth_Err_Raster_Overflow )
        return error;

      /* render pool overflow; we will reduce the render band by half */
      width >>= 1;

      /* this should never happen even with tiny rendering pool */
      if ( width == 0 )
      {
        FT_TRACE7( ( "gray_convert_glyph: rotten glyph\n" ) );
        return FT_THROW( Raster_Overflow );
      }

      band++;
      band[1] = band[0];
      band[0] += width;
    } while ( band >= bands );
  }

  return Smooth_Err_Ok;
}

static int
dense_raster_render( FT_Raster raster, const FT_Raster_Params* params )
{
  const FT_Outline* outline    = (const FT_Outline*)params->source;
  const FT_Bitmap*  target_map = params->target;

#ifndef FT_STATIC_RASTER
  gray_TWorker worker[1];
#endif

  if ( !raster )
    return FT_THROW( Invalid_Argument );

  /* this version does not support monochrome rendering */
  if ( !( params->flags & FT_RASTER_FLAG_AA ) )
    return FT_THROW( Cannot_Render_Glyph );

  if ( !outline )
    return FT_THROW( Invalid_Outline );

  /* return immediately if the outline is empty */
  if ( outline->n_points == 0 || outline->n_contours <= 0 )
    return Smooth_Err_Ok;

  if ( !outline->contours || !outline->points )
    return FT_THROW( Invalid_Outline );

  if ( outline->n_points != outline->contours[outline->n_contours - 1] + 1 )
    return FT_THROW( Invalid_Outline );

  ras.outline = *outline;

  if ( params->flags & FT_RASTER_FLAG_DIRECT )
  {
    if ( !params->gray_spans )
      return Smooth_Err_Ok;

    ras.render_span      = (FT_Raster_Span_Func)params->gray_spans;
    ras.render_span_data = params->user;

    ras.min_ex = params->clip_box.xMin;
    ras.min_ey = params->clip_box.yMin;
    ras.max_ex = params->clip_box.xMax;
    ras.max_ey = params->clip_box.yMax;
  }
  else
  {
    /* if direct mode is not set, we must have a target bitmap */
    if ( !target_map )
      return FT_THROW( Invalid_Argument );

    /* nothing to do */
    if ( !target_map->width || !target_map->rows )
      return Smooth_Err_Ok;

    if ( !target_map->buffer )
      return FT_THROW( Invalid_Argument );

    if ( target_map->pitch < 0 )
      ras.target.origin = target_map->buffer;
    else
      ras.target.origin =
          target_map->buffer +
          ( target_map->rows - 1 ) * (unsigned int)target_map->pitch;

    ras.target.pitch = target_map->pitch;

    ras.render_span      = (FT_Raster_Span_Func)NULL;
    ras.render_span_data = NULL;

    ras.min_ex = 0;
    ras.min_ey = 0;
    ras.max_ex = (FT_Pos)target_map->width;
    ras.max_ey = (FT_Pos)target_map->rows;
  }

  /* exit if nothing to do */
  if ( ras.max_ex <= ras.min_ex || ras.max_ey <= ras.min_ey )
    return Smooth_Err_Ok;

  return gray_convert_glyph( RAS_VAR );
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