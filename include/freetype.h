/***************************************************************************/
/*                                                                         */
/*  freetype.h                                                             */
/*                                                                         */
/*    FreeType high-level API and common types (specification only).       */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef FREETYPE_H
#define FREETYPE_H


  /*************************************************************************/
  /*                                                                       */
  /* The `raster' component duplicates some of the declarations in         */
  /* freetype.h for stand-alone use if _FREETYPE_ isn't defined.           */
  /*                                                                       */
#define _FREETYPE_


  /*************************************************************************/
  /*                                                                       */
  /* The FREETYPE_MAJOR and FREETYPE_MINOR macros are used to version the  */
  /* new FreeType design, which is able to host several kinds of font      */
  /* drivers.  It starts at 2.0.  Note that each driver has its own        */
  /* version number (for example, the TrueType driver is at 1.2, as        */
  /* defined by the macros TT_FREETYPE_MAJOR and TT_FREETYPE_MINOR in the  */
  /* file `ttlib/truetype.h'.                                              */
  /*                                                                       */
#define FREETYPE_MAJOR 2
#define FREETYPE_MINOR 0


  /*************************************************************************/
  /*                                                                       */
  /* To make freetype.h independent from configuration files we check      */
  /* whether EXPORT_DEF has been defined already.                          */
  /*                                                                       */
  /* On some systems and compilers (Win32 mostly), an extra keyword is     */
  /* necessary to compile the library as a DLL.                            */
  /*                                                                       */
#ifndef EXPORT_DEF
#define EXPORT_DEF  extern
#endif

#include <fterrors.h>
#include <ftsystem.h>
#include <ftimage.h>

