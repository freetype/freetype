#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int
test_padded_target( FT_Library  library,
                    int         vertical )
{
  const unsigned char  old_pixel[4] = { 1, 2, 3, 4 };
  unsigned char        gray = vertical ? 255 : 0;
  FT_Bitmap            source;
  FT_Bitmap            target;
  FT_Vector            source_offset = { vertical ? 0 : 64,
                                          vertical ? 64 : 0 };
  FT_Vector            target_offset = { 0, 0 };
  FT_Color             color = { 10, 20, 30, 255 };
  FT_Error             error;
  int                  failed = 0;

  FT_Bitmap_Init( &source );
  FT_Bitmap_Init( &target );

  source.width      = 1;
  source.rows       = 1;
  source.pitch      = 1;
  source.pixel_mode = FT_PIXEL_MODE_GRAY;
  source.num_grays  = 256;
  source.buffer     = &gray;

  target.width      = 1;
  target.rows       = 1;
  target.pitch      = 8;  /* one BGRA pixel and four padding bytes */
  target.pixel_mode = FT_PIXEL_MODE_BGRA;
  target.num_grays  = 256;
  target.buffer     = (unsigned char*)malloc( 8 );
  if ( !target.buffer )
    return 1;
  memcpy( target.buffer, old_pixel, 4 );
  memset( target.buffer + 4, 0xAA, 4 );

  error = FT_Bitmap_Blend( library, &source, source_offset,
                           &target, &target_offset, color );
  if ( error ||
       target.width != ( vertical ? 1U : 2U ) ||
       target.rows  != ( vertical ? 2U : 1U ) ||
       target.pitch != ( vertical ? 4 : 8 ) )
    failed = 1;
  else if ( vertical )
  {
    if ( target_offset.x != 0 || target_offset.y != 64 ||
         target.buffer[3] != 255 ||
         memcmp( target.buffer + 4, old_pixel, 4 ) != 0 )
      failed = 1;
  }
  else
  {
    const unsigned char  transparent[4] = { 0, 0, 0, 0 };

    if ( target_offset.x != 0 || target_offset.y != 0 ||
         memcmp( target.buffer, old_pixel, 4 ) != 0 ||
         memcmp( target.buffer + 4, transparent, 4 ) != 0 )
      failed = 1;
  }

  if ( failed )
    fprintf( stderr, "padded target blend failed (%s)\n",
             vertical ? "vertical" : "horizontal" );
  FT_Bitmap_Done( library, &target );
  return failed;
}

int
main( void )
{
  FT_Library  library;
  int         failed;

  if ( FT_Init_FreeType( &library ) )
    return 1;
  failed = test_padded_target( library, 0 );
  failed |= test_padded_target( library, 1 );
  FT_Done_FreeType( library );
  return failed;
}
