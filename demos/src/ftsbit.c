/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-1998 by                                                  */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*  ftsbit: a _very_ simple embedded bitmap dumper for FreeType 1.x.        */
/*                                                                          */
/*  NOTE:  This is just a test program that is used to show off and         */
/*         debug the current engine.                                        */
/*                                                                          */
/****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freetype.h"


#ifdef HAVE_LIBINTL_H

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <libintl.h>
#include "ftxerr18.h"

#else /* !HAVE_LIBINTL */

#define gettext( x )  ( x )

  /* We ignore error message strings with this function */

  static char*  TT_ErrToString18( FT_Error  error )
  {
    static char  temp[32];

    sprintf( temp, "0x%04lx", error );
    return temp;
  }

#endif /* !HAVE_LIBINTL */


  FT_Error      error;
  FT_Library    engine;
  FT_Resource   resource;

  FT_Face       face;
  FT_Size       instance;
  FT_GlyphSlot  glyph;

  unsigned int  num_glyphs;
  int           ptsize;

  int  Fail;
  int  Num;



  static void  Usage( char*  name )
  {
    printf( gettext( "ftsbit: simple TrueType 'sbit' dumper -- part of the FreeType project\n" ) );
    printf(          "---------------------------------------------------------------------\n" );
    printf(          "\n" );
    printf( gettext( "Usage: %s ppem fontname (index)* (index1-index2)*\n\n" ), name );
    printf( gettext( "   or  %s -a ppem fontname   (dumps all glyphs)\n" ), name );
    printf(          "\n" );

    exit( EXIT_FAILURE );
  }



  static
  void  dump_bitmap( FT_GlyphSlot  glyph, int glyph_index )
  {
    /* Dump the resulting bitmap */
    {
      int             y;
      unsigned char*  line = (unsigned char*)glyph->bitmap.buffer;

      printf( "glyph index %d = %dx%d pixels, ",
              glyph_index, glyph->bitmap.rows, glyph->bitmap.width );

      printf( "advance = %d, minBearing = [%d,%d]\n",
              glyph->metrics.horiAdvance >> 6,
              glyph->metrics.horiBearingX >> 6,
              glyph->metrics.horiBearingY >> 6 );

      for ( y = 0; y < glyph->bitmap.rows; y++, line += glyph->bitmap.cols )
      {
        unsigned char*  ptr = line;
        int             x;
        unsigned char   mask = 0x80;

        for ( x = 0; x < glyph->bitmap.width; x++ )
        {
          printf( "%c", (ptr[0] & mask) ? '*' : '.' );
          mask >>= 1;
          if (mask == 0)
          {
            mask = 0x80;
            ptr++;
          }
        }

        printf( "\n" );
      }
    }
  }


  
  static
  void  dump_range( FT_GlyphSlot  glyph,
                    int           first_glyph,
                    int           last_glyph )
  {
    int  i;
    
    for ( i = first_glyph; i <= last_glyph; i++ )
    {
      error = FT_Load_Glyph( glyph,
                             instance,
                             (unsigned short)i,
                             FT_LOAD_NO_OUTLINE,
                             0 );
      if (error)
      {
        printf( "  no bitmap for glyph %d\n", i );
        printf( gettext( "FreeType error message: %s\n" ),
                TT_ErrToString18( error ) );
        continue;
      }
      
      dump_bitmap(glyph,i);
    }
  }




  int  main( int  argc, char**  argv )
  {
    int    i;
    char   filename[128 + 4];
    char   alt_filename[128 + 4];
    char*  execname;
    char*  fname;
    int    dump_all = 0;


#ifdef HAVE_LIBINTL_H
    setlocale( LC_ALL, "" );
    bindtextdomain( "freetype", LOCALEDIR );
    textdomain( "freetype" );
#endif

    execname = argv[0];

    if ( argc < 3 )
      Usage( execname );

    if ( argv[1][0] == '-' &&
         argv[1][1] == 'a' )
    {
      argv++;
      argc--;
      dump_all = 1;
    }

    if ( sscanf( argv[1], "%d", &ptsize ) != 1 )
      Usage( execname );

    /* Initialize engine */
    if ( (error = FT_Init_FreeType( &engine )) )
    {
      fprintf( stderr, gettext( "Error while initializing engine\n" ) );
      goto Failure;
    }

    /* Now check all files */
    fname = argv[2];
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

    /* Load face */
    error = FT_New_Resource( engine, filename, &resource );
    if (error)
    {
      strcpy( filename, alt_filename );
      error = FT_New_Resource( engine, alt_filename, &resource );
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

    if ( error )
    {
      printf( gettext( "Could not find or open file.\n" ) );
      goto Failure;
    }

    error = FT_New_Face( resource, 0, &face );
    if (error)
    {
      printf( gettext( "Could not create face object.\n  " ) );
      goto Failure;
    }

    /* get face properties */
    num_glyphs = face->num_glyphs;

    /* create instance */
    error = FT_New_Size( face, &instance );
    if ( error )
    {
      printf( gettext( "Could not create instance.\n" ) );
      goto Failure;
    }

    error = FT_Set_Pixel_Sizes( instance, ptsize, ptsize );
    if (error)
    {
      printf( gettext( "Could not set character size.\n" ) );
      goto Failure;
    }

    glyph = face->slot;

    if (dump_all)
      dump_range( glyph, 0, num_glyphs-1 );
    else
    {
      for ( i = 3; i < argc; i++ )
      {
        /* check for range in argument string */
        int    range_check = 0;
        char*  base = argv[i];
        char*  cur  = base;
        int    first, last;
        
        while (*cur)
        {
          if (*cur == '-')
          {
            range_check = 1;
            break;
          }
          cur++;
        }

        if (range_check)
        {
          if ( sscanf( argv[i], "%d-%d", &first, &last ) != 2 )
            Usage( execname );
            
          dump_range( glyph, first, last );
        }
        else
        {
          if ( sscanf( argv[i], "%d", &first ) != 1 )
            Usage( execname );
            
          dump_range( glyph, first, first );
        }
      }
    }

    FT_Done_FreeType( engine );
    exit( EXIT_SUCCESS );      /* for safety reasons */

    return 0;       /* never reached */

  Failure:
    printf( gettext( "FreeType error message: %s\n" ),
            TT_ErrToString18( error ) );
    exit( EXIT_FAILURE );
  }


/* End */
