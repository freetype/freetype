/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-1998 by                                                  */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*  ftlint: a simple TrueType instruction tester.                           */
/*                                                                          */
/*  NOTE:  This is just a test program that is used to show off and         */
/*         debug the current engine.                                        */
/*                                                                          */
/****************************************************************************/

#include "freetype.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define gettext( x )  ( x )

  FT_Error      error;

  FT_Library    library;
  FT_Face       face;

  unsigned int  num_glyphs;
  int           ptsize;

  int  Fail;
  int  Num;



  static void  Usage( char*  name )
  {
    printf( "fttry: simple TrueType instruction tester -- part of the FreeType project\n" );
    printf( "--------------------------------------------------------------------------\n" );
    printf( "\n" );
    printf( "Usage: %s ppem glyph fontname [fontname2..]\n\n", name );
    printf( "    or %s -u glyph fontname [fontname2..]\n", name );
    printf( "          to load an unscaled glyph\n\n" );

    exit( 1 );
  }


  static void  Panic( const char*  message )
  {
    fprintf( stderr, "%s\n  error code = 0x%04x\n", message, error );
    exit(1);
  }

  int  main( int  argc, char**  argv ) 
  {
    int           i, file_index, glyph_index;
    char          filename[128 + 4];
    char          alt_filename[128 + 4];
    char*         execname;
    char*         fname;
    int           load_unscaled = 0;

    execname = argv[0];

    if ( argc < 3 )
      Usage( execname );

    if ( argv[1][0] == '-' &&
         argv[1][1] == 'u' )
    {
      load_unscaled = 1;
    }
    else
    {
      if ( sscanf( argv[1], "%d", &ptsize ) != 1 )
        Usage( execname );
    }
    argc--;
    argv++;

    if ( sscanf( argv[1], "%d", &glyph_index ) != 1 )
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

      if ( i >= 0 )
      {
        strncpy( filename + strlen( filename ), ".ttf", 4 );
        strncpy( alt_filename + strlen( alt_filename ), ".ttc", 4 );
      }

      i     = strlen( filename );
      fname = filename;

      while ( i >= 0 )
        if ( filename[i] == '/' || filename[i] == '\\' )
        {
          fname = filename + i + 1;
          i = -1;
        }
        else 
          i--;

      printf( "%s: ", fname );

      /* Load face */
      error = FT_New_Face( library, filename, 0, &face );
      if (error) Panic( "Could not create face object" );

      num_glyphs = face->num_glyphs;

      error = FT_Set_Char_Size( face, ptsize << 6, 0, 0, 0 );
      if (error) Panic( "Could not set character size" );
      
      error = FT_Load_Glyph( face,
                             glyph_index,
                             load_unscaled ? FT_LOAD_NO_SCALE
                                           : FT_LOAD_DEFAULT );
      if ( error == 0 )
        printf( "OK.\n" );
      else
        printf( "Fail with error 0x%04x\n", error );

      FT_Done_Face( face );
    }

    FT_Done_FreeType(library);
    exit( 0 );      /* for safety reasons */

    return 0;       /* never reached */
  }


/* End */
