/*****************************************************************************/
/*                                                                           */
/*  ftgrays.c  - a new 'perfect' anti-aliasing renderer for FreeType 2       */
/*                                                                           */
/*  (c) 2000 David Turner - <david.turner@freetype.org>                      */
/*                                                                           */
/*  Beware, this code is still in heavy beta..                               */
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
/*  It has the following disadvantages (for now):                            */
/*                                                                           */
/*     - need more memory than the standard scan-converter to render         */
/*       a single outline. Note that this may be changed in a near           */
/*       future (we might be able to pack the data in the TCell structure)   */
/*                                                                           */
/*     - apparently, glyphs rendered with this module are a bit more         */
/*       "fuzzy" than those produced with the standard renderer. I hope      */
/*       to fix this using a gamma table somewhere..                         */
/*                                                                           */
/*                                                                           */

#include <ftimage.h>

#define ErrRaster_Invalid_Outline  -1

#include "ftgrays.h"

#define xxxDEBUG_GRAYS

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

#define  PIXEL_BITS  7
#define  ONE_PIXEL   (1L << PIXEL_BITS)
#define  PIXEL_MASK  (-1L << PIXEL_BITS)
#define  TRUNC(x)    ((x) >> PIXEL_BITS)
#define  SUBPIXELS(x) ((x) << PIXEL_BITS)
#define  FLOOR(x)    ((x) & -ONE_PIXEL)
#define  CEILING(x)  (((x)+ONE_PIXEL-1) & -ONE_PIXEL)
#define  ROUND(x)    (((x)+ONE_PIXEL/2) & -ONE_PIXEL)

#define  UPSCALE(x)  (PIXEL_BITS >= 6 ? (x) << (PIXEL_BITS-6) : (x) >> (6-PIXEL_BITS))

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
    cell->x     = ras.ex - ras.min_ex;
    cell->y     = ras.ey - ras.min_ey;
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

  /* everything is on a single scanline */  
  if ( ey1 == ey2 )
  {
    if (render_scanline( RAS_VAR_ ey1, ras.x, fy1, to_x, fy2 )) goto Error;
    goto Fin;
  }

  /* ok, we'll have to render a run of adjacent cells on the same */
  /* scanline..                                                   */
  /*                                                              */
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
  
  dx = ras.x + to->x - (control->x << 1); if (dx < 0) dx = -dx;
  dy = ras.y + to->y - (control->y << 1); if (dy < 0) dy = -dy;
  if (dx < dy) dx = dy;
  
  level = 1;
  dx = dx/64;
  while ( dx > 0 )
  {
    dx >>= 1;
    level++;
  }

  if (level <= 1)
    return render_line( RAS_VAR_ UPSCALE(to->x), UPSCALE(to->y) );

  arc      = ras.bez_stack;
  arc[0]   = *to;
  arc[1]   = *control;
  arc[2].x = ras.x;
  arc[2].y = ras.y;

  arc[0].x = UPSCALE(arc[0].x);
  arc[0].y = UPSCALE(arc[0].y);
  arc[1].x = UPSCALE(arc[1].x);
  arc[1].y = UPSCALE(arc[1].y);

  levels    = ras.lev_stack;
  top       = 0;
  levels[0] = level;

  for (;;)
  {
    level = levels[top];
    if (level > 1)
    {
      split_conic(arc);
      arc += 2;
      top ++;
      levels[top] = levels[top-1] = level-1;
    }
    else
    {
      if (render_line( RAS_VAR_ arc[0].x, arc[0].y )) return 1;
      top--;
      arc-=2;
      if (top < 0)
        return 0;
    }
  }
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
  
  dx = ras.x + to->x - (control1->x << 1); if (dx < 0) dx = -dx;
  dy = ras.y + to->y - (control1->y << 1); if (dy < 0) dy = -dy;
  if (dx < dy) dx = dy;
  da = dx;
  
  dx = ras.x + to->x - 3*(control1->x + control2->x); if (dx < 0) dx = -dx;
  dy = ras.y + to->y - 3*(control1->x + control2->y); if (dy < 0) dy = -dy;
  if (dx < dy) dx = dy;
  db = dx;
   
  level = 1;
  while ( da > 0 || db > 0 )
  {
    da >>= 1;
    db >>= 2;
    level++;
  }

  if (level <= 1)
    return render_line( RAS_VAR_ UPSCALE(to->x), UPSCALE(to->y) );

  arc      = ras.bez_stack;
  arc[0]   = *to;
  arc[1]   = *control2;
  arc[2]   = *control1;
  arc[3].x = ras.x;
  arc[3].y = ras.y;

  arc[0].x = UPSCALE(arc[0].x);
  arc[0].y = UPSCALE(arc[0].y);
  arc[1].x = UPSCALE(arc[1].x);
  arc[1].y = UPSCALE(arc[1].y);
  arc[2].x = UPSCALE(arc[2].x);
  arc[2].y = UPSCALE(arc[2].y);

  levels    = ras.lev_stack;
  top       = 0;
  levels[0] = level;

  for (;;)
  {
    level = levels[top];
    if (level > 1)
    {
      split_cubic(arc);
      arc += 3;
      top ++;
      levels[top] = levels[top-1] = level-1;
    }
    else
    {
      if (render_line( RAS_VAR_ arc[0].x, arc[0].y )) return 1;
      top --;
      arc -= 3;
      if (top < 0)
        return 0;
    }
  }
}


