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

#include <freetype/t1tables.h>
#include <freetype/internal/psnames.h>

#ifdef __cplusplus
  extern "C" {
#endif



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
    FT_Int      num_chars;
    FT_Int      code_first;
    FT_Int      code_last;

    FT_UShort*  char_index;
    FT_String** char_name;

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

    FT_Byte*     subrs_block;
    FT_Byte*     charstrings_block;
    FT_Byte*     glyph_names_block;

    FT_Int       num_subrs;
    FT_Byte**    subrs;
    FT_Int*      subrs_len;

    FT_Int       num_glyphs;
    FT_String**  glyph_names;       /* array of glyph names       */
    FT_Byte**    charstrings;       /* array of glyph charstrings */
    FT_Int*      charstrings_len;

    FT_Byte      paint_type;
    FT_Byte      font_type;
    FT_Matrix    font_matrix;
    FT_BBox      font_bbox;
    FT_Long      font_id;

    FT_Int       stroke_width;

  } T1_Font;


  typedef struct CID_Subrs_
  {
    FT_UInt    num_subrs;
    FT_Byte**  code;
    
  } CID_Subrs;


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
  typedef struct CID_FaceRec_*  CID_Face;

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

    /* support for multiple masters */
    T1_Blend*     blend;

  } T1_FaceRec;


  typedef struct CID_FaceRec_
  {
    FT_FaceRec    root;
    void*         psnames;
    CID_Info      cid;
    CID_Subrs*    subrs;
  
  } CID_FaceRec;

#endif /* T1TYPES_H */
