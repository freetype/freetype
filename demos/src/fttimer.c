/****************************************************************************/
/*                                                                          */
/*  The FreeType project - a Free and Portable Quality TrueType Renderer.   */
/*                                                                          */
/*  Copyright 1996-1998 by                                                  */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*  fttimer: A simple performance benchmark.  Now with graylevel rendering  */
/*           with the '-g' option.                                          */
/*                                                                          */
/*           Be aware that the timer program benchmarks different things    */
/*           in each release of the FreeType library.  Thus, performance    */
/*           should only be compared between similar release numbers.       */
/*                                                                          */
/*                                                                          */
/*  NOTE: This is just a test program that is used to show off and          */
/*        debug the current engine.  In no way does it shows the final      */
/*        high-level interface that client applications will use.           */
/*                                                                          */
/****************************************************************************/

#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>    /* for clock() */

/* SunOS 4.1.* does not define CLOCKS_PER_SEC, so include <sys/param.h> */
/* to get the HZ macro which is the equivalent.                         */
#if defined(__sun__) && !defined(SVR4) && !defined(__SVR4)
#include <sys/param.h>
#define CLOCKS_PER_SEC HZ
#endif

#define CHARSIZE    400   /* character point size */
#define MAX_GLYPHS  512   /* Maximum number of glyphs rendered at one time */

  char  Header[128];

  FT_Error      error;
  FT_Library    library;

  FT_Face       face;

  int             num_glyphs;
  FT_Glyph        glyphs[MAX_GLYPHS];
  
  int             tab_glyphs;
  int             cur_glyph;

  int             pixel_size   = CHARSIZE;
  int             repeat_count = 1;

  int  Fail;
  int  Num;

  short  antialias = 1; /* smooth fonts with gray levels  */
  short  force_low;


  static
  void  Panic( const char*  message )
  {
    fprintf( stderr, "%s\n", message );
    exit(1);
  }

/*******************************************************************/
/*                                                                 */
/*  Get_Time:                                                      */
/*                                                                 */
/*    Returns the current time in milliseconds.                    */
/*                                                                 */
/*******************************************************************/

  long  Get_Time( void )
  {
    return clock() * 10000 / CLOCKS_PER_SEC;
  }



/*******************************************************************/
/*                                                                 */
/*  LoadChar:                                                      */
/*                                                                 */
/*    Loads a glyph into memory.                                   */
/*                                                                 */
/*******************************************************************/

  FT_Error  LoadChar( int  idx )
  {
    FT_Glyph  glyph;
    
    /* loads the glyph in the glyph slot */
    error = FT_Load_Glyph( face, idx, FT_LOAD_DEFAULT ) ||
            FT_Get_Glyph ( face->glyph, &glyph );
    if ( !error )
    {
      glyphs[cur_glyph++] = glyph;
    }
    return error;
  }



