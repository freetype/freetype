/*******************************************************************
 *
 *  ftraster.c                                                  2.0
 *
 *  The FreeType glyph rasterizer (body).
 *
 *  Copyright 1996-1998 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT. By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *  The "raster" component implements FreeType's scan-line converter,
 *  the one used to generate bitmaps and pixmaps from vectorial outlines
 *  descriptions.
 *
 *  It has been rewritten entirely for FreeType 2.0, in order to become
 *  completely independent of the rest of the library. It should now be
 *  possible to include it more easily in all kinds of libraries and
 *  applications, which do not necessarily need the font engines and
 *  API.
 *
 *  This version features :
 *
 *   - support for third-order bezier arcs
 *
 *   - improved performance of the 5-levels anti-aliasing algorithm
 *
 *   - 17-levels anti-aliasing for smoother curves, though the
 *     difference isn't always noticeable, depending on your palette
 *
 *   - an API to decompose a raster outline into a path (i.e. into
 *     a series of segments and arcs).
 *
 ******************************************************************/

#include "ftraster.h"
#include <freetype.h>  /* for FT_Outline_Decompose */

#ifndef EXPORT_FUNC
#define EXPORT_FUNC  /* nothing */
#endif


#ifndef _xxFREETYPE_

/**************************************************************************/
/*                                                                        */
/* The following defines are used when the raster is compiled as a        */
/* stand-alone object. Each of them is commented, and you're free to      */
/* toggle them to suit your needs..                                       */
/*                                                                        */

/**************************************************************************/
/*                                                                        */
/* FT_RASTER_OPTION_ANTI_ALIAS                                            */
/*                                                                        */
/*   Define this configuration macro if you want to support anti-aliasing */
/*                                                                        */
#define FT_RASTER_OPTION_ANTI_ALIAS

/**************************************************************************/
/*                                                                        */
/* FT_RASTER_OPTION_CONIC_BEZIERS                                         */
/*                                                                        */
/*   Define this configuration macro if your source outlines contain      */
/*   second-order Bezier arcs. Typically, these are TrueType outlines..   */
/*                                                                        */
#define FT_RASTER_CONIC_BEZIERS

/**************************************************************************/
/*                                                                        */
/* FT_RASTER_OPTION_CUBIC_BEZIERS                                         */
/*                                                                        */
/*   Define this configuration macro if your source outlines contain      */
/*   third-order Bezier arcs. Typically, these are Type1 outlines..       */
/*                                                                        */
#define FT_RASTER_CUBIC_BEZIERS

/**************************************************************************/
/*                                                                        */
/* FT_RASTER_ANTI_ALIAS_5                                                 */
/*                                                                        */
/*   Define this configuration macro if you want to enable the 5-grays    */
/*   anti-aliasing mode.. Ignored if FT_RASTER_OPTION_ANTI_ALIAS isn't    */
/*   defined..                                                            */
/*                                                                        */
#define FT_RASTER_ANTI_ALIAS_5

/**************************************************************************/
/*                                                                        */
/* FT_RASTER_ANTI_ALIAS_17                                                */
/*                                                                        */
/*   Define this configuration macro if you want to enable the 17-grays   */
/*   anti-aliasing mode.. Ignored if FT_RASTER_OPTION_ANTI_ALIAS isn't    */
/*   defined..                                                            */
/*                                                                        */
#define FT_RASTER_ANTI_ALIAS_17

/**************************************************************************/
/*                                                                        */
/* FT_RASTER_LITTLE_ENDIAN                                                */
/* FT_RASTER_BIG_ENDIAN                                                   */
/*                                                                        */
/*   The default anti-alias routines are processor-independent, but slow. */
/*   Define one of these macros to suit your own system, and enjoy        */
/*   greatly improved rendering speed                                     */
/*                                                                        */

/* #define FT_RASTER_LITTLE_ENDIAN */
/* #define FT_RASTER_BIG_ENDIAN    */

#else  /* _FREETYPE_ */

/**************************************************************************/
/*                                                                        */
/* The following defines are used when the raster is compiled within      */
/* the FreeType base layer. Don't change these unless you really know     */
/* what you're doing..                                                    */
/*                                                                        */

#ifdef FT_CONFIG_OPTION_ANTI_ALIAS
#define FT_RASTER_OPTION_ANTI_ALIAS
#endif

#define FT_RASTER_CONIC_BEZIERS
#define FT_RASTER_CUBIC_BEZIERS

#define FT_RASTER_ANTI_ALIAS_5
#undef  FT_RASTER_ANTI_ALIAS_17

#ifdef FT_CONFIG_OPTION_LITTLE_ENDIAN
#define FT_RASTER_LITTLE_ENDIAN
#endif

#ifdef FT_CONFIG_OPTION_BIG_ENDIAN
#define FT_RASTER_BIG_ENDIAN
#endif

#endif /* _FREETYPE_ */


/* FT_RASTER_ANY_ENDIAN indicates that no endianess was defined */
/* through one of the configuration macros                      */
/*                                                              */
#if !defined(FT_RASTER_LITTLE_ENDIAN) && !defined(FT_RASTER_BIG_ENDIAN)
#define FT_RASTER_ANY_ENDIAN
#endif


/* The rasterizer is a very general purpose component, please leave */
/* the following redefinitions there (you never know your target    */
/* environment).                                                    */

#ifndef TRUE
#define TRUE   1
#endif

#ifndef FALSE
#define FALSE  0
#endif

#ifndef NULL
#define NULL  (void*)0
#endif


#undef  FAILURE
#define FAILURE  TRUE

#undef  SUCCESS
#define SUCCESS  FALSE


/* Please don't touch the following macros. Their importance is historical */
/* to FreeType, but they have some nice effects, like getting rid of all   */
/* '->' symbols when accessing the raster object.. (replacing them with    */
/* a simple '.' )                                                          */

/* used in function signatures to define the _first_ argument */
#define  RAS_ARGS  FT_Raster  raster,
#define  RAS_ARG   FT_Raster  raster

/* used to call a function within this component, first parameter */
#define  RAS_VARS  raster,
#define  RAS_VAR   raster

/* used to access the current raster object, with a '.' instead of a '->' */
#define  ras       (*raster)

#define  UNUSED_RASTER  (void)raster;

/* For anti-aliasing modes, we use a 2 or 4 lines intermediate bitmap which */
/* is filtered repeatedly to render each pixmap row. The following macro    */
/* defines this buffer's size in bytes (which is part of raster objects)    */
#define ANTI_ALIAS_BUFFER_SIZE   2048


/* Error codes returned by the scan-line converter/raster */

#define ErrRaster_Ok                      0
#define ErrRaster_Uninitialised_Object    1
#define ErrRaster_Overflow                2
#define ErrRaster_Negative_Height         3
#define ErrRaster_Invalid_Outline         4
#define ErrRaster_Invalid_Map             5
#define ErrRaster_AntiAlias_Unsupported   6
#define ErrRaster_Invalid_Pool            7
#define ErrRaster_Unimplemented           8
#define ErrRaster_Bad_Palette_Count       9


#define SET_High_Precision(p)  Set_High_Precision( RAS_VARS p )

/* Fast MulDiv, as 'b' is always < 64, don't use intermediate precision */
#define FMulDiv( a, b, c )  ( (a) * (b) / (c) )


/* Define DEBUG_RASTER if you want to generate a debug version of the  */
/* rasterizer.  This will progressively draw the glyphs while all the  */
/* computation are done directly on the graphics screen (the glyphs    */
/* will be inverted).                                                  */

/* Note that DEBUG_RASTER should only be used for debugging with b/w   */
/* rendering, not with gray levels.                                    */

/* The definition of DEBUG_RASTER should appear in the file            */
/* "ftconfig.h".                                                       */

#ifdef DEBUG_RASTER
  extern char*  vio;  /* A pointer to VRAM or display buffer */
#endif


#define MaxBezier  32   /* The maximum number of stacked Bezier curves. */
                        /* Setting this constant to more than 32 is a   */
                        /* pure waste of space.                         */

#define Pixel_Bits  6   /* fractional bits of *input* coordinates   */
                        /* We always use 26.6, but hackers are free */
                        /* to experiment with different values      */


  /* The type of the pixel coordinates used within the render pool during   */
  /* scan-line conversion. We use longs to store either 26.6 or 22.10 fixed */
  /* float values, depending on the "precision" we want to use (resp. low   */
  /* or high). These are ideals in order to subdivise bezier arcs in halves */
  /* though simple additions and shifts.                                    */

  typedef  long   TPos, *PPos;


  /* The type of a scanline position/coordinate within a map */
  typedef  int    TScan, *PScan;




  /* boolean type */
  typedef  char   TBool;
  
  /* unsigned char type and array */
  typedef  unsigned char   TByte, *PByte;

  /* unsigned short type and array */
  typedef  unsigned short  UShort, *PUShort;

  /* flow */
  enum _TFlow
  {
    FT_Flow_Error = 0,
    FT_Flow_Down  = -1,
    FT_Flow_Up    = 1
  };


  /* states/directions of each line, arc and profile */
  enum  _TDirection
  {
    Unknown,
    Ascending,
    Descending,
    Flat
  };
  typedef enum _TDirection  TDirection;


  struct  _TProfile;
  typedef struct _TProfile  TProfile;
  typedef TProfile*         PProfile;

  struct  _TProfile
  {                                                                     
    TPos      X;           /* current coordinate during sweep          */
    PProfile  link;        /* link to next profile - various purpose   */
    PPos      offset;      /* start of profile's data in render pool   */
    int       flow;        /* Profile orientation: Asc/Descending      */
    TScan     height;      /* profile's height in scanlines            */
    TScan     start;       /* profile's starting scanline              */

    TScan     countL;      /* number of lines to step before this      */
                           /* profile becomes drawable                 */

    PProfile  next;        /* next profile in same contour, used       */
                           /* during drop-out control                  */
  };

  typedef PProfile   TProfileList;
  typedef PProfile*  PProfileList;


  /* Simple record used to implement a stack of bands, required */
  /* by the sub-banding mechanism                               */
  /*                                                            */
  struct  _TBand
  {
    TScan  y_min;   /* band's minimum */
    TScan  y_max;   /* band's maximum */
  };

  typedef struct _TBand  TBand;


/* The size in _TPos_ of a profile record in the render pool */
#define AlignProfileSize  ((sizeof(TProfile)+sizeof(TPos)-1) / sizeof(TPos))


  /* prototypes used for sweep function dispatch */
  typedef void  Function_Sweep_Init( RAS_ARGS int*  min, int*  max );

  typedef void  Function_Sweep_Span( RAS_ARGS  TScan  y,
                                               TPos   x1,
                                               TPos   x2 );

  typedef int   Function_Test_Pixel( RAS_ARGS TScan  y,
                                              int    x );
                                              
  typedef void  Function_Set_Pixel( RAS_ARGS  TScan  y,
                                              int    x,
                                              int    color );

  typedef void  Function_Sweep_Step( RAS_ARG );


/* compute lowest integer coordinate below a given x */
#define FLOOR( x )    ( (x) & ras.precision_mask )

/* compute highest integer coordinate above a given x */
#define CEILING( x )  ( ((x) + ras.precision - 1) & ras.precision_mask )

/* get integer coordinate of a given 26.6 or 22.10 'x' coordinate - no round */
#define TRUNC( x )    ( (signed long)(x) >> ras.precision_bits )

/* get the fractional part of a given coordinate */
#define FRAC( x )     ( (x) & (ras.precision - 1) )

/* scale an 'input coordinate' (as found in FT_Outline structures) into */
/* a 'work coordinate', which depends on current resolution and render  */
/* mode..                                                               */
#define SCALED( x )   ( ((x) << ras.scale_shift) - ras.precision_half )


/* DEBUG_PSET is used to plot a single pixel in VRam during debug mode */
#ifdef DEBUG_RASTER
#define DEBUG_PSET  Pset()
#else
#define DEBUG_PSET  
#endif

  struct  _TPoint
  {
    TPos  x, y;
  };
  typedef struct _TPoint  TPoint;


  /* Note that I have moved the location of some fields in the */
  /* structure to ensure that the most used variables are used */
  /* at the top.  Thus, their offset can be coded with less    */
  /* opcodes, and it results in a smaller executable.          */

  struct  FT_RasterRec_
  {
    PPos      cursor;              /* Current cursor in render pool  */

    PPos      pool;                /* The render pool base address   */
    PPos      pool_size;           /* The render pool's size         */
    PPos      pool_limit;          /* Limit of profiles zone in pool */

    int       bit_width;            /* target bitmap width  */
    PByte     bit_buffer;           /* target bitmap buffer */
    PByte     pix_buffer;           /* target pixmap buffer */

    TPoint    last;
    long      minY, maxY;

    int       error;

    int       precision_bits;       /* precision related variables */
    int       precision;
    int       precision_half;
    long      precision_mask;
    int       precision_shift;
    int       precision_step;
    int       precision_jitter;

    FT_Outline*  outline;
    
    int       n_points;             /* number of points in current glyph   */
    int       n_contours;           /* number of contours in current glyph */
    int       n_turns;              /* number of Y-turns in outline        */

    TPoint*   arc;                  /* current Bezier arc pointer */

    int       num_profs;            /* current number of profiles */

    TBool      fresh;                /* signals a fresh new profile which */ 
                                    /* 'start' field must be completed   */
    TBool      joint;                /* signals that the last arc ended   */
                                    /* exactly on a scanline.  Allows    */
                                    /* removal of doublets               */
    PProfile  cur_prof;             /* current profile                   */
    PProfile  start_prof;           /* head of linked list of profiles   */
    PProfile  first_prof;           /* contour's first profile in case   */
                                    /* of impact                         */
    TDirection state;               /* rendering state */
   
    FT_Bitmap  target;              /* description of target bit/pixmap */

    int       trace_bit;            /* current offset in target bitmap */
    int       trace_pix;            /* current offset in target pixmap */

    int       trace_incr;           /* sweep's increment in target bitmap */

    int       gray_min_x;           /* current min x during gray rendering */
    int       gray_max_x;           /* current max x during gray rendering */

    /* dispatch variables */

    Function_Sweep_Init*  Proc_Sweep_Init;
    Function_Sweep_Span*  Proc_Sweep_Span;
    Function_Sweep_Step*  Proc_Sweep_Step;
    Function_Test_Pixel*  Proc_Test_Pixel;
    Function_Set_Pixel*   Proc_Set_Pixel;

    int       scale_shift;      /* == 0  for bitmaps           */
                                /* == 1  for 5-levels pixmaps  */
                                /* == 2  for 17-levels pixmaps */

    TByte      dropout_mode;     /* current drop_out control method */

    TBool      second_pass;      /* indicates wether a horizontal pass      */
                                /* should be performed to control drop-out */
                                /* accurately when calling Render_Glyph.   */
                                /* Note that there is no horizontal pass   */
                                /* during gray rendering.                  */

    TBool      flipped;          /* this flag is set during the rendering to */
                                /* indicate the second pass..               */
                                
    TBand     band_stack[16];       /* band stack used for sub-banding */
    int       band_top;             /* band stack top                  */

    TPoint    arcs[ 2*MaxBezier+1 ];      /* The Bezier stack */

    void*     memory;
   
#if defined(FT_RASTER_OPTION_ANTI_ALIAS)

    long      grays[20];        /* Palette of gray levels used for render */

    int       gray_width;       /* length in bytes of the onochrome        */
                                /* intermediate scanline of gray_lines.    */
                                /* Each gray pixel takes 2 or 4 bits long  */

                        /* The gray_lines must hold 2 lines, thus with size */
                        /* in bytes of at least 'gray_width*2'              */

    int       grays_count;      /* number of entries in the palette */

    char      gray_lines[ANTI_ALIAS_BUFFER_SIZE];       
                                /* Intermediate table used to render the   */
                                /* graylevels pixmaps.                     */
                                /* gray_lines is a buffer holding 2 or 4   */
                                /* monochrome scanlines                    */

    int       count_table[256];     /* Look-up table used to quickly count */
                                    /* set bits in a gray 2x2 cell         */
#endif
  };



