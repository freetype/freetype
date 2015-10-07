#include <assert.h>
#include <stdint.h>

#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_CACHE_H
#include FT_CACHE_CHARMAP_H
#include FT_CACHE_IMAGE_H
#include FT_CACHE_SMALL_BITMAPS_H
#include FT_SYNTHESIS_H
#include FT_ADVANCES_H
#include FT_OUTLINE_H
#include FT_BBOX_H
#include FT_MODULE_H
#include FT_CFF_DRIVER_H
#include FT_TRUETYPE_DRIVER_H


  static FT_Library  library;
  static int         InitResult = FT_Init_FreeType( &library );


  extern "C" int
  LLVMFuzzerTestOneInput( const uint8_t*  data,
                          size_t          size )
  {
    assert( !InitResult );

    if ( size < 1 )
      return 0;

    FT_Face         face;
    FT_Int32        load_flags  = FT_LOAD_DEFAULT;
    FT_Render_Mode  render_mode = FT_RENDER_MODE_NORMAL;

    if ( !FT_New_Memory_Face( library, data, size, 0, &face ) )
    {
      unsigned int  first_index = 0;

      for ( unsigned i = first_index;
            i < (unsigned int)face->num_glyphs;
            i++ )
      {
        if ( FT_Load_Glyph( face, i, load_flags ) )
          continue;

        // Rendering is the most expensive and the least interesting part.
        //
        // if ( FT_Render_Glyph( face->glyph, render_mode) )
        //   continue;
        // FT_GlyphSlot_Embolden( face->glyph );

#if 0
        FT_Glyph  glyph;

        if ( !FT_Get_Glyph( face->glyph, &glyph ) )
          FT_Done_Glyph( glyph );

        FT_Outline*  outline = &face->glyph->outline;
        FT_Matrix    rot30   = { 0xDDB4, -0x8000, 0x8000, 0xDDB4 };

        FT_Outline_Transform( outline, &rot30 );

        FT_BBox  bbox;

        FT_Outline_Get_BBox( outline, &bbox );
#endif
      }

      FT_Done_Face( face );
    }

    return 0;
  }


/* END */
