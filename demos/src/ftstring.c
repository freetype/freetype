/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-1999 by                                                  */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*                                                                          */
/*  FTString.c - simple text string display                                 */
/*                                                                          */
/****************************************************************************/

#include "freetype.h"
#include "ftglyph.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "graph.h"
#include "grfont.h"

#include "ftgrays.h"

#define  DIM_X   500
#define  DIM_Y   400

#define  CENTER_X   (bit.width/2)
#define  CENTER_Y   (bit.rows/2)

#define  MAXPTSIZE  500                 /* dtp */

  static char  Header[128];
  static char* new_header = 0;

  static unsigned char*  Text = "The quick brown fox jumps over the lazy dog";
  
  static FT_Library    library;      /* the FreeType library            */
  static FT_Face       face;         /* the font face                   */
  static FT_Error      error;        /* error returned by FreeType ?    */

  static grSurface*     surface;     /* current display surface         */
  static grBitmap       bit;         /* current display bitmap          */

  static int  ptsize;                /* current point size */
  static int  Num;
  static int  Rotation = 0;
  static int  Fail;
  
  static int  hinted    = 1;       /* is glyph hinting active ?    */
  static int  antialias = 1;       /* is anti-aliasing active ?    */
  static int  use_sbits = 1;       /* do we use embedded bitmaps ? */
  static int  kerning   = 1;

  static int  res = 72;  /* default resolution in dpi */

  static grColor  fore_color = { 127 };

  static int  graph_init = 0;
  static int  render_mode = 1;
  static int  use_grays   = 0;

  /* the standard raster's interface */
  static FT_Raster_Funcs  std_raster;

  static FT_Matrix      trans_matrix;
  static int            transform = 0;
  
  static FT_Vector      string_center;

  typedef struct TGlyph_
  {
    FT_UInt    glyph_index;    /* glyph index in face      */
    FT_Vector  pos;            /* position of glyph origin */
    FT_Glyph   image;          /* glyph image              */
  
  } TGlyph, *PGlyph;

#define FLOOR(x)  ((x) & -64)
#define CEIL(x)   (((x)+63) & -64)
#define TRUNC(x)  ((x) >> 6)



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****                                                                    ****/
/****    U T I L I T Y   F U N C T I O N S                               ****/
/****                                                                    ****/
/****                                                                    ****/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#define DEBUGxxx

#ifdef DEBUG
#define LOG(x)  LogMessage##x
#else
#define LOG(x)  /* rien */
#endif

#ifdef DEBUG
  static void  LogMessage( const char*  fmt, ... )
  {
    va_list  ap;

    va_start( ap, fmt );
    vfprintf( stderr, fmt, ap );
    va_end( ap );
  }
