/*****************************************************************************/
/*                                                                           */
/*  ftgrays.c  - a new 'perfect' anti-aliasing renderer for FreeType 2       */
/*                                                                           */
/*  Copyright 2000 by The FreeType Project                                   */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                        */
/*                                                                           */
/*  This file is part of the FreeType project, and may only be used          */
/*  modified and distributed under the terms of the FreeType project         */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute       */
/*  this file you indicate that you have read the license and                */
/*  understand and accept it fully.                                          */
/*                                                                           */
/*  This is a new anti-aliasing scan-converter for FreeType 2. The           */
/*  algorithm used here is _very_ different from the one in the standard     */
/*  "ftraster.c". Actually, "ftgrays.c" computes the _exact_ coverage of     */
/*  the outline on each pixel cell.                                          */
/*                                                                           */
/*  It is based on ideas that I initially found in Raph Levien's excellent   */
/*  LibArt graphics library (see www.levien.com/libart for more information, */
/*  though the web pages do not tell anything about the renderer, you'll     */
/*  have to dive in the source code to understand how it works..)            */
/*                                                                           */
/*  Note however that this is a _very_ different implementation from         */
/*  Raph's. Coverage information is stored in a very different way,          */
/*  and I don't use sorted vector paths. Also, it doesn't use floating       */
/*  point values..                                                           */
/*                                                                           */
/*  This renderer has the following advantages:                              */
/*                                                                           */
/*     - doesn't need an intermediate bitmap. Instead, one can supply        */
/*       a callback fuction that will be called by the renderer to           */
/*       draw gray spans on any target surface.. You can thus do direct      */
/*       composition on any kind of bitmap, provided that you give the       */
/*       renderer the right callback..                                       */
/*                                                                           */
/*     - perfect anti-aliaser, i.e. computes the _exact_ coverage on         */
/*       each pixel cell                                                     */
/*                                                                           */
/*     - performs a single pass on the outline (the 'standard' FT2           */
/*       renderer performs two passes).                                      */
/*                                                                           */
/*     - can easily be modified to render to _any_ number of gray levels     */
/*       cheaply..                                                           */
/*                                                                           */
/*     - faster than the standard renderer for small (< 20) pixel sizes      */
/*                                                                           */

#include <ftgrays.h>

#if 1
#include <string.h>  /* for memcpy */
#endif

#define ErrRaster_Invalid_Outline  -1

#ifdef _STANDALONE_
#error "implementation of FT_Outline_Decompose missing !!!"
#else
#include <freetype.h>  /* to link to FT_Outline_Decompose */
#endif

/* define this to dump debugging information */
#define xxxDEBUG_GRAYS

/* as usual, for the speed hungry :-) */
#ifndef FT_STATIC_RASTER

  #define  RAS_ARG    PRaster  raster
  #define  RAS_ARG_   PRaster  raster,

  #define  RAS_VAR    raster
  #define  RAS_VAR_   raster,

  #define  ras (*raster)

#else

  #define  RAS_ARG
  #define  RAS_ARG_
  #define  RAS_VAR
  #define  RAS_VAR_

  static TRaster  ras;

#endif

/* must be at least 6 bits !! */
#define  PIXEL_BITS  8

#define  ONE_PIXEL   (1L << PIXEL_BITS)
#define  PIXEL_MASK  (-1L << PIXEL_BITS)
#define  TRUNC(x)    ((x) >> PIXEL_BITS)
#define  SUBPIXELS(x) ((x) << PIXEL_BITS)
#define  FLOOR(x)    ((x) & -ONE_PIXEL)
#define  CEILING(x)  (((x)+ONE_PIXEL-1) & -ONE_PIXEL)
#define  ROUND(x)    (((x)+ONE_PIXEL/2) & -ONE_PIXEL)

#if PIXEL_BITS >= 6
#define  UPSCALE(x)  ((x) << (PIXEL_BITS-6))
#define  DOWNSCALE(x)  ((x) >> (PIXEL_BITS-6))
#else
#define  UPSCALE(x)  ((x) >> (6-PIXEL_BITS))
#define  DOWNSCALE(x)  ((x) << (6-PIXEL_BITS))
#endif

/* define if you want to use more compact storage, this increases the number */
/* of cells available in the render pool but slows down the rendering a bit  */
/* useful when you have a really tiny render pool                            */
#define  xxxGRAYS_COMPACT



/****************************************************************************/
/*                                                                          */
/*   TYPE DEFINITIONS                                                       */
/*                                                                          */

typedef int   TScan;   /* integer scanline/pixel coordinate */
typedef long  TPos;    /* sub-pixel coordinate              */

/* maximum number of gray spans in a call to the span callback */
#define FT_MAX_GRAY_SPANS  32


#ifdef GRAYS_COMPACT  
typedef struct TCell_
{
  short  x     : 14;
  short  y     : 14;
  int    cover : PIXEL_BITS+2;
  int    area  : PIXEL_BITS*2+2;

} TCell, *PCell;
#else
typedef struct TCell_
{
  TScan  x;
  TScan  y;
  int    cover;
  int    area;

} TCell, *PCell;
#endif


