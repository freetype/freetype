/***************************************************************************/
/*                                                                         */
/*  t2load.h                                                               */
/*                                                                         */
/*    OpenType glyph data/program tables loader (specification).           */
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


#ifndef T2LOAD_H
#define T2LOAD_H

#include <freetype/internal/t2types.h>
#include <freetype/internal/psnames.h>

#ifdef __cplusplus
  extern "C" {
#endif

  FT_LOCAL
  FT_String*  T2_Get_Name( CFF_Index*  index,
                           FT_UInt     element );

  FT_LOCAL
  FT_String*  T2_Get_String( CFF_Index*          index,
                             FT_UInt             sid,
                             PSNames_Interface*  interface );

  FT_LOCAL
  FT_Error  T2_Access_Element( CFF_Index*  index,
                               FT_UInt     element,
                               FT_Byte**   pbytes,
                               FT_ULong*   pbyte_len );

  FT_LOCAL
  void  T2_Forget_Element( CFF_Index*  index,
                           FT_Byte**   pbytes );

  FT_LOCAL
  FT_Error  T2_Load_CFF_Font( FT_Stream  stream,
                              FT_Int     face_index,
                              CFF_Font*  font );

  FT_LOCAL
  void  T2_Done_CFF_Font( CFF_Font*  font );

  FT_LOCAL
  FT_Byte  CFF_Get_FD( CFF_FD_Select*  select,
                       FT_UInt         glyph_index );


#ifdef __cplusplus
  }
#endif


#endif /* T2LOAD_H */


/* END */
