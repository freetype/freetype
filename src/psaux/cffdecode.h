
#ifndef CFFDECODE_H_
#define CFFDECODE_H_


#include <ft2build.h>


FT_BEGIN_HEADER

  FT_LOCAL( void )
  cff_decoder_init( CFF_Decoder*    decoder,
                    TT_Face         face,
                    CFF_Size        size,
                    CFF_GlyphSlot   slot,
                    FT_Bool         hinting,
                    FT_Render_Mode  hint_mode);

  FT_LOCAL( FT_Error )
  cff_decoder_prepare( CFF_Decoder*  decoder,
                       CFF_Size      size,
                       FT_UInt       glyph_index );


  FT_LOCAL( FT_Int )
  cff_lookup_glyph_by_stdcharcode( CFF_Font  cff,
                                   FT_Int    charcode );


  #ifdef CFF_CONFIG_OPTION_OLD_ENGINE
    FT_LOCAL( FT_Error )
    cff_decoder_parse_charstrings( CFF_Decoder*  decoder,
                                   FT_Byte*      charstring_base,
                                   FT_ULong      charstring_len,
                                   FT_Bool       in_dict );
  #endif


FT_END_HEADER

#endif


/* END */