typedef struct TRaster_
{
  PCell   cells;
  int     max_cells;
  int     num_cells;

  TScan   min_ex, max_ex;
  TScan   min_ey, max_ey;

  int     area;
  int     cover;
  int     invalid;

  TScan   ex, ey;
  TScan   cx, cy;
  TPos    x,  y;

  TScan   last_ey;

  FT_Vector   bez_stack[32*3];
  int         lev_stack[32];

  FT_Outline  outline;
  FT_Bitmap   target;

  FT_Span     gray_spans[ FT_MAX_GRAY_SPANS ];
  int         num_gray_spans;

  FT_Raster_Span_Func  render_span;
  void*                render_span_data;
  int                  span_y;

  int         band_size;
  int         band_shoot;
  int         conic_level;
  int         cubic_level;

  void*       memory;

} TRaster, *PRaster;




/****************************************************************************/
/*                                                                          */
/*   INITIALIZE THE CELLS TABLE                                             */
/*                                                                          */
static
void  init_cells( RAS_ARG_  void*  buffer, long byte_size )
{
  ras.cells     = (PCell)buffer;
  ras.max_cells = byte_size / sizeof(TCell);
  ras.num_cells = 0;
  ras.area      = 0;
  ras.cover     = 0;
  ras.invalid   = 1;
}


/****************************************************************************/
/*                                                                          */
/*   COMPUTE THE OUTLINE BOUNDING BOX                                       */
/*                                                                          */
static
void  compute_cbox( RAS_ARG_ FT_Outline*  outline )
{
  FT_Vector*  vec   = outline->points;
  FT_Vector*  limit = vec + outline->n_points;

  if ( outline->n_points <= 0 )
  {
    ras.min_ex = ras.max_ex = 0;
    ras.min_ey = ras.max_ey = 0;
    return;
  }

  ras.min_ex = ras.max_ex = vec->x;
  ras.min_ey = ras.max_ey = vec->y;
  vec++;

  for ( ; vec < limit; vec++ )
  {
    TPos  x = vec->x;
    TPos  y = vec->y;

    if ( x < ras.min_ex ) ras.min_ex = x;
    if ( x > ras.max_ex ) ras.max_ex = x;
    if ( y < ras.min_ey ) ras.min_ey = y;
    if ( y > ras.max_ey ) ras.max_ey = y;
  }

  /* truncate the bounding box to integer pixels */
  ras.min_ex = ras.min_ex >> 6;
  ras.min_ey = ras.min_ey >> 6;
  ras.max_ex = ( ras.max_ex+63 ) >> 6;
  ras.max_ey = ( ras.max_ey+63 ) >> 6;
}


/****************************************************************************/
/*                                                                          */
/*   RECORD THE CURRENT CELL IN THE TABLE                                   */
/*                                                                          */
static
int record_cell( RAS_ARG )
{
  PCell cell;

  if (!ras.invalid && (ras.area | ras.cover))
  {
    if ( ras.num_cells >= ras.max_cells )
      return 1;

    cell        = ras.cells + ras.num_cells++;
    cell->x     = (ras.ex - ras.min_ex);
    cell->y     = (ras.ey - ras.min_ey);
    cell->area  = ras.area;
    cell->cover = ras.cover;
  }
  return 0;
}


/****************************************************************************/
/*                                                                          */
/*   SET THE CURRENT CELL TO A NEW POSITION                                 */
/*                                                                          */
static
int   set_cell( RAS_ARG_ TScan ex, TScan ey )
{
  int  invalid, record, clean;

  /* move the cell pointer to a new position. We set the "invalid"       */
  /* flag to indicate that the cell isn't part of those we're interested */
  /* in during the render phase.. This means that:                       */
  /*                                                                     */
  /* the new vertical position must be within min_ey..max_ey-1.          */
  /* the new horizontal position must be strictly less than max_ey       */
  /*                                                                     */
  /* Note that we a cell is to the left of the clipping region, it is    */
  /* actually set to the (min_ex-1) horizontal position                  */
  /*                                                                     */
  record  = 0;
  clean   = 1;
  invalid = ( ey < ras.min_ey || ey >= ras.max_ey || ex >= ras.max_ex );
  if (!invalid)
  {
    /* all cells that are on the left of the clipping region go to the */
    /* min_ex-1 horizontal position..                                  */
    if (ex < ras.min_ex)
      ex = ras.min_ex-1;

    /* if our position is new, then record the previous cell */
    if (ex != ras.ex || ey != ras.ey)
      record = 1;
    else
      clean = ras.invalid;  /* do not clean if we didn't move from */
                            /* a valid cell..                      */
  }

  /* record the previous cell if needed (i.e. if we changed the cell */
  /* position, of changed the 'invalid' flag..)                      */
  if ( (ras.invalid != invalid || record) && record_cell( RAS_VAR ) )
    return 1;

  if (clean)
  {
    ras.area  = 0;
    ras.cover = 0;
  }

  ras.invalid = invalid;
  ras.ex      = ex;
  ras.ey      = ey;
  return 0;
}



/****************************************************************************/
/*                                                                          */
/*   START A NEW CONTOUR AT A GIVEN CELL                                    */
/*                                                                          */
static
void  start_cell( RAS_ARG_  TScan  ex, TScan  ey )
{
  if (ex < ras.min_ex)
    ex = ras.min_ex-1;

  ras.area    = 0;
  ras.cover   = 0;
  ras.ex      = ex;
  ras.ey      = ey;
  ras.last_ey = SUBPIXELS(ey);
  ras.invalid = 0;

  (void)set_cell( RAS_VAR_  ex, ey );
}


