/*******************************************************************
 *
 *  t1types.h                                                   1.0
 *
 *    Basic Type1/Type2 type definitions and interface.
 *
 *  This code is shared by the Type1 and Type2 drivers
 *
 *
 *  Copyright 1996-1999 by
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

#include <freetype.h>


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
  /* <Struct> T1_FontInfo                                                */
  /*                                                                     */
  /* <Description>                                                       */
  /*    The FontInfo dictionary structure.                               */
  /*                                                                     */
  /* <Fields>                                                            */
  /*    version             ::                                           */
  /*    notice              ::                                           */
  /*    full_name           ::                                           */
  /*    family_name         ::                                           */
  /*    weight              ::                                           */
  /*    italic_angle        ::                                           */
  /*    is_fixed_pitch      ::                                           */
  /*    underline_position  ::                                           */
  /*    underline_thickness ::                                           */
  /*                                                                     */
  typedef struct T1_FontInfo_
  {
    T1_String*     version;
    T1_String*     notice;
    T1_String*     full_name;
    T1_String*     family_name;
    T1_String*     weight;
    T1_Long        italic_angle;
    T1_Bool        is_fixed_pitch;
    T1_Short       underline_position;
    T1_UShort      underline_thickness;

  } T1_FontInfo;


  /***********************************************************************/
  /*                                                                     */
  /* <Struct> T1_Private                                                 */
  /*                                                                     */
  /* <Description>                                                       */
  /*    The Private dictionary structure.                                */
  /*                                                                     */
  /* <Fields>                                                            */
  /*    unique_id :: the font's unique id                                */
  /*    lenIV     :: length of decrypt padding                           */
  /*                                                                     */
  /*    num_blues              :: number of blue values                  */
  /*    num_other_blues        :: number of other blue values            */
  /*    num_family_blues       :: number of family blue values           */
  /*    num_family_other_blues :: number of family other blue values     */
  /*                                                                     */
  /*    blue_values        :: array of blue values                       */
  /*    other_blues        :: array of other blue values                 */
  /*    family_blues       :: array of family blue values                */
  /*    family_other_blues :: array of family other blue values          */
  /*                                                                     */
  /*    blue_scale         ::                                            */
  /*    blue_shift         ::                                            */
  /*    blue_scale         ::                                            */
  /*                                                                     */
  /*    standard_width     ::                                            */
  /*    standard_height    ::                                            */
  /*                                                                     */
  /*    num_snap_widths    ::                                            */
  /*    num_snap_heights   ::                                            */
  /*    force_bold         ::                                            */
  /*    round_stem_up      ::                                            */
  /*                                                                     */
  /*    stem_snap_widths   ::                                            */
  /*    stem_snap_heights  ::                                            */
  /*                                                                     */
  /*    language_group     ::                                            */
  /*    password           ::                                            */
  /*                                                                     */
  /*    min_feature        ::                                            */
  /*                                                                     */
  /*                                                                     */
  typedef struct T1_Private_
  {
    T1_Int       unique_id;
    T1_Int       lenIV;

    T1_Byte      num_blues;
    T1_Byte      num_other_blues;
    T1_Byte      num_family_blues;
    T1_Byte      num_family_other_blues;

    T1_Short     blue_values[14];
    T1_Short     other_blues[10];

    T1_Short     family_blues      [14];
    T1_Short     family_other_blues[10];

    T1_Fixed     blue_scale;
    T1_Int       blue_shift;
    T1_Int       blue_fuzz;

    T1_UShort    standard_width;
    T1_UShort    standard_height;

    T1_Byte      num_snap_widths;
    T1_Byte      num_snap_heights;
    T1_Bool      force_bold;
    T1_Bool      round_stem_up;

    T1_Short     stem_snap_widths [13];  /* reserve one place for the std */
    T1_Short     stem_snap_heights[13];  /* reserve one place for the std */

    T1_Long      language_group;
    T1_Long      password;

    T1_Short     min_feature[2];

  } T1_Private;



  /***********************************************************************/
  /*                                                                     */
  /* <Struct> T1_Private                                                 */
  /*                                                                     */
  /* <Description>                                                       */
  /*    The Private dictionary structure.                                */
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

    T1_Short*   char_index;
    T1_String** char_name;

  } T1_Encoding;



/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/***                                                                   ***/
/***                                                                   ***/
/***                  ORIGINAL TT_FACE CLASS DEFINITION                ***/
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
    FT_FaceRec      root;

    T1_FontInfo     font_info;
    FT_String*      font_name;

    T1_Encoding     encoding;

    T1_Byte*        subrs_block;
    T1_Byte*        charstrings_block;

    T1_Int          num_subrs;
    T1_Byte**       subrs;
    T1_Int*         subrs_len;

    T1_Int          num_glyphs;
    T1_String**     glyph_names;       /* array of glyph names       */
    T1_Byte**       charstrings;       /* array of glyph charstrings */
    T1_Int*         charstrings_len;

    T1_Byte         paint_type;
    T1_Byte         font_type;
    T1_Matrix       font_matrix;
    T1_BBox         font_bbox;
    T1_Long         unique_id;
    T1_Long         font_id;

    T1_Int          stroke_width;
    T1_Private      private_dict;

  } T1_FaceRec;


#endif /* T1TYPES_H */
