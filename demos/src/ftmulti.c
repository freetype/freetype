/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-2000 by                                                  */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*                                                                          */
/*  FTMulti- a simple multiple masters font viewer                          */
/*                                                                          */
/*  Press F1 when running this program to have a list of key-bindings       */
/*                                                                          */
/****************************************************************************/

#include <freetype/freetype.h>
#include <freetype/ftmm.h>

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "graph.h"
#include "grfont.h"

#define  DIM_X   500
#define  DIM_Y   400

#define  CENTER_X   ( bit.width / 2 )
#define  CENTER_Y   ( bit.rows / 2 )

#define  MAXPTSIZE  500                 /* dtp */

  char  Header[128];
  char* new_header = 0;

  const unsigned char*  Text = (unsigned char*)
    "The quick brown fox jumped over the lazy dog 0123456789 "
    "\342\352\356\373\364\344\353\357\366\374\377\340\371\351\350\347 "
    "&#~\"\'(-`_^@)=+\260 ABCDEFGHIJKLMNOPQRSTUVWXYZ "
    "$\243^\250*\265\371%!\247:/;.,?<>";

  FT_Library    library;      /* the FreeType library        */
  FT_Face       face;         /* the font face               */
  FT_Size       size;         /* the font size               */
  FT_GlyphSlot  glyph;        /* the glyph slot              */

  FT_Error      error;        /* error returned by FreeType? */

  grSurface*    surface;      /* current display surface     */
  grBitmap      bit;          /* current display bitmap      */

  int  num_glyphs;            /* number of glyphs */
  int  ptsize;                /* current point size */

  int  hinted    = 1;         /* is glyph hinting active?    */
  int  antialias = 1;         /* is anti-aliasing active?    */
  int  use_sbits = 1;         /* do we use embedded bitmaps? */
  int  low_prec  = 0;         /* force low precision         */
  int  Num;                   /* current first glyph index   */

  int  res       = 72;

  static grColor  fore_color = { 255 };

  int            Fail;
  unsigned char  autorun;

  int  graph_init  = 0;

  int  render_mode = 1;
  int  use_grays   = 1;

  FT_Multi_Master  multimaster;
  FT_Long          design_pos[T1_MAX_MM_AXIS];

#define RASTER_BUFF_SIZE  32768
  char             raster_buff[RASTER_BUFF_SIZE];

#define DEBUGxxx

#ifdef DEBUG
#define LOG( x )  LogMessage##x
#else
#define LOG( x )  /* empty */
#endif

#ifdef DEBUG
  static
  void  LogMessage( const char*  fmt, ... )
  {
    va_list  ap;


    va_start( ap, fmt );
    vfprintf( stderr, fmt, ap );
    va_end( ap );
  }
#endif


  /* PanicZ */
  static
  void PanicZ( const char*  message )
  {
    fprintf( stderr, "%s\n  error = 0x%04x\n", message, error );
    exit( 1 );
  }


  /* Clears the Bit bitmap/pixmap */
  static
  void  Clear_Display( void )
  {
    long  size = (long)bit.pitch * bit.rows;


    if ( size < 0 )
      size = -size;
    memset( bit.buffer, 0, size );
  }


  /* Initialize the display bitmap named `bit' */
  static
  void  Init_Display( void )
  {
    grInitDevices();

    bit.mode  = gr_pixel_mode_gray;
    bit.width = DIM_X;
    bit.rows  = DIM_Y;
    bit.grays = 256;

    surface = grNewSurface( 0, &bit );
    if ( !surface )
      PanicZ( "could not allocate display surface\n" );

    graph_init = 1;
  }


#define MAX_BUFFER  300000

