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

#include "freetype.h"
#include "ftoutln.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>    /* for clock() */

#include "graph.h"
#include "ftgrays.h"

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
  FT_Size       size;
  FT_GlyphSlot  glyph;

  FT_Outline    outline;

  FT_Pos*   cur_x;
  FT_Pos*   cur_y;

  unsigned short*  cur_endContour;
  unsigned char*   cur_touch;

  FT_Outline  outlines[MAX_GLYPHS];

  int             num_glyphs;
  int             tab_glyphs;
  int             cur_glyph;
  int             cur_point;
  unsigned short  cur_contour;
  
  int             pixel_size   = CHARSIZE;
  int             repeat_count = 1;
  int             use_grays    = 0;

  FT_Bitmap      Bit;
  grBitmap       bit;

  int  Fail;
  int  Num;

  int  vio_Height, vio_Width;

  short  visual;      /* display glyphs while rendering */
  short  antialias; /* smooth fonts with gray levels  */
  short  force_low;

  
#define RASTER_BUFF_SIZE   128000
  char     raster_buff[ RASTER_BUFF_SIZE ];


  static void Clear_Buffer();

  static void Panic( const char* message )
  {
    fprintf( stderr, "%s\n  error code = 0x%04x\n", message, error );
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
/*  Init_Engine:                                                   */
/*                                                                 */
/*    Allocates bitmap, render pool and other structs...           */
/*                                                                 */
/*******************************************************************/

  void  Init_Engine( void )
  {
    Bit.rows       = bit.rows;
    Bit.width      = bit.width;
    Bit.pitch      = bit.pitch;
    Bit.buffer     = bit.buffer;
    Bit.pixel_mode = antialias ? ft_pixel_mode_grays : ft_pixel_mode_mono;
    Bit.num_grays  = bit.grays;
    Clear_Buffer();
  }


/*******************************************************************/
/*                                                                 */
/*  Clear_Buffer:                                                  */
/*                                                                 */
/*    Clears current bitmap.                                       */
/*                                                                 */
/*******************************************************************/

  static void  Clear_Buffer( void )
  {
    long size = Bit.rows * Bit.pitch;

    memset( Bit.buffer, 0, size );
  }


/*******************************************************************/
/*                                                                 */
/*  LoadTrueTypeChar:                                              */
/*                                                                 */
/*    Loads a glyph into memory.                                   */
/*                                                                 */
/*******************************************************************/

  FT_Error  LoadChar( int  idx )
  {
    error = FT_Load_Glyph( face, idx, FT_LOAD_DEFAULT );
    if ( error )
      return error;

    glyph->outline.flags |= ft_outline_single_pass |
                                    ft_outline_ignore_dropouts;

    if (force_low)
      glyph->outline.flags &= ~ft_outline_high_precision;

    /* debugging */
#if 0
    if ( idx == 0 && !visual )
    {
      printf( "points = %d\n", outline.points );
      for ( j = 0; j < outline.points; j++ )
        printf( "%02x  (%01hx,%01hx)\n",
                 j, outline.xCoord[j], outline.yCoord[j] );
      printf( "\n" );
    }
#endif

    /* create a new outline */
    FT_Outline_New( library,
                    glyph->outline.n_points, 
                    glyph->outline.n_contours, 
                    &outlines[cur_glyph] );

    /* copy the glyph outline into it */
    glyph->outline.flags |= ft_outline_single_pass;
    if (force_low)
      glyph->outline.flags &= ~ft_outline_high_precision;

    FT_Outline_Copy( &glyph->outline, &outlines[cur_glyph] );

    /* center outline around 0 */
    {
      FT_BBox  bbox;
      
      FT_Outline_Get_CBox( &glyph->outline, &bbox );
      FT_Outline_Translate( &outlines[cur_glyph],
                            - ( bbox.xMax - bbox.xMin )/2,
                            - ( bbox.yMax - bbox.yMin )/2 );
    }
    /* translate it */
    FT_Outline_Translate( &outlines[cur_glyph],
                          Bit.width * 32 ,
                          Bit.rows  * 32 );
    cur_glyph++;

    return FT_Err_Ok;
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
    outlines[index].flags |= ~ft_outline_single_pass;
    return FT_Outline_Get_Bitmap( library, &outlines[index], &Bit );
  }


  static void Usage()
  {
      fprintf( stderr, "fttimer: simple performance timer -- part of the FreeType project\n" );
      fprintf( stderr, "-----------------------------------------------------------------\n\n" );
      fprintf( stderr, "Usage: fttimer [options] fontname[.ttf|.ttc]\n\n" );
      fprintf( stderr, "options:\n");
      fprintf( stderr, "   -r : repeat count to be used (default is 1)\n" );
      fprintf( stderr, "   -s : character pixel size (default is 600)\n" );
      fprintf( stderr, "   -v : display results..\n" );
      fprintf( stderr, "   -g : render anti-aliased glyphs\n" );
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
    grSurface*  surface = 0;

    long   t, t0, tz0;


    execname    = argv[0];

    antialias = 0;
    visual      = 0;
    force_low   = 0;

    while ( argc > 1 && argv[1][0] == '-' )
    {
      switch ( argv[1][1] )
      {
      case 'g':
        antialias = 1;
        break;

      case 'a':
        use_grays = 1;
        break;
        
      case 'l':
        force_low = 1;
        break;

      case 'v':
        visual = 1;
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

    /* set-up smooth anti-aliaser */
    if (use_grays)
    {
      error = FT_Set_Raster( library, &ft_grays_raster );
      if (error) Panic( "Could not initialize smooth anti-aliasing renderer" );
    }
    
    /* Load face */

    error = FT_New_Face( library, filename, 0, &face );
    if ( error == FT_Err_Cannot_Open_Stream )
      Panic( "Could not find/open font resource" );
    else if ( error )
      Panic( "Error while opening font resource" );

    /* get face properties and allocate preload arrays */

    num_glyphs = face->num_glyphs;
    glyph      = face->glyph;
        
    tab_glyphs = MAX_GLYPHS;
    if ( tab_glyphs > num_glyphs )
      tab_glyphs = num_glyphs;

    /* create size */

    error = FT_Set_Pixel_Sizes( face, pixel_size, pixel_size );
    if ( error ) Panic( "Could not reset instance" );

    bit.mode  = antialias ? gr_pixel_mode_gray : gr_pixel_mode_mono;
    bit.width = 640;
    bit.rows  = 480;
    bit.grays = 128;
 
    if ( visual )
    {
      if ( !grInitDevices() )
        Panic( "Could not initialize graphics.\n" );
        
      surface = grNewSurface( 0, &bit );
      if (!surface)
        Panic( "Could not open graphics window/screen.\n" );
    }
    else
    {
      if ( grNewBitmap( bit.mode,
                        bit.grays,
                        bit.width,
                        bit.rows,
                       &bit ) )
        Panic( "Could not create rendering buffer.\n" );
    }

    Init_Engine();

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
      cur_point   = 0;
      cur_contour = 0;
  
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

            if ( Num == 0 && visual )
            {
              sprintf( Header, "Glyph: %5d", Num );
              grSetTitle( surface, Header );
              grRefreshSurface( surface );
              Clear_Buffer();
            }
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
        FT_Outline_Done( library, &outlines[Num] );
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
