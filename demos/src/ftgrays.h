#ifndef FTGRAYS_H
#define FTGRAYS_H

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
                                        

typedef struct TCell_
{
  TScan  x;
  TScan  y;
  int    area;
  int    cover;

} TCell, *PCell;


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

  FT_GraySpan gray_spans[ FT_MAX_GRAY_SPANS ];
  int         num_gray_spans;

  FT_GraySpan_Func  render_span;
  void*             render_span_closure;
  int               span_y;

} TRaster, *PRaster;

  extern
  int  grays_raster_render( TRaster*     raster,
                            FT_Outline*  outline,
                            FT_Bitmap*   target_map );

  extern
  int  grays_raster_init( FT_Raster    raster,
                          const char*  pool_base,
                          long         pool_size );

#endif