/****************************************************************************/
/*                                                                          */
/*   RENDER A SCANLINE AS ONE OR MORE CELLS                                 */
/*                                                                          */
static
int  render_scanline( RAS_ARG_  TScan  ey, TPos x1, TScan y1,
                                           TPos x2, TScan y2 )
{
  TScan  ex1, ex2, fx1, fx2, delta;
  long   p, first, dx;
  int    incr, lift, mod, rem;

  dx = x2-x1;

  ex1 = TRUNC(x1); /* if (ex1 >= ras.max_ex) ex1 = ras.max_ex-1; */
  ex2 = TRUNC(x2); /* if (ex2 >= ras.max_ex) ex2 = ras.max_ex-1; */
  fx1 = x1 - SUBPIXELS(ex1);
  fx2 = x2 - SUBPIXELS(ex2);

  /* trivial case. Happens often */
  if (y1 == y2)
    return set_cell( RAS_VAR_  ex2, ey );


  /* everything is located in a single cell, that is easy ! */
  /*                                                        */
  if ( ex1 == ex2 )
  {
    delta      = y2-y1;
    ras.area  += (fx1+fx2)*delta;
    ras.cover += delta;
    return 0;
  }

  /* ok, we'll have to render a run of adjacent cells on the same */
  /* scanline..                                                   */
  /*                                                              */
  p     = (ONE_PIXEL-fx1)*(y2-y1);
  first = ONE_PIXEL;
  incr  = 1;
  if ( dx < 0 )
  {
    p     = fx1*(y2-y1);
    first = 0;
    incr  = -1;
    dx    = -dx;
  }

  delta = p / dx;
  mod   = p % dx;
  if (mod < 0)
  {
    delta--;
    mod += dx;
  }

  ras.area  += (fx1+first)*delta;
  ras.cover += delta;

  ex1 += incr;
  if (set_cell( RAS_VAR_ ex1, ey )) goto Error;
  y1  += delta;

  if (ex1 != ex2)
  {
    p     = ONE_PIXEL*(y2-y1);
    lift  = p / dx;
    rem   = p % dx;
    if (rem < 0)
    {
      lift--;
      rem += dx;
    }

    mod -= dx;

    while (ex1 != ex2)
    {
      delta = lift;
      mod  += rem;
      if (mod >= 0)
      {
        mod -= dx;
        delta++;
      }
      ras.area  += ONE_PIXEL*delta;
      ras.cover += delta;
      y1        += delta;
      ex1       += incr;
      if (set_cell( RAS_VAR_ ex1, ey )) goto Error;
    }
  }

  delta      = y2-y1;
  ras.area  += (fx2+ONE_PIXEL-first)*delta;
  ras.cover += delta;

  return 0;
Error:
  return 1;
}

/****************************************************************************/
/*                                                                          */
/*   RENDER A GIVEN LINE AS A SERIES OF SCANLINES                           */
/*                                                                          */
static
int  render_line( RAS_ARG_ TPos  to_x, TPos  to_y )
{
  TScan  ey1, ey2, fy1, fy2;
  TPos   dx, dy, x, x2;
  int    p, rem, mod, lift, delta, first, incr;

  ey1 = TRUNC(ras.last_ey);
  ey2 = TRUNC(to_y); /* if (ey2 >= ras.max_ey) ey2 = ras.max_ey-1; */
  fy1 = ras.y - ras.last_ey;
  fy2 = to_y - SUBPIXELS(ey2);

  dx = to_x - ras.x;
  dy = to_y - ras.y;

  /* we should do something about the trivial case where dx == 0, */
  /* as it happens very often !! ... XXXXX                        */

  /* perform vertical clipping */
  {
    TScan  min, max;
    min = ey1;
    max = ey2;
    if (ey1 > ey2)
    {
      min = ey2;
      max = ey1;
    }
    if (min >= ras.max_ey || max < ras.min_ey)
      goto Fin;
  }

  /* everything is on a single scanline */
  if ( ey1 == ey2 )
  {
    if (render_scanline( RAS_VAR_ ey1, ras.x, fy1, to_x, fy2 )) goto Error;
    goto Fin;
  }

  /* ok, we'll have to render several scanlines */
  p     = (ONE_PIXEL-fy1)*dx;
  first = ONE_PIXEL;
  incr  = 1;
  if ( dy < 0 )
  {
    p     = fy1*dx;
    first = 0;
    incr  = -1;
    dy    = -dy;
  }

  delta = p / dy;
  mod   = p % dy;
  if (mod < 0)
  {
    delta--;
    mod += dy;
  }

  x = ras.x + delta;
  if (render_scanline( RAS_VAR_ ey1, ras.x, fy1, x, first )) goto Error;

  ey1 += incr;
  if (set_cell( RAS_VAR_ TRUNC(x), ey1 )) goto Error;

  if (ey1 != ey2)
  {
    p     = ONE_PIXEL*dx;
    lift  = p / dy;
    rem   = p % dy;
    if (rem < 0)
    {
      lift--;
      rem += dy;
    }
    mod -= dy;

    while (ey1 != ey2)
    {
      delta = lift;
      mod  += rem;
      if (mod >= 0)
      {
        mod -= dy;
        delta++;
      }
      x2   = x + delta;
      if (render_scanline( RAS_VAR_ ey1, x, ONE_PIXEL-first, x2, first )) goto Error;
      x    = x2;
      ey1 += incr;
      if (set_cell( RAS_VAR_ TRUNC(x), ey1 )) goto Error;
    }
  }

  if (render_scanline( RAS_VAR_ ey1, x, ONE_PIXEL-first, to_x, fy2 )) goto Error;

Fin:
  ras.x       = to_x;
  ras.y       = to_y;
  ras.last_ey = SUBPIXELS(ey2);
  return 0;
Error:
  return 1;
}


