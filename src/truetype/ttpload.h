/***************************************************************************/
/*                                                                         */
/*  ttpload.h                                                              */
/*                                                                         */
/*    TrueType glyph data/program tables loader (specification).           */
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


#ifndef TTPLOAD_H
#define TTPLOAD_H

#include <tttypes.h>


#ifdef __cplusplus
  extern "C" {
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Locations                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the locations table.                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_Locations( TT_Face    face,
                               FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_CVT                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the control value table into a face object.                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: A handle to the input stream.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_CVT( TT_Face    face,
                         FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Progams                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the font program and the cvt program.                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: A handle to the input stream.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_Programs( TT_Face    face,
                              FT_Stream  stream );

#endif /* TTPLOAD_H */


/* END */
