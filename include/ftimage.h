/***************************************************************************/
/*                                                                         */
/*  ftimage.h                                                              */
/*                                                                         */
/*  This file defines the glyph image formats recognized by FreeType, as   */
/*  well as the default raster interface.                                  */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg                       */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef FTIMAGE_H
#define FTIMAGE_H

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Pos                                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The type FT_Pos is a 32-bit integer used to store vectorial        */
  /*    coordinates.  Depending on the context, these can represent        */
  /*    distances in integer font units, or 26.6 fixed float pixel         */
  /*    coordinates.                                                       */
  /*                                                                       */
  typedef signed long  FT_Pos;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Vector                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to store a 2d vector; coordinates are of   */
  /*    the FT_Pos type.                                                   */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    x :: The horizontal coordinate.                                    */
  /*    y :: The vertical coordinate.                                      */
  /*                                                                       */
  typedef struct  FT_Vector_
  {
    FT_Pos  x;
    FT_Pos  y;

  } FT_Vector;


 /*************************************************************************
  *
  *  <Enum>
  *    FT_Pixel_Mode
  *
  *  <Description>
  *    An enumeration type used to describe the format of pixels
  *    in a given bitmap. Note that additional formats may be added
  *    in the future.
  *
  *  <Fields>
  *    ft_pixel_mode_mono   :: a monochrome bitmap (1 bit/pixel)
  *
  *    ft_pixel_mode_grays  :: an 8-bit gray-levels bitmap. Note that
  *                            the total number of gray levels is given
  *                            in the `num_grays' field of the FT_Bitmap
  *                            structure.
  *
  *    ft_pixel_mode_pal2   :: a 2-bit paletted bitmap.
  *                            currently unused by FreeType.
  *
  *    ft_pixel_mode_pal4   :: a 4-bit paletted bitmap.
  *                            currently unused by FreeType.
  *
  *    ft_pixel_mode_pal8   :: an 8-bit paletted bitmap.
  *                            currently unused by FreeType.
  *
  *    ft_pixel_mode_rgb15  :: a 15-bit RGB bitmap. Uses 5:5:5 encoding
  *                            currently unused by FreeType.
  *
  *    ft_pixel_mode_rgb16  :: a 16-bit RGB bitmap. Uses 5:6:5 encoding
  *                            currently unused by FreeType.
  *
  *    ft_pixel_mode_rgb24  :: a 24-bit RGB bitmap.
  *                            currently unused by FreeType.
  *
  *    ft_pixel_mode_rgb32  :: a 32-bit RGB bitmap.
  *                            currently unused by FreeType.
  *
  * <Note>
  *    Some anti-aliased bitmaps might be embedded in TrueType fonts
  *    using formats pal2 or pal4, though no fonts presenting those
  *    have been found to date..
  *
  *************************************************************************/

  typedef enum FT_Pixel_Mode_
  {
    ft_pixel_mode_none = 0,
    ft_pixel_mode_mono,
    ft_pixel_mode_grays,
    ft_pixel_mode_pal2,
    ft_pixel_mode_pal4,
    ft_pixel_mode_pal8,
    ft_pixel_mode_rgb15,
    ft_pixel_mode_rgb16,
    ft_pixel_mode_rgb24,
    ft_pixel_mode_rgb32,

    ft_pixel_mode_max      /* do not remove */

  } FT_Pixel_Mode;



 /*************************************************************************
  *
  *  <Enum>
  *    FT_Palette_Mode
  *
  *  <Description>
  *    An enumeration type used to describe the format of a bitmap
  *    palette, used with ft_pixel_mode_pal4 and ft_pixel_mode_pal8
  *
  *  <Fields>
  *    ft_palette_mode_rgb  :: the palette is an array of 3-bytes RGB records
  *
  *    ft_palette_mode_rgba :: the palette is an array of 4-bytes RGBA records
  *
  *  <Note>
  *    As ft_pixel_mode_pal2, pal4 and pal8 are currently unused by
  *    FreeType, these types are not handled by the library itself.
  *
  *************************************************************************/

  typedef enum FT_Palette_Mode_
  {
    ft_palette_mode_rgb = 0,
    ft_palette_mode_rgba,

    ft_palettte_mode_max   /* do not remove */

  } FT_Palette_Mode;


  /*************************************************************************
   *
   * <Struct>
   *    FT_Bitmap
   *
   * <Description>
   *    A structure used to describe a bitmap or pixmap to the raster.
   *    Note that we now manage pixmaps of various depths through the
   *    `pixel_mode' field.
   *
   * <Fields>
   *    rows         :: The number of bitmap rows.
   *
   *    width        :: The number of pixels in bitmap row.
   *
   *    pitch        :: The pitch's absolute value is the number of bytes
   *                    taken by one bitmap row, including padding. However,
   *                    the pitch is positive when the bitmap has a `down'
   *                    flow, and negative when it has an `up' flow. In all
   *                    cases, the pitch is an offset to add to a bitmap
   *                    pointer in order to go down one row.
   *
   *    buffer       :: A typeless pointer to the bitmap buffer. This value
   *                    should be aligned on 32-bit boundaries in most cases.
   *
   *    num_grays    :: this field is only used with ft_pixel_mode_grays,
   *                    it gives the number of gray levels used in the
   *                    bitmap.
   *
   *    pixel_mode   :: the pixel_mode, i.e. how pixel bits are stored
   *
   *    palette_mode :: this field is only used with paletted pixel modes,
   *                    it indicates how the palette is stored
   *
   *    palette      :: a typeless pointer to the bitmap palette. only used
   *                    for paletted pixel modes.
   *
   * <Note>
   *   When using pixel modes pal2, pal4 and pal8 with a void `palette'
   *   field, a gray pixmap with respectively 4, 16 and 256 levels of gray
   *   is assumed. This, in order to be compatible with some embedded bitmap
   *   formats defined in the TrueType spec.
   *
   *   Note that no font was found presenting such embedded bitmaps, so this
   *   is currently completely unhandled by the library.
   *
   *
   *************************************************************************/

  typedef struct FT_Bitmap_
  {
    int    rows;
    int    width;
    int    pitch;
    void*  buffer;
    short  num_grays;
    char   pixel_mode;
    char   palette_mode;
    void*  palette;

  } FT_Bitmap;



  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Outline                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This structure is used to describe an outline to the scan-line     */
  /*    converter.  It's a copy of the TT_Outline type that was defined    */
  /*    in FreeType 1.x.                                                   */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    n_contours :: The number of contours in the outline.               */
  /*                                                                       */
  /*    n_points   :: The number of points in the outline.                 */
  /*                                                                       */
  /*    points     :: A pointer to an array of `n_points' FT_Vector        */
  /*                  elements, giving the outline's point                 */
  /*                  coordinates.                                         */
  /*                                                                       */
  /*    tags       :: A pointer to an array of `n_points' chars,           */
  /*                  giving each outline point's type.  If bit 0 is       */
  /*                  unset, the point is 'off' the curve, i.e. a          */
  /*                  Bezier control point, while it is `on' when          */
  /*                  unset.                                               */
  /*                                                                       */
  /*                  Bit 1 is meaningful for `off' points only.  If       */
  /*                  set, it indicates a third-order Bezier arc           */
  /*                  control point; and a second-order control point      */
  /*                  if unset.                                            */
  /*                                                                       */
  /*    contours   :: An array of `n_contours' shorts, giving the end      */
  /*                  point of each contour within the outline.  For       */
  /*                  example, the first contour is defined by the         */
  /*                  points `0' to `contours[0]', the second one is       */
  /*                  defined by the points `contours[0]+1' to             */
  /*                  `contours[1]', etc.                                  */
  /*                                                                       */
  /*    flags      :: a set of bit flags used to characterize the          */
  /*                  outline and give hints to the scan-converter         */
  /*                  and hinter on how to convert/grid-fit it..           */
  /*                  see FT_Outline_Flags..                               */
  /*                                                                       */
  typedef struct  FT_Outline_
  {
    short       n_contours;      /* number of contours in glyph        */
    short       n_points;        /* number of points in the glyph      */

    FT_Vector*  points;          /* the outline's points               */
    char*       tags;            /* the points flags                   */
    short*      contours;        /* the contour end points             */

    int         flags;           /* outline masks                      */

  } FT_Outline;

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*   FT_Outline_Flags                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*   A simple type used to enumerates the flags in an outline's          */
  /*   "outline_flags" field.                                              */
  /*                                                                       */
  /* <Fields>                                                              */
  /*   ft_outline_owner  ::                                                */
  /*       when set, this flag indicates that the outline's field arrays   */
  /*       (i.e. "points", "flags" & "contours") are "owned" by the        */
  /*       outline object, and should thus be freed when it is destroyed.  */
  /*                                                                       */
  /*   ft_outline_even_odd_fill ::                                         */
  /*       by default, outlines are filled using the non-zero winding      */
  /*       rule. When set to 1, the outline will be filled using the       */
  /*       even-odd fill rule.. (XXX: unimplemented)                       */
  /*                                                                       */
  /*   ft_outline_reverse_fill ::                                          */
  /*       By default, outside contours of an outline are oriented in      */
  /*       clock-wise direction, as defined in the TrueType specification. */
  /*       This flag is set when the outline uses the opposite direction,  */
  /*       (typically for Type 1 fonts). This flag is ignored by the       */
  /*       scan-converter. However, it is very important for the           */
  /*       auto-hinter..                                                   */
  /*                                                                       */
  /*   ft_outline_ignore_dropouts ::                                       */
  /*       By default, the scan converter will try to detect drop-outs     */
  /*       in an outline and correct the glyph bitmap to ensure consistent */
  /*       shape continuity. When set, this flag hints the scan-line       */
  /*       converter to ignore such cases.                                 */
  /*                                                                       */
  /*   ft_outline_high_precision ::                                        */
  /*       this flag indicates that the scan-line converter should try     */
  /*       to convert this outline to bitmaps with the highest possible    */
  /*       quality. It is typically set for small character sizes. Note    */
  /*       that this is only a hint, that might be completely ignored      */
  /*       by a given scan-converter.                                      */
  /*                                                                       */
  /*   ft_outline_single_pass ::                                           */
  /*       this flag is set to force a given scan-converter to only        */
  /*       use a single pass over the outline to render a bitmap glyph     */
  /*       image. Normally, it is set for very large character sizes.      */
  /*       It is only a hint, that might be completely ignored by a        */
  /*       given scan-converter.                                           */
  /*                                                                       */
  typedef enum FT_Outline_Flags_
  {
    ft_outline_none            = 0,
    ft_outline_owner           = 1,
    ft_outline_even_odd_fill   = 2,
    ft_outline_reverse_fill    = 4,
    ft_outline_ignore_dropouts = 8,
    ft_outline_high_precision  = 256,
    ft_outline_single_pass     = 512

  } FT_Outline_Flags;



