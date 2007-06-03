/* Copyright (C) 2005 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* modified by Werner Lemberg <wl@gnu.org>       */
/* This file is now part of the FreeType library */


#ifdef STANDALONE

#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#define true   1
#define false  0

  static int  check_outlines = 0;
  static int  nohints        = 0;
  static int  rasterize      = 0;

#endif /* STANDALONE */


  static int
  FT_MoveTo( const FT_Vector  *to,
             void             *user )
  {
    return 0;
  }


  static int
  FT_LineTo( const FT_Vector  *to,
             void             *user )
  {
    return 0;
  }


  static int
  FT_ConicTo( const FT_Vector  *_cp,
              const FT_Vector  *to,
              void             *user )
  {
    return 0;
  }


  static int
  FT_CubicTo( const FT_Vector  *cp1,
              const FT_Vector  *cp2,
              const FT_Vector  *to,
              void             *user )
  {
    return 0;
  }


  static FT_Outline_Funcs outlinefuncs =
  {
    FT_MoveTo,
    FT_LineTo,
    FT_ConicTo,
    FT_CubicTo,
    0, 0          /* No shift, no delta */
  };


  static void
  TestFace( FT_Face  face )
  {
    int  gid;
    int  load_flags = FT_LOAD_DEFAULT;


    if ( check_outlines                               &&
         ( face->face_flags & FT_FACE_FLAG_SCALABLE ) )
      load_flags = FT_LOAD_NO_BITMAP;

    if ( nohints )
      load_flags |= FT_LOAD_NO_HINTING;

    FT_Set_Char_Size( face, 0, (int)( 12 * 64 ), 72, 72 );

    for ( gid = 0; gid < face->num_glyphs; ++gid )
    {
      if ( check_outlines                               &&
           ( face->face_flags & FT_FACE_FLAG_SCALABLE ) )
      {
        if ( !FT_Load_Glyph( face, gid, load_flags ) )
          FT_Outline_Decompose( &face->glyph->outline, &outlinefuncs, NULL );
      }
      else
        FT_Load_Glyph( face, gid, load_flags );

      if ( rasterize )
        FT_Render_Glyph( face->glyph, ft_render_mode_normal );
    }

    FT_Done_Face( face );
  }


#ifdef STANDALONE
  static void
  usage( FILE*  out,
         char*  name )
  {
    fprintf( out, "%s [options] <font> -- Run test program used by ftrandom\n"
                  "  on the given font.\n\n", name );

    fprintf( out, "  --check-outlines  Make sure we can parse the outlines of each glyph.\n" );
    fprintf( out, "  --help            Print this.\n" );
    fprintf( out, "  --nohints         Turn off hinting.\n" );
    fprintf( out, "  --rasterize       Attempt to rasterize each glyph.\n" );
  }
#endif /* STANDALONE */


#ifdef STANDALONE
  int
  main( int     argc,
        char**  argv )
#else 
  static void
  ExecuteTest( char*  testfont )
#endif
  {
    FT_Library  context;
    FT_Face     face;
    int         i, num;
#ifdef STANDALONE
    char*       testfont;
    char*       pt;
#endif


#if STANDALONE
    for ( i = 1; i < argc; ++i )
    {
      pt = argv[i];
      if ( pt[0] == '-' && pt[1] == '-' )
        ++pt;

      if ( strcmp( pt, "-check-outlines" ) == 0 )
        check_outlines = true;
      else if ( strcmp( pt, "-help" ) == 0 )
      {
        usage( stdout, argv[0] );
        exit( 0 );
      }
      else if ( strcmp( pt, "-nohints" ) == 0 )
        nohints = true;
      else if ( strcmp( pt, "-rasterize" ) == 0 )
        rasterize = true;
      else if ( pt[0] == '-' )
      {
        usage( stderr, argv[0] );
        exit( 1 );
      }
      else
        break;
    }

    if ( i == argc )
    {
      usage( stderr, argv[0] );
      exit( 1 );
    }

    testfont = argv[i];
#endif /* STANDALONE */

    if ( FT_Init_FreeType( &context ) )
    {
      fprintf( stderr, "Can't initialize FreeType.\n" );
      exit( 1 );
    }

    if ( FT_New_Face( context, testfont, 0, &face ) )
    {
      /* The font is erroneous, so if this fails that's ok. */
#ifdef STANDALONE
      fprintf( stderr, "Faulty font properly rejected.\n" );
#endif
      exit( 0 );
    }

    if ( face->num_faces == 1 )
      TestFace( face );
    else
    {
      num = face->num_faces;
      FT_Done_Face( face );

      for ( i = 0; i < num; ++i )
      {
        if ( !FT_New_Face( context, testfont, i, &face ) )
          TestFace( face );
      }
    }

#ifdef STANDALONE
    fprintf( stderr, "Font passed.\n" );
#endif

    exit( 0 );
  }


/* EOF */