/* a macro comparing two cell pointers. returns true if a <= b */
#define LESS_THAN(a,b)   ( (a)->y<(b)->y || ((a)->y==(b)->y && (a)->x<=(b)->x) )
#define SWAP_CELLS(a,b,temp)  { temp = *(a); *(a) = *(b); *(b) = temp; }
#define DEBUG_SORT
#define SHELL_SORT

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

#define QSORT_THRESHOLD  4   /* below this size, a sub-array will be sorted */
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
    PCell i, j;
    
    if ( len > QSORT_THRESHOLD)
    {
      /* we use base+len/2 as the pivot */
      SWAP_CELLS( base, base+len/2, temp );
      i = base+1;
      j = limit-1;

      /* now ensure that *i <= *base <= *j */
      if (LESS_THAN(j,i))
        SWAP( i, j, temp );
        
      if (LESS_THAN(base,i))
        SWAP( base, i, temp );
        
      if (LESS_THAN(j,base))
        SWAP( base, j, temp );
        
      for (;;)
      {
        do i++ while (LESS_THAN(i,base));
        do j-- while (LESS_THAN(base,j));
        if (i > j)
          break;
          
        SWAP( i,j );
      }
      /* move pivot to correct place */
      SWAP( base, j, temp );
      
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
          SWAP( j+1, j, temp );
          if (j == base)
            break;
        }
      }
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