#ifdef __cplusplus
  extern "C" {
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Bool                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef of unsigned char, used for simple booleans.              */
  /*                                                                       */
  typedef unsigned char  FT_Bool;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_FWord                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 16-bit integer used to store a distance in original font  */
  /*    units.                                                             */
  /*                                                                       */
  typedef signed short    FT_FWord;   /* Distance in FUnits */


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UFWord                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An unsigned 16-bit integer used to store a distance in original    */
  /*    font units.                                                        */
  /*                                                                       */
  typedef unsigned short  FT_UFWord;  /* Unsigned distance */


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Char                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the _signed_ char type.                       */
  /*                                                                       */
  typedef signed char  FT_Char;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Byte                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the _unsigned_ char type.                     */
  /*                                                                       */
  typedef unsigned char  FT_Byte;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_String                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the char type, usually used for strings.      */
  /*                                                                       */
  typedef char  FT_String;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Short                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for signed short.                                        */
  /*                                                                       */
  typedef signed short  FT_Short;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UShort                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for unsigned short.                                      */
  /*                                                                       */
  typedef unsigned short  FT_UShort;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Int                                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for the int type.                                        */
  /*                                                                       */
  typedef int  FT_Int;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UInt                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for the unsigned int type.                               */
  /*                                                                       */
  typedef unsigned int  FT_UInt;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Long                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for signed long.                                         */
  /*                                                                       */
  typedef signed long  FT_Long;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_ULong                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for unsigned long.                                       */
  /*                                                                       */
  typedef unsigned long  FT_ULong;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_F2Dot14                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 2.14 fixed float type used for unit vectors.              */
  /*                                                                       */
  typedef signed short  FT_F2Dot14;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_F26Dot6                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 26.6 fixed float type used for vectorial pixel            */
  /*    coordinates.                                                       */
  /*                                                                       */
  typedef signed long  FT_F26Dot6;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Fixed                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This type is used to store 16.16 fixed float values, like scales   */
  /*    or matrix coefficients.                                            */
  /*                                                                       */
  typedef signed long  FT_Fixed;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Error                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The FreeType error code type.  A value of 0 is always interpreted  */
  /*    as a successful operation.                                         */
  /*                                                                       */
  typedef int  FT_Error;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Pointer                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for a typeless pointer.                           */
  /*                                                                       */
  typedef void*  FT_Pointer;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_UnitVector                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to store a 2d vector unit vector.  Uses    */
  /*    FT_F2Dot14 types.                                                  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    x :: Horizontal coordinate.                                        */
  /*    y :: Vertical coordinate.                                          */
  /*                                                                       */
  typedef struct  FT_UnitVector_
  {
    FT_F2Dot14  x;
    FT_F2Dot14  y;

  } FT_UnitVector;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Matrix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to store a 2x2 matrix.  Coefficients are   */
  /*    in 16.16 fixed float format.  The computation performed is:        */
  /*                                                                       */
  /*       {                                                               */
  /*          x' = x*xx + y*xy                                             */
  /*          y' = x*yx + y*yy                                             */
  /*       }                                                               */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    xx :: Matrix coefficient.                                          */
  /*    xy :: Matrix coefficient.                                          */
  /*    yx :: Matrix coefficient.                                          */
  /*    yy :: Matrix coefficient.                                          */
  /*                                                                       */
  typedef struct  FT_Matrix_
  {
    FT_Fixed  xx, xy;
    FT_Fixed  yx, yy;

  } FT_Matrix;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_BBox                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold an outline's bounding box, i.e., the      */
  /*    coordinates of its extrema in the horizontal and vertical          */
  /*    directions.                                                        */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    xMin :: The horizontal minimum (left-most).                        */
  /*    yMin :: The vertical minimum (bottom-most).                        */
  /*    xMax :: The horizontal maximum (right-most).                       */
  /*    yMax :: The vertical maximum (top-most).                           */
  /*                                                                       */
  typedef struct  FT_BBox_
  {
    FT_Pos  xMin, yMin;
    FT_Pos  xMax, yMax;

  } FT_BBox;


  /*************************************************************************/
  /*                                                                       */
  /* <Macro>                                                               */
  /*    FT_MAKE_TAG                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This macro converts four letter tags which are used to label       */
  /*    TrueType tables into an unsigned long to be used within FreeType.  */
  /*                                                                       */
#define FT_MAKE_TAG( _x1, _x2, _x3, _x4 ) \
          (((FT_ULong)_x1 << 24) |        \
           ((FT_ULong)_x2 << 16) |        \
           ((FT_ULong)_x3 << 8)  |        \
            (FT_ULong)_x4)


  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                    L I S T   M A N A G E M E N T                      */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_ListNode                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*     Many elements and objects in FreeType are listed through a        */
  /*     FT_List record (see FT_ListRec).  As its name suggests, a         */
  /*     FT_ListNode is a handle to a single list element.                 */
  /*                                                                       */
  typedef struct FT_ListNodeRec_*  FT_ListNode;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_List                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a list record (see FT_ListRec).                        */
  /*                                                                       */
  typedef struct FT_ListRec_*  FT_List;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_ListNodeRec                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold a single list element.                    */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    prev :: Previous element in the list.  NULL if first.              */
  /*    next :: Next element in the list.  NULL if last.                   */
  /*    data :: Typeless pointer to the listed object.                     */
  /*                                                                       */
  typedef struct  FT_ListNodeRec_
  {
    FT_ListNode  prev;
    FT_ListNode  next;
    void*        data;

  } FT_ListNodeRec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_ListRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold a simple doubly-linked list.  These are   */
  /*    used in many parts of FreeType.                                    */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    head :: Head (first element) of doubly-linked list.                */
  /*    tail :: Tail (last element) of doubly-linked list.                 */
  /*                                                                       */
  typedef struct  FT_ListRec_
  {
    FT_ListNode  head;
    FT_ListNode  tail;

  } FT_ListRec;


#define FT_IS_EMPTY(list)  ( (list).head == 0 )


  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                        B A S I C   T Y P E S                          */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Glyph_Metrics                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model the metrics of a single glyph.  Note     */
  /*    that values are expressed in 26.6 fractional pixel format or in    */
  /*    font units, depending on context.                                  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    width        :: The glyph's width.                                 */
  /*    height       :: The glyph's height.                                */
  /*                                                                       */
  /*    horiBearingX :: Horizontal left side bearing.                      */
  /*    horiBearingY :: Horizontal top side bearing.                       */
  /*    horiAdvance  :: Horizontal advance width.                          */
  /*                                                                       */
  /*    vertBearingX :: Vertical left side bearing.                        */
  /*    vertBearingY :: Vertical top side bearing.                         */
  /*    vertAdvance  :: Vertical advance height.                           */
  /*                                                                       */
  typedef struct  FT_Glyph_Metrics_
  {
    FT_Pos  width;         /* glyph width  */
    FT_Pos  height;        /* glyph height */

    FT_Pos  horiBearingX;  /* left side bearing in horizontal layouts */
    FT_Pos  horiBearingY;  /* top side bearing in horizontal layouts  */
    FT_Pos  horiAdvance;   /* advance width for horizontal layout     */

    FT_Pos  vertBearingX;  /* left side bearing in vertical layouts */
    FT_Pos  vertBearingY;  /* top side bearing in vertical layouts  */
    FT_Pos  vertAdvance;   /* advance height for vertical layout    */

  } FT_Glyph_Metrics;


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Generic_Finalizer                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Describes a function used to destroy the `client' data of any      */
  /*    FreeType object.  See the description of the FT_Generic type for   */
  /*    details of usage.                                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    The address of the FreeType object which is under finalisation.    */
  /*    Its client data is accessed through its `generic' field.           */
  /*                                                                       */
  typedef void  (*FT_Generic_Finalizer)(void*  object);


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Generic                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Client applications often need to associate their own data to a    */
  /*    variety of FreeType core objects.  For example, a text layout API  */
  /*    might want to associate a glyph cache to a given size object.      */
  /*                                                                       */
  /*    Most FreeType object contains a `generic' field, of type           */
  /*    FT_Generic, which usage is left to client applications and font    */
  /*    servers.                                                           */
  /*                                                                       */
  /*    It can be used to store a pointer to client-specific data, as well */
  /*    as the address of a `finalizer' function, which will be called by  */
  /*    FreeType when the object is destroyed (for example, the previous   */
  /*    client example would put the address of the glyph cache destructor */
  /*    in the `finalizer' field).                                         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    data      :: A typeless pointer to any client-specified data. This */
  /*                 field is completely ignored by the FreeType library.  */
  /*                                                                       */
  /*    finalizer :: A pointer to a `generic finalizer' function, which    */
  /*                 will be called when the object is destroyed.  If this */
  /*                 field is set to NULL, no code will be called.         */
  /*                                                                       */
  typedef struct  FT_Generic_
  {
    void*                 data;
    FT_Generic_Finalizer  finalizer;

  } FT_Generic;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Bitmap_Size                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An extremely simple structure used to model the size of a bitmap   */
  /*    strike (i.e., a bitmap instance of the font for a given            */
  /*    resolution) in a fixed-size font face.  This is used for the       */
  /*    `available_sizes' field of the FT_Face_Properties structure.       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    height :: The character height in pixels.                          */
  /*    width  :: The character width in pixels.                           */
  /*                                                                       */
  typedef struct  FT_Bitmap_Size_
  {
    FT_Short  height;
    FT_Short  width;

  } FT_Bitmap_Size;


  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                     O B J E C T   C L A S S E S                       */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Library                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a FreeType library instance.  Each `library' is        */
  /*    completely independent from the others; it is the `root' of a set  */
  /*    of objects like fonts, faces, sizes, etc.                          */
  /*                                                                       */
  /*    It also embeds a system object (see FT_System), as well as a       */
  /*    scan-line converter object (see FT_Raster).                        */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Library objects are created through FT_Init_FreeType().            */
  /*                                                                       */
  typedef struct FT_LibraryRec_  *FT_Library;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Driver                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a given FreeType font driver object.  Each font driver */
  /*    is able to create faces, sizes, glyph slots, and charmaps from the */
  /*    resources whose format it supports.                                */
  /*                                                                       */
  /*    A driver can support either bitmap, graymap, or scalable font      */
  /*    formats.                                                           */
  /*                                                                       */
  typedef struct FT_DriverRec_*  FT_Driver;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Face                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a given driver face object.  A face object contains    */
  /*    all the instance and glyph independent data of a font file         */
  /*    typeface.                                                          */
  /*                                                                       */
  /*    A face object is created from a resource object through the        */
  /*    new_face() method of a given driver.                               */
  /*                                                                       */
  typedef struct FT_FaceRec_*  FT_Face;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Size                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a given driver size object.  Such an object models the */
  /*    _resolution_ AND _size_ dependent state of a given driver face     */
  /*    size.                                                              */
  /*                                                                       */
  /*    A size object is always created from a given face object.  It is   */
  /*    discarded automatically by its parent face.                        */
  /*                                                                       */
  typedef struct FT_SizeRec_*  FT_Size;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_GlyphSlot                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a given `glyph slot'.  A slot is a container where it  */
  /*    is possible to load any of the glyphs contained within its parent  */
  /*    face.                                                              */
  /*                                                                       */
  /*    A glyph slot is created from a given face object.  It is discarded */
  /*    automatically by its parent face.                                  */
  /*                                                                       */
  typedef struct FT_GlyphSlotRec_*  FT_GlyphSlot;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_CharMap                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a given character map.  A charmap is used to translate */
  /*    character codes in a given encoding into glyph indexes for its     */
  /*    parent's face.  Some font formats may provide several charmaps per */
  /*    font.                                                              */
  /*                                                                       */
  /*    A charmap is created from a given face object.  It is discarded    */
  /*    automatically by its parent face.                                  */
  /*                                                                       */
  typedef struct FT_CharMapRec_*  FT_CharMap;


  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_Encoding                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration used to specify encodings supported by charmaps.    */
  /*    Used in the FT_Select_CharMap() API function.                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Because of 32-bit charcodes defined in Unicode (i.e., surrogates), */
  /*    all character codes must be expressed as FT_Longs.                 */
  /*                                                                       */
  typedef enum  FT_Encoding_
  {
    ft_encoding_none    = 0,
    ft_encoding_symbol  = 0,
    ft_encoding_unicode = FT_MAKE_TAG('u','n','i','c'),
    ft_encoding_latin_2 = FT_MAKE_TAG('l','a','t','2'),
    ft_encoding_sjis    = FT_MAKE_TAG('s','j','i','s'),
    ft_encoding_big5    = FT_MAKE_TAG('b','i','g','5'),

    ft_encoding_adobe_standard = FT_MAKE_TAG('a','d','o','b'),
    ft_encoding_adobe_expert   = FT_MAKE_TAG('a','d','b','e'),
    ft_encoding_adobe_custom   = FT_MAKE_TAG('a','d','b','c'),

    ft_encoding_apple_roman    = FT_MAKE_TAG('a','r','m','n')

    /* other encodings might be defined in the future */

  } FT_Encoding;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_CharMapRec                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The base charmap class.                                            */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    face        :: A handle to the parent face object.                 */
  /*                                                                       */
  /*    flags       :: A set of bit flags used to describe the charmap.    */
  /*                   Each bit indicates that a given encoding is         */
  /*                   supported.                                          */
  /*                                                                       */
  /*    platform_id :: An ID number describing the platform for the        */
  /*                   following encoding ID.  This comes directly from    */
  /*                   the TrueType specification and should be emulated   */
  /*                   for other formats.                                  */
  /*                                                                       */
  /*    encoding_id :: A platform specific encoding number.  This also     */
  /*                   comes from the TrueType specification and should be */
  /*                   emulated similarly.                                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    We STRONGLY recommmend emulating a Unicode charmap for drivers     */
  /*    that do not support TrueType or OpenType.                          */
  /*                                                                       */
  typedef struct  FT_CharMapRec_
  {
    FT_Face      face;
    FT_Encoding  encoding;
    FT_UShort    platform_id;
    FT_UShort    encoding_id;

  } FT_CharMapRec;





  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                 B A S E   O B J E C T   C L A S S E S                 */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/



  /*************************************************************************/
  /*                                                                       */
  /*                       FreeType base face class                        */
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_FaceRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    FreeType root face class structure.  A face object models the      */
  /*    resolution and point-size independent data found in a font file.   */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_faces           :: In the case where the face is located in a  */
  /*                           collection (i.e., a resource which embeds   */
  /*                           several faces), this is the total number of */
  /*                           faces found in the resource.  1 by default. */
  /*                                                                       */
  /*    face_index          :: The index of the face in its resource.      */
  /*                           Usually, this is 0 for all normal font      */
  /*                           formats.  It can be more in the case of     */
  /*                           collections (which embed several fonts in a */
  /*                           single resource/file).                      */
  /*                                                                       */
  /*    face_flags          :: A set of bit flags that give important      */
  /*                           information about the face; see the         */
  /*                           FT_FACE_FLAG_XXX macros for details.        */
  /*                                                                       */
  /*    style_flags         :: A set of bit flags indicating the style of  */
  /*                           the face (i.e., italic, bold, underline,    */
  /*                           etc).                                       */
  /*                                                                       */
  /*    num_glyphs          :: The total number of glyphs in the face.     */
  /*                                                                       */
  /*    family_name         :: The face's family name.  This is an ASCII   */
  /*                           string, usually in English, which describes */
  /*                           the typeface's family (like `Times New      */
  /*                           Roman', `Bodoni', `Garamond', etc).  This   */
  /*                           is a least common denominator used to list  */
  /*                           fonts.  Some formats (TrueType & OpenType)  */
  /*                           provide localized and Unicode versions of   */
  /*                           this string.  Applications should use the   */
  /*                           format specific interface to access them.   */
  /*                                                                       */
  /*    style_name          :: The face's style name.  This is an ASCII    */
  /*                           string, usually in English, which describes */
  /*                           the typeface's style (like `Italic',        */
  /*                           `Bold', `Condensed', etc).  Not all font    */
  /*                           formats provide a style name, so this field */
  /*                           is optional, and can be set to NULL.  As    */
  /*                           for `family_name', some formats provide     */
  /*                           localized/Unicode versions of this string.  */
  /*                           Applications should use the format specific */
  /*                           interface to access them.                   */
  /*                                                                       */
  /*    num_fixed_sizes     :: The number of fixed sizes available in this */
  /*                           face.  This should be set to 0 for scalable */
  /*                           fonts, unless its resource includes a       */
  /*                           complete set of glyphs (called a `strike')  */
  /*                           for the specified size.                     */
  /*                                                                       */
  /*    available_sizes     :: An array of sizes specifying the available  */
  /*                           bitmap/graymap sizes that are contained in  */
  /*                           in the font resource.  Should be set to     */
  /*                           NULL if the field `num_fixed_sizes' is set  */
  /*                           to 0.                                       */
  /*                                                                       */
  /*    num_charmaps        :: The total number of character maps in the   */
  /*                           face.                                       */
  /*                                                                       */
  /*    charmaps            :: A table of pointers to the face's charmaps  */
  /*                           Used to scan the list of available charmaps */
  /*                           this table might change after a call to     */
  /*                           FT_Attach_File/Stream (e.g. when it used    */
  /*                           to hook and additional encoding/CMap to     */
  /*                           the face object).                           */
  /*                                                                       */
  /*    generic             :: A field reserved for client uses.  See the  */
  /*                           FT_Generic type description.                */
  /*                                                                       */
  /*    bbox                :: The font bounding box.  Coordinates are     */
  /*                           expressed in font units (see units_per_EM). */
  /*                           The box is large enough to contain any      */
  /*                           glyph from the font.  Thus, bbox.yMax can   */
  /*                           be seen as the `maximal ascender',          */
  /*                           bbox.yMin as the `minimal descender', and   */
  /*                           the maximum glyph width is given by         */
  /*                           `bbox.xMax-bbox.xMin' (not to be confused   */
  /*                           with the maximum _advance_width_).  Only    */
  /*                           relevant for scalable formats.              */
  /*                                                                       */
  /*    units_per_EM        :: The number of font units per EM square for  */
  /*                           this face.  This is typically 2048 for      */
  /*                           TrueType fonts, 1000 for Type1 fonts, and   */
  /*                           should be set to the (unrealistic) value 1  */
  /*                           for fixed-sizes fonts.  Only relevant for   */
  /*                           scalable formats.                           */
  /*                                                                       */
  /*    ascender            :: The face's ascender is the vertical         */
  /*                           distance from the baseline to the topmost   */
  /*                           point of any glyph in the face.  This       */
  /*                           field's value is positive, expressed in     */
  /*                           font units.  Some font designs use a value  */
  /*                           different from `bbox.yMax'.  Only relevant  */
  /*                           for scalable formats.                       */
  /*                                                                       */
  /*    descender           :: The face's descender is the vertical        */
  /*                           distance from the baseline to the           */
  /*                           bottommost point of any glyph in the face.  */
  /*                           This field's value is positive, expressed   */
  /*                           in font units.  Some font designs use a     */
  /*                           value different from `-bbox.yMin'.  Only    */
  /*                           relevant for scalable formats.              */
  /*                                                                       */
  /*    height              :: The face's height is the vertical distance  */
  /*                           from one baseline to the next when writing  */
  /*                           several lines of text.  Its value is always */
  /*                           positive, expressed in font units.  The     */
  /*                           value can be computed as                    */
  /*                           `ascender+descender+line_gap' where the     */
  /*                           value of `line_gap' is also called          */
  /*                           `external leading'.  Only relevant for      */
  /*                           scalable formats.                           */
  /*                                                                       */
  /*    max_advance_width   :: The maximum advance width, in font units,   */
  /*                           for all glyphs in this face.  This can be   */
  /*                           used to make word wrapping computations     */
  /*                           faster.  Only relevant for scalable         */
  /*                           formats.                                    */
  /*                                                                       */
  /*    max_advance_height  :: The maximum advance height, in font units,  */
  /*                           for all glyphs in this face.  This is only  */
  /*                           relevant for vertical layouts, and should   */
  /*                           be set to the `height' for fonts that do    */
  /*                           not provide vertical metrics.  Only         */
  /*                           relevant for scalable formats.              */
  /*                                                                       */
  /*    underline_position  :: The position, in font units, of the         */
  /*                           underline line for this face.  It's the     */
  /*                           center of the underlining stem.  Only       */
  /*                           relevant for scalable formats.              */
  /*                                                                       */
  /*    underline_thickness :: The thickness, in font units, of the        */
  /*                           underline for this face.  Only relevant for */
  /*                           scalable formats.                           */
  /*                                                                       */
  /*    driver              :: A handle to the face's parent driver        */
  /*                           object.                                     */
  /*                                                                       */
  /*    memory              :: A handle to the face's parent memory        */
  /*                           object.  Used for the allocation of         */
  /*                           subsequent objects.                         */
  /*                                                                       */
  /*    stream              :: A handle to the face's stream.              */
  /*                                                                       */
  /*    glyph               :: The face's associated glyph slot(s).  This  */
  /*                           object is created automatically with a new  */
  /*                           face object.  However, certain kinds of     */
  /*                           applications (mainly tools like converters) */
  /*                           can need more than one slot to ease their   */
  /*                           task.                                       */
  /*                                                                       */
  /*    sizes_list          :: The list of child sizes for this face.      */
  /*                                                                       */
  /*    max_points          :: The maximum number of points used to store  */
  /*                           the vectorial outline of any glyph in this  */
  /*                           face.  If this value cannot be known in     */
  /*                           advance, or if the face isn't scalable,     */
  /*                           this should be set to 0.  Only relevant for */
  /*                           scalable formats.                           */
  /*                                                                       */
  /*    max_contours        :: The maximum number of contours used to      */
  /*                           store the vectorial outline of any glyph in */
  /*                           this face.  If this value cannot be known   */
  /*                           in advance, or if the face isn't scalable,  */
  /*                           this should be set to 0.  Only relevant for */
  /*                           scalable formats.                           */
  /*                                                                       */
  /*    transform_matrix    :: a 2x2 matrix of 16.16 coefficients used     */
  /*                           to transform glyph outlines after they're   */
  /*                           loaded from the font. Only used by the      */
  /*                           convenience functions.                      */
  /*                                                                       */
  /*    transform_delta     :: a translation vector used to transform      */
  /*                           glyph outlines after they're loaded from    */
  /*                           the font. Only used by the convenience      */
  /*                           functions.                                  */
  /*                                                                       */
  /*    transform_flags     :: some flags used to classify the transform.  */
  /*                           Only used by the convenience functions.     */
  /*                                                                       */
  typedef struct  FT_FaceRec_
  {
    FT_Long          num_faces;
    FT_Long          face_index;

    FT_Long          face_flags;
    FT_Long          style_flags;

    FT_Long          num_glyphs;

    FT_String*       family_name;
    FT_String*       style_name;

    FT_Int           num_fixed_sizes;
    FT_Bitmap_Size*  available_sizes;

    /* the face's table of available charmaps */
    FT_Int           num_charmaps;
    FT_CharMap*      charmaps;

    FT_Generic       generic;

    /* the following are only relevant for scalable outlines */
    FT_BBox          bbox;

    FT_UShort        units_per_EM;
    FT_Short         ascender;
    FT_Short         descender;
    FT_Short         height;

    FT_Short         max_advance_width;
    FT_Short         max_advance_height;

    FT_Short         underline_position;
    FT_Short         underline_thickness;

    /************************************************************/
    /* The following fields should be considered private and    */
    /* rarely, if ever, used by client applications..           */

    FT_Driver        driver;
    FT_Memory        memory;
    FT_Stream        stream;

    FT_GlyphSlot     glyph;
    FT_Size          size;
    FT_CharMap       charmap;
    FT_ListRec       sizes_list;

    void*            extensions;

    FT_UShort        max_points;
    FT_Short         max_contours;

    FT_Matrix        transform_matrix;
    FT_Vector        transform_delta;
    FT_Int           transform_flags;

  } FT_FaceRec;


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_FACE_FLAG_SCALABLE                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used to indicate that a given face provides  */
  /*    vectorial outlines (i.e., TrueType or Type1).  This doesn't        */
  /*    prevent embedding of bitmap strikes though, i.e., a given face can */
  /*    have both this bit set, and a `num_fixed_sizes' property > 0.      */
  /*                                                                       */
#define FT_FACE_FLAG_SCALABLE  1


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_FACE_FLAG_FIXED_WIDTH                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used to indicate that a given face contains  */
  /*    fixed-width characters (like Courier, MonoType, etc).              */
  /*                                                                       */
#define FT_FACE_FLAG_FIXED_WIDTH  4


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_FACE_FLAG_FIXED_WIDTH                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used to indicate that a given face contains  */
  /*    `fixed sizes', i.e., bitmap strikes for some given pixel sizes.    */
  /*    See the `num_fixed_sizes' and `available_sizes' face properties    */
  /*    for more information.                                              */
  /*                                                                       */
#define FT_FACE_FLAG_FIXED_SIZES  2


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_FACE_FLAG_SFNT                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used to indicate that a given face uses the  */
  /*    `sfnt' storage fomat.  For now, this means TrueType or OpenType.   */
  /*                                                                       */
#define FT_FACE_FLAG_SFNT  8


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_FACE_FLAG_HORIZONTAL                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used to indicate that a given face contains  */
  /*    horizontal glyph metrics.  This should be set for all common       */
  /*    formats, but who knows...                                          */
  /*                                                                       */
#define FT_FACE_FLAG_HORIZONTAL  0x10


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_FACE_FLAG_VERTICAL                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used to indicate that a given face contains  */
  /*    vertical glyph metrics.  If not set, the glyph loader will         */
  /*    synthetize vertical metrics itself to help display vertical text   */
  /*    correctly.                                                         */
  /*                                                                       */
#define FT_FACE_FLAG_VERTICAL  0x20


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_FACE_FLAG_KERNING                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used to indicate that a given face contains  */
  /*    kerning information.  When set, this information can be retrieved  */
  /*    through the function FT_Get_Kerning().  Note that when unset, this */
  /*    function will always return the kerning vector (0,0).              */
  /*                                                                       */
#define FT_FACE_FLAG_KERNING  0x40


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_FACE_FLAG_FAST_GLYPHS                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used to indicate that the glyphs in a given  */
  /*    font can be retrieved very quickly, and that a glyph cache is thus */
  /*    not necessary for any of its child size objects.                   */
  /*                                                                       */
  /*    This flag should really be set for fixed-size formats like FNT,    */
  /*    where each glyph bitmap is available directly in binary form       */
  /*    without any kind of compression.                                   */
  /*                                                                       */
#define FT_FACE_FLAG_FAST_GLYPHS  0x80


#define FT_HAS_HORIZONTAL(face)  (face->face_flags & FT_FACE_FLAG_HORIZONTAL)
#define FT_HAS_VERTICAL(face)    (face->face_flags & FT_FACE_FLAG_VERTICAL)
#define FT_HAS_KERNING(face)     (face->face_flags & FT_FACE_FLAG_KERNING)
#define FT_IS_SCALABLE(face)     (face->face_flags & FT_FACE_FLAG_SCALABLE)
#define FT_IS_SFNT(face)         (face->face_flags & FT_FACE_FLAG_SFNT)
#define FT_IS_FIXED_WIDTH(face)  (face->face_flags & FT_FACE_FLAG_FIXED_WIDTH)
#define FT_HAS_FIXED_SIZES(face) (face->face_flags & FT_FACE_FLAG_FIXED_SIZES)
#define FT_HAS_FAST_GLYPHS(face) (face->face_flags & FT_FACE_FLAG_FAST_GLYPHS)


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_STYLE_FLAG_ITALIC                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used to indicate that a given face is        */
  /*    italicized.                                                        */
  /*                                                                       */
#define FT_STYLE_FLAG_ITALIC  1


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_STYLE_FLAG_BOLD                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used to indicate that a given face is        */
  /*    emboldened.                                                        */
  /*                                                                       */
#define FT_STYLE_FLAG_BOLD  2



  /*************************************************************************/
  /*                                                                       */
  /*                    FreeType base size metrics                         */
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Size_Metrics                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The size metrics structure returned scaled important distances for */
  /*    a given size object.                                               */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    pointSize    :: The current point size in 26.6 points, where       */
  /*                    1 point equals 1/72 inch.                          */
  /*                                                                       */
  /*    x_ppem       :: The character width, expressed in integer pixels.  */
  /*                    This is the width of the EM square expressed in    */
  /*                    pixels, hence the term `ppem' (pixels per EM).     */
  /*                                                                       */
  /*    y_ppem       :: The character height, expressed in integer pixels. */
  /*                    This is the height of the EM square expressed in   */
  /*                    pixels, hence the term `ppem' (pixels per EM).     */
  /*                                                                       */
  /*    x_scale      :: A simple 16.16 fixed point format coefficient used */
  /*                    to scale horizontal distances expressed in font    */
  /*                    units to fractional (26.6) pixel coordinates.      */
  /*                                                                       */
  /*    y_scale      :: A simple 16.16 fixed point format coefficient used */
  /*                    to scale vertical distances expressed in font      */
  /*                    units to fractional (26.6) pixel coordinates.      */
  /*                                                                       */
  /*    x_resolution :: The horizontal device resolution for this size     */
  /*                    object, expressed in integer dots per inches       */
  /*                    (dpi).  As a convention, fixed font formats set    */
  /*                    this value to 72.                                  */
  /*                                                                       */
  /*    y_resolution :: The vertical device resolution for this size       */
  /*                    object, expressed in integer dots per inches       */
  /*                    (dpi).  As a convention, fixed font formats set    */
  /*                    this value to 72.                                  */
  /*                                                                       */
  /*    ascender     :: The ascender, expressed in 26.6 fixed point        */
  /*                    pixels.  Always positive.                          */
  /*                                                                       */
  /*    descender    :: The descender, expressed in 26.6 fixed point       */
  /*                    pixels.  Always positive.                          */
  /*                                                                       */
  /*    height       :: The text height, expressed in 26.6 fixed point     */
  /*                    pixels.  Always positive.                          */
  /*                                                                       */
  /*    max_advance  :: Maximum horizontal advance, expressed in 26.6      */
  /*                    fixed point pixels.  Always positive.              */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This structure doesn't return the vertical ascender, descender,    */
  /*    and height, as well as a few other esoteric properties.  One can   */
  /*    however compute these through the size's x_scale and y_scale,      */
  /*    applied to the relevant face properties.                           */
  /*                                                                       */
  typedef struct  FT_Size_Metrics_
  {
    FT_UShort   x_ppem;        /* horizontal pixels per EM               */
    FT_UShort   y_ppem;        /* vertical pixels per EM                 */

    FT_Fixed    x_scale;       /* two scales used to convert font units  */
    FT_Fixed    y_scale;       /* to 26.6 frac. pixel coordinates..      */

    FT_Pos      ascender;      /* ascender in 26.6 frac. pixels          */
    FT_Pos      descender;     /* descender in 26.6 frac. pixels         */
    FT_Pos      height;        /* text height in 26.6 frac. pixels       */
    FT_Pos      max_advance;   /* max horizontal advance, in 26.6 pixels */

  } FT_Size_Metrics;


  /*************************************************************************/
  /*                                                                       */
  /*                       FreeType base size class                        */
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_SizeRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    FreeType root size class structure.  A size object models the      */
  /*    resolution and pointsize dependent data of a given face.           */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    face    :: Handle to the parent face object.                       */
  /*                                                                       */
  /*    generic :: A typeless pointer, which is unused by the FreeType     */
  /*               library or any of its drivers.  It can be used by       */
  /*               client applications to link their own data to each size */
  /*               object.                                                 */
  /*                                                                       */
  /*    metrics :: Metrics for this size object.  This field is read-only. */
  /*                                                                       */
  typedef struct  FT_SizeRec_
  {
    FT_Face          face;      /* parent face object              */
    FT_Generic       generic;   /* generic pointer for client uses */
    FT_Size_Metrics  metrics;   /* size metrics                    */

  } FT_SizeRec;



  typedef struct FT_SubGlyph_  FT_SubGlyph;

  struct FT_SubGlyph_
  {
    FT_Int        index;
    FT_UShort     flags;
    FT_Int        arg1;
    FT_Int        arg2;
    FT_Matrix     transform;
  };


#define FT_SUBGLYPH_FLAG_ARGS_ARE_WORDS            1
#define FT_SUBGLYPH_FLAG_ARGS_ARE_XY_VALUES        2
#define FT_SUBGLYPH_FLAG_ROUND_XY_TO_GRID          4
#define FT_SUBGLYPH_FLAG_SCALE                     8
#define FT_SUBGLYPH_FLAG_XY_SCALE               0x40
#define FT_SUBGLYPH_FLAG_2X2                    0x80
#define FT_SUBGLYPH_FLAG_USE_MY_METRICS        0x200


  /*************************************************************************/
  /*                                                                       */
  /*                  FreeType Glyph Slot base class                       */
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_GlyphSlotRec                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    FreeType root glyph slot class structure. A glyph slot is a        */
  /*    container where individual glyphs can be loaded, be they           */
  /*    vectorial or bitmap/graymaps..                                     */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    face     :: A handle to the parent face object.                    */
  /*                                                                       */
  /*    next     :: In some cases (like some font tools), several glyph    */
  /*                slots per face object can be a good thing.  As this is */
  /*                rare, the glyph slots are listed through a direct,     */
  /*                single-linked list using its `next' field.             */
  /*                                                                       */
  /*    metrics  :: The metrics of the last loaded glyph in the slot.  The */
  /*                returned values depend on the last load flags (see the */
  /*                FT_Load_Glyph() API function) and can be expressed     */
  /*                either in 26.6 fractional pixels or font units.        */
  /*                                                                       */
  /*    metrics2 :: This field can be used to return alternate glyph       */
  /*                metrics after a single load.  It can contain either    */
  /*                the glyph's metrics in font units, or the scaled but   */
  /*                unhinted ones.  See the load flags that apply when     */
  /*                calling the API function FT_Load_Glyph().              */
  /*                                                                       */
  /*    generic  :: A typeless pointer which is unused by the FreeType     */
  /*                library or any of its drivers.  It can be used by      */
  /*                client applications to link their own data to each     */
  /*                size object.                                           */
  /*                                                                       */
  /*    outline  :: The outline descriptor for the current glyph, if it    */
  /*                is a vectorial one.  The nature of the last loaded     */
  /*                glyph can be retrieved through the result value        */
  /*                returned by FT_Load_Glyph().                           */
  /*                                                                       */
  /*    bitmap   :: The bitmap/graymap descriptor for the current glyph,   */
  /*                if it is a fixed-width one.  The nature of the last    */
  /*                loaded glyph can be retrieved through the result value */
  /*                returned by FT_Load_Glyph().                           */
  /*                                                                       */
  /*                                                                       */
  typedef struct  FT_GlyphSlotRec_
  {
    FT_Face           face;
    FT_GlyphSlot      next;

    FT_Glyph_Metrics  metrics;
    FT_Glyph_Metrics  metrics2;

    FT_Glyph_Format   format;
    FT_Bitmap         bitmap;
    FT_Outline        outline;

    FT_Int            num_subglyphs;
    FT_Int            max_subglyphs;
    FT_SubGlyph*      subglyphs;

    void*             control_data;
    long              control_len;

    void*             other;

  } FT_GlyphSlotRec;


  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                         F U N C T I O N S                             */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Init_FreeType                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a new FreeType library object.  The set of drivers     */
  /*    that are registered by this function is determined at build time.  */
  /*                                                                       */
  /* <Output>                                                              */
  /*    library :: A handle to a new library object.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Init_FreeType( FT_Library*  library );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_FreeType                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroys a given FreeType library object, and all of its childs,   */
  /*    including resources, drivers, faces, sizes, etc.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a target library object.                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Done_FreeType( FT_Library  library );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Stream_Type                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration used to list the possible ways to open a new        */
  /*    input stream. It is used by the FT_Open_Args structure..           */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    ft_stream_memory   :: this is a memory-based stream                */
  /*    ft_stream_copy     :: copy the stream from the "stream" field      */
  /*    ft_stream_pathname :: create a new input stream from a C pathname  */
  /*                                                                       */
  typedef enum {

    ft_stream_memory   = 1,
    ft_stream_copy     = 2,
    ft_stream_pathname = 3

  } FT_Stream_Type;

 /*************************************************************************
  *
  * <Struct>
  *    FT_Open_Args
  *
  * <Description>
  *    A structure used to indicate how to open a new font file/stream.
  *    A pointer to such a structure can be used as a parameter for the
  *    function FT_Open_Face & FT_Attach_Stream.
  *
  * <Fields>
  *    stream_type  :: type of input stream
  *
  *    memory_base  :: first byte of file in memory
  *    memory_size  :: size in bytes of file in memory
  *
  *    pathname     :: a pointer to an 8-bit file pathname
  *
  *    stream       :: handle to a source stream object
  *
  *    driver       :: this field is exclusively used by FT_Open_Face,
  *                    it simply specifies the font driver to use to open
  *                    the face. If set to 0, FreeType will try to load
  *                    the face with each one of the drivers in its list.
  *
  * <Note>
  *    The stream_type determines which fields are used to create a new
  *    input stream.
  *
  *    If it is ft_stream_memory, a new memory-based stream will be created
  *    using the memory block specified by "memory_base" and "memory_size"
  *
  *    If it is ft_stream_pathname, a new stream will be created with the
  *    "pathname" field, calling the system-specific FT_New_Stream function
  *
  *    It is is ft_stream_copy, then the content of "stream" will be copied
  *    to a new input stream object. The object will be closed and destroyed
  *    when the face is destroyed itself.. Note that this means that you
  *    should not close the stream before the library does !!
  *
  *************************************************************************/

  typedef struct FT_Open_Args_
  {
    FT_Stream_Type  stream_type;
    FT_Byte*        memory_base;
    FT_Long         memory_size;
    FT_String*      pathname;
    FT_Stream       stream;
    FT_Driver       driver;

  } FT_Open_Args;

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_New_Face                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new face object from a given font file and typeface      */
  /*    index.                                                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library      :: handle to the root FreeType library instance       */
  /*                                                                       */
  /*    filepathname :: an 8-bit pathname naming the font file             */
  /*                                                                       */
  /*    face_index   :: the index of the face within the font file.        */
  /*                    The first face has index 0.                        */
  /* <Output>                                                              */
  /*    face       :: A handle to a new face object.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If your font file is in memory, or if you want to provide your     */
  /*    own input stream object, see FT_Open_Face below.                   */
  /*                                                                       */
  /*    FT_New_Face creates a new stream object. The stream will be        */
  /*    closed with the face (when calling FT_Done_Face or even            */
  /*    FT_Done_FreeType).                                                 */
  /*                                                                       */
  /*    Unlike FreeType 1.1, this function automatically creates a glyph   */
  /*    slot for the face object which can be accessed directly through    */
  /*    `face->slot'. Note that additional slots can be added to each face */
  /*    through the FT_New_GlyphSlot() API function.  Slots are linked in  */
  /*    a single list through their `next' field.                          */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_New_Face( FT_Library   library,
                         const char*  filepathname,
                         FT_Long      face_index,
                         FT_Face*     face );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Open_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new face object from a given input stream & typeface     */
  /*    index. This function is very similar to FT_New_Face, except that   */
  /*    it can accept any form of input stream..                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library      :: handle to the root FreeType library instance       */
  /*                                                                       */
  /*    args         :: an FT_Open_Args structure used to describe the     */
  /*                    input stream to FreeType.                          */
  /*                                                                       */
  /*    face_index   :: the index of the face within the font file.        */
  /*                    The first face has index 0.                        */
  /* <Output>                                                              */
  /*    face       :: A handle to a new face object.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Note that if the stream is in-memory or specified through an       */
  /*    8-bit pathname, a new stream is created by this function. It       */
  /*    is only closed when the face is destroyed (FT_Done_Face).          */
  /*                                                                       */
  /*    Note that when specifying directly an existing FT_Stream, this     */
  /*    function creates a new FT_Stream object and copies the contents    */
  /*    of the original stream within it. The stream will be closed        */
  /*    when the face is destroyed. This means calling the stream's        */
  /*    "close" function.                                                  */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Open_Face( FT_Library    library,
                          FT_Open_Args* args,
                          FT_Long       face_index,
                          FT_Face*      face );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Attach_File                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    "Attach" a given font file to an existing face. This is usually    */
  /*    to read additionnal information for a single face object. For      */
  /*    example, it is used to read the AFM files that come with Type 1    */
  /*    fonts in order to add kerning data and other metrics..             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face         :: target face object                                 */
  /*                                                                       */
  /*    filepathname :: an 8-bit pathname naming the 'metrics' file.       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If your font file is in memory, or if you want to provide your     */
  /*    own input stream object, see FT_Attach_Stream.                     */
  /*                                                                       */
  /*    The meaning of the "attach" (i.e. what really happens when the     */
  /*    new file is read) is not fixed by FreeType itself. It really       */
  /*    depends on the font format (and thus the font driver).             */
  /*                                                                       */
  /*    Client applications are expected to know what they're doing        */
  /*    when invoking this function. Most drivers simply do not implement  */
  /*    file attachments..                                                 */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Attach_File( FT_Face      face,
                            const char*  filepathname );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Attach_Stream                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is similar to FT_Attach_File with the exception      */
  /*    that it reads the attachment from an arbitrary stream.             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: target face object                                         */
  /*                                                                       */
  /*    args :: a pointer to an FT_Open_Args structure used to describe    */
  /*            the input stream to FreeType                               */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /*    The meaning of the "attach" (i.e. what really happens when the     */
  /*    new file is read) is not fixed by FreeType itself. It really       */
  /*    depends on the font format (and thus the font driver).             */
  /*                                                                       */
  /*    Client applications are expected to know what they're doing        */
  /*    when invoking this function. Most drivers simply do not implement  */
  /*    file attachments..                                                 */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Attach_Stream( FT_Face       face,
                              FT_Open_Args* parameters );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Discards a given face object, as well as all of its child slots    */
  /*    and sizes.                                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to a target face object.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Done_Face( FT_Face  face );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Char_Size                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets the character dimensions of a given size object.  The         */
  /*    `char_size' value is used for the width and height, expressed in   */
  /*    26.6 fractional points.  1 point = 1/72 inch.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    size      :: A handle to a target size object.                     */
  /*    char_size :: The character size, in 26.6 fractional points.        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    When dealing with fixed-size faces (i.e., non-scalable formats),   */
  /*    use the function FT_Set_Pixel_Sizes().                             */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Set_Char_Size( FT_Face     face,
                              FT_F26Dot6  char_width,
                              FT_F26Dot6  char_height,
                              FT_UInt     horz_resolution,
                              FT_UInt     vert_resolution );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Pixel_Sizes                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets the character dimensions of a given size object.  The width   */
  /*    and height are expressed in integer pixels.                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    size         :: A handle to a target size object.                  */
  /*    pixel_width  :: The character width, in integer pixels.            */
  /*    pixel_height :: The character height, in integer pixels.           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Set_Pixel_Sizes( FT_Face    face,
                                FT_UInt    pixel_width,
                                FT_UInt    pixel_height );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Load_Glyph                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to load a single glyph within a given glyph slot,  */
  /*    for a given size.                                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: A handle to the target face object where the glyph  */
  /*                   will be loaded.                                     */
  /*                                                                       */
  /*    glyph_index :: The index of the glyph in the font file.            */
  /*                                                                       */
  /*    load_flags  :: A flag indicating what to load for this glyph.  The */
  /*                   FT_LOAD_XXX constants can be used to control the    */
  /*                   glyph loading process (e.g., whether the outline    */
  /*                   should be scaled, whether to load bitmaps or not,   */
  /*                   whether to hint the outline, etc).                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Load_Glyph( FT_Face  face,
                           FT_UInt  glyph_index,
                           FT_Int   load_flags );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Load_Char                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to load a single glyph within a given glyph slot,  */
  /*    for a given size, according to its character code !                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: A handle to a target face object where the glyph    */
  /*                   will be loaded.                                     */
  /*                                                                       */
  /*    char_code   :: The glyph's character code, according to the        */
  /*                   current charmap used in the face.                   */
  /*                                                                       */
  /*    load_flags  :: A flag indicating what to load for this glyph.  The */
  /*                   FT_LOAD_XXX constants can be used to control the    */
  /*                   glyph loading process (e.g., whether the outline    */
  /*                   should be scaled, whether to load bitmaps or not,   */
  /*                   whether to hint the outline, etc).                  */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If the face has no current charmap, or if the character code       */
  /*    is not defined in the charmap, this function will return an        */
  /*    error..                                                            */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Load_Char( FT_Face   face,
                          FT_ULong  char_code,
                          FT_Int    load_flags );

  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_LOAD_NO_SCALE                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit field constant, used with FT_Load_Glyph() to indicate that   */
  /*    the vector outline being loaded should not be scaled to 26.6       */
  /*    fractional pixels, but kept in notional units.                     */
  /*                                                                       */
#define FT_LOAD_NO_SCALE  1


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_LOAD_NO_HINTING                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used with FT_Load_Glyph() to indicate that   */
  /*    the vector outline being loaded should not be fitted to the pixel  */
  /*    grid but simply scaled to 26.6 fractional pixels.                  */
  /*                                                                       */
  /*    This flag is ignored when FT_LOAD_NO_SCALE is set.                 */
  /*                                                                       */
#define FT_LOAD_NO_HINTING  2


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_LOAD_NO_OUTLINE                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used with FT_Load_Glyph() to indicate that   */
  /*    the function should not load the vector outline of a given glyph.  */
  /*    If an embedded bitmap exists for the glyph in the font, it will be */
  /*    loaded, otherwise nothing is returned and an error is produced.    */
  /*                                                                       */
#define FT_LOAD_NO_OUTLINE  4


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_LOAD_NO_BITMAP                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used with FT_Load_Glyph() to indicate that   */
  /*    the function should not load the bitmap or pixmap of a given       */
  /*    glyph.  If an outline exists for the glyph in the font, it is      */
  /*    loaded, otherwise nothing is returned and an error is produced.    */
  /*                                                                       */
#define FT_LOAD_NO_BITMAP  8


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_LOAD_LINEAR                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used with FT_Load_Glyph() to indicate that   */
  /*    the function should return the linearly scaled metrics for the     */
  /*    glyph in `slot->metrics2' (these metrics are not grid-fitted).     */
  /*    Otherwise, `metrics2' gives the original font units values.        */
  /*                                                                       */
#define FT_LOAD_LINEAR  16

  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_LOAD_FORCE_AUTOHINT                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used with FT_Load_Glyph() to indicate that   */
  /*    the function should try to auto-hint the glyphs, even if a driver- */
  /*    -specific hinter is available..                                    */
  /*                                                                       */
#define FT_LOAD_FORCE_AUTOHINT  32

  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_LOAD_PEDANTIC                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used with FT_Load_Glyph() to indicate that   */
  /*    the glyph loader should perform a pedantic bytecode.               */
  /*    interpretation.  Many popular fonts come with broken glyph         */
  /*    programs.  When this flag is set, loading them will return an      */
  /*    error.  Otherwise, errors are ignored by the loader, sometimes     */
  /*    resulting in ugly glyphs.                                          */
  /*                                                                       */
#define FT_LOAD_PEDANTIC  128


  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used with FT_Load_Glyph() to indicate that   */
  /*    the glyph loader should ignore the global advance width defined    */
  /*    in the font. As far as we know, this is only used by the           */
  /*    X-TrueType font server, in order to deal correctly with the        */
  /*    incorrect metrics contained in DynaLab's TrueType CJK fonts..      */
  /*                                                                       */
#define FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH 512

  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_LOAD_NO_RECURSE                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used with FT_Load_Glyph() to indicate that   */
  /*    the glyph loader should not load composite glyph recursively.      */
  /*    Rather, when a composite glyph is encountered, it should set       */
  /*    the values of `num_subglyphs' and `subglyphs', as well as set      */
  /*    `face->glyph.format' to ft_glyph_format_composite.                 */
  /*                                                                       */
  /*    This is for use by the auto-hinter and possibly other tools        */
  /*    For nearly all applications, this flags should be left unset       */
  /*    when invoking FT_Load_Glyph().                                     */
  /*                                                                       */
  /*    Note that the flag forces the load of unscaled glyphs              */
  /*                                                                       */
#define FT_LOAD_NO_RECURSE 1024

  /*************************************************************************/
  /*                                                                       */
  /* <Constant>                                                            */
  /*    FT_LOAD_DEFAULT                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A bit-field constant, used with FT_Load_Glyph() to indicate that   */
  /*    the function should try to load the glyph normally, i.e.,          */
  /*    embedded bitmaps are favored over outlines, vectors are always     */
  /*    scaled and grid-fitted.                                            */
  /*                                                                       */
#define FT_LOAD_DEFAULT  0


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Kerning                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the kerning vector between two glyphs of a same face.      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: A handle to a source face object.                   */
  /*                                                                       */
  /*    left_glyph  :: The index of the left glyph in the kern pair.       */
  /*                                                                       */
  /*    right_glyph :: The index of the right glyph in the kern pair.      */
  /*                                                                       */
  /* <Output>                                                              */
  /*    kerning     :: The kerning vector.  This is in font units for      */
  /*                   scalable formats, and in pixels for fixed-sizes     */
  /*                   formats.                                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Only horizontal layouts (left-to-right & right-to-left) are        */
  /*    supported by this method.  Other layouts, or more sophisticated    */
  /*    kernings, are out of the scope of this API function -- they can be */
  /*    implemented through format-specific interfaces.                    */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Get_Kerning( FT_Face     face,
                            FT_UInt     left_glyph,
                            FT_UInt     right_glyph,
                            FT_Vector*  kerning );



/* XXX : Not implemented yet, but should come soon */
#if 0
  EXPORT_DEF
  FT_Error  FT_Select_Charmap( FT_Face      face,
                               FT_Encoding  encoding );


  EXPORT_DEF
  FT_Error  FT_Set_Charmap( FT_Face     face,
                            FT_CharMap  charmap );
#endif

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Char_Index                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the glyph index of a given character code.  This function  */
  /*    uses a charmap object to do the translation.                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    charmap  :: A handle to a filter charmap object.                   */
  /*    charcode :: The character code.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The glyph index.  0 means `undefined character code'.              */
  /*                                                                       */
  EXPORT_DEF
  FT_UInt  FT_Get_Char_Index( FT_Face   face,
                              FT_ULong  charcode );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_MulDiv                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation `(A*B)/C'   */
  /*    with maximum accuracy (it uses a 64-bit intermediate integer       */
  /*    whenever necessary).                                               */
  /*                                                                       */
  /*    This function isn't necessarily as fast as some processor specific */
  /*    operations, but is at least completely portable.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.                                        */
  /*    c :: The divisor.                                                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*b)/c'.  This function never traps when trying to */
  /*    divide by zero, it simply returns `MaxInt' or `MinInt' depending   */
  /*    on the signs of `a' and `b'.                                       */
  /*                                                                       */
  EXPORT_DEF
  FT_Long  FT_MulDiv( FT_Long  a,
                      FT_Long  b,
                      FT_Long  c );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_MulFix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation             */
  /*    `(A*B)/0x10000' with maximum accuracy.  Most of the time, this is  */
  /*    used to multiply a given value by a 16.16 fixed float factor.      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.  Use a 16.16 factor here whenever      */
  /*         possible (see note below).                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*b)/0x10000'.                                     */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function has been optimized for the case where the absolute   */
  /*    value of `a' is less than 2048, and `b' is a 16.16 scaling factor. */
  /*    As this happens mainly when scaling from notional units to         */
  /*    fractional pixels in FreeType, it resulted in noticeable speed     */
  /*    improvements between versions 2.0 and 1.x.                         */
  /*                                                                       */
  /*    As a conclusion, always try to place a 16.16 factor as the         */
  /*    _second_ argument of this function; this can make a great          */
  /*    difference.                                                        */
  /*                                                                       */
  EXPORT_DEF
  FT_Long  FT_MulFix( FT_Long  a,
                      FT_Long  b );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_DivFix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation             */
  /*    `(A*0x10000)/B' with maximum accuracy.  Most of the time, this is  */
  /*    used to divide  a given value by a 16.16 fixed float factor.       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.  Use a 16.16 factor here whenever      */
  /*         possible (see note below).                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*0x10000)/b'.                                     */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The optimisation for FT_DivFix() is simple : if (a << 16) fits     */
  /*    in 32 bits, then the division is computed directly. Otherwise,     */
  /*    we use a specialised version of the old FT_MulDiv64                */
  /*                                                                       */
  EXPORT_DEF
  FT_Long  FT_DivFix( FT_Long  a,
                      FT_Long  b );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Get_Bitmap                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Renders an outline within a bitmap.  The outline's image is simply */
  /*    or-ed to the target bitmap.                                        */
  /*                                                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a FreeType library object.                  */
  /*    outline :: A pointer to the source outline descriptor.             */
  /*    map     :: A pointer to the target bitmap descriptor.              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    YES.  Rendering is synchronized, so that concurrent calls to the   */
  /*    scan-line converter will be serialized.                            */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function does NOT CREATE the bitmap, it only renders an       */
  /*    outline image within the one you pass to it!                       */
  /*                                                                       */
  /*    It will use the raster correponding to the default glyph format.   */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Outline_Get_Bitmap( FT_Library   library,
                                   FT_Outline*  outline,
                                   FT_Bitmap*   map );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Render                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Renders an outline within a bitmap using the current scan-convert  */
  /*    This functions uses a FT_Raster_Params as argument, allowing       */
  /*    advanced features like direct composition/translucency, etc..      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a FreeType library object.                  */
  /*    outline :: A pointer to the source outline descriptor.             */
  /*    params  :: A pointer to a FT_Raster_Params used to describe        */
  /*               the rendering operation                                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    YES.  Rendering is synchronized, so that concurrent calls to the   */
  /*    scan-line converter will be serialized.                            */
  /*                                                                       */
  /* <Note>                                                                */
  /*    You should know what you're doing and the role of FT_Raster_Params */
  /*    to use this function.                                              */
  /*                                                                       */
  /*    the field "params.source" will be set to "outline" before the      */
  /*    scan converter is called, which means that the value you give it   */
  /*    is actually ignored..                                              */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Outline_Render( FT_Library        library,
                               FT_Outline*       outline,
                               FT_Raster_Params* params );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Decompose                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Walks over an outline's structure to decompose it into individual  */
  /*    segments and Bezier arcs.  This function is also able to emit      */
  /*    `move to' and `close to' operations to indicate the start and end  */
  /*    of new contours in the outline.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline   :: A pointer to the source target.                       */
  /*                                                                       */
  /*    funcs     :: A table of `emitters', i.e,. function pointers called */
  /*                 during decomposition to indicate path operations.     */
  /*                                                                       */
  /*    user      :: A typeless pointer which is passed to each emitter    */
  /*                 during the decomposition.  It can be used to store    */
  /*                 the state during the decomposition.                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means sucess.                                       */
  /*                                                                       */
  EXPORT_DEF
  int  FT_Outline_Decompose( FT_Outline*        outline,
                             FT_Outline_Funcs*  funcs,
                             void*              user );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_New                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new outline of a given size.                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library     :: A handle to the library object from where the       */
  /*                   outline is allocated.  Note however that the new    */
  /*                   outline will NOT necessarily be FREED when          */
  /*                   destroying the library, by FT_Done_FreeType().      */
  /*                                                                       */
  /*    numPoints   :: The maximum number of points within the outline.    */
  /*                                                                       */
  /*    numContours :: The maximum number of contours within the outline.  */
  /*                                                                       */
  /* <Output>                                                              */
  /*    outline     :: A handle to the new outline.  NULL in case of       */
  /*                   error.                                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    No.                                                                */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The reason why this function takes a `library' parameter is simply */
  /*    to use the library's memory allocator.  You can copy the source    */
  /*    code of this function, replacing allocations with `malloc()' if    */
  /*    you want to control where the objects go.                          */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Outline_New( FT_Library   library,
                            FT_UInt      numPoints,
                            FT_Int       numContours,
                            FT_Outline*  outline );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Done                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroys an outline created with FT_Outline_New().                 */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle of the library object used to allocate the     */
  /*               outline.                                                */
  /*                                                                       */
  /*    outline :: A pointer to the outline object to be discarded.        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    No.                                                                */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If the outline's `owner' field is not set, only the outline        */
  /*    descriptor will be released.                                       */
  /*                                                                       */
  /*    The reason why this function takes an `outline' parameter is       */
  /*    simply to use FT_Alloc()/FT_Free().  You can copy the source code  */
  /*    of this function, replacing allocations with `malloc()' in your    */
  /*    application if you want something simpler.                         */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Outline_Done( FT_Library   library,
                             FT_Outline*  outline );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Get_CBox                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns an outline's `control box'.  The control box encloses all  */
  /*    the outline's points, including Bezier control points.  Though it  */
  /*    coincides with the exact bounding box for most glyphs, it can be   */
  /*    slightly larger in some situations (like when rotating an outline  */
  /*    which contains Bezier outside arcs).                               */
  /*                                                                       */
  /*    Computing the control box is very fast, while getting the bounding */
  /*    box can take much more time as it needs to walk over all segments  */
  /*    and arcs in the outline.  To get the latter, you can use the       */
  /*    `ftbbox' component which is dedicated to this single task.         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline :: A pointer to the source outline descriptor.             */
  /*                                                                       */
  /* <Output>                                                              */
  /*    cbox    :: The outline's control box.                              */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  EXPORT_DEF
  void  FT_Outline_Get_CBox( FT_Outline*  outline,
                             FT_BBox*     cbox );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Translate                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Applies a simple translation to the points of an outline.          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline :: A pointer to the target outline descriptor.             */
  /*    xOffset :: The horizontal offset.                                  */
  /*    yOffset :: The vertical offset.                                    */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  EXPORT_DEF
  void  FT_Outline_Translate( FT_Outline*  outline,
                              FT_Pos       xOffset,
                              FT_Pos       yOffset );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Raster                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Register a given raster to the library.                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library      :: A handle to a target library object.               */
  /*    raster_funcs :: pointer to the raster's interface                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function will do the following:                               */
  /*                                                                       */
  /*    - a new raster object is created through raster_func.raster_new    */
  /*      if this fails, then the function returns                         */
  /*                                                                       */
  /*    - if a raster is already registered for the glyph format           */
  /*      specified in raster_funcs, it will be destroyed                  */
  /*                                                                       */
  /*    - the new raster is registered for the glyph format                */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Set_Raster( FT_Library        library,
                           FT_Raster_Funcs*  raster_funcs );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Unset_Raster                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Removes a given raster from the library.                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library      :: A handle to a target library object.               */
  /*    raster_funcs :: pointer to the raster's interface                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function should never be used by a normal client application  */
  /*    as FT_Set_Raster unregisters the previous raster for a given       */
  /*    glyph format..                                                     */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Unset_Raster( FT_Library        library,
                             FT_Raster_Funcs*  raster_funcs );


 /*************************************************************************
  *
  * <Function>
  *   FT_Get_Raster
  *
  * <Description>
  *   Return a pointer to the raster corresponding to a given glyph
  *   format tag.
  *
  * <Input>
  *   library      :: handle to source library object
  *   glyph_format :: glyph format tag
  *
  * <Output>
  *   raster_funcs :: if this field is not 0, returns a pointer to the
  *                   raster's interface/descriptor..
  *
  * <Return>
  *   a pointer to the corresponding raster object.
  *
  *************************************************************************/

  EXPORT_DEF
  FT_Raster  FT_Get_Raster( FT_Library        library,
                            FT_Glyph_Format   glyph_format,
                            FT_Raster_Funcs  *raster_funcs );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Raster_Mode                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Set a raster-specific mode.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a target library object.                    */
  /*    format  :: the glyph format used to select the raster              */
  /*    mode    :: the raster-specific mode descriptor                     */
  /*    args    :: the mode arguments                                      */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Set_Raster_Mode( FT_Library      library,
                                FT_Glyph_Format format,
                                unsigned long   mode,
                                void*           args );


 /***************************************************************************/
 /***************************************************************************/
 /***************************************************************************/
 /*****                                                                 *****/
 /*****       C O N V E N I E N C E   F U N C T I O N S                 *****/
 /*****                                                                 *****/
 /*****                                                                 *****/
 /*****    The following functions are provided as a convenience        *****/
 /*****    to client applications. However, their compilation might     *****/
 /*****    be discarded if FT_CONFIG_OPTION_NO_CONVENIENCE_FUNCS        *****/
 /*****    is defined in "config/ftoption.h".                           *****/
 /*****                                                                 *****/
 /***************************************************************************/
 /***************************************************************************/
 /***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Copy                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Copies an outline into another one.  Both objects must have the    */
  /*    same sizes (number of points & number of contours) when this       */
  /*    function is called.                                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    source :: A handle to the source outline.                          */
  /*    target :: A handle to the target outline.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Outline_Copy( FT_Outline*  source,
                             FT_Outline*  target );



  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Transform                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Applies a simple 2x2 matrix to all of an outline's points.  Useful */
  /*    for applying rotations, slanting, flipping, etc.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline :: A pointer to the target outline descriptor.             */
  /*    matrix  :: A pointer to the transformation matrix.                 */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  /* <Note>                                                                */
  /*    You can use FT_Outline_Translate() if you need to translate the    */
  /*    outline's points.                                                  */
  /*                                                                       */
  EXPORT_DEF
  void  FT_Outline_Transform( FT_Outline*  outline,
                              FT_Matrix*   matrix );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Reverse                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Reverse the drawing direction of an outline. This is used to       */
  /*    ensure consistent fill conventions for mirrored glyphs..           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline :: A pointer to the target outline descriptor.             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This functions toggles the bit flag ft_outline_reverse_fill in     */
  /*    the outline's "flags" field..                                      */
  /*                                                                       */
  /*    It shouldn't be used by a normal client application, unless it     */
  /*    knows what it's doing..                                            */
  /*                                                                       */
  EXPORT_DEF
  void  FT_Outline_Reverse( FT_Outline*  outline );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Vector_Transform                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Transforms a single vector through a 2x2 matrix.                   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    vector :: The target vector to transform                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    matrix :: A pointer to the source 2x2 matrix.                      */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  EXPORT_DEF
  void  FT_Vector_Transform( FT_Vector*  vector,
                             FT_Matrix*  matrix );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Matrix_Multiply                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Performs the matrix operation `b = a*b'.                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: A pointer to matrix `a'.                                      */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    b :: A pointer to matrix `b'.                                      */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  EXPORT_DEF
  void  FT_Matrix_Multiply( FT_Matrix*  a,
                            FT_Matrix*  b );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Matrix_Invert                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Inverts a 2x2 matrix.  Returns an error if it can't be inverted.   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    matrix :: A pointer to the target matrix.  Remains untouched in    */
  /*              case of error.                                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <MT-Note>                                                             */
  /*    Yes.                                                               */
  /*                                                                       */
  EXPORT_DEF
  FT_Error  FT_Matrix_Invert( FT_Matrix*  matrix );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Default_Drivers                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Adds the set of default drivers to a given library object.         */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library :: A handle to a new library object.                       */
  /*                                                                       */
  EXPORT_DEF
  void  FT_Default_Drivers( FT_Library  library );

#ifdef __cplusplus
  }
#endif


#endif /* FREETYPE_H */


/* END */
