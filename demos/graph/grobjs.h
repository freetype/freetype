/***************************************************************************
 *
 *  grobjs.h
 *
 *    basic object classes defintions
 *
 *  Copyright 1999 - The FreeType Development Team - www.freetype.org
 *
 *
 *
 *
 ***************************************************************************/

#ifndef GROBJS_H
#define GROBJS_H

#include "graph.h"
#include "grconfig.h"
#include "grtypes.h"


  typedef struct grBiColor_
  {
    grColor   foreground;
    grColor   background;
    
    int       num_levels;
    int       max_levels;
    grColor*  levels;

  } grBiColor;



 /**********************************************************************
  *
  * Technical note : explaining how the blitter works.
  *
  *   The blitter is used to "draw" a given source bitmap into
  *   a given target bitmap.
  *
  *   The function called 'compute_clips' is used to compute clipping
  *   constraints. These lead us to compute two areas :
  *
  *   - the read area : is the rectangle, within the source bitmap,
  *                     which will be effectively "drawn" in the
  *                     target bitmap.
  *
  *   - the write area : is the rectangle, within the target bitmap,
  *                      which will effectively "receive" the pixels
  *                      from the read area
  *
  *   Note that both areas have the same dimensions, but are
  *   located in distinct surfaces.
  *
  *   These areas are computed by 'compute_clips' which is called
  *   by each blitting function.
  *
  *   Note that we use the Y-downwards convention within the blitter
  *
  **********************************************************************/

  typedef struct grBlitter_
  {
    int  width;   /* width in pixels of the areas  */
    int  height;  /* height in pixels of the areas */

    int  xread;   /* x position of start point in read area */
    int  yread;   /* y position of start point in read area */

    int  xwrite;  /* x position of start point in write area */
    int  ywrite;  /* y position of start point in write area */

    int  right_clip;   /* amount of right clip               */

    unsigned char*  read;   /* top left corner of read area in source map  */
    unsigned char*  write;  /* top left corner of write area in target map */

    int    read_line;  /* byte increment to go down one row in read area  */
    int    write_line; /* byte increment to go down one row in write area */

    grBitmap  source;  /* source bitmap descriptor */
    grBitmap  target;  /* target bitmap descriptor */

  } grBlitter;



  typedef void (*grBlitterFunc)( grBlitter*  blitter,
                                 grColor     color );

  typedef void (*grSetTitleFunc)( grSurface*   surface,
                                  const char*  title_string );

  typedef void (*grRefreshRectFunc)( grSurface*  surface,
                                     int         x,
                                     int         y,
                                     int         width,
                                     int         height );
                                    
  typedef void (*grDoneSurfaceFunc)( grSurface*  surface );

  typedef int  (*grListenEventFunc)( grSurface* surface,
                                     int        event_mode,
                                     grEvent   *event );
 


  struct grSurface_
  {
    grDevice*          device;
    grBitmap           bitmap;
    grBool             refresh;
    grBool             owner;
     
    const byte*        saturation;  /* used for gray surfaces only   */
    grBlitterFunc      blit_mono;   /* 0 by default, set by grBlit.. */

    grRefreshRectFunc  refresh_rect;
    grSetTitleFunc     set_title;
    grListenEventFunc  listen_event;
    grDoneSurfaceFunc  done;
  };



 /********************************************************************
  *
  * <Function>
  *   grAlloc
  *
  * <Description>
  *   Simple memory allocation. The returned block is always zero-ed
  *
  * <Input>
  *   size  :: size in bytes of the requested block
  *
  * <Return>
  *   the memory block address. 0 in case of error
  *
  ********************************************************************/

  extern  char*  grAlloc( long size );
  

 /********************************************************************
  *
  * <Function>
  *   grRealloc
  *
  * <Description>
  *   Simple memory re-allocation.
  *
  * <Input>
  *   block :: original memory block address
  *   size  :: new requested block size in bytes
  *
  * <Return>
  *   the memory block address. 0 in case of error
  *
  ********************************************************************/

  extern  char*  grRealloc( const char*  block, long size );


 /********************************************************************
  *
  * <Function>
  *   grFree
  *
  * <Description>
  *   Simple memory release
  *
  * <Input>
  *   block :: target block
  *
  ********************************************************************/

  extern  void   grFree( const void*  block );


  extern grDevice*  gr_devices[];
  extern int        gr_num_devices;
  extern int        gr_max_devices;

#endif /* GROBJS_H */
