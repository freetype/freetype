/***************************************************************************/
/*                                                                         */
/*  fterrors.h                                                             */
/*                                                                         */
/*  FreeType error codes (specification).                                  */
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


#define  FT_Err_Ok                       0x000

#define  FT_Err_Cannot_Open_Resource     0x001
#define  FT_Err_Unknown_File_Format      0x002
#define  FT_Err_Invalid_File_Format      0x002

#define  FT_Err_Invalid_Argument         0x003
#define  FT_Err_Invalid_Handle           0x004
#define  FT_Err_Invalid_Glyph_Index      0x00A
#define  FT_Err_Invalid_Character_Code   0x00B

#define  FT_Err_Unimplemented_Feature    0x010
#define  FT_Err_Invalid_Glyph_Format     0x00D

#define  FT_Err_Invalid_Library_Handle   0x004
#define  FT_Err_Invalid_Driver_Handle    0x005
#define  FT_Err_Invalid_Face_Handle      0x006
#define  FT_Err_Invalid_Size_Handle      0x007
#define  FT_Err_Invalid_Slot_Handle      0x008
#define  FT_Err_Invalid_CharMap_Handle   0x009
#define  FT_Err_Invalid_Outline          0x00B
#define  FT_Err_Invalid_Dimensions       0x00C

#define  FT_Err_Unavailable_Outline      0x011
#define  FT_Err_Unavailable_Bitmap       0x012
#define  FT_Err_Unavailable_Pixmap       0x013
#define  FT_Err_File_Is_Not_Collection   0x014
#define  FT_Err_Too_Many_Drivers         0x015
#define  FT_Err_Too_Many_Glyph_Formats   0x016
#define  FT_Err_Too_Many_Extensions      0x017

#define  FT_Err_Out_Of_Memory            0x100
#define  FT_Err_Unlisted_Object          0x101

#define  FT_Err_Invalid_Resource_Handle  0x200
#define  FT_Err_Invalid_Stream_Handle    0x201
#define  FT_Err_Cannot_Open_Stream       0x202
#define  FT_Err_Invalid_Stream_Seek      0x203
#define  FT_Err_Invalid_Stream_Skip      0x204
#define  FT_Err_Invalid_Stream_Read      0x205
#define  FT_Err_Invalid_Stream_Operation 0x206
#define  FT_Err_Invalid_Frame_Operation  0x207
#define  FT_Err_Nested_Frame_Access      0x208
#define  FT_Err_Invalid_Frame_Read       0x209

#define  FT_Err_Too_Many_Points          0x300
#define  FT_Err_Too_Many_Contours        0x301
#define  FT_Err_Invalid_Composite        0x302
#define  FT_Err_Too_Many_Hints           0x303
#define  FT_Err_Too_Many_Edges           0x304
#define  FT_Err_Too_Many_Strokes         0x305

/* range 0x400 - 0x4FF is reserved for TrueType specific stuff */

/* range 0x500 - 0x5FF is reserved for TrueDoc  specific stuff */

/* range 0x600 - 0x6FF is reserved for Type1    specific stuff */

#define  FT_Err_Raster_Uninitialized     0xF00
#define  FT_Err_Raster_Corrupted         0xF01
#define  FT_Err_Raster_Overflow          0xF02


/* END */