#endif

  /* PanicZ */
  static void PanicZ( const char* message )
  {
    fprintf( stderr, "%s\n  error = 0x%04x\n", message, error );
    exit(1);
  }


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****                                                                    ****/
/****    D I S P L A Y   M A N A G E M E N T                             ****/
/****                                                                    ****/
/****                                                                    ****/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#define MAX_GLYPHS 512

 /***********************************************************************
  *
  *  The following arrays are used to store the glyph set that makes
  *  up a string of text..
  *
  */
  static TGlyph  glyphs[ MAX_GLYPHS ];
  static int     num_glyphs;


 /**************************************************************
  *
  *  Initialise the display surface
  *
  */
  static int  init_display( void )
  {
    grInitDevices();

    bit.mode   = gr_pixel_mode_gray;
    bit.width  = DIM_X;
    bit.rows   = DIM_Y;
    bit.grays  = 128;

    surface = grNewSurface( 0, &bit );
    if (!surface)
      PanicZ( "could not allocate display surface\n" );

    graph_init = 1;
    return 0;
  }

 /**************************************************************
  *
  *  Clears the display surface
  *
  */
  static void  clear_display( void )
  {
    long  size = (long)bit.pitch * bit.rows;
    
    if (size < 0) size = -size;
    memset( bit.buffer, 0, size );
  }


  static FT_Error  reset_scale( int  pointSize )
  {
    FT_Error  error;

    error = FT_Set_Char_Size( face, pointSize << 6,
                                    pointSize << 6,
                                    res,
                                    res );
    return FT_Err_Ok;
  }
  

 /**************************************************************
  *
  *  Compute the dimension of a string of glyphs in pixels
  *
  */
  static void  compute_bbox( FT_BBox*  abbox )
  {
    PGlyph  glyph = glyphs;
    FT_BBox bbox;
    int     n;
    
    bbox.xMin = 32000; bbox.xMax = -32000;
    bbox.yMin = 32000; bbox.yMax = -32000;
    
    for ( n = 0; n < num_glyphs; n++, glyph++ )
    {
      FT_BBox  cbox;
      FT_Pos   x, y;
      
      if (!glyph->image) continue;
      
      x = glyph->pos.x >> 6;
      y = glyph->pos.y >> 6;

      FT_Glyph_Get_Box( glyph->image, &cbox );

      cbox.xMin += x;
      cbox.yMin += y;
      cbox.xMax += x;
      cbox.yMax += y;
      
      if (cbox.xMin < bbox.xMin) bbox.xMin = cbox.xMin;
      if (cbox.xMax > bbox.xMax) bbox.xMax = cbox.xMax;
      if (cbox.yMin < bbox.yMin) bbox.yMin = cbox.yMin;
      if (cbox.yMax > bbox.yMax) bbox.yMax = cbox.yMax;
    }
    *abbox = bbox;
  }


 /**************************************************************
  *
  *  Layout a string of glyphs
  *
  */
  static void  layout_glyphs( void )
  {
    PGlyph    glyph = glyphs;
    FT_Error  error;
    int       n;
    FT_Vector origin;
    FT_Pos    origin_x = 0;
    FT_UInt   load_flags;
    FT_UInt   num_grays;
    FT_UInt   prev_index = 0;

    load_flags = FT_LOAD_DEFAULT;
    if( !hinted )
      load_flags |= FT_LOAD_NO_HINTING;
      
    num_grays = 128;
    if (!antialias)
      num_grays = 0;
    
    for ( n = 0; n < num_glyphs; n++, glyph++ )
    {
      /* compute glyph origin */
      if (kerning)
      {
        if (prev_index)
        {
          FT_Vector  kern;
          
          FT_Get_Kerning( face, prev_index, glyph->glyph_index, &kern );
          kern.x = FT_MulFix( kern.x, face->size->metrics.x_scale );
          if (hinted) kern.x = (kern.x+32) & -64;
          
          origin_x += kern.x;
        }
        prev_index = glyph->glyph_index;
      }

      origin.x = origin_x;
      origin.y = 0;

      if (transform)
        FT_Vector_Transform( &origin, &trans_matrix );

      /* clear existing image if there is one */
      if (glyph->image)
        FT_Done_Glyph(glyph->image);
    
      /* load the glyph image                       */
      /* for now, we take a monochrome glyph bitmap */
      error = FT_Get_Glyph_Bitmap( face, glyph->glyph_index,
                                   load_flags,
                                   num_grays,
                                   &origin,
                                   (FT_BitmapGlyph*)&glyph->image );
      if (error) continue;
      
      glyph->pos = origin;
      
      origin_x  += glyph->image->advance;
    }
    string_center.x = origin_x / 2;
    string_center.y = 0;
    if (transform)
      FT_Vector_Transform( &string_center, &trans_matrix );
  }

 /**************************************************************
  *
  *  Renders a given glyph vector set
  *
  */
  static  void  render_string( FT_Pos  x, FT_Pos  y )
  {
    PGlyph    glyph = glyphs;
    grBitmap  bit3;
    int       n;
    
    for ( n = 0; n < num_glyphs; n++, glyph++ )
    {
      if (!glyph->image)
        continue;
        
      switch (glyph->image->glyph_type)
      {
        case ft_glyph_type_bitmap:
          {
            /* this is a bitmap, we simply blit it to our target surface */
            FT_BitmapGlyph  bitm   = (FT_BitmapGlyph)glyph->image;
            FT_Bitmap*      source = &bitm->bitmap;
            FT_Pos          x_top, y_top;
            
            bit3.rows   = source->rows;
            bit3.width  = source->width;
            bit3.pitch  = source->pitch;
            bit3.buffer = source->buffer;
            
            switch (source->pixel_mode)
            {
              case ft_pixel_mode_mono:
                bit3.mode  = gr_pixel_mode_mono;
                break;
                
              case ft_pixel_mode_grays:
                bit3.mode  = gr_pixel_mode_gray;
                bit3.grays = source->num_grays;
                break;
              
              default:
                continue;
            }
            
            /* now render the bitmap into the display surface */
            x_top = x + (glyph->pos.x >> 6) + bitm->left;
            y_top = y - (glyph->pos.y >> 6) - bitm->top;
            grBlitGlyphToBitmap( &bit, &bit3, x_top, y_top, fore_color );
          }
          break;
#if 0        
        case ft_glyph_type_outline:
          {
            /* in the case of outlines, we directly render it into the */
            /* target surface with the smooth renderer..               */
            FT_OutlineGlyph  out = (FT_OutlineGlyph)glyph->image;
            
            FT_Outline_Translate( (x+pen_pos[n]) << 6, (y+
            error = FT_Outline_Render( 
          }
          break;
#endif
        default:
          ;
      }
    }
  }


 /**************************************************************
  *
  *  Convert a string of text into a glyph vector
  *
  *  XXX: For now, we perform a trivial conversion
  *
  */
  static  void  prepare_text( const char*  string )
  {
    const unsigned char*  p     = (const unsigned char*)string;
    PGlyph                glyph = glyphs;
    FT_UInt               glyph_index;
    
    num_glyphs = 0;
    while (*p)
    {
      glyph_index = FT_Get_Char_Index( face, (FT_ULong)*p );
      glyph->glyph_index = glyph_index;
      glyph++;
      num_glyphs++;
      if (num_glyphs >= MAX_GLYPHS)
        break;
      p++;
    }
  }  


  static void  reset_transform( void )
  {
    double    angle   = Rotation*3.14159/64.0;
    FT_Fixed  cosinus = (FT_Fixed)(cos(angle)*65536.0);
    FT_Fixed  sinus   = (FT_Fixed)(sin(angle)*65536.0);
    
    transform       = (angle != 0);
    trans_matrix.xx = cosinus;
    trans_matrix.xy = -sinus;
    trans_matrix.yx = sinus;
    trans_matrix.yy = cosinus;
    
    FT_Set_Transform(face,&trans_matrix, 0);
  }

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****                                                                    ****/
/****    E V E N T   H A N D L I N G                                     ****/
/****                                                                    ****/
/****                                                                    ****/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


  static void Help( )
  {
    grEvent  dummy_event;

    clear_display();
    grGotoxy( 0, 0 );
    grSetMargin( 2, 1 );
    grGotobitmap( &bit );
    
    grWriteln("FreeType String Viewer - part of the FreeType test suite" );
    grLn();
    grWriteln("This program is used to display a string of text using" );
    grWriteln("the new convenience API of the FreeType 2 library.");
    grLn();
    grWriteln("Use the following keys :");
    grLn();
    grWriteln("  F1 or ?   : display this help screen" );
    grWriteln("  a         : toggle anti-aliasing" );
    grWriteln("  h         : toggle outline hinting" );
    grWriteln("  k         : toggle kerning" );
    grWriteln("  g         : toggle between 'smooth' and 'standard' anti-aliaser" );
    grLn();
    grWriteln("  Up        : increase pointsize by 1 unit" );
    grWriteln("  Down      : decrease pointsize by 1 unit" );
    grWriteln("  Page Up   : increase pointsize by 10 units" );
    grWriteln("  Page Down : decrease pointsize by 10 units" );
    grLn();
    grWriteln("  Right     : rotate counter-clockwise" );
    grWriteln("  Left      : rotate clockwise" );
    grWriteln("  F7        : big rotate counter-clockwise");
    grWriteln("  F8        : big rotate clockwise");
    grLn();
    grWriteln("press any key to exit this help screen");
        
    grRefreshSurface( surface );
    grListenSurface( surface, gr_event_key, &dummy_event );
  }


  static void  reset_raster( void )
  {
    FT_Error  error;
    
    error = 1;
    if ( use_grays && antialias )
      error = FT_Set_Raster( library, &ft_grays_raster );
      
    if (error)
      (void)FT_Set_Raster( library, &std_raster );
  }


  static int  Process_Event( grEvent*  event )
  {
    int  i;
    
    switch ( event->key )
    {
    case grKeyEsc:            /* ESC or q */
    case grKEY('q'):
      return 0;

    case grKEY('k'):
      kerning = !kerning;
      new_header = ( kerning
                   ? "kerning is now active"
                   : "kerning is now ignored" );
      return 1;
      
    case grKEY('a'):
      antialias = !antialias;
      new_header = ( antialias
                   ? "anti-aliasing is now on"
                   : "anti-aliasing is now off" );
      reset_raster();
      return 1;

    case grKEY('b'):
      use_sbits  = !use_sbits;
      new_header = ( use_sbits
                   ? "embedded bitmaps are now used when available"
                   : "embedded bitmaps are now ignored" );
      return 1;
      
    case grKEY('n'):
    case grKEY('p'):
      return (int)event->key;

    case grKEY('g'):
      use_grays = !use_grays;
      new_header = ( use_grays
                   ? "now using the smooth anti-aliaser"
                   : "now using the standard anti-aliaser" );
      reset_raster();
      break;
                   
    case grKEY('h'):
      hinted = !hinted;
      new_header = ( hinted
                   ? "glyph hinting is now active"
                   : "glyph hinting is now ignored" );
      break;

    case grKEY(' '):
      render_mode ^= 1;
      new_header = ( render_mode
                   ? "rendering all glyphs in font"
                   : "rendering test text string" );
      break;

    case grKeyF1:
    case grKEY('?'):
      Help();
      return 1;

#if 0
    case grKeyF3:  i =  16; goto Do_Rotate;
    case grKeyF4:  i = -16; goto Do_Rotate;
    case grKeyF5:  i =   1; goto Do_Rotate;
    case grKeyF6:  i =  -1; goto Do_Rotate;
#endif

    case grKeyPageUp:   i =  10; goto Do_Scale;
    case grKeyPageDown: i = -10; goto Do_Scale;
    case grKeyUp:       i =   1; goto Do_Scale;
    case grKeyDown:     i =  -1; goto Do_Scale;

    case grKeyLeft:  i =  -1; goto Do_Rotate;
    case grKeyRight: i =   1; goto Do_Rotate;
    case grKeyF7:    i = -10; goto Do_Rotate;
    case grKeyF8:    i =  10; goto Do_Rotate;
    default:
      ;
    }
    return 1;

  Do_Rotate:
    Rotation = (Rotation + i) & 127;
    return 1;

  Do_Scale:
    ptsize += i;
    if (ptsize < 1)         ptsize = 1;
    if (ptsize > MAXPTSIZE) ptsize = MAXPTSIZE;
    return 1;

#if 0    
  Do_Glyph:
    Num += i;
    if (Num < 0)           Num = 0;
    if (Num >= num_glyphs) Num = num_glyphs-1;
    return 1;
#endif    
  }


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****                                                                    ****/
/****    M A I N   P R O G R A M                                         ****/
/****                                                                    ****/
/****                                                                    ****/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


  static void  usage( char*  execname )
  {
    fprintf( stderr,  "\n" );
    fprintf( stderr,  "ftview: simple string viewer -- part of the FreeType project\n" );
    fprintf( stderr,  "------------------------------------------------------------\n" );
    fprintf( stderr,  "\n" );
    fprintf( stderr,  "Usage: %s [options below] ppem fontname[.ttf|.ttc] ...\n",
             execname );
    fprintf( stderr,  "\n" );
    fprintf( stderr,  "  -r R        use resolution R dpi (default: 72 dpi)\n" );
    fprintf( stderr,  "  -m message  message to display\n" );
    fprintf( stderr,  "\n" );

    exit( 1 );
  }


  int  main( int  argc, char**  argv ) 
  {
    int    i, old_ptsize, orig_ptsize, file;
    int    first_glyph = 0;
    int    XisSetup = 0;
    char   filename[128 + 4];
    char   alt_filename[128 + 4];
    char*  execname;
    int    option;
    int    file_loaded;

    FT_Error  error;
    grEvent   event;
 
    execname = ft_basename( argv[0] );

    while ( 1 )
    {
      option = getopt( argc, argv, "m:r:" );

      if ( option == -1 )
        break;

      switch ( option )
      {
      case 'r':
        res = atoi( optarg );
        if ( res < 1 )
          usage( execname );
        break;

      case 'm':
        if (argc < 3)
          usage( execname );
        Text = optarg;
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
    if (error) PanicZ( "Could not initialise FreeType library" );

    /* retrieve the standard raster's interface */
    (void)FT_Get_Raster( library, ft_glyph_format_outline, &std_raster );

  NewFile:
    ptsize      = orig_ptsize;
    hinted      = 1;
    file_loaded = 0;

#ifndef macintosh
    i = strlen( argv[file] );
    while ( i > 0 && argv[file][i] != '\\' && argv[file][i] != '/' )
    {
      if ( argv[file][i] == '.' )
        i = 0;
      i--; 
    }
#endif

    filename[128] = '\0';
    alt_filename[128] = '\0';

    strncpy( filename, argv[file], 128 );
    strncpy( alt_filename, argv[file], 128 );

#ifndef macintosh
    if ( i >= 0 )
    {
      strncpy( filename + strlen( filename ), ".ttf", 4 );
      strncpy( alt_filename + strlen( alt_filename ), ".ttc", 4 );
    }
#endif

    /* Load face */

    error = FT_New_Face( library, filename, 0, &face );
    if (error) goto Display_Font;

    /* prepare the text to be rendered */
    prepare_text( Text );

    file_loaded++;
    
    error = reset_scale( ptsize );
    if (error) goto Display_Font;
        
  Display_Font:
    /* initialise graphics if needed */
    if ( !XisSetup )
    {
      XisSetup = 1;
      init_display();
    }
  
    grSetTitle( surface, "FreeType String Viewer - press F1 for help" );
    old_ptsize = ptsize;

    if ( file_loaded >= 1 )
    {
      Fail = 0;
      Num  = first_glyph;
    
      if ( Num >= num_glyphs )
        Num = num_glyphs-1;
      
      if ( Num < 0 )
        Num = 0;
    }

    for ( ;; )
    {
      int  key;

      clear_display();

      if ( file_loaded >= 1 )
      {
        /* layout & render string */
        {
          FT_BBox  bbox;
          
          reset_transform();
          layout_glyphs();
          compute_bbox( &bbox );
          render_string( (bit.width-(string_center.x >> 5))/2,
                         (bit.rows +(string_center.y >> 5))/2 );
        }

        sprintf( Header, "%s %s (file %s)",
                         face->family_name,
                         face->style_name,
                         ft_basename( filename ) );

        if (!new_header)
          new_header = Header;
              
        grWriteCellString( &bit, 0, 0, new_header, fore_color );
        new_header = 0;

        sprintf( Header, "at %d points, rotation = %d",
                         ptsize,
                         Rotation );
      }
      else
      {
        sprintf( Header, "%s : is not a font file or could not be opened",
                         ft_basename(filename) );
      }

      grWriteCellString( &bit, 0, 8, Header, fore_color );
      grRefreshSurface( surface );
      
      grListenSurface( surface, 0, &event );
      if ( !( key = Process_Event( &event ) ) )
        goto Fin;

      if ( key == 'n' )
      {
        if (file_loaded >= 1)
          FT_Done_Face( face );
          
        if ( file < argc - 1 )
          file++;

        goto NewFile;
      }

      if ( key == 'p' )
      {
        if (file_loaded >= 1)
          FT_Done_Face( face );
          
        if ( file > 1 )
          file--;

        goto NewFile;
      }

      if ( ptsize != old_ptsize )
      {
        if ( reset_scale( ptsize ) )
          PanicZ( "Could not resize font." );

        old_ptsize = ptsize;
      }
    }

  Fin:
#if 0
    grDoneSurface(surface);
    grDone();
#endif
    printf( "Execution completed successfully.\n" );
    printf( "Fails = %d\n", Fail );

    exit( 0 );      /* for safety reasons */
    return 0;       /* never reached */
}


/* End */

