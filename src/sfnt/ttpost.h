/***************************************************************************/
/*                                                                         */
/*  ttpost.h                                                               */
/*                                                                         */
/*    Postcript name table processing for TrueType and OpenType fonts      */
/*    (specification).                                                     */
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


  /*************************************************************************/
  /*                                                                       */
  /* The post table is not completely loaded by the core engine.  This     */
  /* file loads the missing PS glyph names and implements an API to access */
  /* them.                                                                 */
  /*                                                                       */
  /*************************************************************************/


#ifndef TTPOST_H
#define TTPOST_H

#include <ftconfig.h>
#include <tttypes.h>

#ifdef __cplusplus
extern "C" {
#endif


#define TT_Err_Invalid_Post_Table_Format  0x0B00
#define TT_Err_Invalid_Post_Table         0x0B01

#if 0
  /* the 258 standard Mac glyph names, used for format 1.0 and 2.5 */
  LOCAL_DEF
  const TT_String*  TT_Mac_Postscript_Names[];
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Get_PS_Name                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Gets the PostScript glyph name of a glyph.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    index  :: The glyph index.                                         */
  /*                                                                       */
  /*    PSname :: The address of a string pointer.  Will be NULL in case   */
  /*              of error, otherwise it is a pointer to the glyph name.   */
  /*                                                                       */
  /*              You must not modify the returned string!                 */
  /*                                                                       */
  /* <Output>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  EXPORT_DEF
  TT_Error TT_Get_PS_Name( TT_Face      face,
                           TT_UInt      index,
                           TT_String**  PSname );


  LOCAL_DEF
  void  TT_Free_Post_Names( TT_Face  face );


#ifdef __cplusplus
}
#endif


#endif /* TTPOST_H */


/* END */
