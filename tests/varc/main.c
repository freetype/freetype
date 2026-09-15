#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H


typedef struct  TestFont_
{
  const char*  filename;
  FT_UInt      glyph_index;
  FT_UInt      point_count;
  FT_Pos       first_x;

} TestFont;


static int
test_font( FT_Library       library,
           const char*      testdata_dir,
           const TestFont*  test_font )
{
  FT_Face  face = NULL;
  char     filepath[FILENAME_MAX];

  int  ret = 1;


  snprintf( filepath, sizeof ( filepath ), "%s/%s",
            testdata_dir, test_font->filename );

  if ( FT_New_Face( library, filepath, 0, &face ) )
  {
    fprintf( stderr, "Could not open file: %s\n", filepath );
    goto Exit;
  }

  if ( FT_HAS_MULTIPLE_MASTERS( face ) )
  {
    fprintf( stderr, "%s unexpectedly exposes variation axes\n",
             test_font->filename );
    goto Exit;
  }

  if ( FT_Load_Glyph( face, test_font->glyph_index,
                      FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING ) )
  {
    fprintf( stderr, "Could not load VARC glyph from %s\n",
             test_font->filename );
    goto Exit;
  }

  if ( face->glyph->format != FT_GLYPH_FORMAT_OUTLINE                   ||
       (FT_UInt)face->glyph->outline.n_points != test_font->point_count ||
       face->glyph->outline.points[0].x != test_font->first_x           )
  {
    fprintf( stderr, "Unexpected VARC outline from %s\n",
             test_font->filename );
    goto Exit;
  }

  {
    FT_MM_Var*  master = NULL;


    if ( !FT_Get_MM_Var( face, &master ) )
    {
      fprintf( stderr, "%s exposes private VARC axes through fvar\n",
               test_font->filename );
      FT_Done_MM_Var( library, master );
      goto Exit;
    }
  }

  ret = 0;

Exit:
  FT_Done_Face( face );
  return ret;
}


int
main( void )
{
  static const TestFont  test_fonts[] =
  {
    { "varc-static-gvar.ttf", 1,  3, 50 },
    { "varc-static-cff2.otf", 2, 60, 36 }
  };

  FT_Library  library;
  const char*  testdata_dir = getenv( "FREETYPE_TESTS_DATA_DIR" );

  int      ret = 1;
  FT_UInt  i;


  if ( !testdata_dir )
    testdata_dir = "../tests/data";

  if ( FT_Init_FreeType( &library ) )
  {
    fprintf( stderr, "Could not initialize FreeType\n" );
    return ret;
  }

  ret = 0;
  for ( i = 0;
        i < sizeof ( test_fonts ) / sizeof ( test_fonts[0] );
        i++ )
    ret |= test_font( library, testdata_dir, &test_fonts[i] );

  FT_Done_FreeType( library );
  return ret;
}


/* EOF */
