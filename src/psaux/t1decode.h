/***************************************************************************/
/*                                                                         */
/*  t1decode.h                                                             */
/*                                                                         */
/*    PostScript Type 1 decoding routines (specification).                 */
/*                                                                         */
/*  Copyright 2000 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef T1DECODE_H
#define T1DECODE_H

#include <freetype/internal/psaux.h>
#include <freetype/internal/psnames.h>
#include <freetype/internal/t1types.h>


#ifdef __cplusplus
  extern "C" {
#endif


  FT_CALLBACK_TABLE
  const T1_Decoder_Funcs  t1_decoder_funcs;


  FT_LOCAL
  FT_Error  T1_Decoder_Parse_Glyph( T1_Decoder*  decoder,
                                    FT_UInt      glyph_index );

  FT_LOCAL
  FT_Error  T1_Decoder_Parse_Charstrings( T1_Decoder*  decoder,
                                          FT_Byte*     base,
                                          FT_UInt      len );

  FT_LOCAL
  FT_Error  T1_Decoder_Init( T1_Decoder*          decoder,
                             FT_Face              face,
                             FT_Size              size,
                             FT_GlyphSlot         slot,
                             FT_Byte**            glyph_names,
                             T1_Blend*            blend,
                             T1_Decoder_Callback  parse_glyph );

  FT_LOCAL
  void  T1_Decoder_Done( T1_Decoder*  decoder );


#ifdef __cplusplus
  }
#endif


#endif /* T1DECODE_H */


/* END */