#ifdef DEBUG_RASTER

  /************************************************/
  /*                                              */
  /* Pset:                                        */
  /*                                              */
  /*  Used for debugging only.  Plots a point     */
  /*  in VRAM during rendering (not afterwards).  */
  /*                                              */
  /* NOTE:  This procedure relies on the value    */
  /*        of cProfile->start, which may not     */
  /*        be set when Pset is called sometimes. */
  /*        This will usually result in a dot     */
  /*        plotted on the first screen scanline  */
  /*        (far away its original position).     */
  /*                                              */
  /*        This "bug" reflects nothing wrong     */
  /*        in the current implementation, and    */
  /*        the bitmap is rendered correctly,     */
  /*        so don't panic if you see 'flying'    */
  /*        dots in debugging mode.               */
  /*                                              */
  /*  - David                                     */
  /*                                              */
  /************************************************/

  static void  Pset( RAS_ARG )
  {
    long  o;
    long  x;

    x = ras.cursor[-1];

    switch ( ras.cur_prof->flow )
    {
    case FT_Flow_Up:
      o = Vio_ScanLineWidth *
         ( ras.cursor - ras.cur_prof->offset + ras.cur_prof->start ) +
         ( x / (ras.precision*8) );
      break;

    case FT_Flow_Down:
      o = Vio_ScanLineWidth *
         ( ras.cur_prof->start - ras.cursor + ras.cur_prof->offset ) +
         ( x / (ras.precision*8) );
      break;
    }

    if ( o > 0 )
      Vio[o] |= (unsigned)0x80 >> ( (x/ras.precision) & 7 );
  }


  static void  Clear_Band( RAS_ARGS Int  y1, Int  y2 )
  {
    MEM_Set( Vio + y1*Vio_ScanLineWidth, (y2-y1+1)*Vio_ScanLineWidth, 0 );
  }

#endif /* DEBUG_RASTER */


/************************************************************************/
/*                                                                      */
/* <Function>   Set_High_Precision                                      */
/*                                                                      */
/* <Description> Sets precision variables according to param flag.      */
/*                                                                      */
/* <Input>       High ::  set to True for high precision (typically for */
/*                       ppem < 18), false otherwise.                   */
/*                                                                      */
/************************************************************************/

  static 
  void  Set_High_Precision( RAS_ARGS int  High )
  {
    if ( High )
    {
      ras.precision_bits   = 10;
      ras.precision_step   = 128;
      ras.precision_jitter = 24;
    }
    else
    {
      ras.precision_bits   = 6;
      ras.precision_step   = 32;
      ras.precision_jitter = 2;
    }

    ras.precision       = 1 << ras.precision_bits;
    ras.precision_half  = ras.precision / 2;
    ras.precision_shift = ras.precision_bits - Pixel_Bits;
    ras.precision_mask  = -ras.precision;
  }


/**************************************************************************/
/*                                                                        */
/* <Function>   New_Profile                                               */
/*                                                                        */
/* <Description> Creates a new Profile in the render pool.                */
/*                                                                        */
/* <Input>                                                                */
/*    aState  ::  state/orientation of the new Profile                    */
/*                                                                        */
/* <Return>                                                               */
/*    SUCCESS or FAILURE                                                  */
/*                                                                        */
/**************************************************************************/

  static 
  TBool  New_Profile( RAS_ARGS TDirection  direction )
  {
    if ( ras.start_prof == NULL )
    {
      ras.cur_prof   = (PProfile)ras.cursor;
      ras.start_prof = ras.cur_prof;
      ras.cursor    += AlignProfileSize;
    }

    if ( ras.cursor >= ras.pool_limit )
    {
      ras.error = ErrRaster_Overflow;
      return FAILURE;
    }

    switch ( direction )
    {
    case Ascending:
      ras.cur_prof->flow = FT_Flow_Up;
      break;

    case Descending:
      ras.cur_prof->flow = FT_Flow_Down;
      break;

    default:
      ras.error = ErrRaster_Invalid_Map;
      return FAILURE;
    }

    {
      PProfile  cur = ras.cur_prof;
      
      cur->start  = 0;
      cur->height = 0;
      cur->offset = ras.cursor;
      cur->link   = (PProfile)0;
      cur->next   = (PProfile)0;
    }

    if ( ras.first_prof == NULL )
      ras.first_prof = ras.cur_prof;

    ras.state  = direction;
    ras.fresh  = TRUE;
    ras.joint  = FALSE;

    return SUCCESS;
  }


/****************************************************************************/
/*                                                                          */
/* <Function> End_Profile                                                   */
/*                                                                          */
/* <Description> Finalizes the current Profile.                             */
/*                                                                          */
/* <Return>                                                                 */
/*    SUCCESS or FAILURE                                                    */
/*                                                                          */
/****************************************************************************/

  static 
  TBool  End_Profile( RAS_ARG )
  {
    int  h;

    h = ras.cursor - ras.cur_prof->offset;

    if ( h < 0 )
    {
      ras.error = ErrRaster_Negative_Height;
      return FAILURE;
    }

    if ( h > 0 )
    {
      PProfile  old, new;
      
      old          = ras.cur_prof;
      old->height  = h;
      ras.cur_prof = new = (PProfile)ras.cursor;

      ras.cursor  += AlignProfileSize;

      new->height  = 0;
      new->offset  = ras.cursor;
      old->next    = new;

      ras.num_profs++;
    }

    if ( ras.cursor >= ras.pool_limit )
    {
      ras.error = ErrRaster_Overflow;
      return FAILURE;
    }

    ras.joint = FALSE;

    return SUCCESS;
  }


/****************************************************************************/
/*                                                                          */
/* <Function>   Insert_Y_Turn                                               */
/*                                                                          */
/* <Description>                                                            */
/*     Insert a salient into the sorted list placed on top                  */
/*     of the render pool                                                   */
/*                                                                          */
/* <Input>                                                                  */
/*     y   ::  new scanline position                                        */
/*                                                                          */
/****************************************************************************/

  static  
  TBool  Insert_Y_Turn( RAS_ARGS  TScan  y )
  {
    PPos   y_turns;
    TScan  y2;
    int    n;

    n       = ras.n_turns-1;
    y_turns = ras.pool_size - ras.n_turns;

    /* look for first y value that is <= */
    while ( n >= 0 && y < y_turns[n] )
      n--;

    /* if it is <, simply insert it, ignore if == */
    if ( n >= 0 && y > y_turns[n] )
      while (n >= 0)
      {
        y2 = y_turns[n];
        y_turns[n] = y;
        y = y2;
        n--;
      }

    if (n < 0)
    {
      ras.pool_limit--;
      ras.n_turns++;
      ras.pool_size[-ras.n_turns ] = y;

      if ( ras.pool_limit <= ras.cursor )
      {
        ras.error = ErrRaster_Overflow;
        return FAILURE;
      }
    }
    return SUCCESS;
  }


/****************************************************************************/
/*                                                                          */
/* <Function>   Finalize_Profile_Table                                      */
/*                                                                          */
/* <Description>                                                            */
/*     Adjusts all links in the Profiles list.                              */
/*                                                                          */
/****************************************************************************/

  static 
  TBool  Finalize_Profile_Table( RAS_ARG )
  {
    int       n, bottom, top;
    PProfile  p;


    n = ras.num_profs;

    if ( n > 1 )
    {
      p = ras.start_prof;
      while ( n > 0 )
      {
        if (n > 1)
          p->link = (PProfile)( p->offset + p->height );
        else
          p->link = NULL;

        switch (p->flow)
        {
          case FT_Flow_Down:
            bottom     = p->start - p->height+1;
            top        = p->start;
            p->start   = bottom;
            p->offset += p->height-1;
            break;

          case FT_Flow_Up:
          default:
            bottom = p->start;
            top    = p->start + p->height-1;
        }

        if ( Insert_Y_Turn( RAS_VARS  bottom ) ||
             Insert_Y_Turn( RAS_VARS  top+1 )  )
          return FAILURE;

        p = p->link;
        n--;
      }
    }
    else
      ras.start_prof = NULL;
      
    return SUCCESS;
  }



/****************************************************************************/
/*                                                                          */
/* <Function>   Line_Up                                                     */
/*                                                                          */
/* <Description>                                                            */
/*     Computes the scan-line intersections of an ascending line segment    */
/*     and stores them in the render pool.                                  */
/*                                                                          */
/* <Input>                                                                  */
/*     x1  :: start x coordinate                                            */
/*     y1  :: start y coordinates                                           */
/*     x2  :: end x coordinate                                              */
/*     y2  :: end y coordinate                                              */
/*   miny  :: minimum vertical grid coordinate                              */
/*   maxy  :: maximum vertical grid coordinate                              */
/*                                                                          */
/* <Return>                                                                 */
/*     SUCCESS or FAILURE.                                                  */
/*                                                                          */
/****************************************************************************/

  static 
  TBool  Line_Up( RAS_ARGS TPos  x1,   TPos  y1, 
                          TPos  x2,   TPos  y2,
                          TPos  miny, TPos  maxy )
  {
    TPos   Dx, Dy;
    int    e1, e2, f1, f2, size;
    TPos   Ix, Rx, Ax;

    PPos  top;


    Dx = x2 - x1;
    Dy = y2 - y1;

    if ( Dy <= 0 || y2 < miny || y1 > maxy )
      return SUCCESS;

    if ( y1 < miny )
    {
      x1 += FMulDiv( Dx, miny - y1, Dy );
      e1  = TRUNC( miny );
      f1  = 0;
    }
    else
    {
      e1 = TRUNC( y1 );
      f1 = FRAC( y1 );
    }

    if ( y2 > maxy )
    {
      /* x2 += FMulDiv( Dx, maxy-y2, Dy );  UNNECESSARY */
      e2  = TRUNC( maxy );
      f2  = 0;
    }
    else
    {
      e2 = TRUNC( y2 );
      f2 = FRAC( y2 );
    }

    if ( f1 > 0 )
    {
      if ( e1 == e2 ) return SUCCESS;
      else
      {
        x1 += FMulDiv( Dx, ras.precision - f1, Dy );
        e1 += 1;
      }
    }
    else
      if ( ras.joint )
      {
        ras.cursor--;
        ras.joint = FALSE;
      }

    ras.joint = ( f2 == 0 );

    if ( ras.fresh )
    {
      ras.cur_prof->start = e1;
      ras.fresh           = FALSE;
    }

    size = e2 - e1 + 1;
    if ( ras.cursor + size >= ras.pool_limit )
    {
      ras.error = ErrRaster_Overflow;
      return FAILURE;
    }

    if ( Dx > 0 )
    {
      Ix = (ras.precision*Dx) / Dy;
      Rx = (ras.precision*Dx) % Dy;
      Dx = 1;
    }
    else
    {
      Ix = -( (ras.precision*-Dx) / Dy );
      Rx =    (ras.precision*-Dx) % Dy;
      Dx = -1;
    }

    Ax  = -Dy;
    top = ras.cursor;

    while ( size > 0 ) 
    {
      *top++ = x1;

      DEBUG_PSET;

      x1 += Ix;
      Ax += Rx;
      if ( Ax >= 0 )
      {
        Ax -= Dy;
        x1 += Dx;
      }
      size--;
    }

    ras.cursor = top;
    return SUCCESS;
  }


/****************************************************************************/
/*                                                                          */
/* <Function>   Line_Down                                                   */
/*                                                                          */
/* <Description>                                                            */
/*     Computes the scan-line intersections of a descending line segment    */
/*     and stores them in the render pool.                                  */
/*                                                                          */
/* <Input>                                                                  */
/*     x1  :: start x coordinate                                            */
/*     y1  :: start y coordinates                                           */
/*     x2  :: end x coordinate                                              */
/*     y2  :: end y coordinate                                              */
/*   miny  :: minimum vertical grid coordinate                              */
/*   maxy  :: maximum vertical grid coordinate                              */
/*                                                                          */
/* <Return>                                                                 */
/*     SUCCESS or FAILURE.                                                  */
/*                                                                          */
/****************************************************************************/

  static 
  TBool  Line_Down( RAS_ARGS TPos  x1,   TPos  y1, 
                            TPos  x2,   TPos  y2,
                            TPos  miny, TPos  maxy )
  {
    TBool result, fresh;

    fresh  = ras.fresh;
    result = Line_Up( RAS_VARS x1, -y1, x2, -y2, -maxy, -miny );

    if ( fresh && !ras.fresh ) 
      ras.cur_prof->start = -ras.cur_prof->start;

    return result;
  }