static
void  split_conic( FT_Vector*  base )
{
  TPos  a, b;

  base[4].x = base[2].x;
  b = base[1].x;
  a = base[3].x = ( base[2].x + b )/2;
  b = base[1].x = ( base[0].x + b )/2;
  base[2].x = ( a + b ) / 2;

  base[4].y = base[2].y;
  b = base[1].y;
  a = base[3].y = ( base[2].y + b )/2;
  b = base[1].y = ( base[0].y + b )/2;
  base[2].y = ( a + b ) / 2;
}


static
int  render_conic( RAS_ARG_ FT_Vector* control, FT_Vector* to )
{
  TPos        dx, dy;
  int         top, level;
  int*        levels;
  FT_Vector*  arc;

  dx = DOWNSCALE(ras.x) + to->x - (control->x << 1); if (dx < 0) dx = -dx;
  dy = DOWNSCALE(ras.y) + to->y - (control->y << 1); if (dy < 0) dy = -dy;
  if (dx < dy) dx = dy;

  level = 1;
  dx = dx/ras.conic_level;
  while ( dx > 0 )
  {
    dx >>= 1;
    level++;
  }

  /* a shortcut to speed things up */
  if (level <= 1)
  {
    /* we compute the mid-point directly in order to avoid */
    /* calling split_conic()..                             */
    TPos   to_x, to_y, mid_x, mid_y;
    
    to_x  = UPSCALE(to->x);
    to_y  = UPSCALE(to->y);
    mid_x = (ras.x + to_x + 2*UPSCALE(control->x))/4;
    mid_y = (ras.y + to_y + 2*UPSCALE(control->y))/4;
    
    return render_line( RAS_VAR_ mid_x, mid_y ) ||
           render_line( RAS_VAR_ to_x, to_y );
  }

  arc      = ras.bez_stack;
  levels    = ras.lev_stack;
  top       = 0;
  levels[0] = level;
  
  arc[0].x = UPSCALE(to->x);
  arc[0].y = UPSCALE(to->y);
  arc[1].x = UPSCALE(control->x);
  arc[1].y = UPSCALE(control->y);
  arc[2].x = ras.x;
  arc[2].y = ras.y;

  while (top >= 0)
  {
    level = levels[top];
    if (level > 1)
    {
      /* check that the arc crosses the current band */
      TPos  min, max, y;
      min = max = arc[0].y;
      y = arc[1].y;
      if ( y < min ) min = y;
      if ( y > max ) max = y;
      y = arc[2].y;
      if ( y < min ) min = y;
      if ( y > max ) max = y;
      if ( TRUNC(min) >= ras.max_ey || TRUNC(max) < 0 )
        goto Draw;

      split_conic(arc);
      arc += 2;
      top ++;
      levels[top] = levels[top-1] = level-1;
      continue;
    }
  Draw:
    {
      TPos   to_x, to_y, mid_x, mid_y;
      
      to_x  = arc[0].x;
      to_y  = arc[0].y;
      mid_x = (ras.x + to_x + 2*arc[1].x)/4;
      mid_y = (ras.y + to_y + 2*arc[1].y)/4;
      
      if ( render_line( RAS_VAR_ mid_x, mid_y ) ||
           render_line( RAS_VAR_ to_x, to_y )   ) return 1;
      top--;
      arc  -= 2;
    }
  }
  return 0;
}


static
void  split_cubic( FT_Vector*  base )
{
  TPos   a, b, c, d;

  base[6].x = base[3].x;
  c = base[1].x;
  d = base[2].x;
  base[1].x = a = ( base[0].x + c ) / 2;
  base[5].x = b = ( base[3].x + d ) / 2;
  c = ( c + d ) / 2;
  base[2].x = a = ( a + c ) / 2;
  base[4].x = b = ( b + c ) / 2;
  base[3].x = ( a + b ) / 2;

  base[6].y = base[3].y;
  c = base[1].y;
  d = base[2].y;
  base[1].y = a = ( base[0].y + c ) / 2;
  base[5].y = b = ( base[3].y + d ) / 2;
  c = ( c + d ) / 2;
  base[2].y = a = ( a + c ) / 2;
  base[4].y = b = ( b + c ) / 2;
  base[3].y = ( a + b ) / 2;
}