#define FT_CURVE_TAG( flag )  (flag & 3)

#define FT_Curve_Tag_On       1
#define FT_Curve_Tag_Conic    0
#define FT_Curve_Tag_Cubic    2

#define FT_Curve_Tag_Touch_X  8   /* reserved for the TrueType hinter */
#define FT_Curve_Tag_Touch_Y  16  /* reserved for the TrueType hinter */

#define FT_Curve_Tag_Touch_Both  ( FT_Curve_Tag_Touch_X | \
                                   FT_Curve_Tag_Touch_Y)

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Outline_MoveTo_Func                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function pointer type used to describe the signature of a `move  */
  /*    to' function during outline walking/decomposition.                 */
  /*                                                                       */
  /*    A `move to' is emitted to start a new contour in an outline.       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    to   :: A pointer to the target point of the `move to'.            */
  /*    user :: A typeless pointer which is passed from the caller of the  */
  /*            decomposition function.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  typedef int  (*FT_Outline_MoveTo_Func)( FT_Vector*  to,
                                          void*       user );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Outline_LineTo_Func                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function pointer type used to describe the signature of a `line  */
  /*    to' function during outline walking/decomposition.                 */
  /*                                                                       */
  /*    A `line to' is emitted to indicate a segment in the outline.       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    to   :: A pointer to the target point of the `line to'.            */
  /*    user :: A typeless pointer which is passed from the caller of the  */
  /*            decomposition function.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  typedef int  (*FT_Outline_LineTo_Func)( FT_Vector*  to,
                                          void*       user );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Outline_ConicTo_Func                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function pointer type use to describe the signature of a `conic  */
  /*    to' function during outline walking/decomposition.                 */
  /*                                                                       */
  /*    A `conic to' is emitted to indicate a second-order Bezier arc in   */
  /*    the outline.                                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    control :: An intermediate control point between the last position */
  /*               and the new target in `to'.                             */
  /*                                                                       */
  /*    to      :: A pointer to the target end point of the conic arc.     */
  /*                                                                       */
  /*    user    :: A typeless pointer which is passed from the caller of   */
  /*               the decomposition function.                             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  typedef int  (*FT_Outline_ConicTo_Func)( FT_Vector*  control,
                                           FT_Vector*  to,
                                           void*       user );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Outline_CubicTo_Func                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function pointer type used to describe the signature of a `cubic */
  /*    to' function during outline walking/decomposition.                 */
  /*                                                                       */
  /*    A `cubic to' is emitted to indicate a third-order Bezier arc.      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    control1 :: A pointer to the first Bezier control point.           */
  /*    control2 :: A pointer to the second Bezier control point.          */
  /*    to       :: A pointer to the target end point.                     */
  /*    user     :: A typeless pointer which is passed from the caller of  */
  /*                the decomposition function.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  typedef int  (*FT_Outline_CubicTo_Func)( FT_Vector*  control1,
                                           FT_Vector*  control2,
                                           FT_Vector*  to,
                                           void*       user );


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Outline_Funcs                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure to hold various function pointers used during outline  */
  /*    decomposition in order to emit segments, conic, and cubic Beziers, */
  /*    as well as `move to' and `close to' operations.                    */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    move_to  :: The `move to' emitter.                                 */
  /*    line_to  :: The segment emitter.                                   */
  /*    conic_to :: The second-order Bezier arc emitter.                   */
  /*    cubic_to :: The third-order Bezier arc emitter.                    */
  /*                                                                       */
  typedef struct  FT_Outline_Funcs_
  {
    FT_Outline_MoveTo_Func   move_to;
    FT_Outline_LineTo_Func   line_to;
    FT_Outline_ConicTo_Func  conic_to;
    FT_Outline_CubicTo_Func  cubic_to;

    int                      shift;
    FT_Pos                   delta;

  } FT_Outline_Funcs;


  /*************************************************************************/
  /*                                                                       */
  /* <Macro>                                                               */
  /*    FT_IMAGE_TAG                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This macro converts four letter tags which are used to label       */
  /*    TrueType tables into an unsigned long to be used within FreeType.  */
  /*                                                                       */
#define FT_IMAGE_TAG( _x1, _x2, _x3, _x4 ) \
          (((unsigned long)_x1 << 24) |        \
           ((unsigned long)_x2 << 16) |        \
           ((unsigned long)_x3 << 8)  |        \
            (unsigned long)_x4)


 /***********************************************************************
  *
  * <Enum>
  *    FT_Glyph_Format
  *
  * <Description>
  *    An enumeration type used to describethe format of a given glyph
  *    image. Note that this version of FreeType only supports two image
  *    formats, even though future font drivers will be able to register
  *    their own format.
  *
  * <Fields>
  *    ft_glyph_format_composite :: the glyph image is a composite of several
  *                                 other images. This glyph format is _only_
  *                                 used with the FT_LOAD_FLAG_NO_RECURSE flag
  *                                 (XXX: Which is currently iunimplemented)
  *
  *    ft_glyph_format_bitmap  :: the glyph image is a bitmap, and can
  *                               be described as a FT_Bitmap
  *
  *    ft_glyph_format_outline :: the glyph image is a vectorial image
  *                               made of bezier control points, and can
  *                               be described as a FT_Outline
  *
  *    ft_glyph_format_plotter :: the glyph image is a vectorial image
  *                               made of plotter lines (some T1 fonts like
  *                               Hershey contain glyph in this format).
  *
  ***********************************************************************/

  typedef enum FT_Glyph_Format_
  {
    ft_glyph_format_none      = 0,
    ft_glyph_format_composite = FT_IMAGE_TAG('c','o','m','p'),
    ft_glyph_format_bitmap    = FT_IMAGE_TAG('b','i','t','s'),
    ft_glyph_format_outline   = FT_IMAGE_TAG('o','u','t','l'),
    ft_glyph_format_plotter   = FT_IMAGE_TAG('p','l','o','t')

  } FT_Glyph_Format;

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****            R A S T E R   D E F I N I T I O N S                *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

 /**************************************************************************
  *
  *
  *
  *
  *
  *
  *
  *
  **************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Raster                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle (pointer) to a raster object.  Each object can be used    */
  /*    independently to convert an outline into a bitmap or pixmap.       */
  /*                                                                       */
  typedef struct FT_RasterRec_*  FT_Raster;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Span                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model a single span of gray (or black) pixels  */
  /*    when rendering a monocrhome or anti-aliased bitmap.                */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    x        :: the span's horizontal start position                   */
  /*    len      :: the span's length in pixels                            */
  /*    coverage :: the span color/coverage, ranging from 0 (background)   */
  /*                to 255 (foreground). Only used for anti-aliased        */
  /*                rendering..                                            */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This structure is used by the span drawing callback type           */
  /*    named FT_Raster_Span_Func, which takes the y coordinate of the     */
  /*    span as a paremeter..                                              */
  /*                                                                       */
  /*    The coverage value is always between 0 and 255, even if the        */
  /*    number of gray levels have been set through FT_Set_Gray_Levels()   */
  /*                                                                       */
  typedef struct FT_Span_
  {
    short          x;
    unsigned short len;
    unsigned char  coverage;

  } FT_Span;


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Raster_Span_Func                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used as a call-back by the anti-aliased renderer in     */
  /*    order to let client applications draw themselves the gray pixel    */
  /*    spans on each scan line.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*   y     :: the scanline's y coordinate                                */
  /*   count :: the number of spans to draw on this scanline               */
  /*   spans :: a table of 'count' spans to draw on the scanline           */
  /*   user  :: user-supplied data that is passed to the callback          */
  /*                                                                       */
  /* <Note>                                                                */
  /*   This callback allows client applications to directly render the     */
  /*   gray spans of the anti-aliased bitmap to any kind of surfaces.      */
  /*                                                                       */
  /*   This can be used to write anti-aliased outlines directly to a       */
  /*   given background bitmap, and even perform translucency..            */
  /*                                                                       */
  /*   Note that the "count" field cannot be greater than a fixed value    */
  /*   defined by the FT_MAX_GRAY_SPANS configuration macro in ftoption.h  */
  /*                                                                       */
  /*   By default, this value is set to 32, which means that if there are  */
  /*   more than 32 spans on a given scanline, the callback will be called */
  /*   several times with the same "y" parameter in order to draw all      */
  /*   callbacks..                                                         */
  /*                                                                       */
  /*   Otherwise, the callback is only called once per scan-line, and      */
  /*   only for those scanlines that do have "gray" pixels on them..       */
  /*                                                                       */
  typedef void (*FT_Raster_Span_Func)( int       y,
                                       int       count,
                                       FT_Span*  spans,
                                       void*     user );

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Raster_BitTest_Func                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used as a call-back by the monochrome scan-converter    */
  /*    to test wether a given target pixel is already set to the drawing  */
  /*    "color". These tests are crucial to implement drop-out control     */
  /*    per-se the TrueType spec..                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*   y     :: the pixel's y coordinate                                   */
  /*   x     :: the pixel's x coordinate                                   */
  /*   user  :: user-supplied data that is passed to the callback          */
  /*                                                                       */
  /* <Return>                                                              */
  /*   1 if the pixel is "set", 0 otherwise                                */
  /*                                                                       */
  typedef int (*FT_Raster_BitTest_Func)( int    y,
                                         int    x,
                                         void*  user );

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Raster_BitSet_Func                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used as a call-back by the monochrome scan-converter    */
  /*    used to set an individual target pixel. This is crucial to         */
  /*    implement drop-out control per-se the TrueType spec..              */
  /*                                                                       */
  /* <Input>                                                               */
  /*   y     :: the pixel's y coordinate                                   */
  /*   x     :: the pixel's x coordinate                                   */
  /*   user  :: user-supplied data that is passed to the callback          */
  /*                                                                       */
  /* <Return>                                                              */
  /*   1 if the pixel is "set", 0 otherwise                                */
  /*                                                                       */
  typedef void (*FT_Raster_BitSet_Func)( int    y,
                                         int    x,
                                         void*  user );


 /**************************************************************************
  *
  * <Enum>
  *    FT_Raster_Flag
  *
  * <Description>
  *    An enumeration used to list the bit flags used in the "flags"
  *    field of a FT_Raster_Params function.
  *
  * <Fields>
  *    ft_raster_flag_default  :: this value is 0
  *
  *    ft_raster_flag_aa       :: resquests the rendering of an anti-aliased
  *                               glyph bitmap. If unset, a monchrome bitmap
  *                               will be rendered.
  *
  *    ft_raster_flag_direct   :: requests direct rendering over the target
  *                               bitmap. Direct rendering uses user-provided
  *                               callbacks in order to perform direct
  *                               drawing or composition over an existing
  *                               bitmap. If this bit is unset, the content
  *                               of the target bitmap **must be zeroed** !
  *
  **************************************************************************/
  typedef enum {

    ft_raster_flag_default = 0,
    ft_raster_flag_aa      = 1,
    ft_raster_flag_direct  = 2

  } FT_Raster_Flag;

 /**************************************************************************
  *
  * <Struct>
  *    FT_Raster_Params
  *
  * <Description>
  *    A structure used to hold the arguments used by a raster's render
  *    function.
  *
  * <Fields>
  *    target      ::  the target bitmap
  *    source      ::  pointer to the source glyph image (e.g. a FT_Outline)
  *    flags       ::  rendering flags
  *    gray_spans  ::  gray span drawing callback
  *    black_spans ::  black span drawing callback
  *    bit_test    ::  bit test callback
  *    bit_set     ::  bit set callback
  *    user        ::  user-supplied data that is passed to each drawing
  *                    callback..
  *
  * <Note>
  *    An anti-aliased glyph bitmap is drawn if the ft_raster_flag_aa bit
  *    flag is set in the "flags" field, otherwise a monochrome bitmap will
  *    be generated.
  *
  *    When the ft_raster_flag_direct bit flag is set in "flags", the raster
  *    will call the "gray_spans" callback to drawn gray pixel spans, in the
  *    case of an aa glyph bitmap, or "black_spans", "bit_test" and "bit_set"
  *    in the case of a monochrome bitmap.
  *
  *    This allows direct composition over a pre-existing bitmap through
  *    user-provided callbacks to perform the span drawing/composition.
  *
  *    Note that the "bit_test" and "bit_set" callbacks are required when
  *    rendering a monochrome bitmap, as they are crucial to implement correct
  *    drop-out control per-se the TrueType specification..
  *
  **************************************************************************/

  typedef struct FT_Raster_Params_
  {
    FT_Bitmap*              target;
    void*                   source;
    int                     flags;
    FT_Raster_Span_Func     gray_spans;
    FT_Raster_Span_Func     black_spans;
    FT_Raster_BitTest_Func  bit_test;
    FT_Raster_BitSet_Func   bit_set;
    void*                   user;

  } FT_Raster_Params;



 /**************************************************************************
  * <FuncType>
  *    FT_Raster_New_Func
  *
  * <Description>
  *    A function used to create a new raster object.
  *
  * <Input>
  *    memory   :: handle to memory allocator.
  *
  * <Output>
  *    raster   :: handle to new raster object
  *
  * <Return>
  *    Error code. 0 means success
  *
  * <Note>
  *    the "memory" parameter is a typeless pointer in order to avoid
  *    un-wanted dependencies on the rest of the FreeType code.
  *
  *    in practice, it is a FT_Memory, i.e. a handle to the standard
  *    FreeType memory allocator. However, this field can be completely
  *    ignored by a given raster implementation..
  *
  **************************************************************************/

  typedef int (*FT_Raster_New_Func)( void*      memory,
                                     FT_Raster *raster );


 /**************************************************************************
  * <FuncType>
  *    FT_Raster_Done_Func
  *
  * <Description>
  *    A function used to destroy a given raster object.
  *
  * <Input>
  *    raster   :: handle to new raster object
  *
  **************************************************************************/

  typedef void (*FT_Raster_Done_Func)( FT_Raster  raster );



 /**************************************************************************
  *
  * <FuncType>
  *    FT_Raster_Reset_Func
  *
  * <Description>
  *    FreeType provides an area of memory called the "render pool",
  *    available to all registered rasters. This pool can be freely
  *    used during a given scan-conversion but is shared by all rasters.
  *    Its content is thus transient.
  *
  *    This function is called each time the render pool changes, or
  *    just after a new raster object is created.
  *
  * <Input>
  *    raster    :: handle to new raster object
  *    pool_base :: address in memory of render pool
  *    pool_size :: size in bytes of render pool
  *
  * <Note>
  *    Rasters can ignore the render pool and rely on dynamic memory
  *    allocation if they want to (a handle to the memory allocator is
  *    passed to the raster constructor). However, this is not recommended
  *    for efficiency purposes..
  *
  **************************************************************************/

  typedef void (*FT_Raster_Reset_Func)( FT_Raster    raster,
                                        const char*  pool_base,
                                        long         pool_size );


 /**************************************************************************
  *
  * <FuncType>
  *    FT_Raster_Set_Mode_Func
  *
  * <Description>
  *    This function is a generic facility to change modes or attributes
  *    in a given raster. This can be used for debugging purposes, or
  *    simply to allow implementation-specific "features" in a given
  *    raster module.
  *
  * <Input>
  *    raster    :: handle to new raster object
  *    mode      :: a 4-byte tag used to name the mode or property           
  *    args      :: a pointer to the new mode/property to use
  *
  **************************************************************************/

  typedef int (*FT_Raster_Set_Mode_Func)( FT_Raster      raster,
                                          unsigned long  mode,
                                          void*          args );

 /**************************************************************************
  *
  * <FuncType>
  *    FT_Raster_Render_Func
  *
  * <Description>
  *   Invokes a given raster to scan-convert a given glyph image into
  *   a target bitmap.
  *
  * <Input>
  *    raster :: handle to raster object
  *    params :: pointer to a FT_Raster_Params structure used to store
  *              the rendering parameters.
  *
  * <Return>
  *    Error code. 0 means success
  *
  * <Note>
  *    The exact format of the source image depends on the raster's
  *    glyph format defined in its FT_Raster_Funcs structure. It can be
  *    an FT_Outline or anything else in order to support a large array
  *    of glyph formats.
  *
  *    Note also that the render function can fail and return a
  *    FT_Err_Unimplemented_Feature error code when the raster used does
  *    not support direct composition.
  *
  *    XXX: For now, the standard raster doesn't support direct composition
  *         but this should change for the final release (see the files
  *         demos/src/ftgrays.c and demos/src/ftgrays2.c for examples of
  *         distinct implementations which support direct composition).
  *
  **************************************************************************/

  typedef int  (*FT_Raster_Render_Func)( FT_Raster          raster,
                                         FT_Raster_Params*  params );


 /**************************************************************************
  *
  * <Struct>
  *    FT_Raster_Funcs
  *
  * <Description>
  *   A structure used to describe a given raster class to the library.
  *
  * <Fields>
  *    glyph_format     :: the supported glyph format for this raster
  *    raster_new       :: the raster constructor
  *    raster_reset     :: used to reset the render pool within the raster
  *    raster_render    :: renders a glyph into a given bitmap
  *    raster_done      :: the raster destructor
  *
  **************************************************************************/


  typedef struct FT_Raster_Funcs_
  {
    FT_Glyph_Format          glyph_format;
    FT_Raster_New_Func       raster_new;
    FT_Raster_Reset_Func     raster_reset;
    FT_Raster_Set_Mode_Func  raster_set_mode;
    FT_Raster_Render_Func    raster_render;
    FT_Raster_Done_Func      raster_done;

  } FT_Raster_Funcs;

#endif /* FTIMAGE_H */


