#ifndef __FT_SERVICE_GLYPH_DICT_H__
#define __FT_SERVICE_GLYPH_DICT_H__

#include FT_INTERNAL_SERVICE_H

FT_BEGIN_HEADER

 /*
  *  a service used to retrieve glyph names, as well as to find the
  *  index of a given glyph name in a font.
  *
  */

#define FT_SERVICE_ID_GLYPH_DICT  "glyph-dict"

  typedef FT_Error  (*FT_GlyphDict_GetNameFunc)
                             ( FT_Face     face,
                               FT_UInt     glyph_index,
                               FT_Pointer  buffer,
                               FT_UInt     buffer_max );

  typedef  FT_UInt  (*FT_GlyphDict_NameIndexFunc)
                              ( FT_Face     face,
                                FT_String*  glyph_name );

  FT_DEFINE_SERVICE( GlyphDict )
  {
    FT_GlyphDict_GetNameFunc    get_name;
    FT_GlyphDict_NameIndexFunc  name_index;  /* optional */
  };

 /* */

FT_END_HEADER

#endif /* __FT_SERVICE_GLYPH_DICT_H__ */