static
int  render_cubic( RAS_ARG_ FT_Vector* control1,
                            FT_Vector* control2,
                            FT_Vector* to )
{
  TPos        dx, dy, da, db;
  int         top, level;
  int*        levels;
  FT_Vector*  arc;

  dx = DOWNSCALE(ras.x) + to->x - (control1->x << 1); if (dx < 0) dx = -dx;
  dy = DOWNSCALE(ras.y) + to->y - (control1->y << 1); if (dy < 0) dy = -dy;
  if (dx < dy) dx = dy;
  da = dx;

  dx = DOWNSCALE(ras.x) + to->x - 3*(control1->x + control2->x); if (dx < 0) dx = -dx;
  dy = DOWNSCALE(ras.y) + to->y - 3*(control1->x + control2->y); if (dy < 0) dy = -dy;
  if (dx < dy) dx = dy;
  db = dx;

  level = 1;
  da    = da/ras.cubic_level;
  db    = db/ras.conic_level;
  while ( da > 0 || db > 0 )
  {
    da >>= 1;
    db >>= 2;
    level++;
  }

  if (level <= 1)
  {
    TPos   to_x, to_y, mid_x, mid_y;
    
    to_x  = UPSCALE(to->x);
    to_y  = UPSCALE(to->y);
    mid_x = (ras.x + to_x + 3*UPSCALE(control1->x+control2->x))/8;
    mid_y = (ras.y + to_y + 3*UPSCALE(control1->y+control2->y))/8;
    
    return render_line( RAS_VAR_ mid_x, mid_y ) ||
           render_line( RAS_VAR_ to_x, to_y );
  }

  arc      = ras.bez_stack;
  arc[0].x = UPSCALE(to->x);
  arc[0].y = UPSCALE(to->y);
  arc[1].x = UPSCALE(control2->x);
  arc[1].y = UPSCALE(control2->y);
  arc[2].x = UPSCALE(control1->x);
  arc[2].y = UPSCALE(control1->y);
  arc[3].x = ras.x;
  arc[3].y = ras.y;

  levels    = ras.lev_stack;
  top       = 0;
  levels[0] = level;

  while (top >= 0)
  {
    level = levels[top];
    if (level > 1)
    {
      /* check that the arc crosses the current band */
      TPos  min, max, y;
      min = max = arc[0].y;
      y = arc[1].y;
      if ( y < min ) min = y;
      if ( y > max ) max = y;
      y = arc[2].y;
      if ( y < min ) min = y;
      if ( y > max ) max = y;
      y = arc[3].y;
      if ( y < min ) min = y;
      if ( y > max ) max = y;
      if ( TRUNC(min) >= ras.max_ey || TRUNC(max) < 0 )
        goto Draw;
      split_cubic(arc);
      arc += 3;
      top ++;
      levels[top] = levels[top-1] = level-1;
      continue;
    }
  Draw:
    {
      TPos   to_x, to_y, mid_x, mid_y;
      
      to_x  = arc[0].x;
      to_y  = arc[0].y;
      mid_x = (ras.x + to_x + 3*(arc[1].x+arc[2].x))/8;
      mid_y = (ras.y + to_y + 3*(arc[1].y+arc[2].y))/8;
      
      if ( render_line( RAS_VAR_ mid_x, mid_y ) ||
           render_line( RAS_VAR_ to_x, to_y )   ) return 1;
      top --;
      arc -= 3;
    }
  }
  return 0;
}


/* a macro comparing two cell pointers. returns true if a <= b */
#if 1
#define PACK(a)          ( ((long)(a)->y << 16) | (a)->x )
#define LESS_THAN(a,b)   ( PACK(a) < PACK(b) )
#else
#define LESS_THAN(a,b)   ( (a)->y<(b)->y || ((a)->y==(b)->y && (a)->x < (b)->x) )
#endif

#define SWAP_CELLS(a,b,temp)  { temp = *(a); *(a) = *(b); *(b) = temp; }
#define DEBUG_SORT
#define QUICK_SORT

#ifdef SHELL_SORT
/* A simple shell sort algorithm that works directly on our */
/* cells table..                                            */
static
void shell_sort ( PCell cells,
                  int   count )
{
  PCell  i, j, limit = cells + count;
  TCell  temp;
  int    gap;

  /* compute initial gap */
  for (gap = 0; ++gap < count; gap *=3 );
  while ( gap /= 3 )
  {
    for ( i = cells+gap; i < limit; i++ )
    {
      for ( j = i-gap; ; j -= gap )
      {
        PCell  k = j+gap;

        if ( LESS_THAN(j,k) )
          break;

        SWAP_CELLS(j,k,temp);

        if ( j < cells+gap )
          break;
      }
    }
  }

}
#endif

#ifdef QUICK_SORT
/* this is a non-recursive quicksort that directly process our cells array */
/* it should be faster than calling the stdlib qsort(), and we can even    */
/* tailor our insertion threshold...                                       */

#define QSORT_THRESHOLD  9   /* below this size, a sub-array will be sorted */
                             /* through a normal insertion sort..           */

