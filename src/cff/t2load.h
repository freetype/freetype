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

#ifdef __cplusplus
  extern "C" {
#endif

  LOCAL_DEF
  FT_String*  T2_Get_Name( CFF_Index*  index,
                           FT_UInt     element );

#if 0  /* will be used later for pure-CFF font support */

  LOCAL_DEF
  TT_String*  T2_Get_String( CFF_Index*          index,
                             TT_UInt             sid,
                             PSNames_Interface*  interface );

#endif

  LOCAL_DEF
  TT_Error  T2_Access_Element( CFF_Index*  index,
                               TT_UInt     element,
                               TT_Byte**   pbytes,
                               TT_ULong*   pbyte_len );

  LOCAL_DEF
  void  T2_Forget_Element( CFF_Index*  index,
                           TT_Byte**   pbytes );

  LOCAL_DEF
  TT_Error  T2_Load_CFF_Font( FT_Stream  stream,
                              TT_Int     face_index,
                              CFF_Font*  font );

  LOCAL_DEF
  void  T2_Done_CFF_Font( CFF_Font*  font );


#ifdef __cplusplus
  }
#endif


#endif /* T2LOAD_H */


/* END */