#ifdef FT_RASTER_CONIC_BEZIERS
/****************************************************************************/
/*                                                                          */
/* <Function>   Split_Conic                                                 */
/*                                                                          */
/* <Description>                                                            */
/*     Subdivides one second-order Bezier arc into two joint sub-arcs in    */
/*     the Bezier stack.                                                    */
/*                                                                          */
/* <Note>                                                                   */
/*     This routine is the "beef" of the component. It is one of _the_      */
/*     inner loops that should be optimized like hell to get the best       */
/*     performance..                                                        */
/*                                                                          */
/****************************************************************************/

  static 
  void  Split_Conic( TPoint*  base )
  {
    TPos   a, b;

    base[4].x = base[2].x;
    b = base[1].x;
    a = base[3].x = ( base[2].x + b )/2;
    b = base[1].x = ( base[0].x + b )/2;
    base[2].x = (a+b)/2;

    base[4].y = base[2].y;
    b = base[1].y;
    a = base[3].y = ( base[2].y + b )/2;
    b = base[1].y = ( base[0].y + b )/2;
    base[2].y = (a+b)/2;
  }
#endif

#ifdef FT_RASTER_CUBIC_BEZIERS
/****************************************************************************/
/*                                                                          */
/* <Function>   Split_Cubic                                                 */
/*                                                                          */
/* <Description>                                                            */
/*     Subdivides a third-order Bezier arc into two joint sub-arcs in the   */
/*     Bezier stack.                                                        */
/*                                                                          */
/* <Note>                                                                   */
/*     This routine is the "beef" of the component. It is one of _the_      */
/*     inner loops that should be optimized like hell to get the best       */
/*     performance..                                                        */
/*                                                                          */
/****************************************************************************/

  static 
  void  Split_Cubic( TPoint*  base )
  {
    TPos   a, b, c, d;

    base[6].x = base[3].x;
    c = base[1].x;
    d = base[2].x;
    base[1].x = a = ( base[0].x + c )/2;
    base[5].x = b = ( base[3].x + d )/2;
    c = (c+d)/2;
    base[2].x = a = (a+c)/2;
    base[4].x = b = (b+c)/2;
    base[3].x = (a+b)/2;
    
    base[6].y = base[3].y;
    c = base[1].y;
    d = base[2].y;
    base[1].y = a = ( base[0].y + c )/2;
    base[5].y = b = ( base[3].y + d )/2;
    c = (c+d)/2;
    base[2].y = a = (a+c)/2;
    base[4].y = b = (b+c)/2;
    base[3].y = (a+b)/2;
  }
#endif




#ifdef FT_RASTER_CONIC_BEZIERS
/****************************************************************************/
/*                                                                          */
/* <Function>   Conic_Up                                                    */
/*                                                                          */
/* <Description>                                                            */
/*     Computes the scan-line intersections of an ascending second-order    */
/*     Bezier arc and stores them in the render pool. The arc is taken      */
/*     from the top of the stack..                                          */
/*                                                                          */
/* <Input>                                                                  */
/*     miny     :: minimum vertical grid coordinate                         */
/*     maxy     :: maximum vertical grid coordinate                         */
/*                                                                          */
/* <Return>                                                                 */
/*     SUCCESS or FAILURE                                                   */
/*                                                                          */
/****************************************************************************/

  static 
  TBool  Conic_Up( RAS_ARGS TPos  miny, TPos  maxy )
  {
    TPos  y1, y2, e, e2, e0;
    int   f1;

    TPoint*  arc;
    TPoint*  start_arc;

    PPos top;


    arc = ras.arc;
    y1  = arc[2].y;
    y2  = arc[0].y;
    top = ras.cursor; 

    if ( y2 < miny || y1 > maxy )
      goto Fin;
    
    e2 = FLOOR( y2 );

    if ( e2 > maxy )
      e2 = maxy;

    e0 = miny;

    if ( y1 < miny )
      e = miny;
    else
    {
      e  = CEILING( y1 );
      f1 = FRAC( y1 );
      e0 = e;

      if ( f1 == 0 )
      {
        if ( ras.joint )
        {
          top--;
          ras.joint = FALSE;
        }

        *top++ = arc[2].x;

        DEBUG_PSET;

        e += ras.precision;
      }
    }

    if ( ras.fresh )
    {
      ras.cur_prof->start = TRUNC( e0 );
      ras.fresh = FALSE;
    }

    if ( e2 < e )
      goto Fin;

    if ( ( top+TRUNC(e2-e)+1 ) >= ras.pool_limit )
    {
      ras.cursor = top;
      ras.error  = ErrRaster_Overflow;
      return FAILURE;
    }

    start_arc = arc;

    while ( arc >= start_arc && e <= e2 )
    {
      ras.joint = FALSE;

      y2 = arc[0].y;

      if ( y2 > e )
      {
        y1 = arc[2].y;
        if ( y2 - y1 >= ras.precision_step )
        {
          Split_Conic( arc );
          arc += 2;
        }
        else
        {
          *top++ = arc[2].x + 
                   FMulDiv( arc[0].x-arc[2].x, 
                            e  - y1, 
                            y2 - y1 );
          DEBUG_PSET;
            
          arc -= 2;
          e   += ras.precision;
        }
      }
      else
      {
        if ( y2 == e )
        {
          ras.joint  = TRUE;
          *top++     = arc[0].x;
        
          DEBUG_PSET;
        
          e += ras.precision;
        }
        arc -= 2;
      }
    }

  Fin:
    ras.cursor = top;
    ras.arc   -= 2;
    return SUCCESS;
  }


/****************************************************************************/
/*                                                                          */
/* <Function>   Conic_Down                                                  */
/*                                                                          */
/* <Description>                                                            */
/*     Computes the scan-line intersections of a descending second-order    */
/*     Bezier arc and stores them in the render pool. The arc is taken      */
/*     from the top of the stack..                                          */
/*                                                                          */
/* <Input>                                                                  */
/*     miny     :: minimum vertical grid coordinate                         */
/*     maxy     :: maximum vertical grid coordinate                         */
/*                                                                          */
/* <Return>                                                                 */
/*     SUCCESS or FAILURE                                                   */
/*                                                                          */
/****************************************************************************/

  static 
  TBool  Conic_Down( RAS_ARGS TPos  miny, TPos  maxy )
  {
    TPoint*  arc = ras.arc;
    TBool     result, fresh;


    arc[0].y = -arc[0].y;
    arc[1].y = -arc[1].y; 
    arc[2].y = -arc[2].y;

    fresh = ras.fresh;

    result = Conic_Up( RAS_VARS -maxy, -miny );

    if ( fresh && !ras.fresh )
      ras.cur_prof->start = -ras.cur_prof->start;

    arc[0].y = -arc[0].y;
    return result;
  }

#endif /* FT_RASTER_CONIC_BEZIERS */



#ifdef FT_RASTER_CUBIC_BEZIERS
/****************************************************************************/
/*                                                                          */
/* <Function>   Cubic_Up                                                    */
/*                                                                          */
/* <Description>                                                            */
/*     Computes the scan-line intersections of an ascending third-order     */
/*     bezier arc and stores them in the render pool                        */
/*                                                                          */
/* <Input>                                                                  */
/*     miny     :: minimum vertical grid coordinate                         */
/*     maxy     :: maximum vertical grid coordinate                         */
/*                                                                          */
/* <Return>                                                                 */
/*     SUCCESS or FAILURE                                                   */
/*                                                                          */
/****************************************************************************/
  static
  TBool  Cubic_Up( RAS_ARGS TPos  miny, TPos  maxy )
  {
    TPos  y1, y2, e, e2, e0;
    int   f1;

    TPoint*  arc;
    TPoint*  start_arc;

    TPos*    top;


    arc = ras.arc;
    y1  = arc[3].y;
    y2  = arc[0].y;
    top = ras.cursor; 

    if ( y2 < miny || y1 > maxy )
      goto Fin;
    
    e2 = FLOOR( y2 );

    if ( e2 > maxy )
      e2 = maxy;

    e0 = miny;

    if ( y1 < miny )
      e = miny;
    else
    {
      e  = CEILING( y1 );
      f1 = FRAC( y1 );
      e0 = e;

      if ( f1 == 0 )
      {
        if ( ras.joint )
        {
          top--;
          ras.joint = FALSE;
        }

        *top++ = arc[3].x;

        DEBUG_PSET;

        e += ras.precision;
      }
    }

    if ( ras.fresh )
    {
      ras.cur_prof->start = TRUNC( e0 );
      ras.fresh = FALSE;
    }

    if ( e2 < e )
      goto Fin;

    if ( ( top+TRUNC(e2-e)+1 ) >= ras.pool_limit )
    {
      ras.cursor = top;
      ras.error  = ErrRaster_Overflow;
      return FAILURE;
    }

    start_arc = arc;

    while ( arc >= start_arc && e <= e2 )
    {
      ras.joint = FALSE;

      y2 = arc[0].y;

      if ( y2 > e )
      {
        y1 = arc[3].y;
        if ( y2 - y1 >= ras.precision_step )
        {
          Split_Cubic( arc );
          arc += 3;
        }
        else
        {
          *top++ = arc[3].x + 
                   FMulDiv( arc[0].x-arc[3].x, 
                            e  - y1, 
                            y2 - y1 );
          DEBUG_PSET;
            
          arc -= 3;
          e   += ras.precision;
        }
      }
      else
      {
        if ( y2 == e )
        {
          ras.joint  = TRUE;
          *top++     = arc[0].x;
        
          DEBUG_PSET;
        
          e += ras.precision;
        }
        arc -= 3;
      }
    }

  Fin:
    ras.cursor = top;
    ras.arc   -= 3;
    return SUCCESS;
  }


/****************************************************************************/
/*                                                                          */
/* <Function>   Cubic_Down                                                  */
/*                                                                          */
/* <Description>                                                            */
/*     Computes the scan-line intersections of a descending third-order     */
/*     bezier arc and stores them in the render pool                        */
/*                                                                          */
/* <Input>                                                                  */
/*     miny     :: minimum vertical grid coordinate                         */
/*     maxy     :: maximum vertical grid coordinate                         */
/*                                                                          */
/* <Return>                                                                 */
/*     SUCCESS or FAILURE                                                   */
/*                                                                          */
/****************************************************************************/
  static 
  TBool  Cubic_Down( RAS_ARGS TPos  miny, TPos  maxy )
  {
    TPoint*  arc = ras.arc;
    TBool     result, fresh;


    arc[0].y = -arc[0].y;
    arc[1].y = -arc[1].y; 
    arc[2].y = -arc[2].y;
    arc[3].y = -arc[3].y;

    fresh = ras.fresh;

    result = Cubic_Up( RAS_VARS -maxy, -miny );

    if ( fresh && !ras.fresh )
      ras.cur_prof->start = -ras.cur_prof->start;

    arc[0].y = -arc[0].y;
    return result;
  }

#endif /* FT_RASTER_CUBIC_BEZIERS */





 /* A function type describing the functions used to split bezier arcs */
  typedef  void  (*TSplitter)( TPoint*  base );