static
void  quick_sort( PCell cells,
                  int   count )
{
  PCell  stack[40];  /* should be enough ;-) */
  PCell* top;        /* top of stack */
  PCell  base, limit;
  TCell  temp;

  limit = cells + count;
  base  = cells;
  top   = stack;
  for (;;)
  {
    int   len = limit-base;
    PCell i, j, pivot;

    if ( len > QSORT_THRESHOLD)
    {
      /* we use base+len/2 as the pivot */
      pivot = base + len/2;
      SWAP_CELLS( base, pivot, temp );

      i     = base + 1;
      j     = limit-1;

      /* now ensure that *i <= *base <= *j */
      if (LESS_THAN(j,i))
        SWAP_CELLS( i, j, temp );

      if (LESS_THAN(base,i))
        SWAP_CELLS( base, i, temp );

      if (LESS_THAN(j,base))
        SWAP_CELLS( base, j, temp );

      for (;;)
      {
        do i++; while (LESS_THAN(i,base));
        do j--; while (LESS_THAN(base,j));
        if (i > j)
          break;

        SWAP_CELLS( i,j, temp );
      }

      SWAP_CELLS( base, j, temp );

      /* now, push the largest sub-array */
      if ( j - base > limit -i )
      {
        top[0] = base;
        top[1] = j;
        base   = i;
      }
      else
      {
        top[0] = i;
        top[1] = limit;
        limit  = j;
      }
      top += 2;
    }
    else
    {
      /* the sub-array is small, perform insertion sort */
      j = base;
      i = j+1;
      for ( ; i < limit; j = i, i++ )
      {
        for ( ; LESS_THAN(j+1,j); j-- )
        {
          SWAP_CELLS( j+1, j, temp );
          if (j == base)
            break;
        }
      }
      if (top > stack)
      {
        top  -= 2;
        base  = top[0];
        limit = top[1];
      }
      else
        break;
    }
  }
}
#endif


#ifdef DEBUG_GRAYS
#ifdef DEBUG_SORT
static
int  check_sort( PCell  cells, int count )
{
  PCell  p, q;

  for ( p = cells + count-2; p >= cells; p-- )
  {
    q = p+1;
    if (!LESS_THAN(p,q))
      return 0;
  }
  return 1;
}
#endif
#endif


  static
  int  Move_To( FT_Vector*  to,
                FT_Raster   raster )
  {
    TPos  x, y;

    /* record current cell, if any */
    record_cell( (PRaster)raster );

    /* start to a new position */
    x = UPSCALE(to->x);
    y = UPSCALE(to->y);
    start_cell( (PRaster)raster, TRUNC(x), TRUNC(y) );
    ((PRaster)raster)->x = x;
    ((PRaster)raster)->y = y;
    return 0;
  }


  static
  int  Line_To( FT_Vector*  to,
                FT_Raster   raster )
  {
    return render_line( (PRaster)raster, UPSCALE(to->x), UPSCALE(to->y) );
  }


  static
  int  Conic_To( FT_Vector*  control,
                 FT_Vector*  to,
                 FT_Raster   raster )
  {
    return render_conic( (PRaster)raster, control, to );
  }


  static
  int  Cubic_To( FT_Vector*  control1,
                 FT_Vector*  control2,
                 FT_Vector*  to,
                 FT_Raster   raster )
  {
    return render_cubic( (PRaster)raster, control1, control2, to );
  }


  static
  void grays_render_span( int y, int count, FT_Span*  spans, PRaster  raster )
  {
    unsigned char *p;
    FT_Bitmap*    map = &raster->target;
    /* first of all, compute the scanline offset */
    p = (unsigned char*)map->buffer - y*map->pitch;
    if (map->pitch >= 0)
      p += (map->rows-1)*map->pitch;

    for ( ; count > 0; count--, spans++ )
    {
      if (spans->coverage)
#if 1      
        memset( p + spans->x, (spans->coverage+1) >> 1, spans->len );
#else        
      {
        q     = p + spans->x;
        limit = q + spans->len;
        for ( ; q < limit; q++ )
          q[0] = (spans->coverage+1) >> 1;
      }
#endif
    }
  }

#ifdef DEBUG_GRAYS
#include <stdio.h>

  static
  void  dump_cells( RAS_ARG )
  {
    PCell  cell, limit;
    int    y = -1;

    cell = ras.cells;
    limit = cell + ras.num_cells;
    for ( ; cell < limit; cell++ )
    {
      if ( cell->y != y )
      {
        fprintf( stderr, "\n%2d: ", cell->y );
        y = cell->y;
      }
      fprintf( stderr, "[%d %d %d]",
               cell->x, cell->area, cell->cover );
    }
    fprintf(stderr, "\n" );
  }
