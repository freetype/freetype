/*****************************************************************************/
/*                                                                           */
/*  ftgrays2.c  - a new version of the standard FreeType anti-aliaser        */
/*                                                                           */
/*  (c) 2000 David Turner - <david.turner@freetype.org>                      */
/*                                                                           */
/*  Beware, this code is still in heavy beta..                               */
/*                                                                           */
/*  After writing a "perfect" anti-aliaser (see ftgrays.c), it is clear      */
/*  that the standard FreeType renderer is better at generating glyph images */
/*  because it uses an approximation that simply produces more contrasted    */
/*  edges, making its output more legible..                                  */
/*                                                                           */
/*  This code is an attempt to rewrite the standard renderer in order to     */
/*  support the following:                                                   */
/*                                                                           */
/*   - get rid of al rendering artifacts produced by the original algorithm  */
/*   - allow direct composition, by generating the output image as a "list"  */
/*     of span in successive scan-lines (the standard code is forced to use  */
/*     an intermediate buffer, and this is just _bad_ :-)                    */
/*                                                                           */
/*                                                                           */
/*  This thing works, but it's slower than the original ftraster.c,          */
/*  probably because the bezier intersection code is different..             */
/*                                                                           */
/*  Note that Type 1 fonts, using a reverse fill algorithm are not           */
/*  supported for now (this should come soon though..)                       */
/*                                                                           */

#include <ftimage.h>

#define _STANDALONE_

#define DEBUG_GRAYS
#define DIRECT_BEZIER
#define PRECISION_STEP  ONE_HALF
#define xxxDYNAMIC_BEZIER_STEPS

#define ErrRaster_Invalid_Outline  -1
#define ErrRaster_Overflow         -2

#include "ftgrays2.h"

/* include the FreeType main header if necessary */
#ifndef _STANDALONE_
#include "freetype.h"  /* for FT_MulDiv & FT_Outline_Decompose */
#endif

#ifdef DEBUG_GRAYS
#include <stdio.h>
#endif

typedef int   TScan;
typedef long  TPos;
typedef float TDist;

#define FT_MAX_GRAY_SPANS  32

typedef struct FT_GraySpan_
{
  short          x;
  short          len;
  unsigned char  coverage;

} FT_GraySpan;

typedef int (*FT_GraySpan_Func)( int           y,
                                 int           count,
                                 FT_GraySpan*  spans,
                                 void*         user );
                                        
typedef enum {

  dir_up    = 0,
  dir_down  = 1,
  dir_right = 2,
  dir_left  = 3,

  dir_horizontal = 2,
  dir_reverse    = 1,
  dir_silent     = 4,
  
  dir_unknown = 8
  
} TDir;


typedef struct TCell_
{
  unsigned short  x;
  unsigned short  y;
  unsigned short  pos;
  TDir            dir;

} TCell, *PCell;



typedef struct TRaster_
{
  PCell   cells;
  PCell   cursor;
  PCell   cell_limit;
  int     max_cells;
  int     num_cells;

  TScan   min_ex, max_ex;
  TScan   min_ey, max_ey;
  TPos    min_x,  min_y;
  TPos    max_x,  max_y;

  TScan   ex, ey;
  TScan   cx, cy;
  TPos    x,  y;

  PCell   contour_cell;  /* first contour cell */

  char    joint;
  char    horizontal;
  TDir    dir;
  PCell   last;
  
  FT_Vector  starter;
  FT_Vector* start;

  int     error;

  FT_Vector*  arc;
  FT_Vector   bez_stack[32*3];
  int         lev_stack[32];

  FT_Outline  outline;
  FT_Bitmap   target;

  FT_GraySpan gray_spans[ FT_MAX_GRAY_SPANS ];
  int         num_gray_spans;

  FT_GraySpan_Func  render_span;
  void*             render_span_closure;
  int               span_y;

} TRaster, *PRaster;



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

#define FMulDiv(a,b,c)  ((long)(a)*(b)/(c))

#ifdef _STANDALONE_
#define SMulDiv(a,b,c)  FMulDiv(a,b,c)  /* XXXX - TO BE CHANGED LATER */
#else
#define SMulDiv(a,b,c)  FT_MulDiv(a,b,c)
#endif

/* note: PIXEL_BITS must not be less than 6 !! */
#define  PIXEL_BITS  6

#define  ONE_PIXEL   (1L << PIXEL_BITS)
#define  ONE_HALF    (ONE_PIXEL/2)
#define  PIXEL_MASK  (-1L << PIXEL_BITS)
#define  TRUNC(x)    ((x) >> PIXEL_BITS)
#define  FRAC(x)     ((x) & (ONE_PIXEL-1))
#define  SUBPIXELS(x) ((x) << PIXEL_BITS)
#define  FLOOR(x)    ((x) & -ONE_PIXEL)
#define  CEILING(x)  (((x)+ONE_PIXEL-1) & -ONE_PIXEL)
#define  ROUND(x)    (((x)+ONE_HALF) & -ONE_PIXEL)

#define  UPSCALE(x)  ((x) << (PIXEL_BITS-6))
#define  DOWNSCALE(x)  ((x) >> (PIXEL_BITS-6))

