/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-1998 by                                                  */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*  ftlint: a simple font tester. This program tries to load all the        */
/*          glyphs of a given font.                                         */
/*                                                                          */
/*  NOTE:  This is just a test program that is used to show off and         */
/*         debug the current engine.                                        */
/*                                                                          */
/****************************************************************************/

#include "freetype.h"
#include "ftobjs.h"
#include "ftdriver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define gettext( x )  ( x )

  FT_Error      error;

  FT_Library    library;
  FT_Face       face;
  FT_Size       size;
  FT_GlyphSlot  slot;

  unsigned int  num_glyphs;
  int           ptsize;

  int  Fail;
  int  Num;



  static void  Usage( char*  name )
  {
    printf( "ftlint: simple font tester -- part of the FreeType project\n" );
    printf( "----------------------------------------------------------\n" );
    printf( "\n" );
    printf( "Usage: %s ppem fontname[.ttf|.ttc] [fontname2..]\n", name );
    printf( "\n" );

    exit( 1 );
  }


  static void  Panic( const char*  message )
  {
    fprintf( stderr, "%s\n  error code = 0x%04x\n", message, error );
    exit(1);
  }


  int  main( int  argc, char**  argv ) 
  {
    int           i, file_index;
    unsigned int  id;
    char          filename[128 + 4];
    char          alt_filename[128 + 4];
    char*         execname;
    char*         fname;


    execname = argv[0];

    if ( argc < 3 )
      Usage( execname );

    if ( sscanf( argv[1], "%d", &ptsize ) != 1 )
      Usage( execname );

    error = FT_Init_FreeType( &library );
    if (error) Panic( "Could not create library object" );

    /* Now check all files */
    for ( file_index = 2; file_index < argc; file_index++ )
    {
      fname = argv[file_index];
      i     = strlen( fname );
      while ( i > 0 && fname[i] != '\\' && fname[i] != '/' )
      {
        if ( fname[i] == '.' )
          i = 0;
        i--;
      }

      filename[128] = '\0';
      alt_filename[128] = '\0';

      strncpy( filename, fname, 128 );
      strncpy( alt_filename, fname, 128 );

#ifndef macintosh
      if ( i >= 0 )
      {
        strncpy( filename + strlen( filename ), ".ttf", 4 );
        strncpy( alt_filename + strlen( alt_filename ), ".ttc", 4 );
      }
#endif
      i     = strlen( filename );
      fname = filename;

      while ( i >= 0 )
#ifndef macintosh
        if ( filename[i] == '/' || filename[i] == '\\' )
#else
        if ( filename[i] == ':' )
#endif
        {
          fname = filename + i + 1;
          i = -1;
        }
        else 
          i--;

      printf( "%s: ", fname );

      /* Load face */
      error = FT_New_Face( library, filename, 0, &face );
      if (error)
      {
        if (error == FT_Err_Invalid_File_Format)
          printf( "unknown format\n" );
        else
          printf( "could not find/open file (error: %d)\n", error );
        continue;
      }
      if (error) Panic( "Could not open file" );

      num_glyphs = face->num_glyphs;

      error = FT_Set_Char_Size( face, ptsize << 6, ptsize << 6, 72, 72 );
      if (error) Panic( "Could not set character size" );

      Fail = 0;
      {
        for ( id = 0; id < num_glyphs; id++ )
        {
          error = FT_Load_Glyph( face, id, FT_LOAD_DEFAULT );
          if (error)
          {
            if ( Fail < 10 )
              printf( "glyph %4u: 0x%04x\n" , id, error );
            Fail++;
          }
        }
      }

      if ( Fail == 0 )
        printf( "OK.\n" );
      else
        if ( Fail == 1 )
          printf( "1 fail.\n" );
        else
          printf( "%d fails.\n", Fail );

      FT_Done_Face( face );
    }

    FT_Done_FreeType(library);
    exit( 0 );      /* for safety reasons */

    return 0;       /* never reached */
  }


/* End */
