/*******************************************************************
 *
 *  t1errors.h
 *
 *    Type1 Error ID definitions
 *
 *  Copyright 1996-1998 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 ******************************************************************/

#ifndef FREETYPE_H
#error "Don't include this file! Use t1driver.h instead."
#endif

#ifndef T1ERRORS_H
#define T1ERRORS_H

  /************************ error codes declaration **************/

  /* The error codes are grouped in 'classes' used to indicate the */
  /* 'level' at which the error happened.                          */
  /* The class is given by an error code's high byte.              */


/* ------------- Success is always 0 -------- */

#define  T1_Err_Ok                       FT_Err_Ok

/* ----------- high level API errors -------- */
  
#define  T1_Err_Invalid_File_Format      FT_Err_Invalid_File_Format
#define  T1_Err_Invalid_Argument         FT_Err_Invalid_Argument
#define  T1_Err_Invalid_Driver_Handle    FT_Err_Invalid_Driver_Handle
#define  T1_Err_Invalid_Face_Handle      FT_Err_Invalid_Face_Handle
#define  T1_Err_Invalid_Size_Handle      FT_Err_Invalid_Size_Handle
#define  T1_Err_Invalid_Glyph_Handle     FT_Err_Invalid_Slot_Handle
#define  T1_Err_Invalid_CharMap_Handle   FT_Err_Invalid_CharMap_Handle
#define  T1_Err_Invalid_Glyph_Index      FT_Err_Invalid_Glyph_Index

#define  T1_Err_Unimplemented_Feature    FT_Err_Unimplemented_Feature
#define  T1_Err_Unavailable_Outline      FT_Err_Unavailable_Outline
#define  T1_Err_Unavailable_Bitmap       FT_Err_Unavailable_Bitmap
#define  T1_Err_Unavailable_Pixmap       FT_Err_Unavailable_Pixmap
#define  T1_Err_File_Is_Not_Collection   FT_Err_File_Is_Not_Collection

#define  T1_Err_Invalid_Engine           FT_Err_Invalid_Driver_Handle

/* ------------- internal errors ------------ */
  
#define  T1_Err_Out_Of_Memory            FT_Err_Out_Of_Memory
#define  T1_Err_Unlisted_Object          FT_Err_Unlisted_Object

/* ------------ general glyph outline errors ------ */

#define  T1_Err_Too_Many_Points          FT_Err_Too_Many_Points
#define  T1_Err_Too_Many_Contours        FT_Err_Too_Many_Contours
#define  T1_Err_Too_Many_Hints           FT_Err_Too_Many_Hints
#define  T1_Err_Invalid_Composite        FT_Err_Invalid_Composite
#define  T1_Err_Too_Many_Edges           FT_Err_Too_Many_Edges
#define  T1_Err_Too_Many_Strokes         FT_Err_Too_Many_Strokes


#define  T1_Err_Syntax_Error             FT_Err_Invalid_File_Format
#define  T1_Err_Stack_Underflow          FT_Err_Invalid_File_Format
#define  T1_Err_Stack_Overflow           FT_Err_Invalid_File_Format

#endif /* TDERRORS_H */


/* END */