#define  WRITE_CELL(top,u,v,dir)  write_cell( RAS_VAR_ top, u, v, dir )

/****************************************************************************/
/*                                                                          */
/*   INITIALIZE THE CELLS TABLE                                             */
/*                                                                          */
static
void  init_cells( RAS_ARG_  void*  buffer, long byte_size )
{
  ras.cells      = (PCell)buffer;
  ras.max_cells  = byte_size / sizeof(TCell);
  ras.cell_limit = ras.cells + ras.max_cells;
  ras.num_cells  = 0;
}


/****************************************************************************/
/*                                                                          */
/*   WRITE ONE CELL IN THE RENDER POOL                                      */
/*                                                                          */
static
int  write_cell( RAS_ARG_  PCell  cell, TPos  u,  TPos  v, TDir  dir )
{
#ifdef DEBUG_GRAYS
  static const char  dirs[5] = "udrl?";
#endif
  if (dir & dir_horizontal)
  {
    /* only keep horizontal cells within our clipping box */
    if ( u < ras.min_y || u >= ras.max_y ||
         v < ras.min_x || v >= ras.max_x ) goto Nope;
         
    /* get rid of horizontal cells with pos == 0, they're irrelevant */
    if ( FRAC(u) == 0 ) goto Nope;
    
    cell->y = (unsigned short)TRUNC( u - ras.min_y );
    cell->x = (unsigned short)TRUNC( v - ras.min_x );
  }
  else
  {
    /* get rid of vertical cells that are below or above our clipping    */
    /* box. Also discard all cells that are on the right of the clipping */
    /* box..                                                             */
    if (u >= ras.max_x || v < ras.min_y || v >= ras.max_y) goto Nope;
    u -= ras.min_x;
    v -= ras.min_y;
    
    /* all cells that are on the left of the clipping box are located */
    /* on the same virtual "border" cell..                            */
    if (u < 0) u = -1;
    cell->x = (unsigned short)TRUNC( u );
    cell->y = (unsigned short)TRUNC( v );
  }
  cell->dir = dir;
  cell->pos = FRAC(u);
  
#ifdef DEBUG_GRAYS
  fprintf( stderr, "[%d,%d,%c,%d]\n",
                   (int)cell->y,
                   (int)cell->x,
                   dirs[dir],
                   cell->pos );
#endif
  return 1;
Nope:
  return 0;
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
    ras.min_x  = ras.max_x  = 0;
    ras.min_y  = ras.max_y  = 0;
    goto Exit;
  }
  
  ras.min_x = ras.max_x = vec->x;
  ras.min_y = ras.max_y = vec->y;
  vec++;
  
  for ( ; vec < limit; vec++ )
  {
    TPos  x = vec->x;
    TPos  y = vec->y;
    
    if ( x < ras.min_x ) ras.min_x = x;
    if ( x > ras.max_x ) ras.max_x = x;
    if ( y < ras.min_y ) ras.min_y = y;
    if ( y > ras.max_y ) ras.max_y = y;
  }

  /* grid-fit the bounding box to integer pixels */  
  ras.min_x  &= -64;
  ras.min_y  &= -64;
  ras.max_x   = (ras.max_x+63) & -64;
  ras.max_y   = (ras.max_y+63) & -64;
  
