/***************************************************************************/
/*                                                                         */
/*  fterrors.h                                                             */
/*                                                                         */
/*    FreeType error codes (specification).                                */
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


  /*************************************************************************/
  /*                                                                       */
  /* This file is used to define the FreeType error enumeration constants. */
  /*                                                                       */
  /* The lower byte gives the error code, the higher byte gives the        */
  /* module.  The base module has error offset 0.  For example, the error  */
  /* `FT_Err_Invalid_File_Format' has value 0x003, the error               */
  /* `TT_Err_Invalid_File_Format' has value 0xB03, the error               */
  /* `T1_Err_Invalid_File_Format' has value 0xC03, etc.                    */
  /*                                                                       */
  /* Undefine the macro FT_CONFIG_OPTION_USE_MODULE_ERRORS (in ftoption.h) */
  /* to make the higher byte always zero.                                  */
  /*                                                                       */
  /* It can also be used to create an error message table easily with      */
  /* something like                                                        */
  /*                                                                       */
  /*   {                                                                   */
  /*     #undef __FTERRORS_H__                                             */
  /*     #define FT_ERRORDEF( e, v, s )   { e, s },                        */
  /*     #define FT_ERROR_START_LIST      {                                */
  /*     #define FT_ERROR_END_LIST        { 0, 0 } };                      */
  /*                                                                       */
  /*     const struct                                                      */
  /*     {                                                                 */
  /*       int          err_code;                                          */
  /*       const char*  err_msg                                            */
  /*     } ft_errors[] =                                                   */
  /*                                                                       */
  /*     #include FT_ERRORS_H                                              */
  /*   }                                                                   */
  /*                                                                       */
  /* To use such a table, all errors must be ANDed with 0x00FF to remove   */
  /* the module error offset.                                              */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FTERRORS_H__
#define __FTERRORS_H__

#include FT_MODULE_ERRORS_H

#undef FT_NEED_EXTERN_C


  /* public interface */

#ifndef FT_ERRORDEF

#define FT_ERRORDEF( e, v, s )  e = v,

