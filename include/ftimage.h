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
  *    FT_Glyph_Tag
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
  
  typedef enum FT_Glyph_Tag_
  {
    ft_glyph_format_none      = 0,
    ft_glyph_format_composite = FT_IMAGE_TAG('c','o','m','p'),
    ft_glyph_format_bitmap    = FT_IMAGE_TAG('b','i','t','s'),
    ft_glyph_format_outline   = FT_IMAGE_TAG('o','u','t','l'),
    ft_glyph_format_plotter   = FT_IMAGE_TAG('p','l','o','t')
  
  } FT_Glyph_Tag;


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
  /* <FuncType>                                                            */
  /*    FT_Raster_Init_Proc                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a fresh raster object which should have been allocated */
  /*    by client applications.  This function is also used to set the     */
  /*    object's render pool.  It can be used repeatedly on a single       */
  /*    object if one wants to change the pool's address or size.          */
  /*                                                                       */
  /*    Note that the render pool has no state and is only used during a   */
  /*    call to FT_Raster_Render().  It is thus theorically possible to    */
  /*    share it between several non-concurrent components of your         */
  /*    applications when memory is a scarce resource.                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    raster    :: a handle to the target raster object.                 */
  /*    pool_base :: the render pool's base address in memory              */
  /*    pool_size :: the render pool's size in bytes.  this must be at     */
  /*                 least 4 kByte.                                        */
  /* <Return>                                                              */
  /*    An error condition, used as a FT_Error in the FreeType library.    */
  /*    0 means success.                                                   */
  /*                                                                       */
  typedef int (*FT_Raster_Init_Proc)( FT_Raster    raster,
                                      const char*  pool_base,
                                      long         pool_size );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Raster_Set_Mode_Proc                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Some raster implementations may have several modes of operation.   */
  /*    This function is used to select one of them, as well as pass some  */
  /*    arguments.                                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    raster  :: The target raster object.                               */
  /*                                                                       */
  /*    mode    :: A pointer used to describe the mode to set. This is     */
  /*               completely raster-specific, and could be, for example,  */
  /*               a text string.                                          */
  /*                                                                       */
  /*    args    :: An argument to the set_mode command. This is completely */
  /*               specific to the raster and the mode used.               */
  /*                                                                       */
  /* <Return>                                                              */
  /*    An error code, used as a FT_Error by the FreeType library.         */
  /*    0 means success.                                                   */
  /*                                                                       */
  typedef int (*FT_Raster_Set_Mode_Proc)( FT_Raster    raster,
                                          const char*  mode,
                                          const char*  args );
                        
                                          
  /*************************************************************************
   *                                                                       
   * <FuncType>                                                            
   *    FT_Raster_Render_Proc                                              
   *                                                                       
   * <Description>                                                         
   *    Renders an outline into a target bitmap/pixmap.                    
   *                                                                       
   * <Input>                                                               
   *    raster        :: A handle to a raster object used during rendering.
   *
   *    source_image  :: a typeless pointer to the source glyph image.
   *                     (usually a FT_Outline*).
   *
   *    target_bitmap :: descriptor to the target bitmap.
   *
   * <Return>                                                              
   *    Error code, interpreted as a FT_Error by FreeType library.         
   *    0 means success.                                                   
   *                                                                       
   *************************************************************************/
   
  typedef int  (*FT_Raster_Render_Proc)( FT_Raster       raster,
                                         void*           source_image,
                                         FT_Bitmap*      target_bitmap );


 /**************************************************************************
  *
  * <Struct>
  *    FT_Raster_Interface
  *
  * <Description>
  *    A structure used to model the default raster interface. A raster
  *    is a module in charge of converting a glyph image into a bitmap.
  * 
  * <Fields>
  *    size      :: the size in bytes of the given raster object. This
  *                 is used to allocate a new raster when calling
  *                 `FT_Set_Raster'.
  *
  *    format    :: the source glyph image format this raster is able to
  *                 handle.
  *
  *    init      :: the raster's initialisation routine
  *
  *    set_mode  :: the raster's mode set routine
  *
  *    render    :: the raster's rendering routine
  *
  **************************************************************************/
  
  typedef struct FT_Raster_Interface_
  {
    long                     size;
    FT_Glyph_Tag             format_tag;
    FT_Raster_Init_Proc      init;
    FT_Raster_Set_Mode_Proc  set_mode;
    FT_Raster_Render_Proc    render;
    
  
  } FT_Raster_Interface;

 
#endif /* FTIMAGE_H */


