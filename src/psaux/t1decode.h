#ifndef T1DECODE_H
#define T1DECODE_H

#include <freetype/internal/psaux.h>
#include <freetype/internal/psnames.h>
#include <freetype/internal/t1types.h>

  LOCAL_DEF
  const T1_Decoder_Funcs   t1_decoder_funcs;

  LOCAL_DEF
  FT_Error   T1_Decoder_Parse_Glyph( T1_Decoder*  decoder,
                                     FT_UInt      glyph_index );

  LOCAL_DEF
  FT_Error   T1_Decoder_Parse_Charstrings( T1_Decoder* decoder,
                                           FT_Byte*    base,
                                           FT_UInt     len );

  LOCAL_DEF
  FT_Error  T1_Decoder_Init( T1_Decoder*            decoder,
                             FT_Face                face,
                             FT_Size                size,
                             FT_GlyphSlot           slot,
                             FT_Byte**              glyph_names,
                             T1_Blend*              blend,
                             T1_Decoder_Parse_Func  parse_glyph );

  LOCAL_DEF
  void  T1_Decoder_Done( T1_Decoder*  decoder );

#endif /* T1DECODE_H */
