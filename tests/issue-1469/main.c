/* Regression for 73720c7c: append phantom points without signed-short wrap.
 * Distributed under the FreeType project license, LICENSE.TXT.
 */

#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H


static int
test_font( FT_Library   library,
           const char* filename,
           FT_UInt     components,
           FT_Fixed    coordinate )
{
  FT_Face   face = NULL;
  FT_Error  error;
  int       result = 1;


  error = FT_New_Face( library, filename, 0, &face );
  if ( error )
  {
    fprintf( stderr, "Face open failed: %d\n", error );
    goto Exit;
  }

  if ( !FT_HAS_MULTIPLE_MASTERS( face ) || face->num_glyphs != 2 )
  {
    fprintf( stderr, "Expected a two-glyph variable font\n" );
    goto Exit;
  }

  error = FT_Set_Pixel_Sizes( face, 16, 16 );
  /* Omitting the API call is the inactive-variation control.  Older
   * FreeType versions set FT_IS_VARIATION even for an explicit zero.
   */
  if ( !error && coordinate )
    error = FT_Set_Var_Design_Coordinates( face, 1, &coordinate );
  if ( error )
  {
    fprintf( stderr, "Variation setup failed: %d\n", error );
    goto Exit;
  }

  if ( !!FT_IS_VARIATION( face ) != !!coordinate )
  {
    fprintf( stderr, "Unexpected variation state\n" );
    goto Exit;
  }

  printf( "LOAD components=%u coordinate=%ld\n",
          components, (long)coordinate );
  fflush( stdout );
  /* Phantom-point setup precedes the NO_RECURSE early return. */
  error = FT_Load_Glyph( face, 1,
                        FT_LOAD_NO_RECURSE | FT_LOAD_NO_HINTING |
                        FT_LOAD_NO_BITMAP );
  if ( error )
  {
    fprintf( stderr, "Glyph load failed: %d\n", error );
    goto Exit;
  }

  if ( face->glyph->format != FT_GLYPH_FORMAT_COMPOSITE ||
       face->glyph->num_subglyphs != components )
  {
    fprintf( stderr, "Unexpected composite result\n" );
    goto Exit;
  }

  printf( "PASS components=%u coordinate=%ld\n",
          components, (long)coordinate );
  result = 0;

Exit:
  if ( face )
    FT_Done_Face( face );
  return result;
}


int
main( int argc, char** argv )
{
  FT_Library  library;
  int         result = 0;
  int         i;
  unsigned long components = 0;
  long          coordinate = 0;
  char*         end;


  if ( argc == 4 )
  {
    components = strtoul( argv[2], &end, 10 );
    if ( end == argv[2] || *end || components < 32764 || components > 32767 )
      return 2;
    coordinate = strtol( argv[3], &end, 10 );
    if ( end == argv[3] || *end || ( coordinate != 0 && coordinate != 65536 ) )
      return 2;
  }
  else if ( argc != 5 )
  {
    fprintf( stderr, "usage: issue-1469 font32764 font32765 font32766 font32767\n"
                     "   or: issue-1469 font count coordinate\n" );
    return 2;
  }
  if ( FT_Init_FreeType( &library ) )
    return 2;

  if ( argc == 4 )
    result = test_font( library, argv[1], (FT_UInt)components, coordinate );
  else
  {
    /* No variation setup is a negative control for every count. */
    for ( i = 0; i < 4; i++ )
      result |= test_font( library, argv[i + 1], 32764U + i, 0 );
    for ( i = 0; i < 4; i++ )
      result |= test_font( library, argv[i + 1], 32764U + i, 0x10000L );
  }

  FT_Done_FreeType( library );
  return result;
}
