/***************************************************************************/
/*                                                                         */
/*  cfferrs.h                                                              */
/*                                                                         */
/*    OpenType error ID definitions (specification only).                  */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __CFFERRORS_H__
#define __CFFERRORS_H__


#include <ft2build.h>


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* Error codes declaration                                               */
  /*                                                                       */
  /* The error codes are grouped in `classes' used to indicate the `level' */
  /* at which the error happened.  The class is given by an error code's   */
  /* high byte.                                                            */
  /*                                                                       */
  /*************************************************************************/


  /* Success is always 0. */

#define CFF_Err_Ok                         FT_Err_Ok

  /* High level API errors. */

#define CFF_Err_Invalid_File_Format        FT_Err_Invalid_File_Format
#define CFF_Err_Invalid_Argument           FT_Err_Invalid_Argument
#define CFF_Err_Invalid_Driver_Handle      FT_Err_Invalid_Driver_Handle
#define CFF_Err_Invalid_Face_Handle        FT_Err_Invalid_Face_Handle
#define CFF_Err_Invalid_Instance_Handle    FT_Err_Invalid_Size_Handle
#define CFF_Err_Invalid_Glyph_Handle       FT_Err_Invalid_Slot_Handle
#define CFF_Err_Invalid_CharMap_Handle     FT_Err_Invalid_CharMap_Handle
#define CFF_Err_Invalid_Glyph_Index        FT_Err_Invalid_Glyph_Index

#define CFF_Err_Unimplemented_Feature      FT_Err_Unimplemented_Feature

#define CFF_Err_Invalid_Engine             FT_Err_Invalid_Driver_Handle

  /* Internal errors. */

#define CFF_Err_Out_Of_Memory              FT_Err_Out_Of_Memory
#define CFF_Err_Unlisted_Object            FT_Err_Unlisted_Object

  /* General glyph outline errors. */

#define CFF_Err_Invalid_Composite          FT_Err_Invalid_Composite

  /* Bytecode interpreter error codes. */

  /* These error codes are produced by the TrueType */
  /* bytecode interpreter.  They usually indicate a */
  /* broken font file, a broken glyph within a font */
  /* file, or a bug in the interpreter!             */

#define CFF_Err_Invalid_Opcode             0x500
#define CFF_Err_Too_Few_Arguments          0x501
#define CFF_Err_Stack_Overflow             0x502
#define CFF_Err_Code_Overflow              0x503
#define CFF_Err_Bad_Argument               0x504
#define CFF_Err_Divide_By_Zero             0x505
#define CFF_Err_Storage_Overflow           0x506
#define CFF_Err_Cvt_Overflow               0x507
#define CFF_Err_Invalid_Reference          0x508
#define CFF_Err_Invalid_Distance           0x509
#define CFF_Err_Interpolate_Twilight       0x50A
#define CFF_Err_Debug_OpCode               0x50B
#define CFF_Err_ENDF_In_Exec_Stream        0x50C
#define CFF_Err_Out_Of_CodeRanges          0x50D
#define CFF_Err_Nested_DEFS                0x50E
#define CFF_Err_Invalid_CodeRange          0x50F
#define CFF_Err_Invalid_Displacement       0x510
#define CFF_Err_Execution_Too_Long         0x511

#define CFF_Err_Too_Many_Instruction_Defs  0x512
#define CFF_Err_Too_Many_Function_Defs     0x513

  /* Other TrueType specific error codes. */

#define CFF_Err_Table_Missing              0x520
#define CFF_Err_Too_Many_Extensions        0x521
#define CFF_Err_Extensions_Unsupported     0x522
#define CFF_Err_Invalid_Extension_Id       0x523

#define CFF_Err_No_Vertical_Data           0x524

#define CFF_Err_Max_Profile_Missing        0x530
#define CFF_Err_Header_Table_Missing       0x531
#define CFF_Err_Horiz_Header_Missing       0x532
#define CFF_Err_Locations_Missing          0x533
#define CFF_Err_Name_Table_Missing         0x534
#define CFF_Err_CMap_Table_Missing         0x535
#define CFF_Err_Hmtx_Table_Missing         0x536
#define CFF_Err_OS2_Table_Missing          0x537
#define CFF_Err_Post_Table_Missing         0x538

#define CFF_Err_Invalid_Horiz_Metrics      0x540
#define CFF_Err_Invalid_CharMap_Format     0x541
#define CFF_Err_Invalid_PPem               0x542
#define CFF_Err_Invalid_Vert_Metrics       0x543

#define CFF_Err_Could_Not_Find_Context     0x550


FT_END_HEADER


#endif /* __CFFERRORS_H__ */


/* END */
