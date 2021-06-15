#include <stdio.h>

#include <freetype/freetype.h>
#include <ft2build.h>

int
main( void )
{
  FT_Library library;
  FT_Face    face;

  /* Assumes this is run from out/ build directory though 'meson test -C out' */
  const char* filepath = "../tests/data/As.I.Lay.Dying.ttf";

  FT_Init_FreeType( &library );
  FT_New_Face( library, filepath, 0, &face );
  if ( !face )
  {
    fprintf( stderr, "Could not open file: %s\n", filepath );
    return 1;
  }

  for ( FT_ULong i = 59; i < 171; i++ )
  {
    FT_UInt  gid  = FT_Get_Char_Index( face, i );
    FT_Error code = FT_Load_Glyph( face, gid, FT_LOAD_DEFAULT );
    if ( code )
      printf( "unknown %d for char %lu, gid %u\n", code, i, gid );
  }

  return 0;
}
