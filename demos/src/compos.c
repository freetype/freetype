/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-1998 by                                                  */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*  compos: this is a very simple program used to test the flag             */
/*          FT_LOAD_NO_RECURSE                                              */
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
    printf( "compos: test FT_LOAD_NO_RECURSE load flag - www.freetype.org\n" );
    printf( "------------------------------------------------------------\n" );
    printf( "\n" );
    printf( "Usage: %s fontname[.ttf|.ttc] [fontname2..]\n", name );
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

    if ( argc < 2 )
      Usage( execname );

    error = FT_Init_FreeType( &library );
    if (error) Panic( "Could not create library object" );

    /* Now check all files */
    for ( file_index = 1; file_index < argc; file_index++ )
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

      printf( "%s:\n", fname );

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

      num_glyphs = face->num_glyphs;
      slot       = face->glyph;

      Fail = 0;
      {
        for ( id = 0; id < num_glyphs; id++ )
        {
          int  has_scale;
          
          error = FT_Load_Glyph( face, id, FT_LOAD_NO_RECURSE );
          if (!error && slot->format == ft_glyph_format_composite)
          {
            int           n;
            FT_SubGlyph*  subg = slot->subglyphs;

            printf( "%4d:", id );
            for ( n = 0; n < slot->num_subglyphs; n++, subg++ )
            {
              has_scale = subg->flags & (
                            FT_SUBGLYPH_FLAG_SCALE |
                            FT_SUBGLYPH_FLAG_XY_SCALE |
                            FT_SUBGLYPH_FLAG_2X2 );
              
              printf( " [%d%c",
                      subg->index,
                      subg->flags & FT_SUBGLYPH_FLAG_USE_MY_METRICS ? '*' : ' ' );
                      
              if ( subg->arg1|subg->arg2 )
              {
                if ( subg->flags & FT_SUBGLYPH_FLAG_ARGS_ARE_XY_VALUES ) 
                  printf( "(%d,%d)", subg->arg1, subg->arg2 );
                else
                  printf( "<%d,%d>", subg->arg1, subg->arg2 );
              }
                
              if (has_scale)
                printf( "-{%0.3f %0.3f %0.3f %0.3f}",
                        subg->transform.xx/65536.0,
                        subg->transform.xy/65536.0,
                        subg->transform.yx/65536.0,
                        subg->transform.yy/65536.0 );
              printf( "]" );
            }
            printf( " adv=%ld lsb=%ld\n",
                    slot->metrics.horiAdvance,
                    slot->metrics.horiBearingX );
          }
        }
      }

      FT_Done_Face( face );
    }

    FT_Done_FreeType(library);
    exit( 0 );      /* for safety reasons */

    return 0;       /* never reached */
  }


/* End */