#define FLOOR( x )  ( (x) & -64 )
#define CEIL( x )   ( ( (x) + 63 ) & -64 )
#define TRUNC( x )  ( (x) >> 6 )

  static
  char  bit_buffer[MAX_BUFFER];


  /* Render a single glyph with the `grays' component */
  static
  FT_Error  Render_Glyph( int  x_offset,
                          int  y_offset )
  {
    grBitmap  bit3;
    FT_Pos    x_top, y_top;
    
    /* first, render the glyph image into a bitmap */
    if (glyph->format != ft_glyph_format_bitmap)
    {
      error = FT_Render_Glyph( glyph, antialias ? ft_render_mode_normal : ft_render_mode_mono );
      if (error) return error;                               
                               
    }
    
    /* now blit it to our display screen */
    bit3.rows   = glyph->bitmap.rows;
    bit3.width  = glyph->bitmap.width;
    bit3.pitch  = glyph->bitmap.pitch;
    bit3.buffer = glyph->bitmap.buffer;

    switch (glyph->bitmap.pixel_mode)
    {
      case ft_pixel_mode_mono:
         bit3.mode   = gr_pixel_mode_mono;
         bit3.grays  = 0;
         break;
         
      case ft_pixel_mode_grays:
         bit3.mode   = gr_pixel_mode_gray;
         bit3.grays  = glyph->bitmap.num_grays;
    }

    /* Then, blit the image to the target surface */
    x_top = x_offset + glyph->bitmap_left;
    y_top = y_offset - glyph->bitmap_top;

    grBlitGlyphToBitmap( &bit, &bit3, x_top, y_top, fore_color );

    return 0;
  }



  static
  FT_Error  Reset_Scale( int  pointSize )
  {
    FT_Error  error;


    error = FT_Set_Char_Size( face, pointSize << 6,
                                    pointSize << 6,
                                    res,
                                    res );
    return FT_Err_Ok;
  }


  static
  FT_Error  LoadChar( int  idx,
                      int  hint )
  {
    int  flags;


    flags = FT_LOAD_DEFAULT;

    if ( !hint )
      flags |= FT_LOAD_NO_HINTING;

    if ( !use_sbits )
      flags |= FT_LOAD_NO_BITMAP;

    return FT_Load_Glyph( face, idx, flags );
  }


  static
  FT_Error  Render_All( int  first_glyph,
                        int  ptsize )
  {
    FT_F26Dot6  start_x, start_y, step_x, step_y, x, y;
    int         i;

    FT_Error    error;


    start_x = 4;
    start_y = 36 + ptsize;

    step_x = size->metrics.x_ppem + 4;
    step_y = size->metrics.y_ppem + 10;

    x = start_x;
    y = start_y;

    i = first_glyph;

#if 0
     while ( i < first_glyph + 1 )
#else
     while ( i < num_glyphs )
#endif
    {
      if ( !( error = LoadChar( i, hinted ) ) )
      {
#ifdef DEBUG
        if ( i <= first_glyph + 6 )
        {
          LOG(( "metrics[%02d] = [%x %x]\n",
                i,
                glyph->metrics.horiBearingX,
                glyph->metrics.horiAdvance ));

          if ( i == first_glyph + 6 )
            LOG(( "-------------------------\n" ));
        }
#endif

        Render_Glyph( x, y );

        x += ( glyph->metrics.horiAdvance >> 6 ) + 1;

        if ( x + size->metrics.x_ppem > bit.width )
        {
          x  = start_x;
          y += step_y;

          if ( y >= bit.rows )
            return FT_Err_Ok;
        }
      }
      else
        Fail++;

      i++;
    }

    return FT_Err_Ok;
  }


  static
  FT_Error  Render_Text( int  first_glyph,
                         int  ptsize )
  {
    FT_F26Dot6  start_x, start_y, step_x, step_y, x, y;
    int         i;

    FT_Error             error;
    const unsigned char* p;


    ptsize=ptsize;

    start_x = 4;
    start_y = 32 + size->metrics.y_ppem;

    step_x = size->metrics.x_ppem + 4;
    step_y = size->metrics.y_ppem + 10;

    x = start_x;
    y = start_y;

    i = first_glyph;
    p = Text;
    while ( i > 0 && *p )
    {
      p++;
      i--;
    }

    while ( *p )
    {
      if ( !( error = LoadChar( FT_Get_Char_Index( face,
                                                   (unsigned char)*p ),
                                hinted ) ) )
      {
#ifdef DEBUG
        if ( i <= first_glyph + 6 )
        {
          LOG(( "metrics[%02d] = [%x %x]\n",
                i,
                glyph->metrics.horiBearingX,
                glyph->metrics.horiAdvance ));

          if ( i == first_glyph + 6 )
          LOG(( "-------------------------\n" ));
        }
#endif

        Render_Glyph( x, y );

        x += ( glyph->metrics.horiAdvance >> 6 ) + 1;

        if ( x + size->metrics.x_ppem > bit.width )
        {
          x  = start_x;
          y += step_y;

          if ( y >= bit.rows )
            return FT_Err_Ok;
        }
      }
      else
        Fail++;

      i++;
      p++;
    }

    return FT_Err_Ok;
  }


  static
  void Help( void )
  {
    grEvent  dummy_event;


    Clear_Display();

    grGotoxy( 0, 0 );
    grSetMargin( 2, 1 );
    grGotobitmap( &bit );

    grWriteln("FreeType Glyph Viewer - part of the FreeType test suite" );
    grLn();
    grWriteln("This program is used to display all glyphs from one or" );
    grWriteln("several font files, with the FreeType library.");
    grLn();
    grWriteln("Use the following keys:");
    grLn();
    grWriteln("  F1 or ?   : display this help screen" );
    grWriteln("  a         : toggle anti-aliasing" );
    grWriteln("  h         : toggle outline hinting" );
    grWriteln("  b         : toggle embedded bitmaps" );
    grWriteln("  l         : toggle low precision rendering" );
    grWriteln("  space     : toggle rendering mode" );
    grLn();
    grWriteln("  Up        : increase pointsize by 1 unit" );
    grWriteln("  Down      : decrease pointsize by 1 unit" );
    grWriteln("  Page Up   : increase pointsize by 10 units" );
    grWriteln("  Page Down : decrease pointsize by 10 units" );
    grLn();
    grWriteln("  Right     : increment first glyph index" );
    grWriteln("  Left      : decrement first glyph index" );
    grLn();
    grWriteln("  F3        : decrement first axis position by 20" );
    grWriteln("  F4        : increment first axis position by 20" );
    grWriteln("  F5        : decrement second axis position by 20" );
    grWriteln("  F6        : increment second axis position by 20" );
    grWriteln("  F7        : decrement third axis position by 20" );
    grWriteln("  F8        : increment third axis position by 20" );
    grLn();
    grWriteln("press any key to exit this help screen");

    grRefreshSurface( surface );
    grListenSurface( surface, gr_event_key, &dummy_event );
  }



  static
  int  Process_Event( grEvent*  event )
  {
    int  i, axis;

    switch ( event->key )
    {
    case grKeyEsc:            /* ESC or q */
    case grKEY( 'q' ):
      return 0;

    case grKeyF1:
    case grKEY( '?' ):
      Help();
      return 1;

    /* mode keys */

    case grKEY( 'a' ):
      antialias = !antialias;
      new_header = antialias ? "anti-aliasing is now on"
                             : "anti-aliasing is now off";
      return 1;

    case grKEY( 'b' ):
      use_sbits  = !use_sbits;
      new_header = use_sbits ? "embedded bitmaps are now used if available"
                             : "embedded bitmaps are now ignored";
      return 1;

    case grKEY( 'n' ):
    case grKEY( 'p' ):
      return (int)event->key;

    case grKEY( 'l' ):
      low_prec = !low_prec;
      new_header = low_prec ? "rendering precision is now forced to low"
                            : "rendering precision is now normal";
      break;

    case grKEY( 'h' ):
      hinted = !hinted;
      new_header = hinted ? "glyph hinting is now active"
                          : "glyph hinting is now ignored";
      break;

    case grKEY( ' ' ):
      render_mode ^= 1;
      new_header = render_mode ? "rendering all glyphs in font"
                               : "rendering test text string";
      break;

    /* MM related keys */

    case grKeyF3:
      i = -20;
      axis = 0;
      goto Do_Axis;

    case grKeyF4:
      i = 20;
      axis = 0;
      goto Do_Axis;

    case grKeyF5:
      i = -20;
      axis = 1;
      goto Do_Axis;

    case grKeyF6:
      i = 20;
      axis = 1;
      goto Do_Axis;

    case grKeyF7:
      i = -20;
      axis = 2;
      goto Do_Axis;

    case grKeyF8:
      i = 20;
      axis = 2;
      goto Do_Axis;

    /* scaling related keys */

    case grKeyPageUp:
      i = 10;
      goto Do_Scale;

    case grKeyPageDown:
      i = -10;
      goto Do_Scale;

    case grKeyUp:
      i = 1;
      goto Do_Scale;

    case grKeyDown:
      i = -1;
      goto Do_Scale;

    /* glyph index related keys */

    case grKeyLeft:
      i = -1;
      goto Do_Glyph;

    case grKeyRight:
      i = 1;
      goto Do_Glyph;

    case grKeyF9:
      i = -100;
      goto Do_Glyph;

    case grKeyF10:
      i = 100;
      goto Do_Glyph;

    case grKeyF11:
      i = -1000;
      goto Do_Glyph;

    case grKeyF12:
      i = 1000;
      goto Do_Glyph;

    default:
      ;
    }
    return 1;

  Do_Axis:
    if ( axis < (int)multimaster.num_axis )
    {
      FT_MM_Axis*  a   = multimaster.axis + axis;
      FT_Long      pos = design_pos[axis];

      
      pos += i;
      if ( pos < a->minimum ) pos = a->minimum;
      if ( pos > a->maximum ) pos = a->maximum;
      
      design_pos[axis] = pos;
      
      FT_Set_MM_Design_Coordinates( face, multimaster.num_axis, design_pos );
    }
    return 1;

  Do_Scale:
    ptsize += i;
    if ( ptsize < 1 )         ptsize = 1;
    if ( ptsize > MAXPTSIZE ) ptsize = MAXPTSIZE;
    return 1;

  Do_Glyph:
    Num += i;
    if ( Num < 0 )           Num = 0;
    if ( Num >= num_glyphs ) Num = num_glyphs - 1;
    return 1;
  }


  static
  void  usage( char*  execname )
  {
    fprintf( stderr,  "\n" );
    fprintf( stderr,  "ftmulti: multiple masters font viewer - part of FreeType\n" );
    fprintf( stderr,  "--------------------------------------------------------\n" );
    fprintf( stderr,  "\n" );
    fprintf( stderr,  "Usage: %s [options below] ppem fontname[.ttf|.ttc] ...\n",
             execname );
    fprintf( stderr,  "\n" );
    fprintf( stderr,  "  -r R      use resolution R dpi (default: 72 dpi)\n" );
    fprintf( stderr,  "  -f index  specify first glyph index to display\n" );
    fprintf( stderr,  "\n" );

    exit( 1 );
  }


  int
  main( int    argc,
        char*  argv[] )
  {
    int    old_ptsize, orig_ptsize, file;
    int    first_glyph = 0;
    int    XisSetup = 0;
    char*  execname;
    int    option;
    int    file_loaded;

    FT_Error  error;
    grEvent   event;

    execname = ft_basename( argv[0] );

    while ( 1 )
    {
      option = getopt( argc, argv, "f:r:" );

      if ( option == -1 )
        break;

      switch ( option )
      {
      case 'f':
        first_glyph = atoi( optarg );
        break;

      case 'r':
        res = atoi( optarg );
        if ( res < 1 )
          usage( execname );
        break;

      default:
        usage( execname );
        break;
      }
    }

    argc -= optind;
    argv += optind;

    if ( argc <= 1 )
      usage( execname );

    if ( sscanf( argv[0], "%d", &orig_ptsize ) != 1 )
      orig_ptsize = 64;

    file = 1;

    /* Initialize engine */
    error = FT_Init_FreeType( &library );
    if ( error )
      PanicZ( "Could not initialize FreeType library" );

  NewFile:
    ptsize      = orig_ptsize;
    hinted      = 1;
    file_loaded = 0;

    /* Load face */
    error = FT_New_Face( library, argv[file], 0, &face );
    if ( error )
      goto Display_Font;

    /* retrieve multiple master information */
    error = FT_Get_Multi_Master( face, &multimaster ); 
    if ( error )
      goto Display_Font;

    /* set the current position to the median of each axis */
    {
      int  n;

      
      for ( n = 0; n < (int)multimaster.num_axis; n++ )
        design_pos[n] =
          ( multimaster.axis[n].minimum + multimaster.axis[n].maximum ) / 2;
    }
    
    error = FT_Set_MM_Design_Coordinates( face,
                                          multimaster.num_axis,
                                          design_pos );
    if ( error )
      goto Display_Font;
    
    file_loaded++;

    error = Reset_Scale( ptsize );
    if ( error )
      goto Display_Font;

    num_glyphs = face->num_glyphs;
    glyph      = face->glyph;
    size       = face->size;

  Display_Font:
    /* initialize graphics if needed */
    if ( !XisSetup )
    {
      XisSetup = 1;
      Init_Display();
    }

    grSetTitle( surface, "FreeType Glyph Viewer - press F1 for help" );
    old_ptsize = ptsize;

    if ( file_loaded >= 1 )
    {
      Fail = 0;
      Num  = first_glyph;

      if ( Num >= num_glyphs )
        Num = num_glyphs - 1;

      if ( Num < 0 )
        Num = 0;
    }

    for ( ;; )
    {
      int  key;


      Clear_Display();

      if ( file_loaded >= 1 )
      {
        switch ( render_mode )
        {
        case 0:
          Render_Text( Num, ptsize );
          break;

        default:
          Render_All( Num, ptsize );
        }

        sprintf( Header, "%s %s (file %s)",
                         face->family_name,
                         face->style_name,
                         ft_basename( argv[file] ) );

        if ( !new_header )
          new_header = Header;

        grWriteCellString( &bit, 0, 0, new_header, fore_color );
        new_header = 0;

        sprintf( Header, "axis: " );
        {
          int  n;


          for ( n = 0; n < (int)multimaster.num_axis; n++ )
          {
            char  temp[32];


            sprintf( temp, "  %s:%ld",
                           multimaster.axis[n].name,
                           design_pos[n] );
            strcat( Header, temp );
          }
        }
        grWriteCellString( &bit, 0, 16, Header, fore_color );
          
        sprintf( Header, "at %d points, first glyph = %d",
                         ptsize,
                         Num );
      }
      else
      {
        sprintf( Header, "%s : not an MM font file, or could not be opened",
                         ft_basename( argv[file] ) );
      }

      grWriteCellString( &bit, 0, 8, Header, fore_color );
      grRefreshSurface( surface );

      grListenSurface( surface, 0, &event );
      if ( !( key = Process_Event( &event ) ) )
        goto End;

      if ( key == 'n' )
      {
        if ( file_loaded >= 1 )
          FT_Done_Face( face );

        if ( file < argc - 1 )
          file++;

        goto NewFile;
      }

      if ( key == 'p' )
      {
        if ( file_loaded >= 1 )
          FT_Done_Face( face );

        if ( file > 1 )
          file--;

        goto NewFile;
      }

      if ( ptsize != old_ptsize )
      {
        if ( Reset_Scale( ptsize ) )
          PanicZ( "Could not resize font." );

        old_ptsize = ptsize;
      }
    }

  End:
#if 0
    grDoneSurface( surface );
    grDone();
#endif

    printf( "Execution completed successfully.\n" );
    printf( "Fails = %d\n", Fail );

    exit( 0 );      /* for safety reasons */
    return 0;       /* never reached */
}


/* End */
