/***************************************************************************/
/*                                                                         */
/*  ttsbit.h                                                               */
/*                                                                         */
/*    TrueType and OpenType embedded bitmap support (specification).       */
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


#ifndef TTSBIT_H
#define TTSBIT_H

#include <ttload.h>


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_SBit_Strikes                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the table of embedded bitmap sizes for this face.            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: The target face object.                                  */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_SBit_Strikes( TT_Face    face,
                                  FT_Stream  stream );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Free_SBit_Strikes                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases the embedded bitmap tables.                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: The target face object.                                    */
  /*                                                                       */
  LOCAL_DEF
  void  TT_Free_SBit_Strikes( TT_Face  face );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_SBit_Image                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a given glyph sbit image from the font resource.  This also  */
  /*    returns its metrics.                                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: The target face object.                             */
  /*                                                                       */
  /*    x_ppem      :: The horizontal resolution in points per EM.         */
  /*                                                                       */
  /*    y_ppem      :: The vertical resolution in points per EM.           */
  /*                                                                       */
  /*    glyph_index :: The current glyph index.                            */
  /*                                                                       */
  /*    stream      :: The input stream.                                   */
  /*                                                                       */
  /* <Output>                                                              */
  /*    map         :: The target pixmap.                                  */
  /*    metrics     :: A big sbit metrics structure for the glyph image.   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.  Returns an error if no     */
  /*    glyph sbit exists for the index.                                   */
  /*                                                                       */
  /*  <Note>                                                               */
  /*    The `map.buffer' field is always freed before the glyph is loaded. */
  /*                                                                       */
  LOCAL_DEF
  TT_Error  TT_Load_SBit_Image( TT_Face           face,
                                TT_Int            x_ppem,
                                TT_Int            y_ppem,
                                TT_UInt           glyph_index,
                                FT_Stream         stream,
                                FT_Bitmap*        map,
                                TT_SBit_Metrics*  metrics );


#endif /* TTSBIT_H */


/* END */