#endif

  static
  void  grays_hline( RAS_ARG_  TScan  x, TScan y, TPos  area, int acount )
  {
    FT_Span*   span;
    int        count;
    int        coverage;

    /* compute the coverage line's coverage, depending on the    */
    /* outline fill rule..                                       */
    /*                                                           */
    /* The coverage percentage is area/(PIXEL_BITS*PIXEL_BITS*2) */
    /*                                                           */

    coverage = area >> (PIXEL_BITS*2+1-8);  /* use range 0..256 */
    if ( ras.outline.flags & ft_outline_even_odd_fill )
    {
      if (coverage < 0)
        coverage = -coverage;

      while (coverage >= 512)
        coverage -= 512;

      if (coverage > 256)
        coverage = 0;
      else if (coverage == 256)
        coverage = 255;
    }
    else
    {
      /* normal non-zero winding rule */
      if (coverage < 0)
        coverage = -coverage;

      if (coverage >= 256)
        coverage = 255;
    }

    y += ras.min_ey;

    if (coverage)
    {
      /* see if we can add this span to the current list */
      count = ras.num_gray_spans;
      span  = ras.gray_spans + count-1;
      if (count > 0 && ras.span_y == y && (int)span->x + span->len == (int)x &&
          span->coverage == coverage)
      {
        span->len += acount;
        return;
      }

      if ( ras.span_y != y || count >= FT_MAX_GRAY_SPANS)
      {
        if (ras.render_span)
          ras.render_span( ras.span_y, count, ras.gray_spans,
                           ras.render_span_data );
        /* ras.render_span( span->y, ras.gray_spans, count ); */

#ifdef DEBUG_GRAYS
        if (ras.span_y >= 0)
        {
        int  n;
        fprintf( stderr, "y=%3d ", ras.span_y );
        span = ras.gray_spans;
        for (n = 0; n < count; n++, span++)
          fprintf( stderr, "[%d..%d]:%02x ",
                   span->x, span->x + span->len-1, span->coverage );
        fprintf( stderr, "\n" );
        }
#endif

        ras.num_gray_spans = 0;
        ras.span_y         = y;

        count = 0;
        span  = ras.gray_spans;
      }
      else
        span++;

      /* add a gray span to the current list */
      span->x        = (short)x;
      span->len      = (unsigned short)acount;
      span->coverage = (unsigned char)coverage;
      ras.num_gray_spans++;
    }
  }


  static
  void  grays_sweep( RAS_ARG_  FT_Bitmap*  target )
  {
    TScan  x, y, cover, area;
    PCell  start, cur, limit;

    (void)target;

    cur   = ras.cells;
    limit = cur + ras.num_cells;

    cover = 0;
    ras.span_y = -1;
    ras.num_gray_spans = 0;

    for (;;)
    {
      start  = cur;
      y      = start->y;
      x      = start->x;

      area   = start->area;
      cover += start->cover;

      /* accumulate all start cells */
      for (;;)
      {
        ++cur;
        if (cur >= limit || cur->y != start->y || cur->x != start->x)
          break;
          
        area  += cur->area;
        cover += cur->cover;
      }

      /* if the start cell has a non-null area, we must draw an */
      /* individual gray pixel there..                          */
      if (area && x >= 0)
      {
        grays_hline( RAS_VAR_ x, y, cover*(ONE_PIXEL*2)-area, 1 );
        x++;
      }

      if (x < 0)
        x = 0;

      if (cur < limit && start->y == cur->y)
      {
        /* draw a gray span between the start cell and the current one */
        if (cur->x > x)
          grays_hline( RAS_VAR_ x, y, cover*(ONE_PIXEL*2), cur->x - x );
      }
      else
      {
        /* draw a gray span until the end of the clipping region */
        if (cover && x < ras.max_ex)
          grays_hline( RAS_VAR_ x, y, cover*(ONE_PIXEL*2), ras.max_ex - x );
        cover = 0;
      }

      if (cur >= limit)
        break;
    }

    if (ras.render_span && ras.num_gray_spans > 0)
      ras.render_span( ras.span_y, ras.num_gray_spans,
                       ras.gray_spans, ras.render_span_data );
#ifdef DEBUG_GRAYS
    {
      int       n;
      FT_Span*  span;

      fprintf( stderr, "y=%3d ", ras.span_y );
      span = ras.gray_spans;
      for (n = 0; n < ras.num_gray_spans; n++, span++)
        fprintf( stderr, "[%d..%d]:%02x ", span->x, span->x+span->len-1,span->coverage );
      fprintf( stderr, "\n" );
    }
#endif
  }

  typedef struct TBand_
  {
    FT_Pos  min, max;
    
  } TBand;

  static
  int  grays_convert_glyph( RAS_ARG_ FT_Outline*  outline )
  {
    static
    FT_Outline_Funcs  interface =
    {
      (FT_Outline_MoveTo_Func)Move_To,
      (FT_Outline_LineTo_Func)Line_To,
      (FT_Outline_ConicTo_Func)Conic_To,
      (FT_Outline_CubicTo_Func)Cubic_To,
      0,
      0
    };

    TBand    bands[40], *band;
    int      n, num_bands;
    TPos     min, max, max_y;

    /* Set up state in the raster object */
    compute_cbox( RAS_VAR_ outline );

    /* clip to target bitmap, exit if nothing to do */
    if ( ras.max_ex <= 0 || ras.min_ex >= ras.target.width ||
         ras.max_ey <= 0 || ras.min_ey >= ras.target.rows  )
      return 0;
        
    if (ras.min_ex < 0) ras.min_ex = 0;
    if (ras.min_ey < 0) ras.min_ey = 0;

    if (ras.max_ex > ras.target.width) ras.max_ex = ras.target.width;
    if (ras.max_ey > ras.target.rows)  ras.max_ey = ras.target.rows;

    /* simple heuristic used to speed-up the bezier decomposition     */
    /* see the code in render_conic and render_cubic for more details */
    ras.conic_level = 32;
    ras.cubic_level = 16;
    {
      int level = 0;
      if (ras.max_ex > 24 || ras.max_ey > 24)
        level++;
      if (ras.max_ex > 120 || ras.max_ey > 120)
        level+=2;
        
      ras.conic_level <<= level;
      ras.cubic_level <<= level;
    }

    /* setup vertical bands */
    num_bands = (ras.max_ey - ras.min_ey)/ras.band_size;
    if (num_bands == 0)  num_bands = 1;
    if (num_bands >= 39) num_bands = 39;
      
    ras.band_shoot = 0;

    min   = ras.min_ey;
    max_y = ras.max_ey;
    for ( n = 0; n < num_bands; n++, min = max )
    {
      max = min + ras.band_size;
      if (n == num_bands-1 || max > max_y)
        max = max_y;
        
      bands[0].min = min;
      bands[0].max = max;
      band         = bands;
            
      while (band >= bands)
      {
        FT_Pos  bottom, top, middle;
        int     error;
  
        ras.num_cells = 0;
        ras.invalid   = 1;
        ras.min_ey    = band->min;
        ras.max_ey    = band->max;
  
        error = FT_Outline_Decompose( outline, &interface, &ras ) ||
                record_cell( RAS_VAR );
  
        if (!error)
        {
          #ifdef SHELL_SORT
          shell_sort( ras.cells, ras.num_cells );
          #else
          quick_sort( ras.cells, ras.num_cells );
          #endif
      
          #ifdef DEBUG_GRAYS
          check_sort( ras.cells, ras.num_cells );
          dump_cells( RAS_VAR );
          #endif
  
          grays_sweep( RAS_VAR_  &ras.target );
          band--;
          continue;
        }
  
        /* render pool overflow, we will reduce the render band by half */
        bottom = band->min;
        top    = band->max;
        middle = bottom + ((top-bottom) >> 1);
  
        /* waoow !! this is too complex for a single scanline, something */
        /* must be really rotten here !!                                 */
        if (middle == bottom)
        {
          #ifdef DEBUG_GRAYS
          fprintf( stderr, "Rotten glyph !!\n" );
          #endif
          return 1;
        }
  
        if (bottom-top >= ras.band_size)
          ras.band_shoot++;
  
        band[1].min = bottom;
        band[1].max = middle;
        band[0].min = middle;
        band[0].max = top;
        band++;
      }
    }

    if (ras.band_shoot > 8 && ras.band_size > 16)
      ras.band_size = ras.band_size/2;

    return 0;
  }


  extern
  int  grays_raster_render( PRaster            raster,
                            FT_Raster_Params*  params )
  {
    FT_Outline*  outline = (FT_Outline*)params->source;
    FT_Bitmap*   target_map = params->target;

    if ( !raster || !raster->cells || !raster->max_cells )
      return -1;

    /* return immediately if the outline is empty */
    if ( outline->n_points == 0 || outline->n_contours <= 0 )
      return 0;

    if ( !outline || !outline->contours || !outline->points )
      return -1;

    if ( outline->n_points != outline->contours[outline->n_contours - 1] + 1 )
      return -1;

    if ( !target_map || !target_map->buffer )
      return -1;

    /* XXXX: this version does not support monochrome rendering yet ! */
    if ( !(params->flags & ft_raster_flag_aa) )
      return -1;

    ras.outline   = *outline;
    ras.target    = *target_map;
    ras.num_cells = 0;
    ras.invalid   = 1;

    ras.render_span      = (FT_Raster_Span_Func)grays_render_span;
    ras.render_span_data = &ras;
    if ( params->flags & ft_raster_flag_direct )
    {
      ras.render_span      = (FT_Raster_Span_Func)params->gray_spans;
      ras.render_span_data = params->user;
    }

    return grays_convert_glyph( (PRaster)raster, outline );
  }


  /**** RASTER OBJECT CREATION : in standalone mode, we simply use *****/
  /****                          a static object ..                *****/