Exit:
  ras.min_ex = ras.min_x >> 6;
  ras.max_ex = ras.max_x >> 6;
  ras.min_ey = ras.min_y >> 6;
  ras.max_ey = ras.max_y >> 6;
}

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    compute_intersects                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Computes the scan-line intersections of a given line and store     */
  /*    the corresonding cells in the render pool..                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    u1   :: The start u coordinate.                                    */
  /*    v1   :: The start v coordinate.                                    */
  /*    u2   :: The end u coordinate.                                      */
  /*    v2   :: The end v coordinate.                                      */
  /*    minv :: The minimum vertical grid coordinate.                      */
  /*    maxv :: The maximum vertical grid coordinate.                      */
  /*    dir  :: The line direction..                                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    error code. 0 means success..                                      */
  /*                                                                       */
  static
  int   compute_intersects( RAS_ARG_ TPos  u1,   TPos  v1,
                                     TPos  u2,   TPos  v2,
                                     TPos  minv, TPos  maxv,
                                     TDir  dir )
  {
    TPos   du, dv, u, v, iu, iv, ru, nu;
    TScan  e1, e2, size;
    PCell  top;
    int    reverse;

    /* exit if dv == 0 */
    if ( v1 == v2 ) goto Exit;
    
    /* adjust to scanline center */
    v1   -= ONE_HALF;
    v2   -= ONE_HALF;
    maxv -= ONE_PIXEL;
    
    /* reverse direction in order to get dv > 0 */
    reverse = 0;
    if ( v2 < v1 )
    {
      TPos  tmp;
      v1   = -v1;   v2   = -v2;
      tmp = minv; minv = -maxv; maxv = -tmp;
      reverse = 1;
    }
    
    /* check that we have an intersection */
    if ( v2 < minv || v1 > maxv )
      goto Exit;

    du = u2 - u1;
    dv = v2 - v1;

    /* set the silent flag */
    if (du > dv)
      dir |= dir_silent;

    /* compute the first scanline in "e1" */
    e1 = CEILING(v1);
    if (e1 == v1 && ras.joint)
      e1 += ONE_PIXEL;

    /* compute the last scanline in "e2" */
    if (v2 <= maxv)
    {
      e2        = FLOOR(v2);
      ras.joint = (v2 == e2);
    }
    else
    {
      e2        = maxv;
      ras.joint = 0;
    }

    size = TRUNC(e2-e1) + 1;
    if (size <= 0) goto Exit;
    
    /* check that there is enough space in the render pool */
    if ( ras.cursor + size > ras.cell_limit )
    {
      ras.error = ErrRaster_Overflow;
      goto Fail;
    }
    
    if (e1-v1 > 0)
      u1 += SMulDiv( e1-v1, du, dv );

    u  = u1;      
    v  = e1; if (reverse) v = -e1;
    v += ONE_HALF;
    iv = (1-2*reverse)*ONE_PIXEL;
    
    /* compute decision variables */
    if (du)
    {
      du <<= PIXEL_BITS;
      iu   = du / dv;
      ru   = du % dv;
      if (ru < 0)
      {
        iu --;
        ru += dv;
      }
    
      nu   = -dv;
      ru <<= 1;
      dv <<= 1;
    }
    else
    {
      iu = 0;
      ru = 0;
      nu = -dv;
    }

    top = ras.cursor;
    for ( ; size > 0; size-- )
    {
      if (WRITE_CELL( top, u, v, dir ))
        top++;

      u  += iu;
      nu += ru;
      if (nu >= 0)
      {
        nu -= dv;
        u++;
      }
      v += iv;
    }
    ras.cursor = top;
    
  Exit:
    return 0;
    
  Fail:
    return 1;
  }



  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    render_line                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function injects a new line segment in the render pool.       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    x      :: target x coordinate (scaled subpixels)                   */
  /*    y      :: target y coordinate (scaled subpixels)                   */
  /*    raster :: A pointer to the current raster object.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  static
  int  render_line( RAS_ARG_  TPos  x, TPos  y )
  {
    TPos  minv, maxv;
    TDir  new_dir;

    minv    = ras.min_y;
    maxv    = ras.max_y;
    if (ras.horizontal)
    {
      minv = ras.min_x;
      maxv = ras.max_x;
    }
    
    new_dir = ras.dir;

    /* first of all, detect a change of direction */
    if ( y != ras.y )
    {
      new_dir = ( y > ras.y ) ? dir_up : dir_down;
      if (ras.horizontal) new_dir |= dir_horizontal;
          
      if ( new_dir != ras.dir )
      {
        ras.joint = 0;
        ras.dir   = new_dir;
      }
    }
    
    /* then compute line intersections */
    if ( compute_intersects( RAS_VAR_  ras.x, ras.y, x, y,
                                       minv, maxv, new_dir ) )
      goto Fail;
    
    ras.x = x;
    ras.y = y;

    return 0;

  Fail:
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



