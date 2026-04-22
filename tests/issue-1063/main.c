#include <freetype/freetype.h>
#include <ft2build.h>

#ifndef _PLAN9
#include <stdio.h>
#endif

int
main( void )
{
  FT_Library  library;
  FT_Face     face = NULL;

  /*
   * We assume that `FREETYPE_TESTS_DATA_DIR` was set by `meson test`.
   * Otherwise we default to `../tests/data`.
   *
   * TODO (David): Rewrite this to pass the test directory through the
   * command-line.
   */
  const char*  testdata_dir = getenv( "FREETYPE_TESTS_DATA_DIR" );
  char         filepath[FILENAME_MAX];


  snprintf( filepath, sizeof( filepath ), "%s/%s",
            testdata_dir ? testdata_dir : "../tests/data",
            "As.I.Lay.Dying.ttf" );

  FT_Init_FreeType( &library );
  if ( FT_New_Face( library, filepath, 0, &face ) != 0 )
  {
    fprintf( stderr, "Could not open file: %s\n", filepath );
    return 1;
  }

  for ( FT_ULong  i = 59; i < 171; i++ )
  {
    FT_UInt   gid  = FT_Get_Char_Index( face, i );
    FT_Error  code = FT_Load_Glyph( face, gid, FT_LOAD_DEFAULT );


    if ( code )
      printf( "unknown %d for char %lu, gid %u\n", code, i, gid );
  }

#ifdef _PLAN9
  exits(0);
#endif
  return 0;
}

/* EOF */
