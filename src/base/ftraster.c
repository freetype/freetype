/***************************************************************************/
/*                                                                         */
/*  ftraster.c                                                             */
/*                                                                         */
/*    The FreeType glyph rasterizer (body).                                */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* The `raster' component implements FreeType's scan-line converter, the */
  /* one used to generate bitmaps and pixmaps from vectorial outline       */
  /* descriptions.                                                         */
  /*                                                                       */
  /* It has been rewritten entirely for FreeType 2.0, in order to become   */
  /* completely independent of the rest of the library.  It should now be  */
  /* possible to include it more easily in all kinds of libraries and      */
  /* applications, which do not necessarily need the font engines and API. */
  /*                                                                       */
  /* This version contains the following features:                         */
  /*                                                                       */
  /* - Support for third-order Bezier arcs.                                */
  /*                                                                       */
  /* - Improved performance of the 5-levels anti-aliasing algorithm.       */
  /*                                                                       */
  /* - 17-levels anti-aliasing for smoother curves, though the difference  */
  /*   isn't always noticeable, depending on your palette.                 */
  /*                                                                       */
  /* - An API to decompose a raster outline into a path (i.e., into a      */
  /*   a series of segments and arcs).                                     */
  /*                                                                       */
  /* Planned additions:                                                    */
  /*                                                                       */
  /* - Getting rid of the second pass for horizontal drop-out detection.   */
  /*   I've got a few ideas, but I'll have to experiment in Pascal with    */
  /*   them.  to avoid damaging of the rendering of glyphs at small sizes. */
  /*                                                                       */
  /* - Adding a `composition' callback, which should be invoked during     */
  /*   anti-aliased rendering.  In short, it will allow line-by-line       */
  /*   composition (i.e., transparencies, etc.) of the output in a fairly  */
  /*   portable way.  Of course, a single sweep is required there.         */
  /*                                                                       */
  /*************************************************************************/

#define OLD


#define xxxDEBUG_RAS
#ifdef DEBUG_RAS
#include <stdio.h>
#endif


#include <ftraster.h>
#ifndef _STANDALONE_
#include <ftconfig.h>
#include <ftdebug.h>
#endif

#ifndef EXPORT_FUNC
#define EXPORT_FUNC  /* nothing */
#endif

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_raster

#ifdef _STANDALONE_

  /*************************************************************************/
  /*                                                                       */
  /* The following defines are used when the raster is compiled as a       */
  /* stand-alone object.  Each of them is commented, and you're free to    */
  /* toggle them to suit your needs.                                       */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* FT_RASTER_INT_IS_32                                                   */
  /*                                                                       */
  /*   Set this configuration macro to the unsigned type which has 32      */
  /*   bits.                                                               */
  /*                                                                       */
#define FT_RASTER_INT_IS_32


  /*************************************************************************/
  /*                                                                       */
  /* FT_RASTER_OPTION_ANTI_ALIAS                                           */
  /*                                                                       */
  /*   Define this configuration macro if you want to support              */
  /*   anti-aliasing.                                                      */
  /*                                                                       */
#undef  FT_RASTER_OPTION_ANTI_ALIAS


  /*************************************************************************/
  /*                                                                       */
  /* FT_RASTER_OPTION_CONIC_BEZIERS                                        */
  /*                                                                       */
  /*   Define this configuration macro if your source outlines contain     */
  /*   second-order Bezier arcs.  Typically, these are TrueType outlines.  */
  /*                                                                       */
#define FT_RASTER_CONIC_BEZIERS


  /*************************************************************************/
  /*                                                                       */
  /* FT_RASTER_OPTION_CUBIC_BEZIERS                                        */
  /*                                                                       */
  /*   Define this configuration macro if your source outlines contain     */
  /*   third-order Bezier arcs.  Typically, these are Type1 outlines.      */
  /*                                                                       */
#define FT_RASTER_CUBIC_BEZIERS


  /*************************************************************************/
  /*                                                                       */
  /* FT_RASTER_CONSTANT_PRECISION                                          */
  /*                                                                       */
  /*   Define this configuration macro if you want to use a constant       */
  /*   precision for the internal sub-pixel coordinates. Otherwise, the    */
  /*   precision is either 64 or 1024 units per pixel, depending on the    */
  /*   outline's "high_precision" flag..                                   */
  /*                                                                       */
  /*   This results in a speed boost, but the macro can be undefined if    */
  /*   it results in rendering errors (mainly changed drop-outs)..         */
  /*                                                                       */
#undef  FT_RASTER_CONSTANT_PRECISION


  /*************************************************************************/
  /*                                                                       */
  /* FT_PRECISION_BITS                                                     */
  /*                                                                       */
  /*   When the macro FT_RASTER_CONSTANT_PRECISION is defined, this        */
  /*   constant holds the number of bits used for the internal sub-pixels  */
  /*                                                                       */
  /*   This number should be at least 6, but use at least 8 if you         */
  /*   intend to generate small glyph images (use 6 for a printer, for     */
  /*   example..)                                                          */
  /*                                                                       */
#define FT_PRECISION_BITS 8


  /*************************************************************************/
  /*                                                                       */
  /* FT_DYNAMIC_BEZIER_STEPS                                               */
  /*                                                                       */
  /*   Set this macro to enable the bezier decomposition to be             */
  /*   dynamically computed. This is interesting when the precision is     */
  /*   constant, as it speeds things a bit while keeping a very good       */
  /*   accuracy on the bezier intersections..                              */
  /*                                                                       */
#undef  FT_DYNAMIC_BEZIER_STEPS


#else /* _STANDALONE_ */

#include <freetype.h>
#include <ftconfig.h>

  /*************************************************************************/
  /*                                                                       */
  /* The following defines are used when the raster is compiled within the */
  /* FreeType base layer.  Don't change these unless you really know what  */
  /* you're doing.                                                         */
  /*                                                                       */
  /*************************************************************************/


#ifdef FT_CONFIG_OPTION_ANTI_ALIAS
#define FT_RASTER_OPTION_ANTI_ALIAS
#endif

#define FT_RASTER_CONIC_BEZIERS
#define FT_RASTER_CUBIC_BEZIERS

#define FT_RASTER_ANTI_ALIAS_5
/* #define  FT_RASTER_ANTI_ALIAS_17 */

#ifdef FT_CONFIG_OPTION_LITTLE_ENDIAN
#define FT_RASTER_LITTLE_ENDIAN
#endif

#ifdef FT_CONFIG_OPTION_BIG_ENDIAN
#define FT_RASTER_BIG_ENDIAN
#endif

#undef  FT_RASTER_CONSTANT_PRECISION
#undef  FT_DYNAMIC_BEZIER_STEPS
#define FT_PRECISION_BITS    8

#endif /* _STANDALONE_ */


/* to keep the compiler happy */
#ifndef FT_TRACE2
#define FT_TRACE2(x)  /*void*/
#endif

#ifndef FT_TRACE4
#define FT_TRACE4(x)  /* void */
#endif

  /*************************************************************************/
  /*                                                                       */
  /* FT_RASTER_ANY_ENDIAN indicates that no endianess was defined by one   */
  /* of the configuration macros.                                          */
  /*                                                                       */
#if !defined( FT_RASTER_LITTLE_ENDIAN ) && !defined( FT_RASTER_BIG_ENDIAN )
#define FT_RASTER_ANY_ENDIAN
#endif


  /*************************************************************************/
  /*                                                                       */
  /* The rasterizer is a very general purpose component.  Please leave the */
  /* following redefinitions here (you never know your target              */
  /* environment).                                                         */
  /*                                                                       */
  /*************************************************************************/

#ifndef TRUE
#define TRUE   1
#endif

#ifndef FALSE
#define FALSE  0
#endif

#ifndef NULL
#define NULL  (void*)0
#endif


#ifndef UNUSED
#define UNUSED( arg )  ( (void)(arg) )
#endif


#undef  FAILURE
#define FAILURE  TRUE

#undef  SUCCESS
#define SUCCESS  FALSE

#ifndef ABS
#define ABS(x)  ( (x) < 0 ? -(x) : (x) )
#endif

  /*************************************************************************/
  /*                                                                       */
  /* Please don't touch the following macros. Their importance is          */
  /* historical to FreeType, but they have some nice effects, like getting */
  /* rid of all `->' symbols when accessing the raster object (replacing   */
  /* them with a simple `.').                                              */
  /*                                                                       */
  /*************************************************************************/

  /* used in function signatures to define the _first_ argument */
#define RAS_ARG_  FT_Raster  raster,
#define RAS_ARG   FT_Raster  raster

  /* used to call a function within this component, first parameter */
#define RAS_VAR_  raster,
#define RAS_VAR   raster

  /* used to access the current raster object, with a `.' instead of a */
  /* `->'                                                              */
#define ras       (*raster)

#define           UNUSED_RASTER   (void)raster;

  /*************************************************************************/
  /*                                                                       */
  /* Error codes returned by the scan-line converter/raster.               */
  /*                                                                       */
#define ErrRaster_Ok                     0
#define ErrRaster_Uninitialized_Object   1
#define ErrRaster_Overflow               2
#define ErrRaster_Negative_Height        3
#define ErrRaster_Invalid_Outline        4
#define ErrRaster_Invalid_Map            5
#define ErrRaster_AntiAlias_Unsupported  6
#define ErrRaster_Invalid_Pool           7
#define ErrRaster_Unimplemented          8
#define ErrRaster_Bad_Palette_Count      9


#define Flow_Up     1
#define Flow_Down  -1

#define SET_High_Precision( p )  Set_High_Precision( RAS_VAR_  p )


  /*************************************************************************/
  /*                                                                       */
  /* Fast MulDiv, as `b' is always < 64.  Don't use intermediate           */
  /* precision.                                                            */
  /*                                                                       */
#define FMulDiv( a, b, c )  ( (a) * (b) / (c) )


  /*************************************************************************/
  /*                                                                       */
  /* Define DEBUG_RASTER if you want to generate a debug version of the    */
  /* rasterizer.  This will progressively draw the glyphs while all the    */
  /* computation are done directly on the graphics screen (the glyphs will */
  /* will be shown inverted).                                              */
  /*                                                                       */
  /* Note that DEBUG_RASTER should only be used for debugging with b/w     */
  /* rendering, not with gray levels.                                      */
  /*                                                                       */
  /* The definition of DEBUG_RASTER should appear in the file              */
  /* `ftconfig.h'.                                                         */
  /*                                                                       */
#ifdef DEBUG_RASTER
  extern char*  vio;  /* A pointer to VRAM or display buffer */
#endif


  /*************************************************************************/
  /*                                                                       */
  /* The maximum number of stacked Bezier curves.  Setting this constant   */
  /* to more than 32 is a pure waste of space.                             */
  /*                                                                       */
#define MaxBezier  32


  /*************************************************************************/
  /*                                                                       */
  /* The number fractional bits of *input* coordinates.  We always use the */
  /* 26.6 format (i.e, 6 bits for the fractional part), but hackers are    */
  /* free to experiment with different values.                             */
  /*                                                                       */
#define INPUT_BITS  6


  /*************************************************************************/
  /*                                                                       */
  /* An unsigned type that is exactly 32 bits on your platform.  This      */
  /* means `unsigned long' on 16-bit machines, and `unsigned int' on       */
  /* others.                                                               */
  /*                                                                       */