#ifndef DIRECT_BEZIER
static
int  render_conic( RAS_ARG_  TPos  x1, TPos y1, TPos x2, TPos y2 )
{
  TPos        x0, y0;
  TPos        dx, dy;
  int         top, level;
  int*        levels;
  FT_Vector*  arc;

  x0 = ras.x;
  y0 = ras.y;
  
  dx    = x0 + x2 - 2*x1; if (dx < 0) dx = -dx;
  dy    = y0 + y2 - 2*y1; if (dy < 0) dy = -dy;
  if (dx < dy) dx = dy;
  level = 1;
  dx    = DOWNSCALE(dx)/32;
  while ( dx > 0 )
  {
    dx >>= 1;
    level++;
  }

  if (level <= 1)
    return render_line( RAS_VAR_ x2, y2 );

  arc      = ras.bez_stack;
  arc[0].x = x2;
  arc[0].y = y2;
  arc[1].x = x1;
  arc[1].y = y1;
  arc[2].x = x0;
  arc[2].y = y0;

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
int  render_cubic( RAS_ARG_ TPos  x1, TPos  y1,
                            TPos  x2, TPos  y2,
                            TPos  x3, TPos  y3 )
{
  TPos        x0, y0;
  TPos        dx, dy, da, db;
  int         top, level;
  int*        levels;
  FT_Vector*  arc;
  
  x0 = ras.x;
  y0 = ras.y;

  dx = x0 + x3 - 2*x1; if (dx < 0) dx = -dx;
  dy = y0 + y3 - 2*y1; if (dy < 0) dy = -dy;
  da = dy; if (da < dx) da = dx;
  
  dx = x0 + x3 - 3*(x1+x2); if (dx < 0) dx = -dx;
  dy = y0 + y3 - 3*(y1+y2); if (dy < 0) dy = -dy;
  db = dy; if (db < dx) db = dx;

  da = DOWNSCALE(da);
  db = DOWNSCALE(db);
  
  level = 1;
  da    = da/64;
  db    = db/128;
  while ( da > 0 || db > 0 )
  {
    da >>= 1;
    db >>= 2;
    level++;
  }

  if (level <= 1)
    return render_line( RAS_VAR_ x3, y3 );

  arc      = ras.bez_stack;
  arc[0].x = x3;
  arc[0].y = y3;
  arc[1].x = x2;
  arc[1].y = y2;
  arc[2].x = x1;
  arc[2].y = y1;
  arc[3].x = x0;
  arc[3].y = y0;

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
#else /* !DIRECT_BEZIER */
 /* A function type describing the functions used to split bezier arcs */
  typedef  void  (*TSplitter)( FT_Vector*  base );

#ifdef DYNAMIC_BEZIER_STEPS
  static
  TPos  Dynamic_Bezier_Threshold( RAS_ARG_ int degree, FT_Vector*  arc )
  {
    TPos    min_x,  max_x,  min_y, max_y, A, B;
    TPos    wide_x, wide_y, threshold;
    
    FT_Vector* cur   = arc;
    FT_Vector* limit = cur + degree;

    /* first of all, set the threshold to the maximum x or y extent */
    min_x = max_x = arc[0].x;
    min_y = max_y = arc[0].y;
    cur++;
    for ( ; cur < limit; cur++ )
    {
      TPos  x = cur->x;
      TPos  y = cur->y;

      if ( x < min_x ) min_x = x;
      if ( x > max_x ) max_x = x;

      if ( y < min_y ) min_y = y;
      if ( y > max_y ) max_y = y;
    }
    wide_x = (max_x - min_x) << 4;
    wide_y = (max_y - min_y) << 4;

    threshold = wide_x;
    if (threshold < wide_y) threshold = wide_y;

    /* now compute the second and third order error values */
    
    wide_x = arc[0].x + arc[1].x - arc[2].x*2;
    wide_y = arc[0].y + arc[1].y - arc[2].y*2;

    if (wide_x < 0) wide_x = -wide_x;
    if (wide_y < 0) wide_y = -wide_y;

    A = wide_x; if ( A < wide_y ) A = wide_y;

    if (degree >= 3)
    {
      wide_x = arc[3].x - arc[0].x + 3*(arc[2].x - arc[3].x);
      wide_y = arc[3].y - arc[0].y + 3*(arc[2].y - arc[3].y);

      if (wide_x < 0) wide_x = -wide_x;
      if (wide_y < 0) wide_y = -wide_y;

      B = wide_x; if ( B < wide_y ) B = wide_y;
    }
    else
      B = 0;

    while ( A > 0 || B > 0 )
    {
      threshold >>= 1;
      A         >>= 2;
      B         >>= 3;
    }

    if (threshold < PRECISION_STEP)
      threshold = PRECISION_STEP;

    return threshold;
  }
#endif /* DYNAMIC_BEZIER_STEPS */

  static
  int   render_bezier( RAS_ARG_ int        degree,
                                TSplitter  splitter,
                                TPos       minv,
                                TPos       maxv,
                                TDir       dir )
  {
    TPos  v1, v2, u, v, e1, e2, threshold;
    int   reverse;

    FT_Vector*  arc;
    FT_Vector   init;

    PCell  top;

    arc  = ras.arc;
    init = arc[0];

    arc[0].y -= ONE_HALF;
    arc[1].y -= ONE_HALF;
    arc[2].y -= ONE_HALF;
    maxv     -= ONE_PIXEL;

    top = ras.cursor;
    
    /* ensure that our segment is ascending */    
    v1 = arc[degree].y;
    v2 = arc[0].y;
    reverse = 0;
    if ( v2 < v1 )
    {
      TPos tmp;
      v1 = -v1;
      v2 = -v2;
      arc[0].y = v2;
      arc[1].y = -arc[1].y;
      arc[degree].y = v1;
      if (degree > 2)
        arc[2].y = -arc[2].y;
        
      tmp = minv; minv = -maxv; maxv = -tmp;
      reverse = 1;
    }

    if ( v2 < minv || v1 > maxv )
      goto Fin;

    /* compute the first scanline in "e1" */
    e1 = CEILING(v1);
    if (e1 == v1 && ras.joint)
      e1 += ONE_PIXEL;

    /* compute the last scanline in "e2" */
    if (v2 <= maxv)
    {
      e2        = FLOOR(v2);
      ras.joint = (v2 == e2);
    }
    else
    {
      e2        = maxv;
      ras.joint = 0;
    }

    /* exit if the current scanline is already above the max scanline */
    if ( e2 < e1 )
      goto Fin;

    /* check for overflow */
    if ( ( top + TRUNC(e2-e1)+1 ) >= ras.cell_limit )
    {
      ras.cursor = top;
      ras.error  = ErrRaster_Overflow;
      return 1;
    }

#ifdef DYNAMIC_BEZIER_STEPS
    /* compute dynamic bezier step threshold */
    threshold = Dynamic_Bezier_Threshold( RAS_VAR_ degree, arc );
#else
    threshold = PRECISION_STEP;
#endif

    /* loop while there is still an arc on the bezier stack */
    /* and the current scan line is below y max == e2       */
    while ( arc >= ras.arc && e1 <= e2 )
    {
      ras.joint = 0;

      v2 = arc[0].y;      /* final y of the top-most arc */
      
      if ( v2 > e1 )   /* the arc intercepts the current scanline */
      {
        v1 = arc[degree].y;  /* start y of top-most arc */

        if ( v2 >= e1 + ONE_PIXEL || v2 - v1 >= threshold )
        {
          /* if the arc's height is too great, split it */
          splitter( arc );
          arc += degree;
        }
        else
        {
          /* otherwise, approximate it as a segment and compute */
          /* its intersection with the current scanline         */
          u = arc[degree].x +
              FMulDiv( arc[0].x-arc[degree].x,
                       e1 - v1,
                       v2 - v1 );
                       
          v = e1; if (reverse) v = -e1;
          v += ONE_HALF;
          if (WRITE_CELL( top, u, v, dir ))
            top++;

          arc -= degree;     /* pop the arc         */
          e1  += ONE_PIXEL;  /* go to next scanline */
        }
      }
      else
      {
        if ( v2 == e1 )       /* if the arc falls on the scanline */
        {                     /* record its _joint_ intersection  */
          ras.joint  = 1;
          u          = arc[degree].x;
          v = e1; if (reverse) v = -e1;
          v += ONE_HALF;
          if (WRITE_CELL( top, u, v, dir ))
            top++;
          
          e1 += ONE_PIXEL; /* go to next scanline */
        }
        arc -= degree;        /* pop the arc */
      }
    }

  Fin:
    ras.arc[0] = init;
    ras.cursor = top;
    return 0;
  }

  
static
int  render_conic( RAS_ARG_  TPos  x1, TPos y1, TPos x2, TPos y2 )
{
  TPos        x0,   y0;
  TPos        minv, maxv;
  FT_Vector*  arc;

  x0 = ras.x;
  y0 = ras.y;

  minv = ras.min_y;
  maxv = ras.max_y;
  if (ras.horizontal)
  {
    minv = ras.min_x;
    maxv = ras.max_x;
  }

  arc = ras.bez_stack;
  arc[2].x = ras.x; arc[2].y = ras.y;
  arc[1].x = x1;    arc[1].y = y1;
  arc[0].x = x2;    arc[0].y = y2;

  do
  {
    TDir  dir;
    TPos  ymin, ymax;
    
    y0 = arc[2].y;
    y1 = arc[1].y;
    y2 = arc[0].y;
    x2 = arc[0].x;

    /* first, categorize the Bezier arc */
    ymin = y0;
    ymax = y2;
    if (ymin > ymax)
    {
      ymin = y2;
      ymax = y0;
    }

    if (y1 < ymin || y1 > ymax)
    {
      /* this arc isn't y-monotonous, split it */
      split_conic( arc );
      arc += 2;
    }
    else if ( y0 == y2 )
    {
      /* this arc is flat, ignore it */
      arc -= 2;
    }
    else
    {
      /* the arc is y-monotonous, either ascending or descending */
      /* detect a change of direction                            */
      dir = ( y0 < y2 ) ? dir_up : dir_down;
      if (ras.horizontal) dir |= dir_horizontal;
      if (dir != ras.dir)
      {
        ras.joint = 0;
        ras.dir   = dir;
      }

      ras.arc = arc;    
      if (render_bezier( RAS_VAR_ 2, split_conic, minv, maxv, dir ))
        goto Fail;
      arc -= 2;
    }
  } while ( arc >= ras.bez_stack );

  ras.x = x2;
  ras.y = y2;
  return 0;
Fail:
  return 1;
}   

static
int  render_cubic( RAS_ARG_  TPos  x1, TPos y1, TPos x2, TPos y2, TPos x3, TPos y3 )
{
  TPos        x0, y0;
  TPos        minv, maxv;
  FT_Vector*  arc;

  x0 = ras.x;
  y0 = ras.y;

  minv = ras.min_y;
  maxv = ras.max_y;
  if (ras.horizontal)
  {
    minv = ras.min_x;
    maxv = ras.max_x;
  }

  arc = ras.bez_stack;
  arc[0].x = ras.x; arc[0].y = ras.y;
  arc[1].x = x1;    arc[1].y = y1;
  arc[2].x = x2;    arc[2].y = y2;
  arc[3].x = x3;    arc[3].y = y3;

  do
  {
    TDir  dir;
    TPos  ymin1, ymax1, ymin2, ymax2;
    
    y0 = arc[3].y;
    y1 = arc[2].y;
    y2 = arc[1].y;
    y3 = arc[0].y;
    x3 = arc[0].x;

    /* first, categorize the Bezier arc */
    ymin1 = y0;
    ymax1 = y3;
    if (ymin1 > ymax1)
    {
      ymin1 = y3;
      ymax1 = y0;
    }

    ymin2 = y1;
    ymax2 = y2;
    if (ymin2 > ymax2)
    {
      ymin2 = y2;
      ymax2 = y1;
    }

    if ( ymin2 < ymin1 || ymax2 > ymax1)
    {
      /* this arc isn't y-monotonous, split it */
      split_cubic( arc );
      arc += 3;
    }
    else if ( y0 == y3 )
    {
      /* this arc is flat, ignore it */
      arc -= 3;
    }
    else
    {
      /* the arc is y-monotonous, either ascending or descending */
      /* detect a change of direction                            */
      dir = ( y0 < y3 ) ? dir_up : dir_down;
      if (ras.horizontal) dir |= dir_horizontal;
      if (dir != ras.dir)
      {
        ras.joint = 0;
        ras.dir   = dir;
      }

      ras.arc = arc;      
      if (render_bezier( RAS_VAR_ 3, split_cubic, minv, maxv, dir ))
        goto Fail;
      arc -= 3;
    }
  } while ( arc >= ras.bez_stack );

  ras.x = x2;
  ras.y = y2;
  return 0;
Fail:
  return 1;
}   
  
#endif /* !DIRECT_BEZIER */


static
int  is_less_than( PCell  a, PCell b )
{
  if (a->y < b->y) goto Yes;
  if (a->y == b->y)
  {
    if (a->x < b->x) goto Yes;
    if (a->x == b->x)
    {
      TDir  ad = a->dir & (dir_horizontal|dir_silent);
      TDir  bd = b->dir & (dir_horizontal|dir_silent);
      if ( ad < bd ) goto Yes;
      if ( ad == bd && a->pos < b->pos) goto Yes;
    }
  }
  return 0;
Yes:
  return 1;
}

/* a macro comparing two cell pointers. returns true if a <= b */
#define LESS_THAN(a,b)   is_less_than( (PCell)(a), (PCell)(b) )
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
      /* move pivot to correct place */
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
      } else
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

#ifdef _STANDALONE_
#if 1
  static
  int  FT_Outline_Decompose( FT_Outline*        outline,
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
  int  FT_Outline_Decompose( FT_Outline*        outline,
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

      v_start = outline->points[first];
      v_last  = outline->points[last];

      v_control = v_start;

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
          error = interface->line_to( &v_start, user );
        break;

      case phase_conic:
        error = interface->conic_to( &v_control, &v_start, user );
        break;

      case phase_cubic2:
        if ( tag == FT_Curve_Tag_On )
          error = interface->cubic_to( &v_control, &v_control2,
                                       &v_start,   user );
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
#endif /* _STANDALONE_ */


  static
  int  Move_To2( FT_Vector*  to,
                 FT_Raster   raster )
  {
    PRaster  rast = (PRaster)raster;
    FT_Pos*  to_x;
    FT_Pos*  to_y;

    to_x = &to->x;
    to_y = &to->y;
    if (rast->horizontal)
    {
      to_x = &to->y;
      to_y = &to->x;
    }
    
    rast->starter.x = UPSCALE(*to_x);
    rast->starter.y = UPSCALE(*to_y);

    rast->joint  = 0;
    rast->dir    = dir_unknown;
    rast->last   = 0;
    rast->start  = 0;

    if ((*to_x & 63) == 32)
    {
      rast->starter.x |= 1;
      rast->start = to;
    }
    if ((*to_y & 63) == 32)
    {
      rast->starter.y |= 1;
      rast->start = to;
    }

    rast->x = rast->starter.x;
    rast->y = rast->starter.y;
    return 0;
  }
  

  static
  int  Line_To2( FT_Vector*  to,
                 FT_Raster   raster )
  {
    TPos     x, y;
    PRaster  rast = (PRaster)raster;

    if ( to == rast->start )
    {
      x = rast->starter.x;
      y = rast->starter.y;
    }
    else
    {
      if ( rast->horizontal )
      {
        x = to->y;
        y = to->x;
      }
      else
      {
        x = to->x;
        y = to->y;
      }
      x = UPSCALE(x);
      y = UPSCALE(y);
    }
    
    return render_line( rast, x, y );
  }


  static
  int  Conic_To2( FT_Vector*  control,
                  FT_Vector*  to,
                  FT_Raster   raster )
  {
    PRaster    rast = (PRaster)raster;
    FT_Vector  ctr, to2;
    
    ctr = *control;
    to2 = *to;
    if (rast->horizontal)
    {
      ctr.x = control->y;
      ctr.y = control->x;
      to2.x = to->y;
      to2.y = to->x;
    }

    if ( to == rast->start )
      to2 = rast->starter;
    else
    {
      to2.x = UPSCALE(to2.x);
      to2.y = UPSCALE(to2.y);
    }

    return render_conic( rast, UPSCALE(ctr.x), UPSCALE(ctr.y), to2.x, to2.y );
  }


  static
  int  Cubic_To2( FT_Vector*  control1,
                  FT_Vector*  control2,
                  FT_Vector*  to,
                  FT_Raster   raster )
  {
    PRaster  rast = (PRaster)raster;
    FT_Vector ctr1, ctr2, to2;
    
    ctr1 = *control1;
    ctr2 = *control2;
    to2  = *to;
    if (rast->horizontal)
    {
      ctr1.x = control1->y; ctr1.y = control1->x;
      ctr2.x = control2->y; ctr2.y = control2->x;
      to2.x  = to->y;  to2.y = to->x;
    }

    if ( to == rast->start )
      to2 = rast->starter;
    else
    {
      to2.x = UPSCALE(to2.x);
      to2.y = UPSCALE(to2.y);
    }

    return render_cubic( rast, UPSCALE(ctr1.x), UPSCALE(ctr1.y),
                               UPSCALE(ctr2.x), UPSCALE(ctr2.y),
                               to2.x, to2.y );
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
          q[0] = spans->coverage >> 1;
      }
    }
  }

#ifdef DEBUG_GRAYS
#include <stdio.h>

  static
  void  dump_cells( RAS_ARG )
  {
    static const char  dirs[5] = "udrl?";
    PCell  cell, limit;
    int    y = -1;
    
    cell = ras.cells;
    limit = cell + ras.num_cells;
    for ( ; cell < limit; cell++ )
    {
      if ( cell->y != y )
      {
        fprintf( stderr, "\n%2d: ", (int)cell->y );
        y = cell->y;
      }
      fprintf( stderr, "[%d %c %d]",
               (int)cell->x,
               dirs[cell->dir & 3],
               cell->pos );
    }
    fprintf(stderr, "\n" );
  }
#endif
  
  static
  void  grays_hline( RAS_ARG_  TScan  y, TScan x, int  coverage, int acount )
  {
    FT_GraySpan*   span;
    int            count;
    
    /* compute the coverage line's coverage, depending on the    */
    /* outline fill rule..                                       */
    /*                                                           */
    /* The coverage percentage is area/ONE_PIXEL                 */
    /*                                                           */

    coverage <<= 1;
    coverage >>= (PIXEL_BITS-6);
      
    if (coverage < 0)
      coverage = -coverage;
      
    if (coverage >= 256)
      coverage = 255;
    
    if (coverage)
    {
      x += ras.min_ex;
    
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
          ras.render_span( ras.min_ey + ras.span_y, count, ras.gray_spans, ras.render_span_closure );
        /* ras.render_span( span->y, ras.gray_spans, count ); */
      
#ifdef DEBUG_GRAYS      
        if (ras.span_y >= 0)
        {
        int  n;
        fprintf( stderr, "y=%3d ", ras.span_y );
        span = ras.gray_spans;
        for (n = 0; n < count; n++, span++)
        {
          if (span->len > 1)
            fprintf( stderr, "[%d..%d]:%02x ", span->x, span->x + span->len-1, span->coverage );
          else
            fprintf( stderr, "[%d]:%02x ", span->x, span->coverage );
        }
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



  static
  void  grays_sweep( RAS_ARG_  FT_Bitmap*  target )
  {
    TScan  x, y, cover, x_black;
    int    varea, harea, hpos;
    PCell  start, cur, limit;

    cur   = ras.cells;
    limit = cur + ras.num_cells;

    cover = 0;
    ras.span_y = -1;
    ras.num_gray_spans = 0;
    
    cover   = 0;
    x_black = 32000;

    /* fprintf( stderr, "%2d:", cur->y ); */
    
    for (;;)
    {
      int  is_black, icover;
      int  area, numv;
      
      start  = cur;
      y      = start->y;
      x      = start->x;
      icover = cover;
      varea  = cover << PIXEL_BITS;
      harea  = 0;
      hpos   = varea;
      numv   = 0;

      /* accumulate all start cells */
      for (;;)
      {
#if 0
        /* we ignore silent cells for now XXXX */
        if (!(cur->dir & dir_silent))
#endif        
        {
          switch ((cur->dir)&3)
          {
            case dir_up:
                varea += ONE_PIXEL - cur->pos;
                if (cur->pos <= 32)
                  hpos = ONE_PIXEL;
                cover++;
                numv++;
                break;
              
            case dir_down:
                varea -= ONE_PIXEL - cur->pos;
                if (cur->pos <= 32)
                  hpos = 0;
                cover--;
                numv++;
                break;
#if 0                
            case dir_left:
                harea += ONE_PIXEL - cur->pos;
                break;
                
            default:
                harea -= ONE_PIXEL - cur->pos;
                break;
#else
            default:
               ;
#endif                
          }
        }

        ++cur;
        if (cur >= limit || cur->y != start->y || cur->x != start->x)
          break;
      }

      /* nom compute the "real" area in the pixel */
      if (varea < 0) varea += ONE_PIXEL;
      if (harea < 0) harea += ONE_PIXEL;
      
      if (varea == 0)
        area = 2*harea;
        
      else if (harea == 0)
        area = 2*varea;
        
      else
        area = (varea+harea+ONE_PIXEL) >> 1;

      is_black = ( area >= 2*ONE_PIXEL );

      /* if the start cell isn't black, we may need to draw a black */
      /* segment from a previous cell..                             */
      if ( !is_black && start->x > x_black )
      {
        /* printf( stderr, " b[%d..%d]", x_black, start->x-1 ); */
        grays_hline( RAS_VAR_ y, x_black, 2*ONE_PIXEL, start->x - x_black );
      }
      
      /* if the cell is black, then record its position in "x_black" */
      if ( is_black )
      {
        if ( x_black > start->x )
          x_black = start->x;
      }
        
      /* if the cell is gray, draw a single gray pixel, then record */
      /* the next cell's position in "x_black" if "cover" is black  */
      else
      {
        x_black = 32000;
        if ( area )
        {
          /* fprintf( stderr, " [%d:%d]", start->x, varea ); */
          grays_hline( RAS_VAR_ y, start->x, area, 1 );
          if (cover)
            x_black = start->x+1;
        }
      }
      

      /* now process scanline changes/end */
      if (cur >= limit || cur->y != start->y)
      {
        if (cover && x_black < ras.max_ex)
        {
          /* fprintf( stderr, " f[%d..%d]", x_black, ras.max_ex-1 ); */
          grays_hline( RAS_VAR_ y, x_black, 2*ONE_PIXEL, ras.max_ex-x_black );
        }

        if (cur >= limit)
          break;
        
        /* fprintf( stderr, "\n%2d:", cur->y ); */
        cover   = 0;
        x_black = 32000;
      }
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
      {
        if (span->len > 1)
          fprintf( stderr, "[%d..%d]:%02x ", span->x, span->x + span->len-1, span->coverage );
        else
          fprintf( stderr, "[%d]:%02x ", span->x, span->coverage );
      }
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
      (FT_Outline_MoveTo_Func)Move_To2,
      (FT_Outline_LineTo_Func)Line_To2,
      (FT_Outline_ConicTo_Func)Conic_To2,
      (FT_Outline_CubicTo_Func)Cubic_To2
    };

    /* Set up state in the raster object */
    compute_cbox( RAS_VAR_ outline );
    
    if (ras.min_ex < 0) ras.min_ex = 0;
    if (ras.min_ey < 0) ras.min_ey = 0;
    
    if (ras.max_ex > ras.target.width) ras.max_ex = ras.target.width;
    if (ras.max_ey > ras.target.rows)  ras.max_ey = ras.target.rows;
    
    ras.min_x = UPSCALE(ras.min_ex << 6);
    ras.min_y = UPSCALE(ras.min_ey << 6);
    ras.max_x = UPSCALE(ras.max_ex << 6);
    ras.max_y = UPSCALE(ras.max_ey << 6);
    
    ras.num_cells    = 0;
    ras.contour_cell = 0;
    ras.horizontal   = 0;

    /* compute vertical intersections */
    if (FT_Outline_Decompose( outline, &interface, &ras ))
      return 1;
#if 0
    /* compute horizontal intersections */
    ras.horizontal   = 1;
    return FT_Outline_Decompose( outline, &interface, &ras );      
#else
    return 0;    
#endif    
  }






  extern
  int  grays2_raster_render( PRaster            raster,
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

    ras.outline   = *outline;
    ras.target    = *target_map;
    ras.num_cells = 0;
    ras.cursor    = ras.cells;

    if (Convert_Glyph( (PRaster)raster, outline ))
      return 1;
      
    ras.num_cells = ras.cursor - ras.cells;
#ifdef SHELL_SORT    
    shell_sort( ras.cells, ras.num_cells );
#else
    quick_sort( ras.cells, ras.num_cells );
#endif
    
#ifdef DEBUG_GRAYS    
    check_sort( ras.cells, ras.num_cells );
    dump_cells( RAS_VAR );
#endif

#if 1
    ras.render_span         = (FT_GraySpan_Func)grays_render_span;
    ras.render_span_closure = &ras;
    
    grays_sweep( (PRaster)raster, target_map );
    return 0;
#else
    return 0;
#endif
  }


  /**** RASTER OBJECT CREATION : in standalone mode, we simply use *****/
  /****                          a static object ..                *****/
#ifdef _STANDALONE_

  static
  int  grays2_raster_new( void*  memory, FT_Raster *araster )
  {
     static TRaster  the_raster;
     *araster = (FT_Raster)&the_raster;
     memset( &the_raster, sizeof(the_raster), 0 );
     return 0;
  }

  static
  void  grays2_raster_done( FT_Raster  raster )
  {
    /* nothing */
    (void)raster;
  }

#else

#include "ftobjs.h"

  static
  int  grays2_raster_new( FT_Memory  memory, FT_Raster*  araster )
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
  void grays2_raster_done( FT_Raster  raster )
  {
    FT_Memory  memory = (FT_Memory)((PRaster)raster)->memory;
    FREE( raster );
  }
  
#endif




  static
  void  grays2_raster_reset( FT_Raster    raster,
                           const char*  pool_base,
                           long         pool_size )
  {
    if (raster && pool_base && pool_size >= 4096)
      init_cells( (PRaster)raster, (char*)pool_base, pool_size );
  }


  FT_Raster_Funcs  ft_grays2_raster =
  {
    ft_glyph_format_outline,
    
    (FT_Raster_New_Func)       grays2_raster_new,
    (FT_Raster_Reset_Func)     grays2_raster_reset,
    (FT_Raster_Set_Mode_Func)  0,
    (FT_Raster_Render_Func)    grays2_raster_render,
    (FT_Raster_Done_Func)      grays2_raster_done
  };