#if 0
  static
  int  FT_Decompose_Outline( FT_Outline*        outline,
                             FT_Outline_Funcs*  interface,
                             void*              user )
  {
    typedef enum  _phases
    {
      phase_point,
      phase_conic,
      phase_cubic,
      phase_cubic2

    } TPhase;

    FT_Vector  v_first;
    FT_Vector  v_last;
    FT_Vector  v_control;
    FT_Vector  v_start;

    FT_Vector* point;
    FT_Vector* limit;
    char*      tags;

    int    n;         /* index of contour in outline     */
    int    first;     /* index of first point in contour */
    int    error;
    char   tag;       /* current point's state           */


    first = 0;

    for ( n = 0; n < outline->n_contours; n++ )
    {
      int  last;  /* index of last point in contour */

      last  = outline->contours[n];
      limit = outline->points + last;

      v_first = outline->points[first];
      v_last  = outline->points[last];

      v_start = v_control = v_first;

      point = outline->points + first;
      tags  = outline->tags  + first;
      tag   = FT_CURVE_TAG( tags[0] );

      /* A contour cannot start with a cubic control point! */
      if ( tag == FT_Curve_Tag_Cubic )
        goto Invalid_Outline;

      /* check first point to determine origin */
      if ( tag == FT_Curve_Tag_Conic )
      {
        /* first point is conic control.  Yes, this happens. */
        if ( FT_CURVE_TAG( outline->tags[last] ) == FT_Curve_Tag_On )
        {
          /* start at last point if it is on the curve */
          v_start = v_last;
          limit--;
        }
        else
        {
          /* if both first and last points are conic,         */
          /* start at their middle and record its position    */
          /* for closure                                      */
          v_start.x = ( v_start.x + v_last.x ) / 2;
          v_start.y = ( v_start.y + v_last.y ) / 2;

          v_last = v_start;
        }
        point--;
        tags--;
      }

      error = interface->move_to( &v_start, user );
      if (error) goto Exit;

      while (point < limit)
      {
        point++;
        tags++;
  
        tag = FT_CURVE_TAG( tags[0] );
        switch (tag)
        {
          case FT_Curve_Tag_On:  /* emit a single line_to */
            {
              error = interface->line_to( point, user );
              if (error) goto Exit;
              continue;
            }

          
          case FT_Curve_Tag_Conic:  /* consume conic arcs */
            {
              v_control = point[0];
              
            Do_Conic:
              if (point < limit)
              {
                FT_Vector  v_middle;
                
                point++;
                tags++;
                tag = FT_CURVE_TAG( tags[0] );
                
                if (tag == FT_Curve_Tag_On)
                {
                  error = interface->conic_to( &v_control, point, user );
                  if (error) goto Exit;
                  continue;
                }
                
                if (tag != FT_Curve_Tag_Conic)
                  goto Invalid_Outline;
  
                v_middle.x = (v_control.x + point->x)/2;
                v_middle.y = (v_control.y + point->y)/2;
  
                error = interface->conic_to( &v_control, &v_middle, user );
                if (error) goto Exit;
                
                v_control = point[0];
                goto Do_Conic;
              }
              
              error = interface->conic_to( &v_control, &v_start, user );
              goto Close;
            }
          
          default:  /* FT_Curve_Tag_Cubic */
            {
              if ( point+1 > limit         ||
                   FT_CURVE_TAG( tags[1] ) != FT_Curve_Tag_Cubic )
                goto Invalid_Outline;
                
              point += 2;
              tags  += 2;
              
              if (point <= limit)
              {
                error = interface->cubic_to( point-2, point-1, point, user );
                if (error) goto Exit;
                continue;
              }
              
              error = interface->cubic_to( point-2, point-1, &v_start, user );
              goto Close;
            }
        }
      }

      /* close the contour with a line segment */
      error = interface->line_to( &v_start, user );
      
   Close:
      if (error) goto Exit;
      first = last+1;
    }

    return 0;
  Exit:
    return error;
    
  Invalid_Outline:
    return -1;
  }