#ifdef _STANDALONE_
#if defined( FT_RASTER_INT_IS_32 )
  typedef unsigned int   FT_Word32;
#elif defined( FT_RASTER_LONG_IS_32 )
  typedef unsigned long  FT_Word32;
#else
#error "no 32bit type found - please check your configuration"
#endif
#endif


  /*************************************************************************/
  /*                                                                       */
  /* A pointer to an unsigned char.                                        */
  /*                                                                       */
  typedef unsigned char Byte, *PByte;
  typedef int           TResult;


  /*************************************************************************/
  /*                                                                       */
  /* The type of the pixel coordinates used within the render pool during  */
  /* scan-line conversion.  We use longs to store either 26.6 or 22.10     */
  /* fixed float values, depending on the `precision' we want to use       */
  /* (i.e., low resp. high precision).  These are ideals in order to       */
  /* subdivise Bezier arcs in halves by simple additions and shifts.       */
  /*                                                                       */
  /* Note that this is an 8-bytes integer on 64 bits systems.              */
  /*                                                                       */
  typedef long  TPos, *PPos;


  /*************************************************************************/
  /*                                                                       */
  /* The type of a scanline position/coordinate within a map.              */
  /*                                                                       */
  typedef int  TScan, *PScan;


  /*************************************************************************/
  /*                                                                       */
  /* States and directions of each line, arc, and profile.                 */
  /*                                                                       */
  typedef enum  _TDirection
  {
    Unknown,
    Ascending,
    Descending,
    Flat

  } TDirection;


  struct  _TProfile;
  typedef struct _TProfile  TProfile;
  typedef TProfile*         PProfile;


  /*************************************************************************/
  /*                                                                       */
  /* The `master' structure used for decomposing outlines.                 */
  /*                                                                       */
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


  /*************************************************************************/
  /*                                                                       */
  /* A simple record used to implement a stack of bands, required by the   */
  /* sub-banding mechanism.                                                */
  /*                                                                       */
  typedef struct  _TBand
  {
    TScan  y_min;   /* band's minimum */
    TScan  y_max;   /* band's maximum */

  } TBand;


  /*************************************************************************/
  /*                                                                       */
  /* The size in _TPos_ of a profile record in the render pool.            */
  /*                                                                       */
#define AlignProfileSize  \
          ( (sizeof ( TProfile ) + sizeof ( TPos ) - 1) / sizeof ( TPos ) )


  /*************************************************************************/
  /*                                                                       */
  /* Prototypes used for sweep function dispatch.                          */
  /*                                                                       */
  typedef void  (*Function_Sweep_Init)( RAS_ARG_ int*  min,
                                                 int*  max );

  typedef void  (*Function_Sweep_Span)( RAS_ARG_ TScan  y,
                                                 TPos   x1,
                                                 TPos   x2 );

  typedef int   (*Function_Test_Pixel)( RAS_ARG_ TScan  y,
                                                 int    x );

  typedef void  (*Function_Set_Pixel)( RAS_ARG_  TScan  y,
                                                 int    x,
                                                 int    color );

  typedef void  (*Function_Sweep_Step)( RAS_ARG );

  typedef struct Raster_Render_
  {
    Function_Sweep_Init  init;
    Function_Sweep_Span  span;
    Function_Sweep_Step  step;
    Function_Test_Pixel  test_pixel;
    Function_Set_Pixel   set_pixel;

  } Raster_Render;


#ifdef FT_RASTER_CONSTANT_PRECISION

  #define PRECISION_BITS    FT_PRECISION_BITS
  #define PRECISION         (1 << PRECISION_BITS)
  #define PRECISION_MASK    (-1L << PRECISION_BITS)
  #define PRECISION_HALF    (PRECISION >> 1)
  #define PRECISION_JITTER  (PRECISION >> 5)
  #define PRECISION_STEP    PRECISION_HALF

#else

  #define PRECISION_BITS    ras.precision_bits
  #define PRECISION         ras.precision
  #define PRECISION_MASK    ras.precision_mask
  #define PRECISION_HALF    ras.precision_half
  #define PRECISION_JITTER  ras.precision_jitter
  #define PRECISION_STEP    ras.precision_step

#endif

  /*************************************************************************/
  /*                                                                       */
  /* Compute lowest integer coordinate below a given value.                */
  /*                                                                       */
#define FLOOR( x )  ( (x) & PRECISION_MASK )


  /*************************************************************************/
  /*                                                                       */
  /* Compute highest integer coordinate above a given value.               */
  /*                                                                       */
#define CEILING( x )  ( ((x) + PRECISION - 1) & PRECISION_MASK )


  /*************************************************************************/
  /*                                                                       */
  /* Get integer coordinate of a given 26.6 or 22.10 `x' coordinate -- no  */
  /* rounding.                                                             */
  /*                                                                       */
#define TRUNC( x )  ( (signed long)(x) >> PRECISION_BITS )


  /*************************************************************************/
  /*                                                                       */
  /* Get the fractional part of a given coordinate.                        */
  /*                                                                       */
#define FRAC( x )  ( (x) & (PRECISION-1) )


  /*************************************************************************/
  /*                                                                       */
  /* Scale an `input coordinate' (as found in FT_Outline structures) into  */
  /* a `work coordinate' which depends on current resolution and render    */
  /* mode.                                                                 */
  /*                                                                       */
#define SCALED( x )  ( ((x) << ras.scale_shift) - ras.scale_delta )


  /*************************************************************************/
  /*                                                                       */
  /* DEBUG_PSET is used to plot a single pixel in VRam during debug mode.  */
  /*                                                                       */
#ifdef DEBUG_RASTER
#define DEBUG_PSET  Pset()
#else
#define DEBUG_PSET
#endif


  /*************************************************************************/
  /*                                                                       */
  /* This structure defines a point in a plane.                            */
  /*                                                                       */
  typedef struct  _TPoint
  {
    TPos  x, y;

  } TPoint;


  /*************************************************************************/
  /*                                                                       */
  /* The most used variables are at the beginning of the structure.  Thus, */
  /* their offset can be coded with less opcodes which results in a        */
  /* smaller executable.                                                   */
  /*                                                                       */
  struct  FT_RasterRec_
  {
    PPos      cursor;              /* Current cursor in render pool  */

    PPos      pool;                /* The render pool base address   */
    PPos      pool_size;           /* The render pool's size         */
    PPos      pool_limit;          /* Limit of profiles zone in pool */

    int       bit_width;           /* target bitmap width  */
    PByte     bit_buffer;          /* target bitmap buffer */
    PByte     pix_buffer;          /* target pixmap buffer */

    TPoint    last;
    long      minY, maxY;

    int       error;

#ifndef FT_RASTER_CONSTANT_PRECISION
    int       precision_bits;       /* precision related variables */
    int       precision;
    int       precision_half;
    long      precision_mask;
    int       precision_shift;
    int       precision_step;
    int       precision_jitter;
#endif

    FT_Outline*  outline;

    int       n_points;             /* number of points in current glyph   */
    int       n_contours;           /* number of contours in current glyph */
    int       n_extrema;            /* number of `extrema' scanlines       */

    TPoint*   arc;                  /* current Bezier arc pointer */

    int       num_profs;            /* current number of profiles */

    char      fresh;                /* signals a fresh new profile which */
                                    /* `start' field must be completed   */
    char      joint;                /* signals that the last arc ended   */
                                    /* exactly on a scanline.  Allows    */
                                    /* removal of doublets               */
    PProfile  cur_prof;             /* current profile                   */
    PProfile  start_prof;           /* head of linked list of profiles   */
    PProfile  first_prof;           /* contour's first profile in case   */
                                    /* of impact                         */
    TDirection  state;              /* rendering state */

    FT_Bitmap target;          /* description of target bit/pixmap */
    void*     memory;

    int       trace_bit;            /* current offset in target bitmap    */
    int       trace_pix;            /* current offset in target pixmap    */
    int       trace_incr;           /* sweep's increment in target bitmap */

    /* dispatch variables */

    Raster_Render     render;

    int       scale_shift;      /* == 0  for bitmaps           */
                                /* == 1  for 5-levels pixmaps  */
                                /* == 2  for 17-levels pixmaps */

    int       scale_delta;      /* ras.precision_half for bitmaps */
                                /* 0 for pixmaps                  */

    char      dropout_mode;     /* current drop_out control method */

    char      second_pass;      /* indicates whether a horizontal pass     */
                                /* should be performed to control drop-out */
                                /* accurately when calling Render_Glyph.   */
                                /* Note that there is no horizontal pass   */
                                /* during gray rendering.                  */

    char      flipped;          /* this flag is set during the rendering to */
                                /* indicate the second pass.                */

    TBand     band_stack[16];   /* band stack used for sub-banding */
    int       band_top;         /* band stack top                  */

    TPoint    arcs[2 * MaxBezier + 1];  /* The Bezier stack */
  };



#ifdef DEBUG_RASTER

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Pset                                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Used for debugging only.  Plots a point in VRAM during rendering   */
  /*    (not afterwards).                                                  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This procedure relies on the value of cProfile->start, which may   */
  /*    not be set when Pset() is called sometimes.  This will usually     */
  /*    result in a dot plotted on the first screen scanline (far away     */
  /*    from its original position).                                       */
  /*                                                                       */
  /*    This `feature' reflects nothing wrong in the current               */
  /*    implementation, and the bitmap is rendered correctly, so don't     */
  /*    panic if you see `flying' dots in debugging mode.                  */
  /*                                                                       */
  static
  void  Pset( RAS_ARG )
  {
    long  o;
    long  x;

    x = ras.cursor[-1];

    switch ( ras.cur_prof->flow )
    {
    case FT_Flow_Up:
      o = Vio_ScanLineWidth *
         ( ras.cursor - ras.cur_prof->offset + ras.cur_prof->start ) +
         ( x / (PRECISION * 8) );
      break;

    case FT_Flow_Down:
      o = Vio_ScanLineWidth *
         ( ras.cur_prof->start - ras.cursor + ras.cur_prof->offset ) +
         ( x / (PRECISION * 8) );
      break;
    }

    if ( o > 0 )
      Vio[o] |= (unsigned)0x80 >> ( (x/PRECISION) & 7 );
  }


  static
  void  Clear_Band( RAS_ARG_ TScan  y1,
                             TScan  y2 )
  {
    MEM_Set( Vio + y1*Vio_ScanLineWidth, (y2-y1+1)*Vio_ScanLineWidth, 0 );
  }