#ifdef __cplusplus
#define FT_NEED_EXTERN_C
  extern "C" {
#endif

#endif /* !FT_ERRORDEF */


#ifndef FT_ERROR_START_LIST

#define FT_ERROR_START_LIST  enum {
#define FT_ERROR_END_LIST    FT_Err_Max };

#endif /* !FT_ERROR_START_LIST */


  /* internal interface */

#ifndef FT_ERRORDEF_

#define FT_ERRORDEF_( e, v, s )   \
          FT_ERRORDEF( FT_Err_ ## e, v + FT_Mod_Err_Base, s )
#define FT_NOERRORDEF_( e, v, s ) \
          FT_ERRORDEF( FT_Err_ ## e, v, s )

#endif /* !FT_ERRORDEF_ */


#ifdef FT_ERROR_START_LIST
  FT_ERROR_START_LIST
#endif


  /* generic errors */

  FT_NOERRORDEF_( Ok,                                        0x00, \
                  "no error" )

  FT_ERRORDEF_( Cannot_Open_Resource,                        0x01, \
               "cannot open resource" )
  FT_ERRORDEF_( Unknown_File_Format,                         0x02, \
               "unknown file format" )
  FT_ERRORDEF_( Invalid_File_Format,                         0x03, \
               "broken file" )
  FT_ERRORDEF_( Invalid_Version,                             0x04, \
               "invalid FreeType version" )
  FT_ERRORDEF_( Lower_Module_Version,                        0x05, \
               "module version is too low" )
  FT_ERRORDEF_( Invalid_Argument,                            0x06, \
               "invalid argument" )
  FT_ERRORDEF_( Unimplemented_Feature,                       0x07, \
               "unimplemented feature" )

  /* glyph/character errors */

  FT_ERRORDEF_( Invalid_Glyph_Index,                         0x10, \
               "invalid glyph index" )
  FT_ERRORDEF_( Invalid_Character_Code,                      0x11, \
               "invalid character code" )
  FT_ERRORDEF_( Invalid_Glyph_Format,                        0x12, \
               "unsupported glyph image format" )
  FT_ERRORDEF_( Cannot_Render_Glyph,                         0x13, \
               "cannot render this glyph format" )
  FT_ERRORDEF_( Invalid_Outline,                             0x14, \
               "invalid outline" )
  FT_ERRORDEF_( Invalid_Composite,                           0x15, \
               "invalid composite glyph" )
  FT_ERRORDEF_( Too_Many_Hints,                              0x16, \
               "too many hints" )
  FT_ERRORDEF_( Invalid_Pixel_Size,                          0x17, \
               "invalid pixel size" )

  /* handle errors */

  FT_ERRORDEF_( Invalid_Handle,                              0x20, \
               "invalid object handle" )
  FT_ERRORDEF_( Invalid_Library_Handle,                      0x21, \
               "invalid library handle" )
  FT_ERRORDEF_( Invalid_Driver_Handle,                       0x22, \
               "invalid module handle" )
  FT_ERRORDEF_( Invalid_Face_Handle,                         0x23, \
               "invalid face handle" )
  FT_ERRORDEF_( Invalid_Size_Handle,                         0x24, \
               "invalid size handle" )
  FT_ERRORDEF_( Invalid_Slot_Handle,                         0x25, \
               "invalid glyph slot handle" )
  FT_ERRORDEF_( Invalid_CharMap_Handle,                      0x26, \
               "invalid charmap handle" )
  FT_ERRORDEF_( Invalid_Cache_Handle,                        0x27, \
               "invalid cache manager handle" )
  FT_ERRORDEF_( Invalid_Stream_Handle,                       0x28, \
               "invalid stream handle" )

  /* driver errors */

  FT_ERRORDEF_( Too_Many_Drivers,                            0x30, \
               "too many modules" )
  FT_ERRORDEF_( Too_Many_Extensions,                         0x31, \
               "too many extensions" )

  /* memory errors */

  FT_ERRORDEF_( Out_Of_Memory,                               0x40, \
               "out of memory" )
  FT_ERRORDEF_( Unlisted_Object,                             0x41, \
               "unlisted object" )

  /* stream errors */

  FT_ERRORDEF_( Cannot_Open_Stream,                          0x51, \
               "cannot open stream" )
  FT_ERRORDEF_( Invalid_Stream_Seek,                         0x52, \
               "invalid stream seek" )
  FT_ERRORDEF_( Invalid_Stream_Skip,                         0x53, \
               "invalid stream skip" )
  FT_ERRORDEF_( Invalid_Stream_Read,                         0x54, \
               "invalid stream read" )
  FT_ERRORDEF_( Invalid_Stream_Operation,                    0x55, \
               "invalid stream operation" )
  FT_ERRORDEF_( Invalid_Frame_Operation,                     0x56, \
               "invalid frame operation" )
  FT_ERRORDEF_( Nested_Frame_Access,                         0x57, \
               "nested frame access" )
  FT_ERRORDEF_( Invalid_Frame_Read,                          0x58, \
               "invalid frame read" )

  /* raster errors */

  FT_ERRORDEF_( Raster_Uninitialized,                        0x60, \
               "raster uninitialized" )
  FT_ERRORDEF_( Raster_Corrupted,                            0x61, \
               "raster corrupted" )
  FT_ERRORDEF_( Raster_Overflow,                             0x62, \
               "raster overflow" )
  FT_ERRORDEF_( Raster_Negative_Height,                      0x63, \
               "negative height while rastering" )

  /* cache errors */

  FT_ERRORDEF_( Too_Many_Caches,                             0x70, \
               "too many registered caches" )

  /* TrueType and SFNT errors */

  FT_ERRORDEF_( Invalid_Opcode,                              0x80, \
               "invalid opcode" )
  FT_ERRORDEF_( Too_Few_Arguments,                           0x81, \
               "too few arguments" )
  FT_ERRORDEF_( Stack_Overflow,                              0x82, \
               "stack overflow" )
  FT_ERRORDEF_( Code_Overflow,                               0x83, \
               "code overflow" )
  FT_ERRORDEF_( Bad_Argument,                                0x84, \
               "bad argument" )
  FT_ERRORDEF_( Divide_By_Zero,                              0x85, \
               "division by zero" )
  FT_ERRORDEF_( Invalid_Reference,                           0x86, \
               "invalid reference" )
  FT_ERRORDEF_( Debug_OpCode,                                0x87, \
               "found debug opcode" )
  FT_ERRORDEF_( ENDF_In_Exec_Stream,                         0x88, \
               "found ENDF opcode in execution stream" )
  FT_ERRORDEF_( Nested_DEFS,                                 0x89, \
               "nested DEFS" )
  FT_ERRORDEF_( Invalid_CodeRange,                           0x8A, \
               "invalid code range" )
  FT_ERRORDEF_( Execution_Too_Long,                          0x8B, \
               "execution context too long" )
  FT_ERRORDEF_( Too_Many_Function_Defs,                      0x8C, \
               "too many function definitions" )
  FT_ERRORDEF_( Too_Many_Instruction_Defs,                   0x8D, \
               "too many instruction definitions" )
  FT_ERRORDEF_( Table_Missing,                               0x8E, \
               "SFNT font table missing" )
  FT_ERRORDEF_( Horiz_Header_Missing,                        0x8F, \
               "horizontal header (hhea) table missing" )
  FT_ERRORDEF_( Locations_Missing,                           0x90, \
               "locations (loca) table missing" )
  FT_ERRORDEF_( Name_Table_Missing,                          0x91, \
               "name table missing" )
  FT_ERRORDEF_( CMap_Table_Missing,                          0x92, \
               "character map (cmap) table missing" )
  FT_ERRORDEF_( Hmtx_Table_Missing,                          0x93, \
               "horizontal metrics (hmtx) table missing" )
  FT_ERRORDEF_( Post_Table_Missing,                          0x94, \
               "PostScript (post) table missing" )
  FT_ERRORDEF_( Invalid_Horiz_Metrics,                       0x95, \
               "invalid horizontal metrics" )
  FT_ERRORDEF_( Invalid_CharMap_Format,                      0x96, \
               "invalid character map (cmap) format" )
  FT_ERRORDEF_( Invalid_PPem,                                0x97, \
               "invalid ppem value" )
  FT_ERRORDEF_( Invalid_Vert_Metrics,                        0x98, \
               "invalid vertical metrics" )
  FT_ERRORDEF_( Could_Not_Find_Context,                      0x99, \
               "could not find context" )
  FT_ERRORDEF_( Invalid_Post_Table_Format,                   0x9A, \
               "invalid PostScript (post) table format" )
  FT_ERRORDEF_( Invalid_Post_Table,                          0x9B, \
               "invalid PostScript (post) table" )

  /* CFF, CID, and Type 1 errors */

  FT_ERRORDEF_( Syntax_Error,                                0xA0, \
               "opcode syntax error" )
  FT_ERRORDEF_( Stack_Underflow,                             0xA1, \
               "argument stack underflow" )


#ifdef FT_ERROR_END_LIST
  FT_ERROR_END_LIST
#endif


#undef FT_ERROR_START_LIST
#undef FT_ERROR_END_LIST
#undef FT_ERRORDEF
#undef FT_ERRORDEF_
#undef FT_NOERRORDEF_


#ifdef FT_NEED_EXTERN_C
  }
#endif

#endif /* __FTERRORS_H__ */


/* END */
