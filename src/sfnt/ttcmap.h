/***************************************************************************/
/*                                                                         */
/*  ttcmap.h                                                               */
/*                                                                         */
/*    TrueType character mapping table (cmap) support (specification).     */
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


#ifndef TTCMAP_H
#define TTCMAP_H

#include <tttypes.h>

#ifdef __cplusplus
  extern "C" {
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_CharMap_Load                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a given TrueType character map into memory.                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the parent face object.                      */
  /*    stream :: A handle to the current stream object.                   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    cmap   :: A pointer to a cmap object.                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The function assumes that the stream is already in use (i.e.,      */
  /*    opened).  In case of error, all partially allocated tables are     */
  /*    released.                                                          */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_CharMap_Load( TT_Face        face,
                             TT_CMapTable*  cmap,
                             FT_Stream      input );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_CharMap_Free                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroys a character mapping table.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to the parent face object.                        */
  /*    cmap :: A handle to a cmap object.                                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_CharMap_Free( TT_Face        face,
                             TT_CMapTable*  cmap );

#ifdef __cplusplus
  }
#endif

#endif /* TTCMAP_H */


/* END */