#else
  static
  int  FT_Decompose_Outline( FT_Outline*        outline,
                             FT_Outline_Funcs*  interface,
                             void*              user )
  {
    typedef enum  _phases
    {
      phase_point,
      phase_conic,
      phase_cubic,
      phase_cubic2

    } TPhase;

    FT_Vector  v_first;
    FT_Vector  v_last;
    FT_Vector  v_control;
    FT_Vector  v_control2;
    FT_Vector  v_start;

    FT_Vector* point;
    char*      tags;

    int    n;         /* index of contour in outline     */
    int    first;     /* index of first point in contour */
    int    index;     /* current point's index           */

    int    error;

    char   tag;       /* current point's state           */
    TPhase phase;


    first = 0;

    for ( n = 0; n < outline->n_contours; n++ )
    {
      int  last;  /* index of last point in contour */


      last = outline->contours[n];

      v_first = outline->points[first];
      v_last  = outline->points[last];

      v_start = v_control = v_first;

      tag   = FT_CURVE_TAG( outline->tags[first] );
      index = first;

      /* A contour cannot start with a cubic control point! */

      if ( tag == FT_Curve_Tag_Cubic )
        return ErrRaster_Invalid_Outline;


      /* check first point to determine origin */

      if ( tag == FT_Curve_Tag_Conic )
      {
        /* first point is conic control.  Yes, this happens. */
        if ( FT_CURVE_TAG( outline->tags[last] ) == FT_Curve_Tag_On )
        {
          /* start at last point if it is on the curve */
          v_start = v_last;
        }
        else
        {
          /* if both first and last points are conic,         */
          /* start at their middle and record its position    */
          /* for closure                                      */
          v_start.x = ( v_start.x + v_last.x ) / 2;
          v_start.y = ( v_start.y + v_last.y ) / 2;

          v_last = v_start;
        }
        phase = phase_conic;
      }
      else
        phase = phase_point;


      /* Begin a new contour with MOVE_TO */

      error = interface->move_to( &v_start, user );
      if ( error )
        return error;

      point = outline->points + first;
      tags = outline->tags  + first;

      /* now process each contour point individually */

      while ( index < last )
      {
        index++;
        point++;
        tags++;

        tag = FT_CURVE_TAG( tags[0] );

        switch ( phase )
        {
        case phase_point:     /* the previous point was on the curve */

          switch ( tag )
          {
            /* two succesive on points -> emit segment */
          case FT_Curve_Tag_On:
            error = interface->line_to( point, user );
            break;

            /* on point + conic control -> remember control point */
          case FT_Curve_Tag_Conic:
            v_control = point[0];
            phase     = phase_conic;
            break;

            /* on point + cubic control -> remember first control */
          default:
            v_control = point[0];
            phase     = phase_cubic;
            break;
          }
          break;

        case phase_conic:   /* the previous point was a conic control */

          switch ( tag )
          {
            /* conic control + on point -> emit conic arc */
          case  FT_Curve_Tag_On:
            error = interface->conic_to( &v_control, point, user );
            phase = phase_point;
            break;

            /* two successive conics -> emit conic arc `in between' */
          case FT_Curve_Tag_Conic:
            {
              FT_Vector  v_middle;


              v_middle.x = (v_control.x + point->x)/2;
              v_middle.y = (v_control.y + point->y)/2;

              error = interface->conic_to( &v_control,
                                           &v_middle, user );
              v_control = point[0];
            }
             break;

          default:
            error = ErrRaster_Invalid_Outline;
          }
          break;

        case phase_cubic:  /* the previous point was a cubic control */

          /* this point _must_ be a cubic control too */
          if ( tag != FT_Curve_Tag_Cubic )
            return ErrRaster_Invalid_Outline;

          v_control2 = point[0];
          phase      = phase_cubic2;
          break;


        case phase_cubic2:  /* the two previous points were cubics */

          /* this point _must_ be an on point */
          if ( tag != FT_Curve_Tag_On )
            error = ErrRaster_Invalid_Outline;
          else
            error = interface->cubic_to( &v_control, &v_control2,
                                         point, user );
          phase = phase_point;
          break;
        }

        /* lazy error testing */
        if ( error )
          return error;
      }

      /* end of contour, close curve cleanly */
      error = 0;

      tag = FT_CURVE_TAG( outline->tags[first] );

      switch ( phase )
      {
      case phase_point:
        if ( tag == FT_Curve_Tag_On )
          error = interface->line_to( &v_first, user );
        break;

      case phase_conic:
        error = interface->conic_to( &v_control, &v_start, user );
        break;

      case phase_cubic2:
        if ( tag == FT_Curve_Tag_On )
          error = interface->cubic_to( &v_control, &v_control2,
                                       &v_first,   user );
        else
          error = ErrRaster_Invalid_Outline;
        break;

      default:
        error = ErrRaster_Invalid_Outline;
        break;
      }

      if ( error )
        return error;

      first = last + 1;
    }

    return 0;
  }

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
  void grays_render_span( int y, int count, FT_GraySpan*  spans, PRaster  raster )
  {
    unsigned char *p, *q, *limit;
    FT_Bitmap*    map = &raster->target;
    /* first of all, compute the scanline offset */
    p = (unsigned char*)map->buffer - y*map->pitch;
    if (map->pitch >= 0)
      p += (map->rows-1)*map->pitch;
    
    for ( ; count > 0; count--, spans++ )
    {
      if (spans->coverage)
      {
        q     = p + spans->x;
        limit = q + spans->len;
        for ( ; q < limit; q++ )
          q[0] = (spans->coverage+1) >> 1;
      }
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
  
#if 0
  static
  void  grays_hline( RAS_ARG_  TScan  x, TScan y, TPos  area, int  count )
  {
    if (area)
      fprintf( stderr, "hline( %3d, %3d, %2d, %5.2f )\n",
               y, x, count, (float)area/(2.0*ONE_PIXEL*ONE_PIXEL) );
  }
#else
  static
  void  grays_hline( RAS_ARG_  TScan  x, TScan y, TPos  area, int acount )
  {
    FT_GraySpan*   span;
    int            count;
    int            coverage;
    
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
          ras.render_span( ras.span_y, count, ras.gray_spans, ras.render_span_closure );
        /* ras.render_span( span->y, ras.gray_spans, count ); */
      
#ifdef DEBUG_GRAYS      
        if (ras.span_y >= 0)
        {
        int  n;
        fprintf( stderr, "y=%3d ", ras.span_y );
        span = ras.gray_spans;
        for (n = 0; n < count; n++, span++)
          fprintf( stderr, "[%d..%d]:%02x ", span->x, span->x + span->len-1, span->coverage );
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
      span->len      = (unsigned char)acount;
      span->coverage = (unsigned char)coverage;
      ras.num_gray_spans++;
    }
  }
#endif

  static
  void  grays_sweep( RAS_ARG_  FT_Bitmap*  target )
  {
    TScan  x, y, cover;
    PCell  start, cur, limit;

    cur   = ras.cells;
    limit = cur + ras.num_cells;

    cover = 0;
    ras.span_y = -1;
    ras.num_gray_spans = 0;
    
    for (;;)
    {
      start = cur;
      y     = start->y;
      x     = start->x;
    
      /* accumulate all start cells */
      for (;;)
      {
        ++cur;
        if (cur >= limit || cur->y != start->y || cur->x != start->x)
          break;
        start->area  += cur->area;
        start->cover += cur->cover;
      }
      
      /* compute next cover */
      cover += start->cover;

      /* if the start cell has a non-null area, we must draw an */
      /* individual gray pixel there..                          */
      if (start->area && x >= 0)
      {
        grays_hline( RAS_VAR_ x, y, cover*(ONE_PIXEL*2)-start->area, 1 );
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
                       ras.gray_spans, ras.render_span_closure );
#ifdef DEBUG_GRAYS
    {
      int  n;
      FT_GraySpan*  span;
      
      fprintf( stderr, "y=%3d ", ras.span_y );
      span = ras.gray_spans;
      for (n = 0; n < ras.num_gray_spans; n++, span++)
        fprintf( stderr, "[%d..%d]:%02x ", span->x, span->x+span->len-1,span->coverage );
      fprintf( stderr, "\n" );
    }
#endif
  }
  
  static
  int  Convert_Glyph( RAS_ARG_ FT_Outline*  outline )
  {
    static
    FT_Outline_Funcs  interface =
    {
      (FT_Outline_MoveTo_Func)Move_To,
      (FT_Outline_LineTo_Func)Line_To,
      (FT_Outline_ConicTo_Func)Conic_To,
      (FT_Outline_CubicTo_Func)Cubic_To
    };

    /* Set up state in the raster object */
    compute_cbox( RAS_VAR_ outline );
    if (ras.min_ex < 0) ras.min_ex = 0;
    if (ras.min_ey < 0) ras.min_ey = 0;
    
    if (ras.max_ex > ras.target.width) ras.max_ex = ras.target.width;
    if (ras.max_ey > ras.target.rows)  ras.max_ey = ras.target.rows;
    
    ras.num_cells = 0;

    /* Now decompose curve */
    if ( FT_Decompose_Outline( outline, &interface, &ras ) )
      return 1;
    /* XXX: the error condition is in ras.error */

    /* record the last cell */
    return record_cell( RAS_VAR );
  }


  extern
  int  grays_raster_render( TRaster*     raster,
                            FT_Outline*  outline,
                            FT_Bitmap*   target_map )
  {
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

    ras.outline   = *outline;
    ras.target    = *target_map;
    ras.num_cells = 0;
    ras.invalid   = 1;

    if (Convert_Glyph( (PRaster)raster, outline ))
      return 1;
      
    shell_sort( ras.cells, ras.num_cells );
    
#ifdef DEBUG_GRAYS    
    check_sort( ras.cells, ras.num_cells );
    dump_cells( RAS_VAR );
#endif
    ras.render_span         = (FT_GraySpan_Func)grays_render_span;
    ras.render_span_closure = &ras;
    
    grays_sweep( (PRaster)raster, target_map );    
    return 0;
  }






  extern
  int  grays_raster_init( FT_Raster    raster,
                          const char*  pool_base,
                          long         pool_size )
  {
/*    static const char  default_palette[5] = { 0, 1, 2, 3, 4 }; */

    /* check the object address */
    if ( !raster )
      return -1;

    /* check the render pool - we won't go under 4 Kb */
    if ( !pool_base || pool_size < 4096 )
      return -1;

    /* save the pool */
    init_cells( (PRaster)raster, (char*)pool_base, pool_size );

    return 0;
  }



  FT_Raster_Interface  ft_grays_raster =
  {
    sizeof( TRaster ),
    ft_glyph_format_outline,

    (FT_Raster_Init_Proc)     grays_raster_init,
    (FT_Raster_Set_Mode_Proc) 0,
    (FT_Raster_Render_Proc)   grays_raster_render
  };