/*******************************************************************/
/*                                                                 */
/*  ConvertRaster:                                                 */
/*                                                                 */
/*    Performs scan conversion.                                    */
/*                                                                 */
/*******************************************************************/

  FT_Error  ConvertRaster( int  index )
  {
    FT_Glyph  bitmap;
    FT_Error  error;
    
    bitmap = glyphs[index];
    error = FT_Glyph_To_Bitmap( &bitmap,
                                antialias ? ft_render_mode_normal
                                          : ft_render_mode_mono,
                                0,
                                0 );
    if (!error)
      FT_Done_Glyph( bitmap );
      
    return error;
  }


  static void Usage()
  {
      fprintf( stderr, "fttimer: simple performance timer -- part of the FreeType project\n" );
      fprintf( stderr, "-----------------------------------------------------------------\n\n" );
      fprintf( stderr, "Usage: fttimer [options] fontname[.ttf|.ttc]\n\n" );
      fprintf( stderr, "options:\n");
      fprintf( stderr, "   -r : repeat count to be used (default is 1)\n" );
      fprintf( stderr, "   -s : character pixel size (default is 600)\n" );
      fprintf( stderr, "   -m : render monochrome glyphs (default is anti-aliased)\n" );
      fprintf( stderr, "   -a : use smooth anti-aliaser\n" );
      fprintf( stderr, "   -l : force low quality even at small sizes\n" );
      exit(1);
  }


  int  main( int  argc, char**  argv )
  {
    int    i, total, base, rendered_glyphs;
    char   filename[128 + 4];
    char   alt_filename[128 + 4];
    char*  execname;

    long   t, t0, tz0;


    execname    = argv[0];

    antialias = 1;
    force_low = 0;

    while ( argc > 1 && argv[1][0] == '-' )
    {
      switch ( argv[1][1] )
      {
      case 'm':
        antialias = 0;
        break;

      case 'l':
        force_low = 1;
        break;

      case 's':
        argc--;
        argv++;
        if ( argc < 2 ||
            sscanf( argv[1], "%d", &pixel_size ) != 1 )
          Usage();
        break;

      case 'r':
        argc--;
        argv++;
        if ( argc < 2 ||
             sscanf( argv[1], "%d", &repeat_count ) != 1 )
          Usage();
        if (repeat_count < 1)
          repeat_count = 1;
        break;

      default:
        fprintf( stderr, "Unknown argument '%s'!\n", argv[1] );
		Usage();
      }
      argc--;
      argv++;
    }

    if ( argc != 2 )
	  Usage();

    i = strlen( argv[1] );
    while ( i > 0 && argv[1][i] != '\\' )
    {
      if ( argv[1][i] == '.' )
        i = 0;
      i--;
    }

    filename[128] = '\0';
    alt_filename[128] = '\0';

    strncpy( filename, argv[1], 128 );
    strncpy( alt_filename, argv[1], 128 );

    if ( i >= 0 )
    {
      strncpy( filename + strlen( filename ), ".ttf", 4 );
      strncpy( alt_filename + strlen( alt_filename ), ".ttc", 4 );
    }

    /* Initialize engine */

    if ( (error = FT_Init_FreeType( &library )) )
      Panic( "Error while initializing engine" );

    /* Load face */

    error = FT_New_Face( library, filename, 0, &face );
    if ( error == FT_Err_Cannot_Open_Stream )
      Panic( "Could not find/open font resource" );
    else if ( error )
      Panic( "Error while opening font resource" );

    /* get face properties and allocate preload arrays */

    num_glyphs = face->num_glyphs;

    tab_glyphs = MAX_GLYPHS;
    if ( tab_glyphs > num_glyphs )
      tab_glyphs = num_glyphs;

    /* create size */

    error = FT_Set_Pixel_Sizes( face, pixel_size, pixel_size );
    if ( error ) Panic( "Could not reset instance" );

    Num  = 0;
    Fail = 0;

    total = num_glyphs;
    base  = 0;

    rendered_glyphs = 0;

    t0 = 0;  /* Initial time */

    tz0 = Get_Time();

    while ( total > 0 )
    {
      int  repeat;

      /* First, preload 'tab_glyphs' in memory */
      cur_glyph   = 0;

      printf( "loading %d glyphs", tab_glyphs );

      for ( Num = 0; Num < tab_glyphs; Num++ )
      {
        error = LoadChar( base + Num );
        if ( error )
          Fail++;

        total--;
      }

      base += tab_glyphs;

      if ( tab_glyphs > total )
        tab_glyphs = total;

      printf( ", rendering... " );

      /* Now, render the loaded glyphs */

      t = Get_Time();

      for ( repeat = 0; repeat < repeat_count; repeat++ )
      {
        for ( Num = 0; Num < cur_glyph; Num++ )
        {
          if ( (error = ConvertRaster( Num )) )
            Fail++;

          else
  	  {
            rendered_glyphs ++;
	  }
        }
      }

      t = Get_Time() - t;
      if ( t < 0 )
        t += 1000 * 60 * 60;

      printf( " = %f s\n", (double)t / 10000 );
      t0 += t;

      /* Now free all loaded outlines */
      for ( Num = 0; Num < cur_glyph; Num++ )
        FT_Done_Glyph( glyphs[Num] );
    }

    tz0 = Get_Time() - tz0;

    FT_Done_Face( face );

    printf( "\n" );
    printf( "rendered glyphs  = %d\n", rendered_glyphs );
    printf( "render time      = %f s\n", (double)t0 / 10000 );
    printf( "fails            = %d\n", Fail );
    printf( "average glyphs/s = %f\n",
             (double)rendered_glyphs / t0 * 10000 );

    printf( "total timing     = %f s\n", (double)tz0 / 10000 );
    printf( "Fails = %d\n", Fail );

    FT_Done_FreeType( library );

    exit( 0 );      /* for safety reasons */

    return 0;       /* never reached */
  }


/* End */
