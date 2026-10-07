/* Shared lookups must preserve the input glyphs' script classification.
 * Distributed under the FreeType project license, LICENSE.TXT.
 */

#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_DRIVER_H
#include FT_MODULE_H


/* The experimental glyph-to-script-map contains style indices and flags. */
#define STYLE_MASK  0x1FFF


static int
test_font( FT_Library   library,
           const char*  filename,
           FT_Bool      cyrillic )
{
  static const FT_UInt  latin_substitutes[] = { 3, 4, 5, 6, 12, 13 };

  FT_Face                    face = NULL;
  FT_Prop_GlyphToScriptMap   prop;
  FT_Error                   error;
  FT_UShort                  latin, fallback;
  unsigned int               i;
  int                        result = 1;


  error = FT_New_Face( library, filename, 0, &face );
  if ( error )
    goto Exit;

  prop.face = face;
  prop.map  = NULL;
  error = FT_Property_Get( library, "autofitter", "glyph-to-script-map",
                           &prop );
  if ( error )
    goto Exit;

  latin    = prop.map[1] & STYLE_MASK;
  fallback = prop.map[0] & STYLE_MASK;

#ifdef TEST_DYNAMIC_HARFBUZZ
  /* Dynamic builds can run without a usable HarfBuzz library.  Without
   * HarfBuzz the small-cap glyph keeps the fallback style, too.
   */
  if ( ( prop.map[10] & STYLE_MASK ) == fallback )
  {
    result = 77;
    goto Exit;
  }
#endif

  for ( i = 0; i < sizeof ( latin_substitutes ) /
                     sizeof ( latin_substitutes[0] ); i++ )
  {
    FT_UInt  glyph = latin_substitutes[i];


    if ( ( prop.map[glyph] & STYLE_MASK ) != latin )
    {
      fprintf( stderr, "%s: glyph %u did not inherit Latin style\n",
               filename, glyph );
      goto Exit;
    }
  }

  if ( ( prop.map[8] & STYLE_MASK ) !=
         ( cyrillic ? ( prop.map[7] & STYLE_MASK ) : fallback ) ||
       ( prop.map[11] & STYLE_MASK ) != fallback                ||
       ( prop.map[10] & STYLE_MASK ) == latin                   ||
       ( prop.map[10] & STYLE_MASK ) == fallback                ||
       ( cyrillic && ( prop.map[7] & STYLE_MASK ) == latin )    )
  {
    fprintf( stderr, "%s: wrong Cyrillic, unreachable, or small-cap style\n",
             filename );
    goto Exit;
  }

  error = FT_Set_Pixel_Sizes( face, 0, 20 );
  if ( error )
    goto Exit;

  /* Initializing metrics must not disable hinting for these substitutes. */
  for ( i = 0; i < sizeof ( latin_substitutes ) /
                     sizeof ( latin_substitutes[0] ); i++ )
  {
    FT_UInt  glyph = latin_substitutes[i];


    error = FT_Load_Glyph( face, glyph,
                           FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT );
    if ( error || ( prop.map[glyph] & STYLE_MASK ) != latin )
    {
      fprintf( stderr, "%s: hinting disabled for glyph %u\n",
               filename, glyph );
      goto Exit;
    }
  }

  result = 0;

Exit:
  if ( error )
    fprintf( stderr, "%s: FreeType error %d\n", filename, error );
  if ( face )
    FT_Done_Face( face );
  return result;
}


int
main( int argc, char** argv )
{
  FT_Library  library;
  int         result;


  if ( argc != 3 || FT_Init_FreeType( &library ) )
    return 2;

  result = test_font( library, argv[1], 0 );
  if ( result != 77 )
    result |= test_font( library, argv[2], 1 );

  FT_Done_FreeType( library );
  return result;
}
