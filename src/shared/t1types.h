/*******************************************************************
 *
 *  t1types.h                                                   1.0
 *
 *    Basic Type1/Type2 type definitions and interface.
 *
 *  This code is shared by the Type1 and Type2 drivers
 *
 *
 *  Copyright 1996-2000 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 ******************************************************************/

#ifndef T1TYPES_H
#define T1TYPES_H

#include <t1tables.h>
#include <psnames.h>

#ifdef __cplusplus
  extern "C" {
#endif



/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/***                                                                   ***/
/***                                                                   ***/
/***                DEFINITIONS OF BASIC DATA TYPES                    ***/
/***                                                                   ***/
/***                                                                   ***/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


/* The REDEFINE macro is used to convert a FreeType generic type into    */
/* a TrueType-specific one. It simply replaces the "FT_" prefix by "T1_" */
/* in order to define compatible T1_Long, T1_Error, T1_Outline, etc..    */
/*                                                                       */
#undef  REDEFINE
#define REDEFINE( type )   typedef FT_##type  T1_##type


  /* <Type> T1_Bool                                                       */
  /*                                                                      */
  /* <Description>                                                        */
  /*    A simple typedef of unsigned char, used for simple booleans.      */
  /*                                                                      */
  REDEFINE( Bool );


  /* <Type> T1_FWord                                                      */
  /*                                                                      */
  /* <Description>                                                        */
  /*    a signed 16-bit integer used to store a distance in original      */
  /*    font units.                                                       */
  /*                                                                      */
  REDEFINE( FWord );


  /* <Type> T1_UFWord                                                     */
  /*                                                                      */
  /* <Description>                                                        */
  /*    an unsigned 16-bit integer used to store a distance in original   */
  /*    font units.                                                       */
  /*                                                                      */
  REDEFINE( UFWord );


  /* <Type> T1_Char                                                       */
  /*                                                                      */
  /* <Description>                                                        */
  /*    a simple typedef for the _signed_ char type.                      */
  /*                                                                      */
  REDEFINE( Char );


  /* <Type> T1_Byte                                                       */
  /*                                                                      */
  /* <Description>                                                        */
  /*    a simple typedef for the _unsigned_ char type.                    */
  /*                                                                      */
  REDEFINE( Byte );


  /* <Type> T1_String                                                     */
  /*                                                                      */
  /* <Description>                                                        */
  /*    a simple typedef for the char type, used for strings usually.     */
  /*                                                                      */
  REDEFINE( String );


  /* <Type> T1_Short                                                      */
  /*                                                                      */
  /* <Description>                                                        */
  /*    a typedef for signed short                                        */
  /*                                                                      */
  REDEFINE( Short );


  /* <Type> T1_UShort                                                     */
  /*                                                                      */
  /* <Description>                                                        */
  /*    a typedef for unsigned short                                      */
  /*                                                                      */
  REDEFINE( UShort );


  /* <Type> FT_Int                                                        */
  /*                                                                      */
  /* <Description>                                                        */
  /*    a typedef for the int type                                        */
  /*                                                                      */
  REDEFINE( Int );


  /* <Type> FT_UInt                                                       */
  /*                                                                      */
  /* <Description>                                                        */
  /*    a typedef for the unsigned int type                               */
  /*                                                                      */
  REDEFINE( UInt );


  /* <Type> T1_Long                                                       */
  /*                                                                      */
  /* <Description>                                                        */
  /*    a typedef for signed long                                         */
  /*                                                                      */
  REDEFINE( Long );


  /* <Type> T1_ULong                                                      */
  /*                                                                      */
  /* <Description>                                                        */
  /*    a typedef for unsigned long                                       */
  /*                                                                      */
  REDEFINE( ULong );


  /* <Type> T1_F2Dot14                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    a signed 2.14 fixed float used for unit vectors                    */
  /*                                                                       */
  REDEFINE( F2Dot14 );


  /* <Type> T1_F26Dot6                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    a signed 26.6 fixed float used for vectorial pixel coordinates     */
  /*                                                                       */
  REDEFINE( F26Dot6 );


  /* <Type> T1_Fixed                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*     This type is used to store 16.16 fixed float values, like         */
  /*     scales or matrix coefficients..                                   */
  /*                                                                       */
  REDEFINE( Fixed );


  /* <Type> T1_Pos                                                      */
  /*                                                                    */
  /* <Description>                                                      */
  /*     The type T1_Pos is a 32-bits integer used to store vectorial   */
  /*     coordinates. Depending on the context, these can represent     */
  /*     distances in integer font units, or 26.6 fixed float pixel     */
  /*     coordinates..                                                  */
  /*                                                                    */
  REDEFINE( Pos );


  /* <Struct> T1_Vector                                                 */
  /*                                                                    */
  /* <Description>                                                      */
  /*     A simple structure used to store a 2d vector, coordinates      */
  /*     are of the T1_Pos type.                                        */
  /*                                                                    */
  /* <Fields>                                                           */
  /*    x  ::  horizontal coordinate                                    */
  /*    y  ::  vertical coordinate                                      */
  /*                                                                    */
  REDEFINE( Vector );

  /* <Struct> T1_UnitVector                                             */
  /*                                                                    */
  /* <Description>                                                      */
  /*     A simple structure used to store a 2d vector unit vector.      */
  /*     uses T1_F2Dot14 types.                                         */
  /*                                                                    */
  /* <Fields>                                                           */
  /*    x  ::  horizontal coordinate                                    */
  /*    y  ::  vertical coordinate                                      */
  /*                                                                    */
  REDEFINE( UnitVector );


  /* <Struct> T1_Matrix                                                 */
  /*                                                                    */
  /* <Description>                                                      */
  /*     A simple structure used to store a 2x2 matrix. Coefficients    */
  /*     are in 16.16 fixed float format. The computation performed     */
  /*     is :                                                           */
  /*             {                                                      */
  /*               x' = x*xx + y*xy                                     */
  /*               y' = x*yx + y*yy                                     */
  /*             }                                                      */
  /*                                                                    */
  /* <Fields>                                                           */
  /*     xx  :: matrix coefficient                                      */
  /*     xy  :: matrix coefficient                                      */
  /*     yx  :: matrix coefficient                                      */
  /*     yy  :: matrix coefficient                                      */
  /*                                                                    */
  REDEFINE( Matrix );


  /* <Struct> T1_BBox                                                   */
  /*                                                                    */
  /* <Description>                                                      */
  /*     A structure used to hold an outline's bounding box, i.e.       */
  /*     the coordinates of its extrema in the horizontal and vertical  */
  /*     directions.                                                    */
  /*                                                                    */
  /* <Fields>                                                           */
  /*     xMin   ::  the horizontal minimum  (left-most)                 */
  /*     yMin   ::  the vertical minimum    (bottom-most)               */
  /*     xMax   ::  the horizontal maximum  (right-most)                */
  /*     yMax   ::  the vertical maximum    (top-most)                  */
  /*                                                                    */
  REDEFINE( BBox );


  /* <Type> T1_Error                                                    */
  /*                                                                    */
  /* <Description>                                                      */
  /*    The FreeType error code type. A value of 0 is always            */
  /*    interpreted as a succesful operation.                           */
  /*                                                                    */
  REDEFINE( Error );



/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/***                                                                   ***/
/***                                                                   ***/
/***                REQUIRED TYPE1/TYPE2 TABLES DEFINITIONS            ***/
/***                                                                   ***/
/***                                                                   ***/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

  /***********************************************************************/
  /*                                                                     */
  /* <Struct> T1_Encoding                                                */
  /*                                                                     */
  /* <Description>                                                       */
  /*    A structure modeling a custom encoding                           */
  /*                                                                     */
  /* <Fields>                                                            */
  /*    num_chars   :: number of char codes in encoding. Usually 256     */
  /*    code_first  :: lower char code in encoding                       */
  /*    code_last   :: higher char code in encoding                      */
  /*                                                                     */
  /*    char_code   :: array of character codes                          */
  /*    char_index  :: array of correpsonding glyph indices              */
  /*    char_name   :: array of correpsonding glyph names                */
  /*                                                                     */
  typedef struct T1_Encoding_
  {
    T1_Int      num_chars;
    T1_Int      code_first;
    T1_Int      code_last;

    T1_UShort*  char_index;
    T1_String** char_name;

  } T1_Encoding;


  typedef enum T1_EncodingType_
  {
	t1_encoding_none = 0,
	t1_encoding_array,
	t1_encoding_standard,
	t1_encoding_expert

  } T1_EncodingType;


  typedef struct T1_Font_
  {

 /* font info dictionary */
    T1_FontInfo    font_info;
 
 /* private dictionary */
    T1_Private     private_dict;

 /* top-level dictionary */
    FT_String*   font_name;

    T1_EncodingType  encoding_type;
    T1_Encoding      encoding;

    T1_Byte*     subrs_block;
    T1_Byte*     charstrings_block;
    T1_Byte*     glyph_names_block;

    T1_Int       num_subrs;
    T1_Byte**    subrs;
    T1_Int*      subrs_len;

    T1_Int       num_glyphs;
    T1_String**  glyph_names;       /* array of glyph names       */
    T1_Byte**    charstrings;       /* array of glyph charstrings */
    T1_Int*      charstrings_len;

    T1_Byte      paint_type;
    T1_Byte      font_type;
    T1_Matrix    font_matrix;
    T1_BBox      font_bbox;
    T1_Long      font_id;

    T1_Int       stroke_width;
  
  } T1_Font;
  


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/***                                                                   ***/
/***                                                                   ***/
/***                  ORIGINAL T1_FACE CLASS DEFINITION                ***/
/***                                                                   ***/
/***                                                                   ***/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/***                                                                   ***/
/***                                                                   ***/
/***    This structure/class is defined here because it is common      ***/
/***    to the following formats : TTF, OpenType-TT and OpenType-CFF   ***/
/***                                                                   ***/
/***    Note however that the classes TT_Size, TT_GlyphSlot and        ***/
/***    TT_CharMap are not shared between font drivers, and are        ***/
/***    thus defined normally in "drivers/truetype/ttobjs.h"           ***/
/***                                                                   ***/
/***                                                                   ***/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


  typedef struct T1_FaceRec_*   T1_Face;


  /***************************************************/
  /*                                                 */
  /*  T1_Face :                                      */
  /*                                                 */
  /*    Type1 face record..                          */
  /*                                                 */

  typedef struct T1_FaceRec_
  {
    FT_FaceRec    root;
    T1_Font       type1;
    void*         psnames;
    void*         afm_data;
    FT_CharMapRec charmaprecs[2];
    FT_CharMap    charmaps[2];
    PS_Unicodes   unicode_map;

  } T1_FaceRec;


#endif /* T1TYPES_H */
