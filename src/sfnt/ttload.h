/***************************************************************************/
/*                                                                         */
/*  ttload.h                                                               */
/*                                                                         */
/*    Load the basic TrueType tables, i.e., tables that can be either in   */
/*    TTF or OTF font (specification).                                     */
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


#ifndef TTLOAD_H
#define TTLOAD_H


#include <ftstream.h>
#include <tttypes.h>
/*
#include <ttobjs.h>
*/

#ifdef __cplusplus
  extern "C" {
#endif


  EXPORT_DEF
  TT_Table*  TT_LookUp_Table( TT_Face   face,
                              TT_ULong  tag );

  LOCAL_DEF
  TT_Error   TT_Goto_Table( TT_Face    face,
                            TT_ULong   tag,
                            FT_Stream  stream,
                            TT_ULong  *length );


  LOCAL_DEF
  TT_Error  TT_Load_Format_Tag( TT_Face    face,
                                FT_Stream  stream,
                                TT_Long    faceIndex,
                                TT_ULong  *format_tag );

  LOCAL_DEF
  TT_Error  TT_Load_Directory( TT_Face    face,
                               FT_Stream  stream,
                               TT_Long    faceIndex );


  LOCAL_DEF
  TT_Error  TT_Load_Any( TT_Face   face,
                         TT_ULong  tag,
                         TT_Long   offset,
                         void*     buffer,
                         TT_Long*  length );


  LOCAL_DEF
  TT_Error  TT_Load_Header( TT_Face    face,
                            FT_Stream  stream );


  LOCAL_DEF
  TT_Error  TT_Load_Metrics_Header( TT_Face    face,
                                    FT_Stream  stream,
                                    TT_Bool    vertical );


  LOCAL_DEF
  TT_Error  TT_Load_CMap( TT_Face    face,
                          FT_Stream  stream );


  LOCAL_DEF
  TT_Error  TT_Load_MaxProfile( TT_Face    face,
                                FT_Stream  stream );


  LOCAL_DEF
  TT_Error  TT_Load_Names( TT_Face    face,
                           FT_Stream  stream );


  LOCAL_DEF
  TT_Error  TT_Load_OS2( TT_Face    face,
                         FT_Stream  stream );


  LOCAL_DEF
  TT_Error  TT_Load_PostScript( TT_Face    face,
                                FT_Stream  stream );


  LOCAL_DEF
  TT_Error  TT_Load_Hdmx( TT_Face    face,
                          FT_Stream  stream );


  LOCAL_DEF
  void  TT_Free_Names( TT_Face  face );


  LOCAL_DEF
  void  TT_Free_Hdmx ( TT_Face  face );


  LOCAL_DEF
  TT_Error  TT_Load_Kern( TT_Face    face,
                          FT_Stream  stream );


  LOCAL_DEF
  TT_Error  TT_Load_Gasp( TT_Face    face,
                          FT_Stream  stream );


#endif /* TTLOAD_H */


/* END */