#ifdef FT_DYNAMIC_BEZIER_STEPS
  static
  TPos  Dynamic_Bezier_Threshold( RAS_ARGS int degree, TPoint*  arc )
  {
    TPos    min_x,  max_x,  min_y, max_y, A, B;
    TPos    wide_x, wide_y, threshold;
    TPoint* cur   = arc;
    TPoint* limit = cur + degree;

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
#endif

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Bezier_Up                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Computes the scan-line intersections of an ascending second-order  */
  /*    Bezier arc and stores them in the render pool.  The arc is taken   */
  /*    from the top of the stack.                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    miny :: The minimum vertical grid coordinate.                      */
  /*    maxy :: The maximum vertical grid coordinate.                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS or FAILURE.                                                */
  /*                                                                       */
  static
  TBool    Bezier_Up( RAS_ARGS int        degree,
                               TSplitter  splitter,
                               TPos       miny,
                               TPos       maxy )
  {
    TPos  y1, y2, e, e2, e0, threshold;
    int   f1;

    TPoint*  arc;
    TPoint*  start_arc;

    PPos top;


    arc = ras.arc;
    y1  = arc[degree].y;
    y2  = arc[0].y;
    top = ras.cursor;

    if ( y2 < miny || y1 > maxy )
      goto Fin;

    e2 = FLOOR( y2 );  /* integer end y */

    if ( e2 > maxy )
      e2 = maxy;

    e0 = miny;

    if ( y1 < miny )
    {
      e = e0;        /* integer start y == current scanline */
    }
    else
    {
      e  = CEILING( y1 );   /* integer start y == current scanline */
      f1 = FRAC( y1 );      /* fractional shift of start y         */
      e0 = e;               /* first integer scanline to be pushed */

      if ( f1 == 0 )        /* do we start on an integer scanline? */
      {
        if ( ras.joint )
        {
          top--;
          ras.joint = FALSE;
        }

        *top++ = arc[degree].x;  /* write directly start position */

        DEBUG_PSET;

        e += ras.precision; /* go to next scanline */
      }
    }

    /* record start position if necessary */
    if ( ras.fresh )
    {
      ras.cur_prof->start = TRUNC( e0 );
      ras.fresh = FALSE;
    }

    /* exit if the current scanline is already above the max scanline */
    if ( e2 < e )
      goto Fin;

    /* check for overflow */
    if ( ( top + TRUNC( e2 - e ) + 1 ) >= ras.pool_limit )
    {
      ras.cursor = top;
      ras.error  = ErrRaster_Overflow;
      return FAILURE;
    }

    
#ifdef FT_DYNAMIC_BEZIER_STEPS
    /* compute dynamic bezier step threshold */
    threshold = Dynamic_Bezier_Threshold( RAS_VAR_ degree, arc );
#else
    threshold = ras.precision_step;
#endif

    start_arc = arc;

    /* loop while there is still an arc on the bezier stack */
    /* and the current scan line is below y max == e2       */
    while ( arc >= start_arc && e <= e2 )
    {
      ras.joint = FALSE;

      y2 = arc[0].y;  /* final y of the top-most arc */

      if ( y2 > e )   /* the arc intercepts the current scanline */
      {
        y1 = arc[degree].y;  /* start y of top-most arc */

#ifdef OLD
        if ( y2-y1 >= ras.precision_step )
#else        
        if ( y2 >= e + ras.precision || y2 - y1 >= threshold )
#endif        
        {
          /* if the arc's height is too great, split it */
          splitter( arc );
          arc += degree;
        }
        else
        {
          /* otherwise, approximate it as a segment and compute */
          /* its intersection with the current scanline         */
          *top++ = arc[degree].x +
                   FMulDiv( arc[0].x-arc[degree].x,
                            e  - y1,
                            y2 - y1 );

          DEBUG_PSET;

          arc -= degree;         /* pop the arc         */
          e   += ras.precision;  /* go to next scanline */
        }
      }
      else
      {
        if ( y2 == e )        /* if the arc falls on the scanline */
        {                     /* record its _joint_ intersection  */
          ras.joint  = TRUE;
          *top++     = arc[0].x;

          DEBUG_PSET;

          e += ras.precision; /* go to next scanline */
        }
        arc -= degree;        /* pop the arc */
      }
    }

  Fin:
    ras.cursor = top;
    ras.arc   -= degree;
    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Bezier_Down                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Computes the scan-line intersections of a descending second-order  */
  /*    Bezier arc and stores them in the render pool.  The arc is taken   */
  /*    from the top of the stack.                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    miny :: The minimum vertical grid coordinate.                      */
  /*    maxy :: The maximum vertical grid coordinate.                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS or FAILURE.                                                */
  /*                                                                       */
  static
  TBool    Bezier_Down( RAS_ARGS int        degree,
                                 TSplitter  splitter,
                                 TPos       miny,
                                 TPos       maxy )
  {
    TPoint*  arc = ras.arc;
    TBool    result, fresh;

    arc[0].y = -arc[0].y;
    arc[1].y = -arc[1].y;
    arc[2].y = -arc[2].y;
    if (degree > 2)
      arc[3].y = -arc[3].y;

    fresh = ras.fresh;

    result = Bezier_Up( RAS_VARS degree, splitter, -maxy, -miny );

    if ( fresh && !ras.fresh )
      ras.cur_prof->start = -ras.cur_prof->start;

    arc[0].y = -arc[0].y;
    return result;
  }


/****************************************************************************/
/*                                                                          */
/* <Function>   Check_Contour                                               */
/*                                                                          */
/* <Description>                                                            */
/*     perform some check at contour closure.                               */
/*                                                                          */
/* <Return>                                                                 */
/*     SUCCESS or FAILURE                                                   */
/*                                                                          */
/****************************************************************************/

  static  
  TBool  Check_Contour( RAS_ARG )
  {
    PProfile  lastProfile;
      
    /* We must now see if the extreme arcs join or not */
    if ( ( FRAC( ras.last.y ) == 0     &&
           ras.last.y >= ras.minY      &&
           ras.last.y <= ras.maxY )    )
    {
      if ( ras.first_prof && ras.first_prof->flow == ras.cur_prof->flow )
        ras.cursor--;
    }
    
    lastProfile = ras.cur_prof;
    if ( End_Profile( RAS_VAR ) ) return FAILURE;
    
    /* close the 'next profile in contour' linked list */
    lastProfile->next = ras.first_prof;
    
    return SUCCESS;
  }

/****************************************************************************/
/*                                                                          */
/* <Function>   Move_To                                                     */
/*                                                                          */
/* <Description>                                                            */
/*     This function injects a new contour in the render pool..             */
/*                                                                          */
/* <Input>                                                                  */
/*     to       :: pointer to the contour's first point                     */
/*     raster   :: a pointer to the current raster object                   */
/*                                                                          */
/* <Return>                                                                 */
/*     Error code, 0 means success                                          */
/*                                                                          */
/* <Note>                                                                   */
/*     This function is used as a "FTRasterMoveTo_Func" by the outline      */
/*     decomposer..                                                         */
/*                                                                          */
/****************************************************************************/

  static
  int  Move_To( FT_Vector*  to,
                FT_Raster   raster )
  {
    /* if there was already a contour being built, perform some checks */
    if ( ras.start_prof )
      if ( Check_Contour( RAS_VAR ) )
        return FAILURE;

    /* set the "current last point" */
    if (ras.flipped)
    {
      ras.last.x = to->y;
      ras.last.y = to->x;
    }
    else
    {
      ras.last.x = to->x;
      ras.last.y = to->y;
    }

    ras.state      = Unknown;
    ras.first_prof = NULL;
    
    return SUCCESS;
  }

/****************************************************************************/
/*                                                                          */
/* <Function>   Line_To                                                     */
/*                                                                          */
/* <Description>                                                            */
/*     This function injects a new line segment in the render pool and      */
/*     adjusts the profiles list accordingly..                              */
/*                                                                          */
/* <Input>                                                                  */
/*     to       :: pointer to the target position                           */
/*     raster   :: a pointer to the current raster object                   */
/*                                                                          */
/* <Return>                                                                 */
/*     Error code, 0 means success                                          */
/*                                                                          */
/* <Note>                                                                   */
/*     This function is used as a "FTRasterLineTo_Func" by the outline      */
/*     decomposer..                                                         */
/*                                                                          */
/****************************************************************************/
  
  static 
  int  Line_To( FT_Vector*  to,
                FT_Raster   raster )
  {
    TPos    x;
    TPos    y;
    TDirection  new_state;
    
    if ( ras.flipped )
    {
      x = to->y;
      y = to->x;
    }
    else
    {
      x = to->x;
      y = to->y;
    }
    
    /* First, detect a change of direction */
    if ( y != ras.last.y )
    {
      new_state = ( y > ras.last.y ? Ascending : Descending );
      if (new_state != ras.state)
      {
        if (ras.state != Unknown && End_Profile( RAS_VAR ))
          goto Fail;
          
        if ( New_Profile( RAS_VARS  new_state) )
          goto Fail;
      }
    }

    /* Then compute the lines */
    if ( (ras.state == Ascending ? Line_Up : Line_Down)
            ( RAS_VARS  ras.last.x, ras.last.y, x, y, ras.minY, ras.maxY ) )
      goto Fail;
      
    ras.last.x = x;
    ras.last.y = y;

    return SUCCESS;
  Fail:
    return FAILURE;
  }

#ifdef FT_RASTER_CONIC_BEZIERS

/****************************************************************************/
/*                                                                          */
/* <Function>   Conic_To                                                    */
/*                                                                          */
/* <Description>                                                            */
/*      Injects a new conic bezier arc and adjusts the profile list         */
/*      accordingly.                                                        */
/*                                                                          */
/* <Input>                                                                  */
/*      control  :: pointer to intermediate control point                   */
/*      to       :: pointer to end point                                    */
/*      raster   :: handle to current raster object                         */
/*                                                                          */
/* <Return>                                                                 */
/*     Error code, 0 means success                                          */
/*                                                                          */
/* <Note>                                                                   */
/*     This function is used as a "FTRasterConicTo_Func" by the outline     */
/*     decomposer..                                                         */
/*                                                                          */
/****************************************************************************/

    static 
    void  Push_Conic( RAS_ARGS FT_Vector*  p2,
                               FT_Vector*  p3 )
    {
      #undef  STORE
      #define STORE( _arc, point )                  \
              {                                     \
                TPos  x = point->x;                 \
                TPos  y = point->y;                 \
                if (ras.flipped)                    \
                {                                   \
                  _arc.x = y;                       \
                  _arc.y = x;                       \
                }                                   \
                else                                \
                {                                   \
                  _arc.x = x;                       \
                  _arc.y = y;                       \
                }                                   \
              }
  
      TPoint*  arc;
      ras.arc = arc = ras.arcs;
  
      arc[2] = ras.last;
      STORE( arc[1], p2 );
      STORE( arc[0], p3 );
      #undef  STORE
    }


  static 
  int  Conic_To( FT_Vector*  control,
                 FT_Vector*  to,
                 FT_Raster   raster )
  {
    TPos        y1, y2, y3, x3;
    TDirection  state_bez;


    Push_Conic( RAS_VARS  control, to );

    do
    {
      y1 = ras.arc[2].y;
      y2 = ras.arc[1].y;
      y3 = ras.arc[0].y;
      x3 = ras.arc[0].x;

      /* first, categorize the bezier arc */

      if( y1 == y3 )
      {
        if ( y2 == y1 )
          state_bez = Flat;
        else
          state_bez = Unknown;
      }
      else if ( y1 < y3 )
      {
        if ( y2 < y1 || y2 > y3 )
          state_bez = Unknown;
        else
          state_bez = Ascending;
      }
      else
      {
        if ( y2 < y3 || y2 > y1 )
          state_bez = Unknown;
        else
          state_bez = Descending;
      }

      /* split non-monotonic arcs, ignore flat ones, or */
      /* computes the up and down ones                  */

      switch ( state_bez )
      {
      case Flat:
        ras.arc -= 2;
        break;

      case Unknown:
        Split_Conic( ras.arc );
        ras.arc += 2;
        break;

      default:
        /* detect a change of direction */

        if ( ras.state != state_bez )
        {
          if ( ras.state != Unknown && End_Profile( RAS_VAR ) )
            goto Fail;

          if ( New_Profile( RAS_VARS state_bez ) )
            goto Fail;
        }

        /* compute intersections */
        if ( (ras.state == Ascending ? Bezier_Up : Bezier_Down)
                ( RAS_VARS 2, Split_Conic, ras.minY, ras.maxY ) )
          goto Fail;
      }
    } while ( ras.arc >= ras.arcs );

    ras.last.x = x3;
    ras.last.y = y3;

    return 0;
  Fail:
    return FAILURE;
  }

#else
  static 
  int  Conic_To( FT_Vector*  control,
                 FT_Vector*  to,
                 FT_Raster   raster )
  {
    (void)control;
    (void)to;
    (void)raster;
    
    return ErrRaster_Invalid_Outline;
  }
#endif /* CONIC_BEZIERS */


#ifdef FT_RASTER_CUBIC_BEZIERS
/****************************************************************************/
/*                                                                          */
/* <Function>   Cubic_To                                                    */
/*                                                                          */
/* <Description>                                                            */
/*      Injects a new cubic bezier arc and adjusts the profile list         */
/*      accordingly.                                                        */
/*                                                                          */
/* <Input>                                                                  */
/*      control1 :: pointer to first control point                          */
/*      control2 :: pointer to second control point                         */
/*      to       :: pointer to end point                                    */
/*      raster   :: handle to current raster object                         */
/*                                                                          */
/* <Return>                                                                 */
/*     Error code, 0 means success                                          */
/*                                                                          */
/* <Note>                                                                   */
/*     This function is used as a "FTRasterCubicTo_Func" by the outline     */
/*     decomposer..                                                         */
/*                                                                          */
/****************************************************************************/

      static 
      void  Push_Cubic( RAS_ARGS FT_Vector*  p2,
                                 FT_Vector*  p3,
                                 FT_Vector*  p4 )
      {
        #undef  STORE
        #define STORE( _arc, point )                  \
                {                                     \
                  TPos  x = point->x;                 \
                  TPos  y = point->y;                 \
                  if (ras.flipped)                    \
                  {                                   \
                    _arc.x = y;                       \
                    _arc.y = x;                       \
                  }                                   \
                  else                                \
                  {                                   \
                    _arc.x = x;                       \
                    _arc.y = y;                       \
                  }                                   \
                }
    
        TPoint*  arc;
        ras.arc = arc = ras.arcs;
    
        arc[3] = ras.last;
        STORE( arc[2], p2 );
        STORE( arc[1], p3 );
        STORE( arc[0], p4 );
    
        #undef STORE
      }


  static 
  int  Cubic_To( FT_Vector*  control1,
                 FT_Vector*  control2,
                 FT_Vector*  to,
                 FT_Raster   raster )
  {
    TPos        y1, y2, y3, y4, x4;
    TDirection  state_bez;

   
    Push_Cubic( RAS_VARS  control1, control2, to );

    do
    {
      y1 = ras.arc[3].y;
      y2 = ras.arc[2].y;
      y3 = ras.arc[1].y;
      y4 = ras.arc[0].y;
      x4 = ras.arc[0].x;

      /* first, categorize the bezier arc */
      if ( y1 == y4 )
      {
        if ( y1 == y2 && y1 == y3 )
          state_bez = Flat;
        else
          state_bez = Unknown;
      }
      else if ( y1 < y4 )
      {
        if ( y2 < y1 || y2 > y4 || y3 < y1 || y3 > y4 )
          state_bez = Unknown;
        else
          state_bez = Ascending;
      }
      else
      {
        if ( y2 < y4 || y2 > y1 || y3 < y4 || y3 > y1 )
          state_bez = Unknown;
        else
          state_bez = Descending;
      }

      /* split non-monotonic arcs, ignore flat ones, or */
      /* computes the up and down ones                  */

      switch ( state_bez )
      {
      case Flat:
        ras.arc -= 3;
        break;

      case Unknown:
        Split_Cubic( ras.arc );
        ras.arc += 3;
        break;

      default:
        /* detect a change of direction */

        if ( ras.state != state_bez )
        {
          if ( ras.state != Unknown && End_Profile( RAS_VAR ) )
            goto Fail;

          if ( New_Profile( RAS_VARS state_bez ) )
            goto Fail;
        }

        /* compute intersections */
        if ( (ras.state == Ascending ? Bezier_Up : Bezier_Down)
                ( RAS_VARS  3, Split_Cubic, ras.minY, ras.maxY ) )
          goto Fail;
      }
    } while ( ras.arc >= ras.arcs );

    ras.last.x = x4;
    ras.last.y = y4;

    return 0;
  Fail:
    return FAILURE;
  }

#else
  int  Cubic_To( FT_Vector*  control1,
                 FT_Vector*  control2,
                 FT_Vector*  to,
                 FT_Raster   raster )
  {
    (void)control1;
    (void)control2;
    (void)to;
    (void)raster;
    
    return ErrRaster_Invalid_Outline;
  }
#endif /* CUBIC_BEZIERS */


/****************************************************************************/
/*                                                                          */
/* <Function>   Convert_Glyph                                               */
/*                                                                          */
/* <Description>                                                            */
/*     Converts a glyph into a series of segments and arcs and makes        */
/*     a profiles list with them..                                          */
/*                                                                          */
/* <Input>                                                                  */
/*     outline  :: glyph outline                                            */
/*                                                                          */
/* <Return>                                                                 */
/*     SUCCESS or FAILURE                                                   */
/*                                                                          */
/****************************************************************************/

  static 
  TBool  Convert_Glyph( RAS_ARGS  FT_Outline*  outline )
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

    /* Set up state in the raster object */
    ras.start_prof = NULL;
    ras.joint      = FALSE;
    ras.fresh      = FALSE;

    ras.pool_limit  = ras.pool_size - AlignProfileSize;

    ras.n_turns = 0;

    ras.cur_prof         = (PProfile)ras.cursor;
    ras.cur_prof->offset = ras.cursor;
    ras.num_profs        = 0;

    interface.shift = ras.scale_shift;
    interface.delta = ras.precision_half;

    /* Now decompose curve */
    if ( FT_Outline_Decompose( outline, &interface, &ras ) ) return FAILURE;
    /* XXX : the error condition is in ras.error */

    /* Check the last contour if needed */
    if ( Check_Contour( RAS_VAR ) ) return FAILURE;
    
    /* Finalize profiles list */
    return Finalize_Profile_Table( RAS_VAR );
  }


/************************************************/
/*                                              */
/*  Init_Linked                                 */
/*                                              */
/*    Inits an empty linked list.               */
/*                                              */
/************************************************/

  static void  Init_Linked( TProfileList*  l )
  {
    *l = NULL;
  }


/************************************************/
/*                                              */
/*  InsNew :                                    */
/*                                              */
/*    Inserts a new Profile in a linked list.   */
/*                                              */
/************************************************/

  static void  InsNew( PProfileList  list,
                       PProfile      profile )
  {
    PProfile  *old, current;
    TPos       x;


    old     = list;
    current = *old;
    x       = profile->X;

    while ( current )
    {
      if ( x < current->X )
        break;
      old     = &current->link;
      current = *old;
    }

    profile->link = current;
    *old          = profile;
  }


/*************************************************/
/*                                               */
/*  DelOld :                                     */
/*                                               */
/*    Removes an old Profile from a linked list. */
/*                                               */
/*************************************************/

  static void  DelOld( PProfileList  list,
                       PProfile      profile )
  {
    PProfile  *old, current;


    old     = list;
    current = *old;

    while ( current )
    {
      if ( current == profile )
      {
        *old = current->link;
        return;
      }

      old     = &current->link;
      current = *old;
    }

    /* we should never get there, unless the Profile was not part of */
    /* the list.                                                     */
  }

/************************************************/
/*                                              */
/*  Update :                                    */
/*                                              */
/*    Update all X offsets of a drawing list    */
/*                                              */
/************************************************/

  static void  Update( PProfile  first )
  {
    PProfile  current = first;

    while (current)
    {
      current->X       = *current->offset;
      current->offset += current->flow;
      current->height--;
      current = current->link;
    }
  }

/************************************************/
/*                                              */
/*  Sort :                                      */
/*                                              */
/*    Sorts a trace list.  In 95%, the list     */
/*    is already sorted.  We need an algorithm  */
/*    which is fast in this case.  Bubble sort  */
/*    is enough and simple.                     */
/*                                              */
/************************************************/

  static void  Sort( PProfileList  list )
  {
    PProfile  *old, current, next;


    /* First, set the new X coordinate of each profile */
    Update( *list );

    /* Then sort them */
    old     = list;
    current = *old;

    if ( !current )
      return;

    next = current->link;

    while ( next )
    {
      if ( current->X <= next->X )
      {
        old     = &current->link;
        current = *old;

        if ( !current )
          return;
      }
      else
      {
        *old          = next;
        current->link = next->link;
        next->link    = current;

        old     = list;
        current = *old;
      }

      next = current->link;
    }
  }


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/********                                                         *********/
/********            Vertical Bitmap Sweep Routines               *********/
/********                                                         *********/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


/***********************************************************************/
/*                                                                     */
/* <Function> Vertical_Sweep_Init                                      */
/*                                                                     */
/* <Description>                                                       */
/*     Initialise the vertical bitmap sweep. Called by the generic     */
/*     sweep/draw routine before its loop..                            */
/*                                                                     */
/* <Input>                                                             */
/*    min  :: address of current minimum scanline                      */
/*    max  :: address of current maximum scanline                      */
/*                                                                     */
/***********************************************************************/

  static 
  void  Vertical_Sweep_Init( RAS_ARGS int*  min, int*  max )
  {
    long  pitch = ras.target.pitch;
    
    (void)max;
    
    ras.trace_incr = -pitch;
    ras.trace_bit  = -*min*pitch;
    if (pitch > 0)
      ras.trace_bit += (ras.target.rows-1)*pitch;
      
    ras.gray_min_x = 0;
    ras.gray_max_x = 0;
  }

/***********************************************************************/
/*                                                                     */
/* <Function> Vertical_Sweep_Span                                      */
/*                                                                     */
/* <Description>                                                       */
/*     draws a single horizontal bitmap span during the vertical       */
/*     bitmap sweep.                                                   */
/*                                                                     */
/* <Input>                                                             */
/*     y  :: current scanline                                          */
/*     x1 :: left span edge                                            */
/*     x2 :: right span edge                                           */
/*                                                                     */
/***********************************************************************/

  static void  Vertical_Sweep_Span( RAS_ARGS TScan  y,
                                             TPos   x1,
                                             TPos   x2 )
  {
    TPos   e1, e2;
    int    c1, c2;
    TByte   f1, f2;
    TByte*  target;

    /* Drop-out control */
    (void)y;

    e1 = TRUNC( CEILING( x1 ) );
    if ( x2-x1-ras.precision <= ras.precision_jitter )
      e2 = e1;
    else
      e2 = TRUNC( FLOOR( x2 ) );

    if ( e1 <= e2 && e2 >= 0 && e1 < ras.bit_width )
    {
      if ( e1 < 0 )              e1 = 0;
      if ( e2 >= ras.bit_width ) e2 = ras.bit_width-1;

      c1 = e1 >> 3;
      c2 = e2 >> 3;

      f1 =  ((unsigned char)0xFF >> (e1 & 7));
      f2 = ~((unsigned char)0x7F >> (e2 & 7));

      target = ras.bit_buffer + ras.trace_bit + c1;
      c2 -= c1;

      if ( c2 > 0 )
      {
        target[0] |= f1;

        /* memset is slower than the following code on many platforms   */
        /* this is due to the fact that, in the vast majority of cases, */
        /* the span length in bytes is relatively small..               */
        c2--;
        while (c2 > 0)
        {
          * ++target = 0xFF;
          c2--;
        }
        target[1] |= f2;
      }
      else
        *target |= ( f1 & f2 );
    }
  }

/***********************************************************************/
/*                                                                     */
/* <Function> Vertical_Test_Pixel                                      */
/*                                                                     */
/* <Description>                                                       */
/*     test a pixel 'light' during the vertical bitmap sweep. Used     */
/*     during drop-out control only..                                  */
/*                                                                     */
/* <Input>                                                             */
/*     y  :: current scanline                                          */
/*     x  :: current x coordinate                                      */
/*                                                                     */
/***********************************************************************/

  static 
  int  Vertical_Test_Pixel( RAS_ARGS TScan  y,
                                     int    x )
  {
    int c1 = x >> 3;
    (void)y;
    return ( x >= 0 && x < ras.bit_width && 
             ras.bit_buffer[ras.trace_bit + c1] & (0x80 >> (x & 7)) );
  }

/***********************************************************************/
/*                                                                     */
/* <Function> Vertical_Set_Pixel                                       */
/*                                                                     */
/* <Description>                                                       */
/*     Sets a single pixel in a bitmap during the vertical sweep.      */
/*     Used during drop-out control..                                  */
/*                                                                     */
/* <Input>                                                             */
/*     y  :: current scanline                                          */
/*     x  :: current x coordinate                                      */
/* color  :: ignored by this function                                  */
/*                                                                     */
/***********************************************************************/

  static 
  void  Vertical_Set_Pixel( RAS_ARGS  int  y,
                                      int  x,
                                      int  color )
  {
    (void)color;  /* unused here */
    (void)y;
    
    if ( x >= 0 && x < ras.bit_width )
    {
      int c1 = x >> 3;

      if ( ras.gray_min_x > c1 ) ras.gray_min_x = c1;
      if ( ras.gray_max_x < c1 ) ras.gray_max_x = c1;

      ras.bit_buffer[ras.trace_bit+c1] |= (char)(0x80 >> (x & 7));
    }
  }

/***********************************************************************/
/*                                                                     */
/* <Function> Vertical_Sweep_Step                                      */
/*                                                                     */
/* <Description>                                                       */
/*     Called whenever the sweep jumps to anothr scanline.             */
/*     Only updates the pointers in the vertical bitmap sweep          */
/*                                                                     */
/***********************************************************************/

  static
  void Vertical_Sweep_Step( RAS_ARG )
  {
    ras.trace_bit += ras.trace_incr;
  }


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/********                                                         *********/
/********           Horizontal Bitmap Sweep Routines              *********/
/********                                                         *********/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/***********************************************************************/
/*                                                                     */
/* <Function> Horizontal_Sweep_Init                                    */
/*                                                                     */
/* <Description>                                                       */
/*     Initialise the horizontal bitmap sweep. Called by the generic   */
/*     sweep/draw routine before its loop..                            */
/*                                                                     */
/* <Input>                                                             */
/*    min  :: address of current minimum pixel column                  */
/*    max  :: address of current maximum pixel column                  */
/*                                                                     */
/***********************************************************************/

  static void  Horizontal_Sweep_Init( RAS_ARGS int*  min, int*  max )
  {
    /* nothing, really */
    UNUSED_RASTER
    (void)min;
    (void)max;
  }


/***********************************************************************/
/*                                                                     */
/* <Function> Horizontal_Sweep_Span                                    */
/*                                                                     */
/* <Description>                                                       */
/*     draws a single vertical bitmap span during the horizontal       */
/*     bitmap sweep. Actually, this function is only used to           */
/*     check for weird drop-out cases..                                */
/*                                                                     */
/* <Input>                                                             */
/*     y  :: current pixel column                                      */
/*     x1 :: top    span edge                                          */
/*     x2 :: bottom span edge                                          */
/*                                                                     */
/***********************************************************************/

  static void  Horizontal_Sweep_Span( RAS_ARGS TScan  y,
                                               TPos   x1,
                                               TPos   x2 )
  {
    TPos  e1, e2;
    PByte bits;
    TByte  f1;

    (void)y;

    /* During the horizontal sweep, we only take care of drop-outs */
    if ( x2-x1 < ras.precision )
    {
      e1 = CEILING( x1 );
      e2 = FLOOR( x2 );
      
      if ( e1 == e2 )
      {
        bits = ras.bit_buffer + (y >> 3);
        f1   = (TByte)(0x80 >> (y & 7));
        
        e1 = TRUNC( e1 );

        if ( e1 >= 0 && e1 < ras.target.rows )
        {
          long  pitch = ras.target.pitch;
          
          bits -= e1*pitch;
          if (pitch > 0)
            bits += (ras.target.rows-1)*pitch;
            
          bits[0] |= f1;
        }
      }
    }
  }


/***********************************************************************/
/*                                                                     */
/* <Function> Horizontal_Test_Pixel                                    */
/*                                                                     */
/* <Description>                                                       */
/*     test a pixel 'light' during the horizontal bitmap sweep. Used   */
/*     during drop-out control only..                                  */
/*                                                                     */
/* <Input>                                                             */
/*     y  :: current pixel column                                      */
/*     x  :: current row/scanline                                      */
/*                                                                     */
/***********************************************************************/

  static
  int   Horizontal_Test_Pixel( RAS_ARGS  int  y,
                                         int  x )
  {
    char*  bits  = (char*)ras.bit_buffer + (y >> 3);
    int    f1    = (TByte)(0x80 >> (y & 7));
    long   pitch = ras.target.pitch;
    
    bits -= x*pitch;
    if (pitch > 0)
      bits += (ras.target.rows-1)*pitch;
      
    return ( x >= 0 && x < ras.target.rows && (bits[0] & f1) );
  }

/***********************************************************************/
/*                                                                     */
/* <Function> Horizontal_Set_Pixel                                     */
/*                                                                     */
/* <Description>                                                       */
/*     Sets a single pixel in a bitmap during the horizontal sweep.    */
/*     Used during drop-out control..                                  */
/*                                                                     */
/* <Input>                                                             */
/*     y  :: current pixel column                                      */
/*     x  :: current row/scanline                                      */
/* color  :: ignored by this function                                  */
/*                                                                     */
/***********************************************************************/

  static
  void  Horizontal_Set_Pixel( RAS_ARGS  int  y,
                                        int  x,
                                        int  color )
  {
    char*  bits  = (char*)ras.bit_buffer + (y >> 3);
    int    f1    = (TByte)(0x80 >> (y  & 7));
    long   pitch = ras.target.pitch;

    (void)color;   /* unused here */

    bits -= x*pitch;
    if (pitch > 0)
      bits += (ras.target.rows-1)*pitch;
      
    if ( x >= 0 && x < ras.target.rows )
      bits[0] |= f1;
  }

/***********************************************************************/
/*                                                                     */
/* <Function> Horizontal_Sweep_Step                                    */
/*                                                                     */
/* <Description>                                                       */
/*     Called whenever the sweep jumps to another pixel column.        */
/*                                                                     */
/***********************************************************************/

  static void Horizontal_Sweep_Step( RAS_ARG )
  {
    /* Nothing, really */
    UNUSED_RASTER;
  }


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/********                                                         *********/
/********      Anti-Aliased Vertical Bitmap Sweep Routines        *********/
/********                                                         *********/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

#ifdef FT_RASTER_OPTION_ANTI_ALIAS

#ifdef FT_RASTER_ANTI_ALIAS_5
/***********************************************************************/
/*                                                                     */
/*  Vertical Gray Sweep Procedure Set :                                */
/*                                                                     */
/*  These two routines are used during the vertical gray-levels        */
/*  sweep phase by the generic Draw_Sweep() function.                  */
/*                                                                     */
/*                                                                     */
/*  NOTES:                                                             */
/*                                                                     */
/*  - The target pixmap's width *must* be a multiple of 4.             */
/*                                                                     */
/*  - you have to use the function Vertical_Sweep_Span() for           */
/*    the gray span call.                                              */
/*                                                                     */
/***********************************************************************/

  static void  Vertical_Gray5_Sweep_Init( RAS_ARGS int*  min, int*  max )
  {
    long  pitch;
    
    *min = *min & -2;
    *max = ( *max+3 ) & -2;

    ras.trace_bit = 0;

    pitch          = ras.target.pitch;
    ras.trace_incr = -pitch;
    ras.trace_pix  = - (*min/2)*pitch;
    if (pitch > 0)
      ras.trace_pix += (ras.target.rows-1)*pitch;
      
    ras.gray_min_x =  32000;
    ras.gray_max_x = -32000;
  }


  static void  Vertical_Gray5_Sweep_Span( RAS_ARGS TScan  y,
                                                   TPos   x1,
                                                   TPos   x2 )
  {
    static
    const unsigned int  LMask[17] =
#ifdef FT_RASTER_LITTLE_ENDIAN
             { 0xF0F0F0F0, 0xF0F0F070, 0xF0F0F030, 0xF0F0F010,
               0xF0F0F000, 0xF0F07000, 0xF0F03000, 0xF0F01000,
               0xF0F00000, 0xF0700000, 0xF0300000, 0xF0100000,
               0xF0000000, 0x70000000, 0x30000000, 0x10000000,
               0x00000000 };
#else
             { 0xF0F0F0F0, 0x70F0F0F0, 0x30F0F0F0, 0x10F0F0F0,
               0x00F0F0F0, 0x0070F0F0, 0x0030F0F0, 0x0010F0F0,
               0x0000F0F0, 0x000070F0, 0x000030F0, 0x000010F0,
               0x000000F0, 0x00000070, 0x00000030, 0x00000010,
               0x00000000 };
#endif

    TPos   e1, e2;
    int    c1, c2;
    TByte*  target;

    unsigned int    f1, f2;
    unsigned int    shift, fill;

    /* Drop-out control */

    e1 = TRUNC( CEILING( x1 ) );
    if ( x2-x1 <= ras.precision+ras.precision_jitter )
      e2 = e1;
    else
      e2 = FLOOR ( x2 );

    e2 = TRUNC( e2 );
 
    if ( e1 <= e2 && e2 >= 0 && e1 < ras.bit_width )
    {
      if ( e1 < 0 )              e1 = 0;
      if ( e2 >= ras.bit_width ) e2 = ras.bit_width-1;

      shift = (y & 1)*4;

      c1 = e1 >> 4;
      c2 = e2 >> 4;

      fill = LMask[0];
      f1   = LMask[ e1 & 15 ];
      f2   = fill ^ LMask[ 1+(e2 & 15) ];

      f1   >>= shift;
      f2   >>= shift;
      fill >>= shift;

      if ( ras.gray_min_x > c1 ) ras.gray_min_x = c1;
      if ( ras.gray_max_x < c2 ) ras.gray_max_x = c2;

      target = ras.bit_buffer + c1*4;
      c2 -= c1;

      if (c2 > 0)
      {
        int*  slice = (int*)target;
        
        slice[0] |= f1;
        c2--;
        while (c2 > 0)
        {
          * ++slice |= fill;
          c2--;
        }
        slice[1] |= f2;
      }
      else
        ((int*)target)[0] |= ( f1 & f2 );
    }
  }


  static 
  int  Vertical_Gray5_Test_Pixel( RAS_ARGS int  y,
                                           int  x )
  {
    int c1    = x >> 2;
    int f1    = x  & 3;
    int mask  = (0x80 >> f1) >> ((y & 1)*4);

    (void)y;
    
    return ( x >= 0                    && 
             x < ras.bit_width         && 
             ras.bit_buffer[c1] & mask );
  }


  static 
  void  Vertical_Gray5_Set_Pixel( RAS_ARGS  int  y,
                                            int  x,
                                            int  color )
  {
    (void)color;  /* unused here */
    (void)y;

    if ( x >= 0 && x < ras.bit_width )
    {
      int c1   = x >> 2;
      int f1   = x  & 3;
      int mask = (0x80 >> f1) >> ((y & 1)*4);
  
      if ( ras.gray_min_x > c1/4 ) ras.gray_min_x = c1/4;
      if ( ras.gray_max_x < c1/4 ) ras.gray_max_x = c1/4;

      ras.bit_buffer[c1] |= mask;
    }
  }


  static void  Vertical_Gray5_Sweep_Step( RAS_ARG )
  {
    int    c1;
    PByte  pix, bit;
    int*   count = ras.count_table;
    long*  grays;


    ras.trace_bit++;

    if ( ras.trace_bit > 1 )
    {
      pix   = ras.pix_buffer + ras.trace_pix + ras.gray_min_x*8;
      grays = ras.grays;

      if ( ras.gray_max_x >= 0 )
      {
        if ( ras.gray_max_x*8 >= ras.target.width ) 
          ras.gray_max_x = (ras.target.width+7)/8-1;

        if ( ras.gray_min_x < 0 ) 
          ras.gray_min_x = 0;

        bit = ras.bit_buffer + ras.gray_min_x*4;
        c1  = (ras.gray_max_x - ras.gray_min_x + 1);

        while ( c1 >= 0 )
        {
          if ( *(short*)bit )
          {
#if defined( FT_RASTER_LITTLE_ENDIAN )
            /* little endian storage */
            *((long*)pix) = (count[bit[1]] << 16) | count[bit[0]];
#elif defined( FT_RASTER_BIG_ENDIAN )
            /* big-endian storage */
            *((long*)pix) = (count[bit[0]] << 16) | count[bit[1]];
#else
            /* endian-independent storage */
            int  c;
            c = count[bit[1]];
            pix[4] = (TByte)(c >> 8);
            pix[5] = (TByte)(c & 0xFF);
            c = count[bit[0]];
            pix[6] = (TByte)(c >> 8);
            pix[7] = (TByte)(c & 0xFF);
#endif
            *(short*)bit = 0;
          }

          bit += 2;
          
          if ( *(short*)bit )
          {
#if defined( FT_RASTER_LITTLE_ENDIAN )
            /* little endian storage */
            *((long*)pix + 1)= (count[bit[1]] << 16) | count[bit[0]];
#elif defined( FT_RASTER_BIG_ENDIAN )
            /* big-endian storage */
            *((long*)pix + 1) = (count[bit[0]] << 16) | count[bit[1]];
#else
            /* endian-independent storage */
            int  c;
            c = count[bit[1]];
            pix[0] = (TByte)(c >> 8);
            pix[1] = (TByte)(c & 0xFF);
            c = count[bit[0]];
            pix[2] = (TByte)(c >> 8);
            pix[3] = (TByte)(c & 0xFF);
#endif
            *(short*)bit = 0;
          }
          pix += 8;
          bit += 2;
          c1 --;
        }
      }
     
      ras.trace_bit  = 0;
      ras.trace_pix += ras.trace_incr;

      ras.gray_min_x =  32000;
      ras.gray_max_x = -32000;
    }
  }



  static void  Horizontal_Gray5_Sweep_Span( RAS_ARGS TScan  y,
                                                     TPos   x1,
                                                     TPos   x2 )
  {
    /* nothing, really */
    UNUSED_RASTER
    (void)y;
    (void)x1;
    (void)x2;
  }


  static
  int   Horizontal_Gray5_Test_Pixel( RAS_ARGS  TScan  y,
                                               int    x )
  {
    /* don't do anything here */
    UNUSED_RASTER
    (void)x;
    (void)y;
    
    return 0;
  }


  static
  void  Horizontal_Gray5_Set_Pixel( RAS_ARGS  TScan  y,
                                              int    x,
                                              int    color )
  {
    char*  pixel;

    x = x/2;
      
    if (x < ras.target.rows)
    {
      long  pitch;
      
      pixel = (char*)ras.pix_buffer + (y/2);
      pitch = ras.target.pitch;
      pixel -= x*pitch;
      if (pitch > 0)
        pixel += (ras.target.rows-1)*pitch;
        
      if (pixel[0] == ras.grays[0])
        pixel[0] = (char)ras.grays[1+color];
    }
  }
#endif /* FT_RASTER_ANTI_ALIAS_5 */
  
#ifdef FT_RASTER_ANTI_ALIAS_17
/***********************************************************************/
/*                                                                     */
/*  Vertical Gray Sweep Procedure Set :                                */
/*                                                                     */
/*  These two routines are used during the vertical gray-levels        */
/*  sweep phase by the generic Draw_Sweep() function.                  */
/*                                                                     */
/*                                                                     */
/*  NOTES:                                                             */
/*                                                                     */
/*  - The target pixmap's width *must* be a multiple of 4.             */
/*                                                                     */
/*  - you have to use the function Vertical_Sweep_Span() for           */
/*    the gray span call.                                              */
/*                                                                     */
/***********************************************************************/

  static void  Vertical_Gray17_Sweep_Init( RAS_ARGS int*  min, int*  max )
  {
    long  pitch = ras.target.pitch;
    
    *min = *min & -4;
    *max = ( *max+7 ) & -4;

    ras.trace_bit = 0;
    ras.trace_incr = -pitch;
    ras.trace_bit  = - (*min/4)*pitch;
    if (pitch > 0)
      ras.trace_bit += (ras.target.rows-1)*pitch;

    ras.gray_min_x =  32000;
    ras.gray_max_x = -32000;
  }


  static void  Vertical_Gray17_Sweep_Span( RAS_ARGS TScan  y,
                                                    TPos   x1,
                                                    TPos   x2 )
  {
    static
    const unsigned int  LMask[9] =
#ifdef FT_RASTER_LITTLE_ENDIAN
             { 0xF000F000, 0xF0007000, 0xF0003000, 0xF0001000,
               0xF0000000, 0x70000000, 0x30000000, 0x10000000,
               0x00000000 };
#else
             { 0xF000F000, 0x7000F000, 0x3000F000, 0x1000F000,
               0x0000F000, 0x00007000, 0x00003000, 0x00001000,
               0x00000000 };
#endif

    TPos   e1, e2;
    int    c1, c2;
    TByte*  target;

    unsigned int    f1, f2;
    unsigned int    shift, fill;

    /* Drop-out control */

    e1 = TRUNC( CEILING( x1 ) );
    if ( x2-x1 <= ras.precision+ras.precision_jitter )
      e2 = e1;
    else
      e2 = FLOOR ( x2 );
    e2 = TRUNC( e2 );
 
    if ( e1 <= e2 && e2 >= 0 && e1 < ras.bit_width )
    {
      if ( e1 < 0 )              e1 = 0;
      if ( e2 >= ras.bit_width ) e2 = ras.bit_width-1;

      shift = (y & 3)*4;

      c1 = e1 >> 3;
      c2 = e2 >> 3;

      fill = LMask[0];
      f1   = LMask[ e1 & 7 ];
      f2   = fill ^ LMask[ 1+(e2 & 7) ];

      f1   >>= shift;
      f2   >>= shift;
      fill >>= shift;

      if ( ras.gray_min_x > c1/2 ) ras.gray_min_x = c1/2;
      if ( ras.gray_max_x < c2/2 ) ras.gray_max_x = c2/2;

      target = ras.bit_buffer + c1*4;
      c2 -= c1;

      if (c2 > 0)
      {
        int*  slice = (int*)target;

        slice[0] |= f1;
        c2--;
        while (c2 > 0)
        {
          * ++slice |= fill;
          c2--;
        }
        slice[1] |= f2;
      }
      else
        ((int*)target)[0] |= ( f1 & f2 );
    }
  }


  static 
  int  Vertical_Gray17_Test_Pixel( RAS_ARGS int  y,
                                            int  x )
  {
    int c1    = x >> 2;
    int f1    = x  & 3;
    int mask  = (0x8000 >> f1) >> ((y & 3)*4);

    return ( x >= 0                    && 
             x < ras.bit_width         && 
             ((short*)ras.bit_buffer)[c1] & mask );
  }


  static 
  void  Vertical_Gray17_Set_Pixel( RAS_ARGS  int  y,
                                             int  x,
                                             int  color )
  {
    (void)color;  /* unused here */

    if ( x >= 0 && x < ras.bit_width )
    {
      int c1   = x >> 2;
      int f1   = x  & 3;
      int mask = (0x8000 >> f1) >> ((y & 3)*4);
  
      if ( ras.gray_min_x > c1/4 ) ras.gray_min_x = c1/4;
      if ( ras.gray_max_x < c1/4 ) ras.gray_max_x = c1/4;

      ((short*)ras.bit_buffer)[c1] |= mask;
    }
  }


  static void  Vertical_Gray17_Sweep_Step( RAS_ARG )
  {
    int    c1;
    PByte  pix, bit;
    long*  grays;


    ras.trace_bit++;

    if ( ras.trace_bit > 3 )
    {
      pix   = ras.pix_buffer + ras.trace_pix + ras.gray_min_x*4;
      grays = ras.grays;

      if ( ras.gray_max_x >= 0 )
      {
        if ( ras.gray_max_x >= ras.target.width/4 ) 
          ras.gray_max_x = (ras.target.width+3)/4-1;

        if ( ras.gray_min_x < 0 ) 
          ras.gray_min_x = 0;

        bit = ras.bit_buffer + ras.gray_min_x*8;
        c1  = (ras.gray_max_x - ras.gray_min_x + 1);

        while ( c1 >= 0 )
        {
          if ( *(long*)bit || *(long*)(bit+4) )
          {
            int*  table = ras.count_table;

#if defined( FT_RASTER_LITTLE_ENDIAN )
            /* little-endian specific storage */
            *(long*)pix =   grays[ table[ bit[0] ] +
                                   table[ bit[1] ] ]         |
                                      
                          ( grays[ table[ bit[2] ] +
                                   table[ bit[3] ] ] << 8 )  |
                                   
                          ( grays[ table[ bit[4] ] +
                                   table[ bit[5] ] ] << 16 ) |
                                   
                          ( grays[ table[ bit[6] ] +
                                   table[ bit[7] ] ] << 24 );

#elif defined( FT_RASTER_BIG_ENDIAN )
            /* big-endian specific storage */
            *(long*)pix = ( grays[ table[ bit[0] ] +
                                   table[ bit[1] ] ] << 24    |
                                      
                          ( grays[ table[ bit[2] ] +
                                   table[ bit[3] ] ] << 16 )  |
                                   
                          ( grays[ table[ bit[4] ] +
                                   table[ bit[5] ] ] << 8 ) |
                                   
                          ( grays[ table[ bit[6] ] +
                                   table[ bit[7] ] ]  );
#else
            /* endianess-independent storage */
            pix[0] = grays[ table[bit[0]] + table[bit[1]] ];
            pix[1] = grays[ table[bit[2]] + table[bit[3]] ];
            pix[2] = grays[ table[bit[4]] + table[bit[5]] ];
            pix[3] = grays[ table[bit[6]] + table[bit[7]] ];
#endif
            *(long*)( bit ) = 0;
            *(long*)(bit+4) = 0;
          }
          pix += 4;
          bit += 8;
          c1 --;
        }
      }
      ras.trace_bit   = 0;
      ras.trace_pix  += ras.trace_incr;

      ras.gray_min_x =  32000;
      ras.gray_max_x = -32000;
    }
  }



  static void  Horizontal_Gray17_Sweep_Span( RAS_ARGS TScan  y,
                                                      TPos   x1,
                                                      TPos   x2 )
  {
    /* nothing, really */
    UNUSED_RASTER
    (void)y;
    (void)x1;
    (void)x2;
  }


  static
  int   Horizontal_Gray17_Test_Pixel( RAS_ARGS  TScan  y,
                                                int    x )
  {
    /* don't do anything here */
    UNUSED_RASTER
    (void)x;
    (void)y;
    
    return 0;
  }


  static
  void  Horizontal_Gray17_Set_Pixel( RAS_ARGS  TScan  y,
                                               int    x,
                                               int    color )
  {
    char*  pixel;

    x = x/4;
      
    if (x < ras.target.rows)
    {
      long  pitch = ras.target.pitch;
      
      pixel  = (char*)ras.pix_buffer + (y/4);
      pixel -= x*pitch;
      if (pitch > 0)
        pixel += (ras.target.rows-1)*pitch;
        
      if (pixel[0] == ras.grays[0])
        pixel[0] = ras.grays[1+color];
    }
  }
#endif /* FT_RASTER_ANTI_ALIAS_17 */
  
#endif /* FT_RASTER_OPTION_ANTI_ALIAS */

/********************************************************************/
/*                                                                  */
/*  Generic Sweep Drawing routine                                   */
/*                                                                  */
/********************************************************************/

  static TBool  Draw_Sweep( RAS_ARG )
  {
    TScan  y, y_change, y_height;

    PProfile  P, Q, P_Left, P_Right;

    TScan   min_Y, max_Y, top, bottom, dropouts;

    TPos x1, x2, e1, e2;

    TProfileList  wait;
    TProfileList  draw;

    /* Init empty linked lists */

    Init_Linked( &wait );
    Init_Linked( &draw );

    /* first, compute min and max Y */

    P     = ras.start_prof;
    max_Y = TRUNC( ras.minY );
    min_Y = TRUNC( ras.maxY );

    while ( P )
    {
      Q = P->link;

      bottom = P->start;
      top    = P->start + P->height-1;

      if ( min_Y > bottom ) min_Y = bottom;
      if ( max_Y < top    ) max_Y = top;

      P->X = 0;
      InsNew( &wait, P );

      P = Q;
    }

    /* Check the Y-turns */
    if ( ras.n_turns == 0 )
    {
      ras.error = ErrRaster_Invalid_Outline;
      return FAILURE;
    }

    /* Now inits the sweep */

    ras.Proc_Sweep_Init( RAS_VARS  &min_Y, &max_Y );

    /* Then compute the distance of each profile from min_Y */

    P = wait;

    while ( P )
    {
      P->countL = P->start - min_Y;
      P = P->link;
    }

    /* Let's go */

    y        = min_Y;
    y_height = 0;

    if ( ras.n_turns > 0 &&
         ras.pool_size[-ras.n_turns] == min_Y )
      ras.n_turns--;

    while (ras.n_turns > 0)
    {
      PProfile  prof = wait;
      
      /* look in the wait list for new activations */
      while (prof)
      {
        PProfile  next = prof->link;
        
        prof->countL -= y_height;
        if ( prof->countL == 0 )
        {
          DelOld( &wait, prof );
          InsNew( &draw, prof );
        }
        prof = next;
      }

      /* Sort the drawing lists */
      Sort( &draw );

      y_change = ras.pool_size[-ras.n_turns--];
      y_height = y_change - y;

      while ( y < y_change )
      {
        int       window;
        PProfile  left;

        /* Let's trace */
  
        dropouts = 0;

        if (!draw)
          goto Next_Line;
          
        left   = draw;
        window = left->flow;
        prof   = left->link;

        while (prof)
        {
          PProfile  next = prof->link;
          
          window += prof->flow;
          
          if ( window == 0 )
          {
            x1 = left->X;
            x2 = prof->X;
#if 1
            if ( x1 > x2 )
            {
              TPos  xs = x1;
              x1 = x2;
              x2 = xs;
            }
#endif
            if ( x2-x1 <= ras.precision && ras.dropout_mode )
            {
#if 1
              e1 = CEILING( x1 );
              e2 = FLOOR( x2 );
#else
              e1 = FLOOR(x1);
              e2 = CEILING(x2);
#endif
              if ( e1 > e2 || e2 == e1+ ras.precision )
              {
                /* a drop out was detected */
   
                left->X = x1;
                prof->X = x2;

                /* mark profiles for drop-out processing */
                left->countL = 1;
                prof->countL = 2;
                dropouts++;
                goto Skip_To_Next;
              }
            }

            ras.Proc_Sweep_Span( RAS_VARS  y, x1, x2 );
   
   Skip_To_Next:
            left = next;
          }       
          prof = next;
        }

        /* now perform the dropouts _after_ the span drawing   */
        /* drop-outs processing has been moved out of the loop */
        /* for performance tuning                              */
        if (dropouts > 0)
          goto Scan_DropOuts;

   Next_Line:

        ras.Proc_Sweep_Step( RAS_VAR );

        y++;

        if ( y < y_change )
          Sort( &draw );
      }

      /* Now finalize the profiles that needs it */

      {
        PProfile  prof, next;
        prof = draw;
        while (prof)
        {
          next = prof->link;
          if (prof->height == 0)
            DelOld( &draw, prof );
          prof = next;
        }
      }
    }

    /* for gray-scaling, flushes the bitmap scanline cache */
    while (y <= max_Y)
    {
        ras.Proc_Sweep_Step( RAS_VAR );
        y++;
    }

    return SUCCESS;


Scan_DropOuts :
    P_Left = draw;
    while (dropouts > 0)
    {
      TPos      e1,   e2;
      PProfile  left, right;
    
      while (P_Left->countL != 1) P_Left = P_Left->link;
      P_Right = P_Left->link;
      while (P_Right->countL != 2) P_Right = P_Right->link;
       
      P_Left->countL  = 0;
      P_Right->countL = 0;

      /* Now perform the dropout control */      
      x1 = P_Left->X;
      x2 = P_Right->X;

      left  = ( ras.flipped ? P_Right : P_Left  );
      right = ( ras.flipped ? P_Left  : P_Right );
      
      e1 = CEILING( x1 );
      e2 = FLOOR  ( x2 );

      if ( e1 > e2 )
      {
        if ( e1 == e2 + ras.precision )
        {
          switch ( ras.dropout_mode )
          {
          case 1:
            e1 = e2;
            break;

          case 4:
            e1 = CEILING( (x1+x2+1) / 2 );
            break;

          case 2:
          case 5:
          /* Drop-out Control Rule #4 */
  
          /* The spec is not very clear regarding rule #4.  It      */
          /* presents a method that is way too costly to implement  */
          /* while the general idea seems to get rid of 'stubs'.    */
          /*                                                        */
          /* Here, we only get rid of stubs recognized when:        */
          /*                                                        */
          /*  upper stub:                                           */
          /*                                                        */
          /*   - P_Left and P_Right are in the same contour         */
          /*   - P_Right is the successor of P_Left in that contour */
          /*   - y is the top of P_Left and P_Right                 */
          /*                                                        */
          /*  lower stub:                                           */
          /*                                                        */
          /*   - P_Left and P_Right are in the same contour         */
          /*   - P_Left is the successor of P_Right in that contour */
          /*   - y is the bottom of P_Left                          */
          /*                                                        */

            /* upper stub test */
            if ( ( left->next == right && left->height <= 0 ) ||

            /* lower stub test */
                 ( right->next == left && left->start == y )  ||

            /* check that the rightmost pixel isn't set */
                 ras.Proc_Test_Pixel( RAS_VARS  y, TRUNC(e1))       )
              goto Next_Dropout;

            if ( ras.dropout_mode == 2 )
              e1 = e2;
            else
              e1 = CEILING( (x1+x2+1)/2 );
                    
            break;

          default:
            goto Next_Dropout;  /* Unsupported mode */
          }
        }
        else
          goto Next_Dropout;
      }

      ras.Proc_Set_Pixel( RAS_VARS y, 
                                   TRUNC(e1), 
                                   (x2-x1 >= ras.precision_half) & 1 );
    Next_Dropout:
    
      dropouts--;
    }
    goto Next_Line;
  }



/****************************************************************************/
/*                                                                          */
/* Function:    Render_Single_Pass                                          */
/*                                                                          */
/* Description: Performs one sweep with sub-banding.                        */
/*                                                                          */
/* Input:       _XCoord, _YCoord : x and y coordinates arrays               */
/*                                                                          */
/* Returns:     SUCCESS on success                                          */
/*              FAILURE if any error was encountered during render.         */
/*                                                                          */
/****************************************************************************/

  static 
  int  Render_Single_Pass( RAS_ARGS  int  flipped )
  {
    int  i, j, k;

    ras.flipped = flipped;

    while ( ras.band_top >= 0 )
    {
#if 1
      ras.maxY = (long)ras.band_stack[ras.band_top].y_max * ras.precision;
      ras.minY = (long)ras.band_stack[ras.band_top].y_min * ras.precision;
#else    
      ras.maxY = ((long)ras.band_stack[ras.band_top].y_max <<
                    (ras.scale_shift+6))-1;

      ras.minY = (long)ras.band_stack[ras.band_top].y_min <<
                    (ras.scale_shift+6);
#endif
      ras.cursor = ras.pool;
      ras.error  = 0;

      if ( Convert_Glyph( RAS_VARS  ras.outline ) )
      {
        if ( ras.error != ErrRaster_Overflow ) return FAILURE;
        ras.error = ErrRaster_Ok;

        /* sub-banding */

#ifdef DEBUG_RASTER
        ClearBand( RAS_VARS  TRUNC( ras.minY ), TRUNC( ras.maxY ) );
#endif

        i = ras.band_stack[ras.band_top].y_min;
        j = ras.band_stack[ras.band_top].y_max;

        k = ( j-i ) >> 1;

        if ( ras.band_top >= 7 || k == 0 )
        {
          ras.band_top     = 0;
          ras.error = ErrRaster_Invalid_Outline;
          return ras.error;
        }

        ras.band_stack[ras.band_top+1].y_min = i + k;
        ras.band_stack[ras.band_top+1].y_max = j;

        ras.band_stack[ras.band_top].y_max = i + k;

        ras.band_top++;
      }
      else
      {
        if ( ras.start_prof )
          if ( Draw_Sweep( RAS_VAR ) ) return ras.error;
        ras.band_top--;
      }
    }

    return 0;  /* success */
  }


#if 0
  void  Compute_BBox( FT_Outline*  outline,
                      FT_BBox*     bbox )
  {
    int         n;
    FT_Vector*  vec;
    
    vec   = outline->points;
    bbox->xMin = bbox->xMax = vec->x;
    bbox->yMin = bbox->yMax = vec->y;

    n = outline->n_points-1;
    while ( n > 0 )
    {
      FT_Pos  x, y;
      
      vec++;     
       
      x = vec->x;
      if ( bbox->xMin > x ) bbox->xMin = x;
      if ( bbox->xMax < x ) bbox->xMax = x;
      
      y = vec->y;
      if ( bbox->yMin > y ) bbox->yMin = y;
      if ( bbox->yMax < y ) bbox->yMax = y;
      
      n--;
    }
  }
#endif

  static
  int  Raster_Render1( FT_Raster       raster )
  {
    int  error;
    long byte_len;

    byte_len = ras.target.pitch;
    if (byte_len < 0) byte_len = -byte_len;
    
    if ( ras.target.width > byte_len*8 )
      return ErrRaster_Invalid_Map;
    
    ras.scale_shift  = (ras.precision_bits-6);

    /* Vertical Sweep */
    ras.band_top            = 0;
    ras.band_stack[0].y_min = 0;
    ras.band_stack[0].y_max = ras.target.rows - 1;

    ras.Proc_Sweep_Init = Vertical_Sweep_Init;
    ras.Proc_Sweep_Span = Vertical_Sweep_Span;
    ras.Proc_Sweep_Step = Vertical_Sweep_Step;
    ras.Proc_Test_Pixel = Vertical_Test_Pixel;
    ras.Proc_Set_Pixel  = Vertical_Set_Pixel;

    ras.bit_width  = ras.target.width;
    ras.bit_buffer = (unsigned char*)ras.target.buffer;

    if ( (error = Render_Single_Pass( RAS_VARS 0 )) != 0 )
      return error;

    /* Horizontal Sweep */

    if ( ras.second_pass && ras.dropout_mode != 0 )
    {
      ras.Proc_Sweep_Init = Horizontal_Sweep_Init;
      ras.Proc_Sweep_Span = Horizontal_Sweep_Span;
      ras.Proc_Sweep_Step = Horizontal_Sweep_Step;
      ras.Proc_Test_Pixel = Horizontal_Test_Pixel;
      ras.Proc_Set_Pixel  = Horizontal_Set_Pixel;

      ras.band_top            = 0;
      ras.band_stack[0].y_min = 0;
      ras.band_stack[0].y_max = ras.target.width - 1;

      if ( (error = Render_Single_Pass( RAS_VARS  1 )) != 0 )
        return error;
    }

    return ErrRaster_Ok;
  }



#ifdef FT_RASTER_OPTION_ANTI_ALIAS
  static
  int  Raster_Render8( FT_Raster       raster )
  {
    int  error;
    long byte_len;
    
    byte_len = ras.target.pitch;
    if (byte_len < 0) byte_len = -byte_len;
    
    if ( ras.target.width > byte_len )
      return ErrRaster_Invalid_Map;

    /* Vertical Sweep */
    ras.band_top            = 0;
    ras.band_stack[0].y_min = 0;
    ras.band_stack[0].y_max = ras.target.rows;

#if !defined(FT_RASTER_ANTI_ALIAS_5) && !defined(FT_RASTER_ANTI_ALIAS_17)
    return ErrRaster_Unimplemented;
#endif

#ifdef FT_RASTER_ANTI_ALIAS_5
    if ( ras.grays_count == 5 )
    {
      ras.scale_shift  = (ras.precision_bits-5);
      ras.bit_width    = ras.gray_width/2;
      if ( ras.bit_width > (ras.target.width+3)/4 )
        ras.bit_width = (ras.target.width+3)/4;
    
      ras.bit_width  = ras.bit_width * 8;
      ras.bit_buffer = (unsigned char*)ras.gray_lines;
      ras.pix_buffer = (unsigned char*)ras.target.buffer;
    
      ras.Proc_Sweep_Init = Vertical_Gray5_Sweep_Init;
      ras.Proc_Sweep_Span = Vertical_Gray5_Sweep_Span;
      ras.Proc_Sweep_Step = Vertical_Gray5_Sweep_Step;
      ras.Proc_Test_Pixel = Vertical_Gray5_Test_Pixel;
      ras.Proc_Set_Pixel  = Vertical_Gray5_Set_Pixel;
    }
#endif

#ifdef FT_RASTER_ANTI_ALIAS_17
    if ( ras.grays_count == 17 )
    {
      ras.scale_shift  = (ras.precision_bits-4);
      ras.bit_width    = ras.gray_width/4;
      if ( ras.bit_width > (ras.target.width+1)/2 )
        ras.bit_width = (ras.target.width+1)/2;
    
      ras.bit_width  = ras.bit_width * 8;
      ras.bit_buffer = (unsigned char*)ras.gray_lines;
      ras.pix_buffer = (unsigned char*)ras.target.buffer;
    
      ras.Proc_Sweep_Init = Vertical_Gray17_Sweep_Init;
      ras.Proc_Sweep_Span = Vertical_Gray17_Sweep_Span;
      ras.Proc_Sweep_Step = Vertical_Gray17_Sweep_Step;
      ras.Proc_Test_Pixel = Vertical_Gray17_Test_Pixel;
      ras.Proc_Set_Pixel  = Vertical_Gray17_Set_Pixel;
    }
#endif

    error = Render_Single_Pass( RAS_VARS  0 );
    if (error)
      return error;

    /* Horizontal Sweep */

    if ( ras.second_pass && ras.dropout_mode != 0 )
    {
#ifdef FT_RASTER_ANTI_ALIAS_5
      if ( ras.grays_count == 5 )
      {
        ras.Proc_Sweep_Init = Horizontal_Sweep_Init;
        ras.Proc_Sweep_Span = Horizontal_Gray5_Sweep_Span;
        ras.Proc_Sweep_Step = Horizontal_Sweep_Step;
        ras.Proc_Test_Pixel = Horizontal_Gray5_Test_Pixel;
        ras.Proc_Set_Pixel  = Horizontal_Gray5_Set_Pixel;
      }
#endif
      
#ifdef FT_RASTER_ANTI_ALIAS_17
      if ( ras.grays_count == 17 )
      {
        ras.Proc_Sweep_Init = Horizontal_Sweep_Init;
        ras.Proc_Sweep_Span = Horizontal_Gray17_Sweep_Span;
        ras.Proc_Sweep_Step = Horizontal_Sweep_Step;
        ras.Proc_Test_Pixel = Horizontal_Gray17_Test_Pixel;
        ras.Proc_Set_Pixel  = Horizontal_Gray17_Set_Pixel;
      }
#endif
      ras.band_top            = 0;
      ras.band_stack[0].y_min = 0;
      ras.band_stack[0].y_max = ras.target.width-1;

      error = Render_Single_Pass( RAS_VARS  1 );
      if (error)
        return error;
    }

    return ErrRaster_Ok;
  }

#else  /* ANTI_ALIAS */

  static
  int  Raster_Render8( FT_Raster       raster )
  {
    return ErrRaster_Unimplemented;
  }

#endif /* FT_RASTER_OPTION_ANTI_ALIAS */



#ifdef FT_RASTER_OPTION_ANTI_ALIAS
/****************************************************************************/
/*                                                                          */
/* <Function>   Reset_Palette_5                                             */
/*                                                                          */
/* <Description> Resets lookup table when the 5-gray-levels palette changes */
/*                                                                          */
/****************************************************************************/

static
void  Reset_Palette_5( RAS_ARG )
{
  int  i;

  for ( i = 0; i < 256; i++ )
  {
    int  cnt1, cnt2;
    
    cnt1 = ((i & 128) >> 7) +
           ((i & 64)  >> 6) +
           ((i & 8)   >> 3) +
           ((i & 4)   >> 2);
           
    cnt2 = ((i & 32) >> 5) +
           ((i & 16) >> 4) +
           ((i & 2)  >> 1) +
            (i & 1);

  /*                                                                 */
  /* Note that when the endianess isn't specified through one of the */
  /* configuration, we use the big-endian storage in 'count_table'   */
  /*                                                                 */

#if defined( FT_RASTER_LITTLE_ENDIAN )
    ras.count_table[i] = (ras.grays[cnt2] << 8) | ras.grays[cnt1];
#else
    ras.count_table[i] = (ras.grays[cnt1] << 8) | ras.grays[cnt2];
#endif
  }
}

/****************************************************************************/
/*                                                                          */
/* <Function>    Reset_Palette_17                                           */
/*                                                                          */
/* <Description> Resets lookup table when 17-gray-levels palette changes    */
/*                                                                          */
/****************************************************************************/

#ifdef FT_RASTER_ANTI_ALIAS_17
static
void  Reset_Palette_17( RAS_ARG )
{
  int  i;

  for ( i = 0; i < 256; i++ )
    ras.count_table[i] = 
           ((i & 128) >> 7) +
           ((i & 64)  >> 6) +
           ((i & 8)   >> 3) +
           ((i & 4)   >> 2) +
           ((i & 32)  >> 5) +
           ((i & 16)  >> 4) +
           ((i & 2)   >> 1) +
            (i & 1);
}
#endif /* ANTI_ALIAS_17 */

#endif /* TT_RASTER_OPTION_ANTI_ALIAS */



  /**********************************************************************/
  /*                                                                    */
  /* <Function> FT_Raster_SetPalette                                    */
  /*                                                                    */
  /* <Description>                                                      */
  /*     Set the pixmap rendering palette. anti-aliasing modes are      */
  /*     implemented/possible, they differ from the number of           */
  /*     entries in the palette.                                        */
  /*                                                                    */
  /* <Input>                                                            */
  /*     count   :: the number of palette entries. Valid values are     */
  /*                2, 5 and 17, which are the number of intermediate   */
  /*                gray levels supported                               */
  /*                                                                    */
  /*     palette :: an array of 'count' chars giving the 8-bit palette  */
  /*                of intermediate "gray" levels for anti-aliased      */
  /*                rendering.                                          */
  /*                                                                    */
  /*         In all modes, palette[0] corresponds to the background,    */
  /*         while palette[count-1] to the foreground. Hence, a count   */
  /*         of 2 corresponds to no anti-aliasing; a count of 5 uses    */
  /*         3 intermediate levels between the background and           */
  /*         foreground, while a count of 17 uses 15 of them..          */
  /*                                                                    */
  /* <Return>                                                           */
  /*     An error code, used as a FT_Error by the FreeType library.     */
  /*                                                                    */
  /* <Note>                                                             */
  /*     By default, a new object uses mode 5, with a palette of        */
  /*     0,1,2,3 and 4. You don't need to set the palette if you        */
  /*     don't need to render pixmaps..                                 */
  /*                                                                    */
  /**********************************************************************/

  EXPORT_FUNC
  int   FT_Raster_SetPalette( FT_Raster    raster,
                              int          count,
                              const char*  palette )
  {
    switch (count)
    {
#ifdef FT_RASTER_OPTION_ANTI_ALIAS

      /******************************/
      /* The case of 17 gray levels */
      /******************************/
    
      case 17:
#ifdef FT_RASTER_ANTI_ALIAS_17
      {
        int  n;
        
        raster->grays_count = count;
        for ( n = 0; n < count; n++ )
          raster->grays[n] = (unsigned char)palette[n];
        Reset_Palette_17( RAS_VAR );
        break;
      }
#else
      return ErrRaster_Unimplemented;
#endif

      /*****************************/
      /* The case of 5 gray levels */
      /*****************************/

      case 5:      
#ifdef FT_RASTER_ANTI_ALIAS_5
      {
        int  n;
        
        raster->grays_count = count;
        for ( n = 0; n < count; n++ )
          raster->grays[n] = (unsigned char)palette[n];
        Reset_Palette_5( RAS_VAR );
        break;
      }
#else
      return ErrRaster_Unimplemented;
#endif

#endif /* FT_RASTER_OPTION_ANTI_ALIAS */
      default:
        return ErrRaster_Bad_Palette_Count;
    }

    return ErrRaster_Ok;
  }  


  /**** RASTER OBJECT CREATION : in standalone mode, we simply use *****/
  /****                          a static object ..                *****/
#ifdef _STANDALONE_

  static
  int  ft_black2_new( void*  memory, FT_Raster *araster )
  {
     static FT_RasterRec_  the_raster;
     *araster = &the_raster;  
     memset( &the_raster, sizeof(the_raster), 0 );
     return 0;
  }

  static
  void  ft_black2_done( FT_Raster  raster )
  {
    /* nothing */
    raster->init = 0;
  }

#else

#include <ftobjs.h>

  static
  int  ft_black2_new( FT_Memory  memory, FT_Raster  *araster )
  {
    FT_Error   error;
    FT_Raster  raster;
    
    *araster = 0;
    if ( !ALLOC( raster, sizeof(*raster) ))
    {
      raster->memory = memory;
      *araster = raster;
    }
      
    return error;
  }
  
  static
  void ft_black2_done( FT_Raster  raster )
  {
    FT_Memory  memory = (FT_Memory)raster->memory;
    FREE( raster );
  }
  
#endif


  static void ft_black2_reset( FT_Raster         raster,
                               const char*       pool_base,
                               long              pool_size )
  {
    static const char  default_palette[5] = { 0, 1, 2, 3, 4 };
  
    /* check the object address */
    if ( !raster )
      return;
      
    /* check the render pool - we won't go under 4 Kb */
    if ( !pool_base || pool_size < 4096 )
      return;
    
    /* save the pool */
    raster->pool      = (PPos)pool_base;
    raster->pool_size = (PPos)(pool_base + (pool_size & -8));

#ifdef FT_RASTER_OPTION_ANTI_ALIAS
    raster->gray_width = ANTI_ALIAS_BUFFER_SIZE/2;
    /* clear anti-alias intermediate lines */
    {
      char*  p    = raster->gray_lines;
      int    size = ANTI_ALIAS_BUFFER_SIZE;
      do
      {
        *p++ = 0;
        size--;

      } while (size > 0);
    }
#endif
    
    /* set the default palette : 5 levels =  0, 1, 2, 3 and 4 */
    FT_Raster_SetPalette( raster, 5, default_palette );
  }


  static
  int  ft_black2_render( FT_Raster          raster,
                         FT_Raster_Params*  params )
  {
    FT_Outline*  outline    = (FT_Outline*)params->source;
    FT_Bitmap*   target_map = params->target;
    
    if ( !raster || !raster->pool || !raster->pool_size )
      return ErrRaster_Uninitialised_Object;

    if ( !outline || !outline->contours || !outline->points )
      return ErrRaster_Invalid_Outline;

    /* return immediately if the outline is empty */
    if ( outline->n_points == 0 || outline->n_contours <= 0 )
      return ErrRaster_Invalid_Outline;

    if ( outline->n_points != outline->contours[outline->n_contours - 1] + 1 )
      return ErrRaster_Invalid_Outline;

    if ( !target_map || !target_map->buffer )
      return ErrRaster_Invalid_Map;

    /* this version of the raster does not support direct rendering, sorry */
    if ( params->flags & ft_raster_flag_direct )
      return ErrRaster_Unimplemented;

    ras.outline  = outline;
    ras.target   = *target_map;

    ras.dropout_mode = 2;
    ras.second_pass  = !(outline->flags & ft_outline_single_pass);
    SET_High_Precision( (outline->flags & ft_outline_high_precision) );

    return ( params->flags & ft_raster_flag_aa
           ? Raster_Render8(raster)
           : Raster_Render1(raster) );
  }


  FT_Raster_Funcs      ft_black2_raster =
  {
    ft_glyph_format_outline,
    (FT_Raster_New_Func)       ft_black2_new,
    (FT_Raster_Reset_Func)     ft_black2_reset,
    (FT_Raster_Set_Mode_Func)  0,
    (FT_Raster_Render_Func)    ft_black2_render,
    (FT_Raster_Done_Func)      ft_black2_done
  };