#endif /* DEBUG_RASTER */


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Set_High_Precision                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets precision variables according to the parameter flag.          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    High :: Set to True for high precision (typically for ppem < 18),  */
  /*    false otherwise.                                                   */
  /*                                                                       */
  static
  void  Set_High_Precision( RAS_ARG_  char  High )
  {
#ifdef FT_RASTER_CONSTANT_PRECISION
    (void)High;
    (void)&ras;
#else
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

    ras.precision       = 1L << ras.precision_bits;
    ras.precision_half  = ras.precision / 2;
    ras.precision_shift = ras.precision_bits - INPUT_BITS;
    ras.precision_mask  = -ras.precision;
#endif
  }

  /*************************************************************************/
  /*                                                                       */
  /* A simple technical note on how the raster works:                      */
  /*                                                                       */
  /*   Converting an outline into a bitmap is achieved in several steps    */
  /*   which are:                                                          */
  /*                                                                       */
  /*   1 - Decomposing the outline into successive `profiles'.  Each       */
  /*       profile is simply an array of scanline intersections on a given */
  /*       dimension.  A profile's main attributes are                     */
  /*                                                                       */
  /*       o its scanline position boundaries, i.e. `Ymin' and `Ymax'.     */
  /*                                                                       */
  /*       o an array of intersection coordinates for each scanline        */
  /*         between `Ymin' and `Ymax'.                                    */
  /*                                                                       */
  /*       o a direction, indicating wether is was built going `up' or     */
  /*         `down', as this is very important for filling rules.          */
  /*                                                                       */
  /*   2 - Sweeping the target map's scanlines in order to compute segment */
  /*       `spans' which are then filled.  Additionaly, this pass performs */
  /*       drop-out control.                                               */
  /*                                                                       */
  /*   The outline data is parsed during step 1 only.  The profiles are    */
  /*   built from the bottom of the render pool, used as a stack.  The     */
  /*   following graphics shows the profile list under construction:       */
  /*                                                                       */
  /*     ____________________________________________________________ _ _  */
  /*    |         |                   |         |                 |        */
  /*    | profile | coordinates for   | profile | coordinates for |-->     */
  /*    |    1    |  profile 1        |    2    |  profile 2      |-->     */
  /*    |_________|___________________|_________|_________________|__ _ _  */
  /*                                                                       */
  /*    ^                                                         ^        */
  /*    |                                                         |        */
  /*    start of render pool                                   cursor      */
  /*                                                                       */
  /*   The top of the profile stack is kept in the `cursor' variable.      */
  /*                                                                       */
  /*   As you can see, a profile record is pushed on top of the render     */
  /*   pool, which is then followed by its coordinates/intersections.  If  */
  /*   a change of direction is detected in the outline, a new profile is  */
  /*   generated until the end of the outline.                             */
  /*                                                                       */
  /*   Note that when all profiles have been generated, the function       */
  /*   Finalize_Profile_Table() is used to record, for each profile, its   */
  /*   bottom-most scanline as well as the scanline above its upmost       */
  /*   boundary.  These positions are called `extrema' because they (sort  */
  /*   of) correspond to local extrema.  They are stored in a sorted list  */
  /*   built from the top of the render pool as a downwards stack:         */
  /*                                                                       */
  /*      _ _ _______________________________________                      */
  /*                            |                    |                     */
  /*                         <--| sorted list of     |                     */
  /*                         <--|  extrema scanlines |                     */
  /*      _ _ __________________|____________________|                     */
  /*                                                                       */
  /*                            ^                    ^                     */
  /*                            |                    |                     */
  /*                       pool_limit        end of render pool            */
  /*                                                                       */
  /*   This list is later used during the sweep phase in order to          */
  /*   optimize performance (see technical note on the sweep below).       */
  /*                                                                       */
  /*   Of course, the raster detects whether the two stacks collide and    */
  /*   handles the situation propertly.                                    */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    New_Profile                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new Profile in the render pool.                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    aState :: The state/orientation of the new profile.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS or FAILURE.                                                */
  /*                                                                       */
  static
  TResult  New_Profile( RAS_ARG_ TDirection  direction )
  {
    if ( ras.start_prof == NULL )
    {
      ras.cur_prof   = (PProfile)ras.cursor; /* current profile          */
      ras.start_prof = ras.cur_prof;         /* first profile in pool    */
      ras.cursor    += AlignProfileSize;     /* record profile in buffer */
    }

    /* check for overflow */
    if ( ras.cursor >= ras.pool_limit )
    {
      ras.error = ErrRaster_Overflow;
      return FAILURE;
    }

    /* record profile direction */
    switch ( direction )
    {
    case Ascending:
      ras.cur_prof->flow = Flow_Up;
      break;

    case Descending:
      ras.cur_prof->flow = Flow_Down;
      break;

    default:
      ras.error = ErrRaster_Invalid_Map;
      return FAILURE;
    }

    /* initialize a few fields */
    {
      PProfile  cur = ras.cur_prof;


      cur->start  = 0;            /* current start scanline          */
      cur->height = 0;            /* current height                  */
      cur->offset = ras.cursor;   /* address of first coordinate     */
      cur->link   = (PProfile)0;  /* link to next profile in pool    */
      cur->next   = (PProfile)0;  /* link to next profile in contour */
    }

    /* record the first profile in a contour */
    if ( ras.first_prof == NULL )
      ras.first_prof = ras.cur_prof;

    ras.state  = direction;
    ras.fresh  = TRUE;       /* this profile has no coordinates yet */
    ras.joint  = FALSE;

    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    End_Profile                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finalizes the current Profile and computes its height.  If it is   */
  /*    not 0, the profile's fields are updated and a new profile is       */
  /*    pushed on top of its coordinates.  Otherwise the current profile   */
  /*    is kept and the recording of intersections is restarted.           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS or FAILURE.                                                */
  /*                                                                       */
  static
  TResult  End_Profile( RAS_ARG )
  {
    int  h;


    h = ras.cursor - ras.cur_prof->offset;

    if ( h < 0 )
    {
      /* This error should _never_ occur unless the raster is buggy */
      ras.error = ErrRaster_Negative_Height;
      return FAILURE;
    }

    if ( h > 0 )
    {
      PProfile  old, new;

      /* record scanline height in current profile, create a new one */
      /* and set a link from the old one to it                       */
      old          = ras.cur_prof;
      old->height  = h;
      ras.cur_prof = new = (PProfile)ras.cursor;

      ras.cursor  += AlignProfileSize;

      new->height  = 0;
      new->offset  = ras.cursor;
      old->next    = new;

      ras.num_profs++;
    }

    /* check for overflow */
    if ( ras.cursor >= ras.pool_limit )
    {
      ras.error = ErrRaster_Overflow;
      return FAILURE;
    }

    ras.joint = FALSE;

    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Insert_Extrema                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Records that a given scanline contains at least one local          */
  /*    extremum.  The table of extrema is placed at the end of the render */
  /*    pool and grows downwards.  It is used during the sweep phase.      */
  /*                                                                       */
  /* <Input>                                                               */
  /*     y :: The coordinate of the scanline containing an extremum.       */
  /*                                                                       */
  static
  TResult  Insert_Extrema( RAS_ARG_ TScan  y )
  {
    PPos   extrema;
    TScan  y2;
    int    n;


    FT_TRACE2(( "EXTREMA += %d", y ));
    n       = ras.n_extrema - 1;
    extrema = ras.pool_size - ras.n_extrema;

    /* look for first y extremum that is <= */
    while ( n >= 0 && y < extrema[n] )
      n--;

    /* if it is <, simply insert it, ignore if == */
    if ( n >= 0 && y > extrema[n] )
      while ( n >= 0 )
      {
        y2 = extrema[n];
        extrema[n] = y;
        y = y2;
        n--;
      }

    if ( n < 0 )
    {
      ras.pool_limit--;
      ras.n_extrema++;
      ras.pool_size[-ras.n_extrema] = y;

      if ( ras.pool_limit <= ras.cursor )
      {
        ras.error = ErrRaster_Overflow;
        return FAILURE;
      }
    }
    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Finalize_Profile_Table                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Adjusts all links in the profiles list.  Called when the outline   */
  /*    parsing is done.                                                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS or FAILURE.                                                */
  /*                                                                       */
  static
  TResult  Finalize_Profile_Table( RAS_ARG )
  {
    int       n, bottom, top;
    PProfile  p;


    n = ras.num_profs;

    if ( n > 1 )
    {
      p = ras.start_prof;
      while ( n > 0 )
      {
        if ( n > 1 )
          p->link = (PProfile)( p->offset + p->height );
        else
          p->link = NULL;

        switch ( p->flow )
        {
          case Flow_Down:
            FT_TRACE2(( "FLOW DOWN (start = %d, height = %d)",
                      p->start, p->height ));
            bottom     = p->start - p->height+1;
            top        = p->start;
            p->start   = bottom;
            p->offset += p->height-1;
            break;

          case Flow_Up:
          default:
            FT_TRACE2(( "FLOW UP (start = %d, height = %d)",
                      p->start, p->height ));
            bottom = p->start;
            top    = p->start + p->height-1;
        }

        if ( Insert_Extrema( RAS_VAR_  bottom ) ||
             Insert_Extrema( RAS_VAR_  top+1 )  )
          return FAILURE;

        p = p->link;
        n--;
      }
    }
    else
      ras.start_prof = NULL;

    return SUCCESS;
  }

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****         COMPUTE SCAN-LINE INTERSECTIONS FROM SEGMENTS           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/



  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Line_Up                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Computes the scan-line intersections of an ascending line segment  */
  /*    and stores them in the render pool.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    x1   :: The start x coordinate.                                    */
  /*    y1   :: The start y coordinate.                                    */
  /*    x2   :: The end x coordinate.                                      */
  /*    y2   :: The end y coordinate.                                      */
  /*    miny :: The minimum vertical grid coordinate.                      */
  /*    maxy :: The maximum vertical grid coordinate.                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS or FAILURE.                                                */
  /*                                                                       */
  static
  TResult  Line_Up( RAS_ARG_ TPos  x1,   TPos  y1,
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

    /* clip to higher scanline when necessary */
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

    /* clip to lower scanline when necessary */
    if ( y1 < miny )
    {
#ifdef OLD
      x1 += FT_MulDiv( Dx, miny-y1, Dy );
      e1  = TRUNC( miny );
      f1  = 0;
#else    
      TPos  x, y;

      /* we use a binary search to compute the lower
      // clipping intersection. That's because we don't
      // want to use an external function like FT_MulDiv
      // to compute it directly.
      */
      if ( y2 == miny ) goto Exit;
      do
      {
        x = (x1 + x2) >> 1;
        y = (y1 + y2) >> 1;

        if (y <= miny)
        {
          x1 = x;
          y1 = y;
        }
        else
        {
          x2 = x;
          y2 = y;
        }
      }
      while ( y1 < miny );

      e1  = TRUNC( miny );
      f1  = 0;
#endif      
    }
    else
    {
      e1 = TRUNC( y1 );
      f1 = FRAC( y1 );
    }

    /* adjust start point so that we begin on an integer scanline position */
    if ( f1 > 0 )
    {
      if ( e1 == e2 ) goto Exit;
      else
      {
        x1 += FMulDiv( Dx, PRECISION - f1, Dy );
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

    /* if this is a `fresh' profile, record its starting scanline */
    if ( ras.fresh )
    {
      ras.cur_prof->start = e1;
      ras.fresh           = FALSE;
    }

    /* check for overflow */
    size = e2 - e1 + 1;
    if ( ras.cursor + size >= ras.pool_limit )
    {
      ras.error = ErrRaster_Overflow;
      return FAILURE;
    }

#ifdef OLD
    if ( Dx > 0 )
    {
      Ix = ( PRECISION*Dx ) / Dy;
      Rx = ( PRECISION*Dx ) % Dy;
      Dx = 1;
    }
    else
    {
      Ix = -( (PRECISION*-Dx) / Dy );
      Rx =    (PRECISION*-Dx) % Dy;
      Dx = -1;
    }
    
    Ax = -Dy;
#else
    /* compute decision variables and push the intersections on top */
    /* of the render pool                                           */
    Dx <<= PRECISION_BITS;
    Ix   = Dx / Dy;
    Rx   = Dx % Dy;
    if (Rx < 0)
    {
      Ix --;
      Rx += Dy;
    }

    Ax   = -Dy;
    Rx <<= 1;
    Dy <<= 1;
#endif

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
#ifdef OLD
        x1 += Dx;
#else        
        x1 ++;
#endif        
      }
      size--;
    }

    ras.cursor = top;
  Exit:
    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Line_Down                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Computes the scan-line intersections of a descending line segment  */
  /*    and stores them in the render pool.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    x1   :: The start x coordinate.                                    */
  /*    y1   :: The start y coordinate.                                    */
  /*    x2   :: The end x coordinate.                                      */
  /*    y2   :: The end y coordinate.                                      */
  /*    miny :: The minimum vertical grid coordinate.                      */
  /*    maxy :: The maximum vertical grid coordinate.                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS or FAILURE.                                                */
  /*                                                                       */
  static
  TResult  Line_Down( RAS_ARG_ TPos  x1,   TPos  y1,
                               TPos  x2,   TPos  y2,
                               TPos  miny, TPos  maxy )
  {
    TResult  result, fresh;


    /* simply invert the coordinates and call Line_Up */
    fresh  = ras.fresh;
    result = Line_Up( RAS_VAR_ x1, -y1, x2, -y2, -maxy, -miny );

    /* if this was a fresh profile, invert the recorded start position */
    if ( fresh && !ras.fresh )
      ras.cur_prof->start = -ras.cur_prof->start;

    return result;
  }




 /* A function type describing the functions used to split bezier arcs */
  typedef  void  (*TSplitter)( TPoint*  base );

#ifdef FT_DYNAMIC_BEZIER_STEPS
  static
  TPos  Dynamic_Bezier_Threshold( RAS_ARG_ int degree, TPoint*  arc )
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
  TResult  Bezier_Up( RAS_ARG_ int        degree,
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

#ifdef OLD
    if ( e2 > maxy )
      e2 = maxy;
      
    e0 = miny;
#else    
    if ( e2 > maxy )
      e2 = FLOOR(maxy);

    e0 = CEILING(miny);
#endif


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

        e += PRECISION; /* go to next scanline */
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
    threshold = PRECISION_STEP;
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
        if ( y2-y1 >= PRECISION_STEP )
#else        
        if ( y2 >= e + PRECISION || y2 - y1 >= threshold )
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
          e   += PRECISION;  /* go to next scanline */
        }
      }
      else
      {
        if ( y2 == e )        /* if the arc falls on the scanline */
        {                     /* record its _joint_ intersection  */
          ras.joint  = TRUE;
          *top++     = arc[0].x;

          DEBUG_PSET;

          e += PRECISION; /* go to next scanline */
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
  TResult  Bezier_Down( RAS_ARG_ int        degree,
                                 TSplitter  splitter,
                                 TPos       miny,
                                 TPos       maxy )
  {
    TPoint*  arc = ras.arc;
    TResult  result, fresh;

    arc[0].y = -arc[0].y;
    arc[1].y = -arc[1].y;
    arc[2].y = -arc[2].y;
    if (degree > 2)
      arc[3].y = -arc[3].y;

    fresh = ras.fresh;

    result = Bezier_Up( RAS_VAR_ degree, splitter, -maxy, -miny );

    if ( fresh && !ras.fresh )
      ras.cur_prof->start = -ras.cur_prof->start;

    arc[0].y = -arc[0].y;
    return result;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****         SPLITTING CONIC AND CUBIC BEZIERS IN HALF               ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


#ifdef FT_RASTER_CONIC_BEZIERS

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Split_Conic                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Subdivides one second-order Bezier arc into two joint sub-arcs in  */
  /*    the Bezier stack.                                                  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This routine is the `beef' of the component.  It is one of _the_   */
  /*    inner loops that should be optimized like hell to get the best     */
  /*    performance.                                                       */
  /*                                                                       */
  static
  void  Split_Conic( TPoint*  base )
  {
    TPos  a, b;


    base[4].x = base[2].x;
    b = base[1].x;
    a = base[3].x = ( base[2].x + b + 1 ) >> 1;
    b = base[1].x = ( base[0].x + b + 1 ) >> 1;
    base[2].x = ( a + b + 1 ) >> 1;

    base[4].y = base[2].y;
    b = base[1].y;
    a = base[3].y = ( base[2].y + b + 1 ) >> 1;
    b = base[1].y = ( base[0].y + b + 1 ) >> 1;
    base[2].y = ( a + b ) / 2;
  }

#endif

#ifdef FT_RASTER_CUBIC_BEZIERS

  /*************************************************************************/
  /*                                                                       */
  /* <Function>   Split_Cubic                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*     Subdivides a third-order Bezier arc into two joint sub-arcs in    */
  /*     the Bezier stack.                                                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This routine is the `beef' of the component.  It is one of _the_   */
  /*    inner loops that should be optimized like hell to get the best     */
  /*    performance.                                                       */
  /*                                                                       */
  static
  void  Split_Cubic( TPoint*  base )
  {
    TPos   a, b, c, d;


    base[6].x = base[3].x;
    c = base[1].x;
    d = base[2].x;
    base[1].x = a = ( base[0].x + c + 1 ) >> 1;
    base[5].x = b = ( base[3].x + d + 1 ) >> 1;
    c = ( c + d + 1 ) >> 1;
    base[2].x = a = ( a + c + 1 ) >> 1;
    base[4].x = b = ( b + c + 1 ) >> 1;
    base[3].x = ( a + b + 1 ) >> 1;

    base[6].y = base[3].y;
    c = base[1].y;
    d = base[2].y;
    base[1].y = a = ( base[0].y + c + 1 ) >> 1;
    base[5].y = b = ( base[3].y + d + 1 ) >> 1;
    c = ( c + d + 1 ) >> 1;
    base[2].y = a = ( a + c + 1 ) >> 1;
    base[4].y = b = ( b + c + 1 ) >> 1;
    base[3].y = ( a + b + 1 ) >> 1;
  }

#endif /* FT_RASTER_CUBIC_BEZIERS */


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                   PROCESSING OUTLINE SEGMENTS                   ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/



  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Check_Contour                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Performs some checks at contour closure.                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS or FAILURE.                                                */
  /*                                                                       */
  static
  TResult  Check_Contour( RAS_ARG )
  {
    PProfile  lastProfile;

    /* Sometimes, the first and last profile in a contour join on      */
    /* an integer scan-line; we must then remove the last intersection */
    /* from the last profile to get rid of doublets                    */
    if ( ( FRAC( ras.last.y ) == 0     &&
           ras.last.y >= ras.minY      &&
           ras.last.y <= ras.maxY )    )
    {
      if ( ras.first_prof && ras.first_prof->flow == ras.cur_prof->flow )
        ras.cursor--;
    }

    lastProfile = ras.cur_prof;
    if ( End_Profile( RAS_VAR ) )
      return FAILURE;

    /* close the `next profile in contour' linked list */
    lastProfile->next = ras.first_prof;

    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Move_To                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function injects a new contour in the render pool.            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    to     :: A pointer to the contour's first point.                  */
  /*    raster :: A pointer to the current raster object.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function is used as a `FTRasterMoveTo_Func' by the outline    */
  /*    decomposer.                                                        */
  /*                                                                       */
  static
  int  Move_To( FT_Vector*  to,
                FT_Raster   raster )
  {
    TPos  scaled_x, scaled_y;


    /* if there was already a contour being built, perform some checks */
    if ( ras.start_prof )
      if ( Check_Contour( RAS_VAR ) )
        return FAILURE;

    /* set the `current last point' */
    scaled_x = SCALED( to->x );
    scaled_y = SCALED( to->y );

    if ( ras.flipped )
    {
      ras.last.x = scaled_y;
      ras.last.y = scaled_x;
    }
    else
    {
      ras.last.x = scaled_x;
      ras.last.y = scaled_y;
    }

    ras.state      = Unknown;
    ras.first_prof = NULL;

    return SUCCESS;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Line_To                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function injects a new line segment in the render pool and    */
  /*    adjusts the profiles list accordingly.                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    to     :: A pointer to the target position.                        */
  /*    raster :: A pointer to the current raster object.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function is used as a `FTRasterLineTo_Func' by the outline    */
  /*    decomposer.                                                        */
  /*                                                                       */
  static
  int  Line_To( FT_Vector*  to,
                FT_Raster   raster )
  {
    TPos  x, scaled_x;
    TPos  y, scaled_y;


    scaled_x = SCALED( to->x );
    scaled_y = SCALED( to->y );

    if ( ras.flipped )
    {
      x = scaled_y;
      y = scaled_x;
    }
    else
    {
      x = scaled_x;
      y = scaled_y;
    }

    /* First, detect a change of direction */
    if ( y != ras.last.y )
    {
      TDirection  new_state = ( (y > ras.last.y) ? Ascending : Descending );


      if ( ras.state != new_state )
      {
        if ( ras.state != Unknown   &&
             End_Profile( RAS_VAR ) )
          goto Fail;

        if ( New_Profile( RAS_VAR_  new_state ) )
          goto Fail;
      }
    }

    /* Then compute the lines */
    switch ( ras.state )
    {
    case Ascending:
      if ( Line_Up ( RAS_VAR_  ras.last.x, ras.last.y,
                               x, y, ras.minY, ras.maxY ) )
        goto Fail;
      break;

    case Descending:
      if ( Line_Down( RAS_VAR_ ras.last.x, ras.last.y,
                               x, y, ras.minY, ras.maxY ) )
        goto Fail;
      break;

    default:
      ;
    }

    ras.last.x = x;
    ras.last.y = y;

    return SUCCESS;

  Fail:
    return FAILURE;
  }


#ifdef FT_RASTER_CONIC_BEZIERS

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Push_Conic                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Clears the Bezier stack and pushes a new arc on top of it.         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    p2 :: A pointer to the second (control) point.                     */
  /*    p3 :: A pointer to the third (end) point.                          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The first point is taken as `raster->last', so it doesn't appear   */
  /*    in the signature.                                                  */
  /*                                                                       */
  static
  void  Push_Conic( RAS_ARG_ FT_Vector*  p2,
                             FT_Vector*  p3 )
  {
#undef  STORE
#define STORE( _arc, point )                    \
          {                                     \
            TPos  x = SCALED( point->x );       \
            TPos  y = SCALED( point->y );       \
                                                \
                                                \
            if ( ras.flipped )                  \
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


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Conic_To                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Injects a new conic Bezier arc and adjusts the profile list        */
  /*    accordingly.                                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    control :: A pointer to an intermediate control point.             */
  /*    to      :: A pointer to the end point.                             */
  /*    raster  :: A handle to the current raster object.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function is used as a `FTRasterConicTo_Func' by the outline   */
  /*    decomposer.                                                        */
  /*                                                                       */
  static
  int  Conic_To( FT_Vector*  control,
                 FT_Vector*  to,
                 FT_Raster   raster )
  {
    TPos        y1, y2, y3, x3, ymin, ymax;
    TDirection  state_bez;


    Push_Conic( RAS_VAR_  control, to );

    do
    {
      y1 = ras.arc[2].y;
      y2 = ras.arc[1].y;
      y3 = ras.arc[0].y;
      x3 = ras.arc[0].x;

      /* first, categorize the Bezier arc */

      if ( y1 <= y3 )
      {
        ymin = y1;
        ymax = y3;
      }
      else
      {
        ymin = y3;
        ymax = y1;
      }

      if ( y2 < ymin || y2 > ymax )
      {
        /* this arc has no given direction, split it !! */
        Split_Conic( ras.arc );
        ras.arc += 2;
      }
      else if ( y1 == y3 )
      {
        /* this arc is flat, ignore it and pop it from the bezier stack */
        ras.arc -= 2;
      }
      else
      {
        /* the arc is y-monotonous, either ascending or descending */
        /* detect a change of direction                            */
        state_bez =  y1 < y3 ? Ascending : Descending;
        if ( ras.state != state_bez )
        {
          /* finalize current profile if any */
          if ( ras.state != Unknown   &&
               End_Profile( RAS_VAR ) )
            goto Fail;

          /* create a new profile */
          if ( New_Profile( RAS_VAR_ state_bez ) )
            goto Fail;
        }

        /* now call the appropriate routine */
        if ( state_bez == Ascending )
        {
          if ( Bezier_Up( RAS_VAR_  2, Split_Conic, ras.minY, ras.maxY ) )
            goto Fail;
        }
        else
          if ( Bezier_Down( RAS_VAR_  2, Split_Conic, ras.minY, ras.maxY ) )
            goto Fail;
      }

    } while ( ras.arc >= ras.arcs );

    ras.last.x = x3;
    ras.last.y = y3;

    return SUCCESS;

  Fail:
    return FAILURE;
  }

#else /* FT_RASTER_CONIC_BEZIERS */


  static
  int  Conic_To( FT_Vector*  control,
                 FT_Vector*  to,
                 FT_Raster   raster )
  {
    UNUSED( control );
    UNUSED( to );
    UNUSED( raster );

    return ErrRaster_Invalid_Outline;
  }

#endif /* FT_RASTER_CONIC_BEZIERS */


#ifdef FT_RASTER_CUBIC_BEZIERS

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Push_Cubic                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Clears the Bezier stack and pushes a new third-order Bezier arc on */
  /*    top of it.                                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    p2 :: A pointer to the second (control) point.                     */
  /*    p3 :: A pointer to the third (control) point.                      */
  /*    p4 :: A pointer to the fourth (end) point.                         */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The first point is taken as `raster->last', so it doesn't appear   */
  /*    in the signature.                                                  */
  /*                                                                       */
  /*    This is the same as Push_Conic(), except that it deals with        */
  /*    third-order Beziers.                                               */
  /*                                                                       */
  static
  void  Push_Cubic( RAS_ARG_ FT_Vector*  p2,
                             FT_Vector*  p3,
                             FT_Vector*  p4 )
  {
#undef  STORE
#define STORE( _arc, point )                    \
          {                                     \
            TPos  x = SCALED( point->x );       \
            TPos  y = SCALED( point->y );       \
                                                \
            if ( ras.flipped )                  \
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


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Cubic_To                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Injects a new cubic Bezier arc and adjusts the profile list        */
  /*    accordingly.                                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    control1 :: A pointer to the first control point.                  */
  /*    control2 :: A pointer to the second control point.                 */
  /*    to       :: A pointer to the end point.                            */
  /*    raster   :: A handle to the current raster object.                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function is used as a `FTRasterCubicTo_Func' by the outline   */
  /*    decomposer.                                                        */
  /*                                                                       */
  static
  int  Cubic_To( FT_Vector*  control1,
                 FT_Vector*  control2,
                 FT_Vector*  to,
                 FT_Raster   raster )
  {
    TPos        y1, y2, y3, y4, x4, ymin1, ymax1, ymin2, ymax2;
    TDirection  state_bez;


    Push_Cubic( RAS_VAR_  control1, control2, to );

    do
    {
      y1 = ras.arc[3].y;
      y2 = ras.arc[2].y;
      y3 = ras.arc[1].y;
      y4 = ras.arc[0].y;
      x4 = ras.arc[0].x;

      /* first, categorize the Bezier arc */

      if ( y1 <= y4 )
      {
        ymin1 = y1;
        ymax1 = y4;
      }
      else
      {
        ymin1 = y4;
        ymax1 = y1;
      }

      if ( y2 <= y3 )
      {
        ymin2 = y2;
        ymax2 = y3;
      }
      else
      {
        ymin2 = y3;
        ymax2 = y2;
      }

      if ( ymin2 < ymin1 || ymax2 > ymax1 )
      {
        /* this arc has no given direction, split it! */
        Split_Cubic( ras.arc );
        ras.arc += 3;
      }
      else if ( y1 == y4 )
      {
        /* this arc is flat, ignore it and pop it from the bezier stack */
        ras.arc -= 3;
      }
      else
      {
        state_bez = ( y1 <= y4 ) ? Ascending : Descending;

        /* detect a change of direction */
        if ( ras.state != state_bez )
        {
          if ( ras.state != Unknown   &&
               End_Profile( RAS_VAR ) )
            goto Fail;

          if ( New_Profile( RAS_VAR_ state_bez ) )
            goto Fail;
        }

        /* compute intersections */
        if ( state_bez == Ascending )
        {
          if ( Bezier_Up ( RAS_VAR_ 3, Split_Cubic, ras.minY, ras.maxY ) )
            goto Fail;
        }
        else
          if ( Bezier_Down ( RAS_VAR_ 3, Split_Cubic, ras.minY, ras.maxY ) )
            goto Fail;
      }

    } while ( ras.arc >= ras.arcs );

    ras.last.x = x4;
    ras.last.y = y4;

    return SUCCESS;

  Fail:
    return FAILURE;
  }


#else /* FT_RASTER_CUBIC_BEZIERS */


  int  Cubic_To( FT_Vector*  control1,
                 FT_Vector*  control2,
                 FT_Vector*  to,
                 FT_Raster   raster )
  {
    UNUSED( control1 );
    UNUSED( control2 );
    UNUSED( to );
    UNUSED( raster );

    return ErrRaster_Invalid_Outline;
  }


#endif /* FT_RASTER_CUBIC_BEZIERS */



  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Convert_Glyph                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Converts a glyph into a series of segments and arcs and makes a    */
  /*    profiles list with them.                                           */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    outline :: The glyph outline.                                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    SUCCESS or FAILURE.                                                */
  /*                                                                       */
  static
  TResult  Convert_Glyph( RAS_ARG_ FT_Outline*  outline )
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

    ras.pool_limit = ras.pool_size - AlignProfileSize;

    ras.n_extrema = 0;

    ras.cur_prof         = (PProfile)ras.cursor;
    ras.cur_prof->offset = ras.cursor;
    ras.num_profs        = 0;

    /* Now decompose curve */
    if ( FT_Outline_Decompose( outline, &interface, &ras ) )
      return FAILURE;
    /* XXX: the error condition is in ras.error */

    /* Check the last contour if needed */
    if ( Check_Contour( RAS_VAR ) )
      return FAILURE;

    /* Finalize profiles list */
    return Finalize_Profile_Table( RAS_VAR );
  }


  /*************************************************************************/
  /*                                                                       */
  /* Init_Linked                                                           */
  /*                                                                       */
  /*    Inits an empty linked list.                                        */
  /*                                                                       */
  static
  void  Init_Linked( TProfileList*  l )
  {
    *l = NULL;
  }


  /*************************************************************************/
  /*                                                                       */
  /* InsNew                                                                */
  /*                                                                       */
  /*    Inserts a new Profile in a linked list.                            */
  /*                                                                       */
  static
  void  InsNew( PProfileList  list,
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


  /*************************************************************************/
  /*                                                                       */
  /* DelOld                                                                */
  /*                                                                       */
  /*    Removes an old Profile from a linked list.                         */
  /*                                                                       */
  static
  void  DelOld( PProfileList  list,
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

    /* We should never reach this place, unless the Profile was not  */
    /* part of the list.                                             */
  }


  /*************************************************************************/
  /*                                                                       */
  /* Update                                                                */
  /*                                                                       */
  /*    Updates all X offsets of a drawing list.                           */
  /*                                                                       */
  static
  void  Update( PProfile  first )
  {
    PProfile  current = first;


    while ( current )
    {
      current->X       = *current->offset;
      current->offset += current->flow;
      current->height--;
      current = current->link;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* Sort                                                                  */
  /*                                                                       */
  /*    Sorts a trace list.  In 95%, the list is already sorted.  We need  */
  /*    an algorithm which is fast in this case.  Bubble sort is enough    */
  /*    and simple to implement.                                           */
  /*                                                                       */
  static
  void  Sort( PProfileList  list )
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


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /********                                                         ********/
  /********            Vertical Bitmap Sweep Routines               ********/
  /********                                                         ********/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Vertical_Sweep_Init                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes the vertical bitmap sweep.  Called by the generic      */
  /*    sweep/draw routine before its loop.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    min :: The address of the current minimum scanline.                */
  /*    max :: The address of the current maximum scanline.                */
  /*                                                                       */
  static
  void  Vertical_Sweep_Init( RAS_ARG_ int*  min, int*  max )
  {
    long  pitch;

    UNUSED( max );

    pitch          = ras.target.pitch;

    /* start from the bottom line, going up !! */
    ras.trace_bit  = - *min * pitch;
    ras.trace_incr = -pitch;

    if (pitch > 0)
      ras.trace_bit += pitch*(ras.target.rows-1);
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Vertical_Sweep_Span                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Draws a single horizontal bitmap span during the vertical bitmap   */
  /*    sweep.                                                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y  :: The current scanline.                                        */
  /*    x1 :: The left span edge.                                          */
  /*    x2 :: The right span edge.                                         */
  /*                                                                       */
  static
  void  Vertical_Sweep_Span( RAS_ARG_ TScan  y,
                                      TPos   x1,
                                      TPos   x2 )
  {
    TPos   e1, e2;
    int    c1, c2;
    Byte   f1, f2;
    PByte  target;

    UNUSED( y );

    /* Drop-out control */
    e1 = TRUNC( CEILING( x1 ) );
    
    if ( x2 - x1 - PRECISION <= PRECISION_JITTER )
      e2 = e1;
    else
      e2 = TRUNC( FLOOR( x2 ) );

#ifdef OLD
    if ( e2 >= 0 && e1 < ras.bit_width )
#else
    if ( e1 <= e2 && e2 >= 0 && e1 < ras.bit_width )
#endif    
    {
      if ( e1 < 0 )              e1 = 0;
      if ( e2 >= ras.bit_width ) e2 = ras.bit_width - 1;

      c1 = e1 >> 3;
      c2 = e2 >> 3;

      f1 =  ((unsigned char)0xFF >> (e1 & 7));
      f2 = ~((unsigned char)0x7F >> (e2 & 7));

      target = ras.bit_buffer + ras.trace_bit + c1;
      c2 -= c1;

      if ( c2 > 0 )
      {
        target[0] |= f1;

        /* memset() is slower than the following code on many platforms. */
        /* This is due to the fact that, in the vast majority of cases,  */
        /* the span length in bytes is relatively small.                 */
        c2--;
        while ( c2 > 0 )
        {
          *(++target) = 0xFF;
          c2--;
        }
        target[1] |= f2;
      }
      else
        *target |= ( f1 & f2 );
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Vertical_Test_Pixel                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Tests a pixel `light' during the vertical bitmap sweep.  Used      */
  /*    during drop-out control only.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y :: The current scanline.                                         */
  /*    x :: The current x coordinate.                                     */
  /*                                                                       */
  static
  int  Vertical_Test_Pixel( RAS_ARG_ TScan  y,
                                     int    x )
  {
    int c1 = x >> 3;


    UNUSED( y );

    return ( x >= 0 && x < ras.bit_width &&
             ras.bit_buffer[ras.trace_bit + c1] & (0x80 >> (x & 7)) );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Vertical_Set_Pixel                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets a single pixel in a bitmap during the vertical sweep.  Used   */
  /*    during drop-out control.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y     :: The current scanline.                                     */
  /*    x     :: The current x coordinate.                                 */
  /*    color :: Ignored by this function.                                 */
  /*                                                                       */
  static
  void  Vertical_Set_Pixel( RAS_ARG_ int  y,
                                     int  x,
                                     int  color )
  {
    UNUSED( color );
    UNUSED( y );

    if ( x >= 0 && x < ras.bit_width )
      ras.bit_buffer[ras.trace_bit+(x >> 3)] |= (char)(0x80 >> (x & 7));
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Vertical_Sweep_Step                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Called whenever the sweep jumps to another scanline.  Only updates */
  /*    the pointers in the vertical bitmap sweep.                         */
  /*                                                                       */
  static
  void Vertical_Sweep_Step( RAS_ARG )
  {
    ras.trace_bit += ras.trace_incr;
  }


  static
  const  Raster_Render   vertical_render_mono =
  {
    &Vertical_Sweep_Init,
    &Vertical_Sweep_Span,
    &Vertical_Sweep_Step,
    &Vertical_Test_Pixel,
    &Vertical_Set_Pixel
  };

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /********                                                         ********/
  /********           Horizontal Bitmap Sweep Routines              ********/
  /********                                                         ********/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Horizontal_Sweep_Init                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes the horizontal bitmap sweep.  Called by the generic    */
  /*    sweep/draw routine before its loop.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    min :: The address of the current minimum pixel column.            */
  /*    max :: The address of the current maximum pixel column.            */
  /*                                                                       */
  static
  void  Horizontal_Sweep_Init( RAS_ARG_ int*  min,
                                        int*  max )
  {
    UNUSED( ras );
    UNUSED( min );
    UNUSED( max );

    /* nothing, really */
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Horizontal_Sweep_Span                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Draws a single vertical bitmap span during the horizontal bitmap   */
  /*    sweep.  Actually, this function is only used to check for weird    */
  /*    drop-out cases.                                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y  :: The current pixel column.                                    */
  /*    x1 :: The top span edge.                                           */
  /*    x2 :: The bottom span edge.                                        */
  /*                                                                       */
  static
  void  Horizontal_Sweep_Span( RAS_ARG_ TScan  y,
                                        TPos   x1,
                                        TPos   x2 )
  {
    TPos      e1, e2;
    PByte  bits;
    Byte   f1;

    UNUSED( y );

    /* During the horizontal sweep, we only take care of drop-outs */
    if ( x2 - x1 < PRECISION )
    {
      e1 = CEILING( x1 );
      e2 = FLOOR( x2 );

      if ( e1 == e2 )
      {
        bits = ras.bit_buffer + (y >> 3);
        f1   = (Byte)(0x80 >> (y & 7));

        e1 = TRUNC( e1 );

        if ( e1 >= 0 && e1 < ras.target.rows )
        {
          long pitch  = ras.target.pitch;
          long offset = - pitch * e1;

          if (pitch > 0)
            offset += (ras.target.rows-1)*pitch;

          bits[offset] |= f1;
        }
      }
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Horizontal_Test_Pixel                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Tests a pixel `light' during the horizontal bitmap sweep.  Used    */
  /*    during drop-out control only.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y :: The current pixel column.                                     */
  /*    x :: The current row/scanline.                                     */
  /*                                                                       */
  static
  int   Horizontal_Test_Pixel( RAS_ARG_ int  y,
                                        int  x )
  {
    char*  bits   = (char*)ras.bit_buffer + (y >> 3);
    int    f1     = (Byte)(0x80 >> (y & 7));
    long   pitch  = ras.target.pitch;
    long   offset = - pitch * x;

    if (pitch > 0)
      offset += (ras.target.rows-1)*pitch;

    return ( x >= 0 && x < ras.target.rows && (bits[0] & f1) );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Horizontal_Set_Pixel                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets a single pixel in a bitmap during the horizontal sweep.  Used */
  /*    during drop-out control.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y     :: The current pixel column.                                 */
  /*    x     :: The current row/scanline.                                 */
  /*    color :: Ignored by this function.                                 */
  /*                                                                       */
  static
  void  Horizontal_Set_Pixel( RAS_ARG_ int  y,
                                       int  x,
                                       int  color )
  {
    char*  bits = (char*)ras.bit_buffer + (y >> 3);
    int    f1   = (Byte)(0x80 >> (y  & 7));


    UNUSED( color );

    if ( x >= 0 && x < ras.target.rows )
    {
      long pitch  = ras.target.pitch;
      long offset = - x*pitch;

      if (pitch > 0)
        offset += (ras.target.rows-1)*pitch;

      bits[offset] |= f1;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Horizontal_Sweep_Step                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Called whenever the sweep jumps to another pixel column.           */
  /*                                                                       */
  static
  void  Horizontal_Sweep_Step( RAS_ARG )
  {
    UNUSED( ras.target );

    /* Nothing, really */
  }


  static
  const  Raster_Render   horizontal_render_mono =
  {
    &Horizontal_Sweep_Init,
    &Horizontal_Sweep_Span,
    &Horizontal_Sweep_Step,
    &Horizontal_Test_Pixel,
    &Horizontal_Set_Pixel
  };


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /********                                                         ********/
  /********      Anti-Aliased Vertical Bitmap Sweep Routines        ********/
  /********                                                         ********/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#ifdef FT_RASTER_OPTION_ANTI_ALIAS

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Vertical_Gray_Sweep_Init                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes the vertical bitmap sweep.  Called by the generic      */
  /*    sweep/draw routine before its loop.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    min :: The address of the current minimum scanline.                */
  /*    max :: The address of the current maximum scanline.                */
  /*                                                                       */
  static
  void  Vertical_Gray_Sweep_Init( RAS_ARG_ int*  min, int*  max )
  {
    long  pitch;

    UNUSED( max );

    pitch = ras.target.pitch;

    /* start from the bottom line, going up */
    ras.trace_incr = -pitch;
    ras.trace_bit  = - *min * pitch;

    if (pitch > 0)
      ras.trace_bit += (ras.target.rows-1)*pitch;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Vertical_Gray_Sweep_Span                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Draws a single horizontal bitmap span during the vertical bitmap   */
  /*    sweep.                                                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y  :: The current scanline.                                        */
  /*    x1 :: The left span edge.                                          */
  /*    x2 :: The right span edge.                                         */
  /*                                                                       */
  static
  void  Vertical_Gray_Sweep_Span( RAS_ARG_ TScan  y,
                                           TPos   x1,
                                           TPos   x2 )
  {
    TPos   e1, e2;
    int    shift = PRECISION_BITS - 6;
    PByte  target;

    UNUSED( y );

    x1 += PRECISION_HALF;
    x2 += PRECISION_HALF;

    e1 = TRUNC( x1 );
    e2 = TRUNC( x2 );

    if ( e1 <= e2 && e2 >= 0 && e1 < ras.bit_width )
    {
      x1 = FRAC(x1) >> shift;
      x2 = FRAC(x2) >> shift;

      if ( e1 < 0 )
      {
        e1 = 0;
        x1 = 0;
      }

      if ( e2 > ras.bit_width )
      {
        e2 = ras.bit_width-1;
        x2 = 0;
      }

      target = ras.bit_buffer + ras.trace_bit + e1;
      e2    -= e1;

      if ( e2 > 0 )
      {
        if (x1 > 0) target[0] += (Byte)(64-x1) << 1;
               else target[0]  = 127;
        e2--;
        while (e2 > 0)
        {
          *(++target) = 127;
          e2--;
        }
        if (x2)
          target[1] += (Byte)x2 << 1;
      }
      else
      {
        target[0] += (Byte)(x2-x1) << 1;
      }
    }
  }

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Vertical_Gray_Test_Pixel                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Tests a pixel `light' during the vertical bitmap sweep.  Used      */
  /*    during drop-out control only.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y :: The current scanline.                                         */
  /*    x :: The current x coordinate.                                     */
  /*                                                                       */
  static
  int  Vertical_Gray_Test_Pixel( RAS_ARG_ TScan  y,
                                          int    x )
  {
    UNUSED_RASTER
    UNUSED( y );

#if 0
    /* as a rule of thumb, do not add a drop-out if the current */
    /* gray level is over 0.5                                   */

    return ( x >= 0 && x < ras.bit_width &&
             ras.bit_buffer[ras.trace_bit + x] >= 64 );
#else
    UNUSED(x);
    return 0;
#endif
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Vertical_Gray_Set_Pixel                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets a single pixel in a bitmap during the vertical sweep.  Used   */
  /*    during drop-out control.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y     :: The current scanline.                                     */
  /*    x     :: The current x coordinate.                                 */
  /*    color :: Ignored by this function.                                 */
  /*                                                                       */
  static
  void  Vertical_Gray_Set_Pixel( RAS_ARG_ int  y,
                                          int  x,
                                          int  color )
  {
    UNUSED( y );

    if ( x >= 0 && x < ras.bit_width )
    {
      unsigned char*  pixel;

      pixel = ras.bit_buffer + ras.trace_bit + x;

      /* do not add too much to the pixel gray level */
      color += *pixel;
      if (color < 64)
        color = 64;

      *pixel = ( color >= 127 ? 127 : (unsigned char)color );
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Vertical_Sweep_Step                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Called whenever the sweep jumps to another scanline.  Only updates */
  /*    the pointers in the vertical bitmap sweep.                         */
  /*                                                                       */
  static
  void Vertical_Gray_Sweep_Step( RAS_ARG )
  {
    ras.trace_bit += ras.trace_incr;
  }



  static
  const  Raster_Render   vertical_render_gray =
  {
    &Vertical_Gray_Sweep_Init,
    &Vertical_Gray_Sweep_Span,
    &Vertical_Gray_Sweep_Step,
    &Vertical_Gray_Test_Pixel,
    &Vertical_Gray_Set_Pixel
  };



  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /********                                                         ********/
  /********           Horizontal Bitmap Sweep Routines              ********/
  /********                                                         ********/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Horizontal_Sweep_Init                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes the horizontal bitmap sweep.  Called by the generic    */
  /*    sweep/draw routine before its loop.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    min :: The address of the current minimum pixel column.            */
  /*    max :: The address of the current maximum pixel column.            */
  /*                                                                       */
  static
  void  Horizontal_Gray_Sweep_Init( RAS_ARG_ int*  min,
                                             int*  max )
  {
    UNUSED( ras );
    UNUSED( min );
    UNUSED( max );

    /* nothing, really */
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Horizontal_Gray_Sweep_Span                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Draws a single vertical bitmap span during the horizontal bitmap   */
  /*    sweep.                                                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y  :: The current scanline.                                        */
  /*    x1 :: The left span edge.                                          */
  /*    x2 :: The right span edge.                                         */
  /*                                                                       */
  static
  void  Horizontal_Gray_Sweep_Span( RAS_ARG_ TScan  y,
                                             TPos   x1,
                                             TPos   x2 )
  {
    TPos   e1, e2;
    int    shift = PRECISION_BITS - 6;
    int    incr;
    PByte  bits;
    Byte   b;

    UNUSED( y );

    x1 += PRECISION_HALF;
    x2 += PRECISION_HALF;

    e1 = TRUNC( x1 );
    e2 = TRUNC( x2 );

    if ( e1 <= e2 && e2 >= 0 && e1 < ras.bit_width )
    {
      x1 = FRAC(x1) >> shift;
      x2 = FRAC(x2) >> shift;

      if ( e1 < 0 )
      {
        e1 = 0;
        x1 = 0;
      }

      if ( e2 >= ras.bit_width )
      {
        e2 = ras.bit_width;
        x2 = 0;
      }

      incr  = -ras.target.pitch;
      bits  = ras.bit_buffer + y;
      bits += incr * e1;
      if (incr < 0)
        bits -= incr*(ras.target.rows-1);

      e2 -= e1;

      if ( e2 > 0 )
      {
        b = bits[0];
        if (b < 127) b++;
        b = (Byte)((64-x1) + (b >> 1));
        bits[0] = b;

        if ( e2 < 24 )
        {
          e2--;
          while (e2 > 0)
          {
            bits += incr;
            b     = bits[0];

            if (b < 127)
              bits[0] = (Byte)(63+((b+1) >> 1));

            e2--;
          }
        }
        else
          bits += incr*(e2-1);

        if (x2)
        {
          bits += incr;
          b     = bits[0];
          if (b < 127) b++;
          b     = (Byte)(x2 + (b >> 1));
          bits[0] = b;
        }
      }
      else
      {
        b = bits[0];
        if (b < 127) b++;
        b = (Byte)((b >> 1)+(x2-x1));
        bits[0] = b;
      }
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Horizontal_Gray_Test_Pixel                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Tests a pixel `light' during the horizontal bitmap sweep.  Used    */
  /*    during drop-out control only.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y :: The current pixel column.                                     */
  /*    x :: The current row/scanline.                                     */
  /*                                                                       */
  static
  int   Horizontal_Gray_Test_Pixel( RAS_ARG_ int  y,
                                             int  x )
  {
#if 0
    unsigned char*  pixel = (unsigned char*)ras.bit_buffer + y;

    if ( ras.target.flow == Flow_Down )
      pixel += (ras.target.rows-1 - x) * ras.target.cols;
    else
      pixel += x * ras.target.cols;

    return ( x >= 0 && x < ras.target.rows &&
            *pixel >= 64 );
#else
    UNUSED_RASTER
    UNUSED(y);
    UNUSED(x);
    return 0;
#endif
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Horizontal_Set_Pixel                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets a single pixel in a bitmap during the horizontal sweep.  Used */
  /*    during drop-out control.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y     :: The current pixel column.                                 */
  /*    x     :: The current row/scanline.                                 */
  /*    color :: Ignored by this function.                                 */
  /*                                                                       */
  static
  void  Horizontal_Gray_Set_Pixel( RAS_ARG_ int  y,
                                            int  x,
                                            int  color )
  {
    unsigned char*  pixel = (unsigned char*)ras.bit_buffer + y;

    if ( x >= 0 && x < ras.target.rows )
    {
      long  pitch  = ras.target.pitch;

      pixel -= pitch*x;
      if (pitch > 0)
        pixel += pitch*(ras.target.rows-1);

      color += *pixel;
      if (color < 64)
        color = 64;

      *pixel = (color >= 127 ? 127 : (unsigned char)color );
    }
  }


  static
  void Gray_Ignore( void )
  {
    ;
  }


  static
  const  Raster_Render   horizontal_render_gray =
  {
    &Horizontal_Gray_Sweep_Init,
    &Horizontal_Gray_Sweep_Span,

    (Function_Sweep_Step)  &Gray_Ignore,
    &Horizontal_Gray_Test_Pixel,
    &Horizontal_Gray_Set_Pixel,
  };

#endif /* FT_RASTER_OPTION_ANTI_ALIAS */


  /*************************************************************************/
  /*                                                                       */
  /* A technical note to explain how the scanline sweep is performed:      */
  /*                                                                       */
  /*   The function Draw_Sweep() is used to sweep the scanlines of the     */
  /*   target bitmap or pixmap.  For each scanline, it must do the         */
  /*   following:                                                          */
  /*                                                                       */
  /*   - Get the set of all outline intersections for the current          */
  /*     scanline.                                                         */
  /*                                                                       */
  /*   - Sort these intersections (in increasing order).                   */
  /*                                                                       */
  /*   - Pair intersections to create spans (horizontal pixel segments)    */
  /*     that are then `drawn' by calling a `sweep_span' function.         */
  /*                                                                       */
  /*   - Check for dropouts: If a span is too small to be drawn, it must   */
  /*     be re-adjusted in order to make it visible again.                 */
  /*                                                                       */
  /*   The sweep starts from the bottom of the outline (ymin) and goes     */
  /*   upwards (to ymax).  Thus, the function manages the following:       */
  /*                                                                       */
  /*   - A linked list of the profiles which are above the current         */
  /*     scanline.  It is called the `wait' list as it contains all the    */
  /*     profiles waiting to be `activated' during the sweep.  It contains */
  /*     all profiles initially.                                           */
  /*                                                                       */
  /*   - A linked list of the profiles covering the current scanline,      */
  /*     i.e., all the profiles that contain an intersection for the       */
  /*     current scanline.  It is called the `draw' list.                  */
  /*                                                                       */
  /*   A profile travels from the wait list to the draw list if the        */
  /*   current scanline reaches its bottom border (its ymin).  It is also  */
  /*   removed from the draw list (and becomes unlisted) when the current  */
  /*   scanline reaches the scanline above its upper border (its ymax).    */
  /*                                                                       */
  /*   These positions correspond to the `extrema' table built by          */
  /*   Finalize_Profile_Table().                                           */
  /*                                                                       */
  /*   The draw list is always sorted in increasing order of the X         */
  /*   coordinates.  We use a bubble sort because it is easy to implement  */
  /*   on a linked list, and because in 95% cases, the list is already     */
  /*   correctly sorted when going from one scanline to the other.         */
  /*                                                                       */
  /*   The extrema table gives the scanline coordinates at which at least  */
  /*   one profile must be removed from the `draw' list, or another one    */
  /*   must be moved from the `wait' to `draw' lists.                      */
  /*                                                                       */
  /*   Note that when a dropout is detected, the corresponding span is not */
  /*   drawn immediately but kept on a temporary list.  All dropout spans  */
  /*   are drawn after the regular spans on a given scanline.  This is a   */
  /*   requirement of the TrueType specification to properly implement     */
  /*   some drop-out control modes -- yes, it's weird!                     */
  /*                                                                       */
  /*   Finally, the parser contains four function pointers that are called */
  /*   by Draw_Sweep().  Each rendering mode (monochrome, anti-aliased-5,  */
  /*   and anti-aliased-17) provide its own set of such functions.  These  */
  /*   are:                                                                */
  /*                                                                       */
  /*     sweep_init:       Called only when the sweep starts.  Used to set */
  /*                       up some variables.                              */
  /*                                                                       */
  /*     sweep_span:       Used to draw a horizontal span on the current   */
  /*                       scanline.                                       */
  /*                                                                       */
  /*     sweep_test_pixel: Used to test a pixel's intensity, as it is      */
  /*                       required for drop-out control.                  */
  /*                                                                       */
  /*     sweep_put_pixel:  Used to write a single pixel when a drop-out    */
  /*                       needs to be lighted/drawn.                      */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /*  Generic Sweep Drawing routine                                        */
  /*                                                                       */
  static
  TResult  Draw_Sweep( RAS_ARG )
  {
    TScan  y, y_change, y_height;

    PProfile  P, Q, P_Left, P_Right;

    TScan   min_Y, max_Y, top, bottom, dropouts;

    TPos x1, x2, e1, e2;

    TProfileList  wait;
    TProfileList  draw;

    #ifdef DEBUG_RAS
    int   y_set = 0;
    #endif

    /* Init empty linked lists */
    Init_Linked( &wait );
    Init_Linked( &draw );

    /* first, compute min and max Y -- and add profiles to the wait list */
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

    /* Check the extrema table */
    if ( ras.n_extrema == 0 )
    {
      ras.error = ErrRaster_Invalid_Outline;
      return FAILURE;
    }

    /* Now inits the sweep */
    FT_TRACE2(( "draw_sweep: initialize sweep\n" ));
    ras.render.init( RAS_VAR_  &min_Y, &max_Y );
    FT_TRACE2(( "  init min_y = %d, max_y = %d\n", min_Y, max_Y ));

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

    if ( ras.n_extrema > 0 &&
         ras.pool_size[-ras.n_extrema] == min_Y )
      ras.n_extrema--;

    FT_TRACE2(( "starting loop with n_extrema = %d", ras.n_extrema ));
    while ( ras.n_extrema > 0 )
    {
      PProfile  prof = wait;


      /* look in the wait list for new activations */
      while ( prof )
      {
        PProfile  next = prof->link;

        prof->countL -= y_height;
        if ( prof->countL == 0 )
        {
          /* move the profile from the wait list to the draw list */
          DelOld( &wait, prof );
          InsNew( &draw, prof );
        }
        prof = next;
      }

      /* Sort the draw list */
      Sort( &draw );

      /* compute next y extremum scanline; we won't change the */
      /* elements of the wait and draw lists until there       */
      y_change = ras.pool_size[-ras.n_extrema--];
      y_height = y_change - y;

      FT_TRACE2(( ">>> y = %d, y_change = %d, y_height = %d",
                y, y_change, y_height ));

      while ( y < y_change )
      {
        int       window;
        PProfile  left;


        /* Let's trace */
        dropouts = 0;

        /* skip to next line if there is no active profile there */
        if ( !draw ) goto Next_Line;

        left   = draw;
        window = left->flow;
        prof   = left->link;

        FT_TRACE2(( ">>>  line y = %d", y ));

        while ( prof )
        {
          PProfile  next = prof->link;

          window += prof->flow;

          if ( window == 0 )
          {
            x1 = left->X;
            x2 = prof->X;

            if ( x1 > x2 )
            {
              TPos  xs = x1;

              x1 = x2;
              x2 = xs;
            }

            if ( x2 - x1 <= PRECISION && ras.dropout_mode )
            {
              e1 = CEILING( x1 );
              e2 = FLOOR( x2 );

              if ( e1 > e2 || e2 == e1 + PRECISION )
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

            FT_TRACE2(( "drawing span ( y=%d, x1=%d, x2=%d )", y, x1, x2 ));
            #ifdef DEBUG_RAS
            if (!y_set)
            {
              y_set = 1;
              fprintf( stderr, "%3d", y );
            }
            fprintf( stderr, " [%.2f-%.2f]", x1*1.0/PRECISION, x2*1.0/PRECISION );
            #endif
            ras.render.span( RAS_VAR_  y, x1, x2 );

   Skip_To_Next:
            left = next;
          }
          prof = next;
        }

        /* now perform the dropouts _after_ the span drawing   */
        /* drop-outs processing has been moved out of the loop */
        /* for performance tuning                              */
        if ( dropouts > 0 )
          goto Scan_DropOuts;

   Next_Line:
        ras.render.step( RAS_VAR );

        y++;
        #ifdef DEBUG_RAS
        if (y_set)
        {
          fprintf( stderr, "\n" );
          y_set = 0;
        }
        #endif

        if ( y < y_change )
          Sort( &draw );

        FT_TRACE4(( "line sorted for next operation" ));
      }

      /* Now finalize the profiles that needs it */

      FT_TRACE2(( "finalizing profiles..." ));
      {
        PProfile  prof, next;

        prof = draw;
        while ( prof )
        {
          next = prof->link;
          if (prof->height == 0)
            DelOld( &draw, prof );
          prof = next;
        }
      }

      FT_TRACE2(( "profiles finalized for this run" ));
    }

    /* for gray-scaling, flushes the bitmap scanline cache */
    while ( y <= max_Y )
    {
      ras.render.step( RAS_VAR );
      y++;
    }

    return SUCCESS;


Scan_DropOuts :
    P_Left = draw;


    while ( dropouts > 0 )
    {
      TPos      e1,   e2;
      PProfile  left, right;

      while ( P_Left->countL != 1 )
        P_Left = P_Left->link;
      P_Right = P_Left->link;
      while ( P_Right->countL != 2 )
        P_Right = P_Right->link;

      P_Left->countL  = 0;
      P_Right->countL = 0;

      /* Now perform the dropout control */
      x1 = P_Left->X;
      x2 = P_Right->X;

      left  = ( ras.flipped ? P_Right : P_Left  );
      right = ( ras.flipped ? P_Left  : P_Right );

      FT_TRACE2(( "performing drop-out control ( x1= %d, x2 = %d )",
                x1, x2 ));

      #ifdef DEBUG_RAS
      if (!y_set)
      {
        y_set = 1;
        fprintf( stderr, "%3d", y );
      }
      fprintf( stderr, " <%.2f-%.2f>", P_Left->X*1.0/PRECISION, P_Right->X*1.0/PRECISION );
      #endif
      
      e1 = CEILING( x1 );
      e2 = FLOOR  ( x2 );

      if ( e1 > e2 )
      {
        if ( e1 == e2 + PRECISION )
        {
          switch ( ras.dropout_mode )
          {
          case 1:
            e1 = e2;
            break;

          case 4:
            e1 = CEILING( (x1 + x2 + 1) / 2 );
            break;

          case 2:
          case 5:
            /* Drop-out Control Rule #4 */

            /* The spec is not very clear regarding rule #4.  It      */
            /* presents a method that is way too costly to implement  */
            /* while the general idea seems to get rid of `stubs'.    */
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
                 ras.render.test_pixel( RAS_VAR_  y, TRUNC(e1))       )
              goto Next_Dropout;

            if ( ras.dropout_mode == 2 )
              e1 = e2;
            else
              e1 = CEILING( (x1 + x2 + 1)/2 );

            break;

          default:
            goto Next_Dropout;  /* Unsupported mode */
          }
        }
        else
          goto Next_Dropout;
      }

      FT_TRACE2(( "  -> setting pixel" ));
      ras.render.set_pixel( RAS_VAR_ y,
                            TRUNC( e1 ),
                            (x2 - x1) >> ras.scale_shift );
    Next_Dropout:

      dropouts--;
    }
    goto Next_Line;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Render_Single_Pass                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Performs one sweep with sub-banding.                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    flipped :: whether or not we have to flip.                         */
  /*                                                                       */
  /* <Returns>                                                             */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  static
  int  Render_Single_Pass( RAS_ARG_ int  flipped )
  {
    TBand*  band;


    ras.flipped = flipped;

    band = ras.band_stack;

    FT_TRACE2(( "raster: entering render_single_pass (flipped = %d)\n",
              flipped ));

    while ( band >= ras.band_stack )
    {
      ras.maxY = ((long)band[0].y_max << PRECISION_BITS) - 1;
      ras.minY =  (long)band[0].y_min << PRECISION_BITS;

      ras.cursor = ras.pool;
      ras.error  = 0;

      FT_TRACE2(( "raster: band = [ %d, %d ]\n",
                band[0].y_min,
                band[0].y_max ));

      if ( Convert_Glyph( RAS_VAR_  ras.outline ) )
      {
        int  bottom, top, half;


        if ( ras.error != ErrRaster_Overflow )
          return FAILURE;
        ras.error = ErrRaster_Ok;

        FT_TRACE2(( "conversion failure, performing sub-banding\n" ));

        /* sub-banding */

#ifdef DEBUG_RASTER
        ClearBand( RAS_VAR_  TRUNC( ras.minY ), TRUNC( ras.maxY ) );
#endif

        bottom = band[0].y_min;
        top    = band[0].y_max;
        half   = ( top - bottom ) >> 1;

        if ( band >= ras.band_stack + 7 || half == 0 )
        {
          ras.band_top = 0;
          ras.error    = ErrRaster_Invalid_Outline;
          return ras.error;
        }

        band[1].y_min = bottom + half;
        band[1].y_max = top;
        band[0].y_max = bottom + half;

        band ++;
      }
      else
      {
        FT_TRACE2(( "conversion succeeded, span drawing sweep\n" ));
#if 1  /* for debugging */
        if ( ras.start_prof )
          if ( Draw_Sweep( RAS_VAR ) )
            return ras.error;
#endif
        band --;
      }
    }

    FT_TRACE2(( "raster: exiting render_single_pass\n" ));

    return SUCCESS;  /* success */
  }


  static
  int  Raster_Render1( FT_Raster  raster )
  {
    int  error;


    if ( ras.target.width > ABS(ras.target.pitch)*8 )
      return ErrRaster_Invalid_Map;

    ras.scale_shift  = PRECISION_BITS - INPUT_BITS;
    ras.scale_delta  = PRECISION_HALF;

    /* Vertical Sweep */
    ras.band_top            = 0;
    ras.band_stack[0].y_min = 0;
    ras.band_stack[0].y_max = ras.target.rows;

    ras.render     = vertical_render_mono;
    ras.bit_width  = ras.target.width;
    ras.bit_buffer = (unsigned char*)ras.target.buffer;

    if ( (error = Render_Single_Pass( RAS_VAR_ 0 )) != 0 )
      return error;

    /* Horizontal Sweep */

    if ( ras.second_pass && ras.dropout_mode != 0 )
    {
      ras.render              = horizontal_render_mono;
      ras.band_top            = 0;
      ras.band_stack[0].y_min = 0;
      ras.band_stack[0].y_max = ras.target.width;

      if ( (error = Render_Single_Pass( RAS_VAR_  1 )) != 0 )
        return error;
    }

    return ErrRaster_Ok;
  }


#ifdef FT_RASTER_OPTION_ANTI_ALIAS


  static
  int  Raster_Render8( FT_Raster  raster )
  {
    int  error;

    if ( ras.target.width > ABS(ras.target.pitch) )
      return ErrRaster_Invalid_Map;

    /* Vertical Sweep */
    ras.band_top            = 0;
    ras.band_stack[0].y_min = 0;
    ras.band_stack[0].y_max = ras.target.rows;

    ras.scale_shift  = PRECISION_BITS - INPUT_BITS;
    ras.scale_delta  = PRECISION_HALF;
    ras.dropout_mode = 2;

    ras.render     = vertical_render_gray;
    ras.bit_width  = ras.target.width;
    ras.bit_buffer = (unsigned char*)ras.target.buffer;
    ras.pix_buffer = (unsigned char*)ras.target.buffer;

    error = Render_Single_Pass( RAS_VAR_  0 );
    if ( error )
      return error;

#if 1
    /* Horizontal Sweep */
    ras.render              = horizontal_render_gray;
    ras.band_top            = 0;
    ras.bit_width           = ras.target.rows;
    ras.band_stack[0].y_min = 0;
    ras.band_stack[0].y_max = ras.target.width;

    return Render_Single_Pass( RAS_VAR_  1 );
#else
    return 0;
#endif
  }


#else  /* FT_RASTER_OPTION_ANTI_ALIAS */


  static
  int  Raster_Render8( FT_Raster  raster )
  {
    return ErrRaster_Unimplemented;
  }


#endif /* FT_RASTER_OPTION_ANTI_ALIAS */



  /**** RASTER OBJECT CREATION : in standalone mode, we simply use *****/
  /****                          a static object ..                *****/
#ifdef _STANDALONE_

  static
  int  ft_raster_new( void*  memory, FT_Raster *araster )
  {
     static FT_RasterRec_  the_raster;
     *araster = &the_raster;  
     memset( &the_raster, sizeof(the_raster), 0 );
     return 0;
  }

  static
  void  ft_raster_done( FT_Raster  raster )
  {
    /* nothing */
    raster->init = 0;
  }

#else

#include "ftobjs.h"

  static
  int  ft_raster_new( FT_Memory  memory, FT_Raster*  araster )
  {
    FT_Error  error;
    FT_Raster raster;
    
    *araster = 0;
    if ( !ALLOC( raster, sizeof(*raster) ))
    {
      raster->memory = memory;
      *araster = raster;
    }
      
    return error;
  }
  
  static
  void ft_raster_done( FT_Raster  raster )
  {
    FT_Memory  memory = (FT_Memory)raster->memory;
    FREE( raster );
  }
  
#endif


  static void ft_raster_reset( FT_Raster   raster,
                               const char* pool_base,
                               long        pool_size )
  {
    if ( raster && pool_base && pool_size >= 4096 )
    {
      /* save the pool */
      raster->pool      = (PPos)pool_base;
      raster->pool_size = raster->pool + pool_size / sizeof ( TPos );
    }
  }


  static
  int  ft_raster_render( FT_Raster          raster,
                         FT_Raster_Params*  params )
  {
    FT_Outline*  outline    = (FT_Outline*)params->source;
    FT_Bitmap*   target_map = params->target;
    
    if ( !raster || !raster->pool || !raster->pool_size )
      return ErrRaster_Uninitialized_Object;

    if ( !outline || !outline->contours || !outline->points )
      return ErrRaster_Invalid_Outline;

    /* return immediately if the outline is empty */
    if ( outline->n_points == 0 || outline->n_contours <= 0 )
      return ErrRaster_Ok;

    if ( outline->n_points != outline->contours[outline->n_contours - 1] + 1 )
      return ErrRaster_Invalid_Outline;

    if ( !target_map || !target_map->buffer )
      return ErrRaster_Invalid_Map;

    ras.outline  = outline;
    ras.target   = *target_map;
    
    /* Note that we always use drop-out mode 2, because it seems that */
    /* it's the only way to do to get results consistent with Windows */
    /* rendering..                                                    */
    ras.dropout_mode = 2;

    ras.second_pass  = (outline->flags & ft_outline_single_pass) == 0;
    SET_High_Precision( (char)((outline->flags & ft_outline_high_precision)!= 0) );

    /* this version of the raster does not support direct rendering, sorry */
    if ( params->flags & ft_raster_flag_direct )
      return ErrRaster_Unimplemented;

    return ( params->flags & ft_raster_flag_aa
           ? Raster_Render8( raster )
           : Raster_Render1( raster ) );
  }


  FT_Raster_Funcs      ft_default_raster =
  {
    ft_glyph_format_outline,
    (FT_Raster_New_Func)       ft_raster_new,
    (FT_Raster_Reset_Func)     ft_raster_reset,
    (FT_Raster_Set_Mode_Func)  0,
    (FT_Raster_Render_Func)    ft_raster_render,
    (FT_Raster_Done_Func)      ft_raster_done
  };


/* END */
