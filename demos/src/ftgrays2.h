#ifndef FTGRAYS2_H
#define FTGRAYS2_H

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
  
  dir_unknown = 4
  
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

  extern
  int  grays2_raster_render( TRaster*     raster,
                             FT_Outline*  outline,
                             FT_Bitmap*   target_map );

  extern
  int  grays2_raster_init( FT_Raster    raster,
                           const char*  pool_base,
                           long         pool_size );

#endif