#ifdef _STANDALONE_

  static
  int  grays_raster_new( void*  memory, FT_Raster *araster )
  {
     static FT_RasterRec_  the_raster;
     *araster = &the_raster;
     memset( &the_raster, sizeof(the_raster), 0 );
     return 0;
  }

  static
  void  grays_raster_done( FT_Raster  raster )
  {
    /* nothing */
    (void)raster;
  }

#else

#include "ftobjs.h"

  static
  int  grays_raster_new( FT_Memory  memory, FT_Raster*  araster )
  {
    FT_Error  error;
    PRaster   raster;

    *araster = 0;
    if ( !ALLOC( raster, sizeof(TRaster) ))
    {
      raster->memory = memory;
      *araster = (FT_Raster)raster;
    }

    return error;
  }

  static
  void grays_raster_done( FT_Raster  raster )
  {
    FT_Memory  memory = (FT_Memory)((PRaster)raster)->memory;
    FREE( raster );
  }

#endif




  static
  void  grays_raster_reset( FT_Raster    raster,
                           const char*  pool_base,
                           long         pool_size )
  {
    PRaster  rast = (PRaster)raster;
    
    if (raster && pool_base && pool_size >= 4096)
      init_cells( rast, (char*)pool_base, pool_size );
      
    rast->band_size  = (pool_size / sizeof(TCell))/8;
  }


  FT_Raster_Funcs  ft_grays_raster =
  {
    ft_glyph_format_outline,

    (FT_Raster_New_Func)       grays_raster_new,
    (FT_Raster_Reset_Func)     grays_raster_reset,
    (FT_Raster_Set_Mode_Func)  0,
    (FT_Raster_Render_Func)    grays_raster_render,
    (FT_Raster_Done_Func)      grays_raster_done
  };

