#include "grx11.h"

#define TEST

#ifdef TEST
#include "grfont.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

  static void Panic( const char* message )
  {
    fprintf( stderr, "%s", message );
    exit(1);
  }

  typedef struct Translator
  {
    KeySym  xkey;
    grKey   grkey;
    
  } Translator;
  
  static
  Translator  key_translators[] =
  {
    { XK_BackSpace, grKeyBackSpace },
    { XK_Tab,       grKeyTab       },
    { XK_Return,    grKeyReturn    },
    { XK_Escape,    grKeyEsc       },
    { XK_Home,      grKeyHome      },
    { XK_Left,      grKeyLeft      },
    { XK_Up,        grKeyUp        },
    { XK_Right,     grKeyRight     },
    { XK_Down,      grKeyDown      },
    { XK_Page_Up,   grKeyPageUp    },
    { XK_Page_Down, grKeyPageDown  },
    { XK_End,       grKeyEnd       },
    { XK_Begin,     grKeyHome      },
    { XK_F1,        grKeyF1        },
    { XK_F2,        grKeyF2        },
    { XK_F3,        grKeyF3        },
    { XK_F4,        grKeyF4        },
    { XK_F5,        grKeyF5        },
    { XK_F6,        grKeyF6        },
    { XK_F7,        grKeyF7        },
    { XK_F8,        grKeyF8        },
    { XK_F9,        grKeyF9        },
    { XK_F10,       grKeyF10       },
    { XK_F11,       grKeyF11       },
    { XK_F12,       grKeyF12       }
  };


#ifdef TEST
  
#define grAlloc  malloc
  
#endif


  static Display*  display;
  static char*     displayname = "";

  static Cursor  idle;
  static Cursor  busy;

#define MAX_PIXEL_MODES  32

  typedef XPixmapFormatValues  XDepth;

  static int           num_pixel_modes = 0;
  static grPixelMode   pixel_modes[ MAX_PIXEL_MODES ];  
  static XDepth        pixel_depth[ MAX_PIXEL_MODES ];

  typedef struct grXSurface_
  {
    grSurface      root;
    grBitmap       image;

    Window         win;
    Visual*        visual;
    Colormap       colormap;
    int            depth;
    XDepth*        xdepth;
    Bool           gray;
    
    GC             gc;
    
    XColor         color[256];   /* gray levels palette for 8-bit modes */
    XImage*        ximage;

    int            win_org_x;
    int            win_org_y;
    int            win_width;
    int            win_height;
    
    int            image_width;
    int            image_height;
    
  } grXSurface;




  /* close a given window */
  static
  void  done_surface( grXSurface*  surface )
  {
    XUnmapWindow( display, surface->win );
  }



  /* close the device, i.e. the display connection */
  static
  void  done_device( void )
  {
    XCloseDisplay( display );
  }



  static
  void add_pixel_mode( grPixelMode  pixel_mode,
                       XDepth*      depth )
  {
    if ( num_pixel_modes >= MAX_PIXEL_MODES )
      Panic( "X11.Too many pixel modes\n" );
      
    pixel_modes[ num_pixel_modes ] = pixel_mode;
    pixel_depth[ num_pixel_modes ] = *depth;
    
    num_pixel_modes++;
  }



  static
  int  init_device( void )
  {
    XDepth  dummy;
  
    XrmInitialize();
    
    display = XOpenDisplay( displayname );
    if (!display)
    {
      return -1;
     /* Panic( "Gr:error: cannot open X11 display\n" ); */
    }
      
    idle = XCreateFontCursor( display, XC_left_ptr );
    busy = XCreateFontCursor( display, XC_watch );
    
    num_pixel_modes = 0;
    
    /* always enable the 8-bit gray levels pixel mode                */
    /* even if its display is emulated through a constrained palette */
    /* or another color mode                                         */
    dummy.depth          = 8;
    dummy.bits_per_pixel = 8;
    dummy.scanline_pad   = 8;
    add_pixel_mode( gr_pixel_mode_gray, &dummy );

    {
      int          count;
      XDepth*      format;
      XDepth*      formats;
      XVisualInfo  template;

      formats = XListPixmapFormats( display, &count );
      format  = formats;
       
#ifdef TEST
      printf( "available pixmap formats\n" );
      printf( "depth  pixbits  scanpad\n" );
#endif
       
      while ( count-- > 0 )
      {
#ifdef TEST 
        printf( " %3d     %3d      %3d\n",
                format->depth,
                format->bits_per_pixel,
                format->scanline_pad );
#endif
      
        if ( format->depth == 1 )
          /* usually, this should be the first format */
          add_pixel_mode( gr_pixel_mode_mono, format );
          
        else if ( format->depth == 8 )
          add_pixel_mode( gr_pixel_mode_pal8, format );

        /* note, the 32-bit modes return a depth of 24, and 32 bits per pixel */          
        else if ( format->depth == 24 )
        {
#ifdef TEST 
          {
            int           count2;
            XVisualInfo*  visuals;
            XVisualInfo*  visual;
            const char*  string = "unknown";
    
            template.depth = format->depth;
            visuals        = XGetVisualInfo( display,
                                             VisualDepthMask,
                                             &template,
                                             &count2 );
            visual = visuals;
            
            switch (visual->class)
            {
              case TrueColor:   string = "TrueColor";    break;
              case DirectColor: string = "DirectColor";  break;
              case PseudoColor: string = "PseudoColor";  break;
              case StaticGray : string = "StaticGray";   break;
              case StaticColor: string = "StaticColor";  break;
              case GrayScale:   string = "GrayScale";    break;
            }

            printf( ">   RGB %04lx:%04lx:%04lx, colors %3d, bits %2d  %s\n",
                    visual->red_mask,
                    visual->green_mask,
                    visual->blue_mask,
                    visual->colormap_size,
                    visual->bits_per_rgb,
                    string );
            visual++;
          }        
#endif
          if ( format->bits_per_pixel == 24 )
            add_pixel_mode( gr_pixel_mode_rgb24, format );
            
          else if ( format->bits_per_pixel == 32 )
            add_pixel_mode( gr_pixel_mode_rgb32, format );
        }
          
        else if ( format->depth == 16 )
        {
          int           count2;
          XVisualInfo*  visuals;
          XVisualInfo*  visual;

          template.depth = format->depth;
          visuals        = XGetVisualInfo( display,
                                           VisualDepthMask,
                                           &template,
                                           &count2 );
          visual = visuals;
          
          while ( count2-- > 0 )
          {
#ifdef TEST 
            const char*  string = "unknown";
            
            switch (visual->class)
            {
              case TrueColor:   string = "TrueColor";    break;
              case DirectColor: string = "DirectColor";  break;
              case PseudoColor: string = "PseudoColor";  break;
              case StaticGray : string = "StaticGray";   break;
              case StaticColor: string = "StaticColor";  break;
              case GrayScale:   string = "GrayScale";    break;
            }

            printf( ">   RGB %04lx:%04lx:%04lx, colors %3d, bits %2d  %s\n",
                    visual->red_mask,
                    visual->green_mask,
                    visual->blue_mask,
                    visual->colormap_size,
                    visual->bits_per_rgb,
                    string );
                    
#endif
            if ( visual->red_mask   == 0xf800 &&
                 visual->green_mask == 0x07e0 &&
                 visual->blue_mask  == 0x001f )
              add_pixel_mode( gr_pixel_mode_rgb565, format );
              
            else if ( visual->red_mask   == 0x7c00 &&
                      visual->green_mask == 0x03e0 &&
                      visual->blue_mask  == 0x001f )
              add_pixel_mode( gr_pixel_mode_rgb555, format );
              
            /* other 16-bit modes are ignored */  
            visual++;
          }
          
          XFree( visuals );
        }
        
        format++;
      }
      
      XFree( formats );
    }
    
    gr_x11_device.num_pixel_modes = num_pixel_modes;
    gr_x11_device.pixel_modes     = pixel_modes;
    
    return 0;
  }







  static
  void  convert_gray_to_pal8( grXSurface*  surface,
                              int          x,
                              int          y,
                              int          w,
                              int          h )
  {
    grBitmap*  target  = &surface->image;
    grBitmap*  source  = &surface->root.bitmap;
    byte*      write   = (byte*)target->buffer + y*target->pitch + x;
    byte*      read    = (byte*)source->buffer + y*source->pitch + x;
    XColor*    palette = surface->color;
    
    while (h > 0)
    {
      byte*  _write = write;
      byte*  _read  = read;
      byte*  limit  = _write + w;
      
      for ( ; _write < limit; _write++, _read++ )
        *_write = (byte) palette[ *_read ].pixel;

      write += target->pitch;
      read  += source->pitch;
      h--;
    }
  }


  static
  void  convert_gray_to_16( grXSurface*  surface,
                            int          x,
                            int          y,
                            int          w,
                            int          h )
  {
    grBitmap*  target  = &surface->image;
    grBitmap*  source  = &surface->root.bitmap;
    byte*      write   = (byte*)target->buffer + y*target->pitch + 2*x;
    byte*      read    = (byte*)source->buffer + y*source->pitch + x;
    XColor*    palette = surface->color;
    
    while (h > 0)
    {
      byte*  _write = write;
      byte*  _read  = read;
      byte*  limit  = _write + 2*w;
      
      for ( ; _write < limit; _write += 2, _read++ )
        *(short*)_write = (short)palette[ *_read ].pixel;

      write += target->pitch;
      read  += source->pitch;
      h--;
    }
  }


  static
  void  convert_gray_to_24( grXSurface*  surface,
                            int          x,
                            int          y,
                            int          w,
                            int          h )
  {
    grBitmap*  target  = &surface->image;
    grBitmap*  source  = &surface->root.bitmap;
    byte*      write   = (byte*)target->buffer + y*target->pitch + 3*x;
    byte*      read    = (byte*)source->buffer + y*source->pitch + x;
    
    while (h > 0)
    {
      byte*  _write = write;
      byte*  _read  = read;
      byte*  limit  = _write + 3*w;
      
      for ( ; _write < limit; _write += 3, _read++ )
      {
        XColor*   color = surface->color + *_read;
       
        _write[0] = color->red;
        _write[1] = color->green;
        _write[2] = color->blue;
      }

      write += target->pitch;
      read  += source->pitch;
      h--;
    }
  }


  static
  void  convert_gray_to_32( grXSurface*  surface,
                            int          x,
                            int          y,
                            int          w,
                            int          h )
  {
    grBitmap*  target  = &surface->image;
    grBitmap*  source  = &surface->root.bitmap;
    byte*      write   = (byte*)target->buffer + y*target->pitch + 4*x;
    byte*      read    = (byte*)source->buffer + y*source->pitch + x;
    
    while (h > 0)
    {
      byte*  _write = write;
      byte*  _read  = read;
      byte*  limit  = _write + 4*w;
      
      for ( ; _write < limit; _write += 4, _read++ )
      {
        byte  color = *_read;

        *(unsigned long*)_write = surface->color[color].pixel;
      }

      write += target->pitch;
      read  += source->pitch;
      h--;
    }
  }


  static
  void  convert_rectangle( grXSurface*  surface,
                           int          x,
                           int          y,
                           int          w,
                           int          h )
  {
    int  z;
    
    /* first of all, clip to the surface's area */
    if ( x   >= surface->image.width ||
         x+w <= 0                    ||
         y   >= surface->image.rows  ||
         y+h <= 0 )
      return;
 
    if ( x < 0 )
    {
      w += x;
      x  = 0;
    }
 
    z = (x + w) - surface->image.width;
    if (z > 0)
      w -= z;
      
    z = (y + h) - surface->image.rows;
    if (z > 0)
      h -= z;
      
    /* convert the rectangle to the target depth for gray surfaces */
    if (surface->gray)
    {
      switch (surface->xdepth->bits_per_pixel)
      {
        case 8 : convert_gray_to_pal8( surface, x, y, w, h ); break;
        case 16: convert_gray_to_16  ( surface, x, y, w, h ); break;
        case 24: convert_gray_to_24  ( surface, x, y, w, h ); break;
        case 32: convert_gray_to_32  ( surface, x, y, w, h ); break;
      }
    }
  } 


  static
  void  refresh_rectangle( grXSurface*  surface,
                           int          x,
                           int          y,
                           int          w,
                           int          h )
  {
    if (surface->gray)
      convert_rectangle( surface, x, y, w, h );
    
    XPutImage( display,
               surface->win,
               surface->gc,
               surface->ximage,
               x, y, x, y, w, h );
  }

  
  static
  void  set_title( grXSurface*  surface,
                   const char*  title )
  {
    XStoreName( display, surface->win, title );
  }



  static
  grKey  KeySymTogrKey( KeySym  key )
  {
    grKey        k;
    int          count = sizeof(key_translators)/sizeof(key_translators[0]);
    Translator*  trans = key_translators;
    Translator*  limit = trans + count;

    k = grKeyNone;

    while ( trans < limit )
    {
      if ( trans->xkey == key )
      {
        k = trans->grkey;
        break;
      }
      trans++;
    }

    return k;
  }



  static  
  void  listen_event( grXSurface*  surface,
                      int          event_mask,
                      grEvent*     grevent )
  {
    static char     key_buffer[10];
    static int      key_cursor = 0;
    static int      key_number = 0;
    static XEvent   x_event;
           KeySym   key;

    int             bool_exit;
    grKey           grkey;

    XComposeStatus  compose;

    /* XXXX : For now, ignore the event mask, and only exit when */
    /*        a key is pressed..                                 */
    (void)event_mask;

    bool_exit = key_cursor < key_number;

    XDefineCursor( display, surface->win, idle );

    while ( !bool_exit )
    {
      XNextEvent( display, &x_event );

      switch ( x_event.type )
      {
      case KeyPress: 
        key_number = XLookupString( &x_event.xkey,
                                    key_buffer,
                                    sizeof ( key_buffer ),
                                    &key,
                                    &compose );
        key_cursor = 0;

        if ( key_number == 0 ||
             key > 512       )
        {
          /* this may be a special key like F1, F2, etc.. */
          grkey = KeySymTogrKey(key);
          if (grkey != grKeyNone)
            goto Set_Key;  
        }
        else
          bool_exit = 1;
        break;

      case MappingNotify:
        XRefreshKeyboardMapping( &x_event.xmapping );
        break;
      
      case Expose:
        refresh_rectangle( surface,
                           x_event.xexpose.x,
                           x_event.xexpose.y,
                           x_event.xexpose.width,
                           x_event.xexpose.height );
        break;

      /* You should add more cases to handle mouse events, etc. */
      }
    }

    XDefineCursor( display, surface->win, busy );
    XFlush       ( display );

    /* Now, translate the keypress to a grKey */
    /* If this wasn't part of the simple translated keys, simply get the charcode */
    /* from the character buffer                                                  */
    grkey = grKEY(key_buffer[key_cursor++]);
      
  Set_Key:
    grevent->type = gr_key_down;
    grevent->key  = grkey;
  }




  grXSurface*  init_surface( grXSurface*  surface,
                             grBitmap*    bitmap )
  {
    int        screen;
    grBitmap*  image;
    char       grays;
    XDepth*    format;
    int        image_depth;
    
    screen = DefaultScreen( display );
    
    surface->colormap = DefaultColormap( display, screen );
    surface->depth    = DefaultDepth( display, screen );
    surface->visual   = DefaultVisual( display, screen ); 

    image  = &surface->image;

    /* force the surface image depth to 1 if necessary */
    /* as this should be supported by all windows      */
    image_depth = surface->depth;
    if (bitmap->mode == gr_pixel_mode_mono)
      image_depth = 1;

    grays = ( bitmap->mode == gr_pixel_mode_gray &&
              bitmap->grays >= 2 );

    surface->gray = grays;
 
    /* copy dimensions */
    image->width  = bitmap->width;
    image->rows   = bitmap->rows;
    image->mode   = bitmap->mode;
    image->pitch  = 0;
    image->grays  = 0;
    image->buffer = 0;

    /* find the supported format corresponding to the request */
    format = 0;

    if (grays)    
    {
      /* choose the default depth in case of grays rendering */
      int  i;
      for ( i = 0; i < num_pixel_modes; i++ )
        if ( image_depth == pixel_depth[i].depth )
        {
          format = pixel_depth + i;
          surface->xdepth = format;
          break;
        }
    }
    else
    {
      /* otherwise, select the format depending on the pixel mode */
      int  i;
      
      format = 0;
      for ( i = 0; i < num_pixel_modes; i++ )
        if ( pixel_modes[i] == bitmap->mode )
        {
          format = pixel_depth + i;
          surface->xdepth = format;
          break;
        }
    }
    
    if (!format)
    {
      grError = gr_err_bad_argument;
      return 0;
    }


    /* correct surface.depth. This is required because in the case    */
    /* of 32-bits pixels, the value of "format.depth" is 24 under X11 */
    if ( format->depth          == 24 &&
         format->bits_per_pixel == 32 )
      image_depth = 32;
     
    /* allocate surface image */
    {
      int  bits, over;

      bits = image->width * format->bits_per_pixel;
      over = bits % format->scanline_pad;
          
      if (over)
        bits += format->scanline_pad - over;
      
      if (!grays)
      {
        image->width  = bits;
        bitmap->width = bits;
      }
      image->pitch  = bits >> 3;
    }

    image->buffer = grAlloc( image->pitch * image->rows );
    if (!image->buffer) return 0;

    /* now, allocate a gray pal8 pixmap, only when we asked */
    /* for an 8-bit pixmap                                  */
    if ( grays )
    {
      /* pad pitch to 32 bits */
      bitmap->pitch  = (bitmap->width + 3) & -4;
      bitmap->buffer = grAlloc( bitmap->pitch * bitmap->rows );
      if (!bitmap->buffer)
        Panic( "grX11: could not allocate surface bitmap!\n" );
    }
    else  /* otherwise */
    {
      *bitmap = *image;
    }

    surface->root.bitmap = *bitmap;
 
    /* Now create the surface X11 image */
    surface->ximage = XCreateImage( display, 
                                    surface->visual,
                                    format->depth,
                                    format->depth == 1 ? XYBitmap : ZPixmap,
                                    0,
                                    (char*)image->buffer,
                                    image->width,
                                    image->rows,
                                    8,
                                    0 );
    if ( !surface->ximage )
      Panic( "grX11: cannot create surface X11 image\n" );
 

    /* allocate gray levels in the case of gray surface */
    if ( grays )
    {
      XColor*  color = surface->color;
      int      i;
      
      for ( i = 0; i < bitmap->grays; i++, color++ )
      {
        color->red   =
        color->green =
        color->blue  = 65535 - ( i * 65535 ) / bitmap->grays;
  
        if ( !XAllocColor( display, surface->colormap, color ) )
          Panic( "ERROR: cannot allocate Color\n" );
      }
    }
    else if ( image_depth == 1 )
    {
      surface->ximage->byte_order       = MSBFirst;
      surface->ximage->bitmap_bit_order = MSBFirst;
    }

    {
        XTextProperty         xtp;
        XSizeHints            xsh;
        XSetWindowAttributes  xswa;
    
        xswa.border_pixel     = BlackPixel( display, screen );
        xswa.background_pixel = WhitePixel( display, screen );
        xswa.cursor           = busy;
    
        xswa.event_mask = KeyPressMask | ExposureMask;
    
        surface->win = XCreateWindow( display,
                                      RootWindow( display, screen ),
                                      0,
                                      0,
                                      image->width,
                                      image->rows,
                                      10,
                                      surface->depth,
                                      InputOutput, 
                                      surface->visual,
                                      CWBackPixel | CWBorderPixel |
                                        CWEventMask | CWCursor,
                                      &xswa );
    
        XMapWindow( display, surface->win );
 
        surface->gc = XCreateGC( display, RootWindow( display, screen ), 0L, NULL );
        XSetForeground( display, surface->gc, xswa.border_pixel     );
        XSetBackground( display, surface->gc, xswa.background_pixel );
    
    
        /* make window manager happy :-) */
        xtp.value    = (unsigned char*)"FreeType";
        xtp.encoding = 31;
        xtp.format   = 8;
        xtp.nitems   = strlen( (char*)xtp.value );
    
        xsh.x = 0;
        xsh.y = 0;
    
        xsh.width  = image->width;
        xsh.height = image->rows;
        xsh.flags  = (PPosition | PSize);
        xsh.flags  = 0;
    
        XSetWMProperties( display, surface->win, &xtp, &xtp, NULL, 0, &xsh, NULL, NULL );
    }
    
    surface->root.done         = (grDoneSurfaceFunc) done_surface;
    surface->root.refresh_rect = (grRefreshRectFunc) refresh_rectangle;
    surface->root.set_title    = (grSetTitleFunc)    set_title;
    surface->root.listen_event = (grListenEventFunc) listen_event;
    
    convert_rectangle( surface, 0, 0, bitmap->width, bitmap->rows );
    return surface;
  }
  



  grDevice  gr_x11_device =
  {
    sizeof( grXSurface ),
    "x11",
    
    init_device,
    done_device,
    
    (grDeviceInitSurfaceFunc) init_surface,
    
    0,
    0
    
  };

#ifdef TEST

typedef struct grKeyName
{
  grKey       key;
  const char* name;

} grKeyName;


static
const grKeyName  key_names[] =
{
  { grKeyF1,   "F1"  },
  { grKeyF2,   "F2"  },
  { grKeyF3,   "F3"  },
  { grKeyF4,   "F4"  },
  { grKeyF5,   "F5"  },
  { grKeyF6,   "F6"  },
  { grKeyF7,   "F7"  },
  { grKeyF8,   "F8"  },
  { grKeyF9,   "F9"  },
  { grKeyF10,  "F10" },
  { grKeyF11,  "F11" },
  { grKeyF12,  "F12" },
  { grKeyEsc,  "Esc" },
  { grKeyHome, "Home" },
  { grKeyEnd,  "End"  },
    
  { grKeyPageUp,   "Page_Up" },
  { grKeyPageDown, "Page_Down" },
  { grKeyLeft,     "Left" },
  { grKeyRight,    "Right" },
  { grKeyUp,       "Up" },
  { grKeyDown,     "Down" },
  { grKeyBackSpace, "BackSpace" },
  { grKeyReturn,   "Return" }
};

#if 0
int  main( void )
{
  grSurface*  surface;
  int         n;
  
  grInit();
  surface = grNewScreenSurface( 0, gr_pixel_mode_gray, 320, 400, 128 );
  if (!surface)
    Panic("Could not create window\n" );
  else
  {
    grColor      color;
    grEvent      event;
    const char*  string;
    int          x;
    
    grSetSurfaceRefresh( surface, 1 );
    grSetTitle(surface,"X11 driver demonstration" );
    
    for ( x = -10; x < 10; x++ )
    {
      for ( n = 0; n < 128; n++ )
      {
        color.value = (n*3) & 127;
        grWriteCellChar( surface,
                         x + ((n % 60) << 3),
                         80 + (x+10)*8*3 + ((n/60) << 3), n, color );
      }

    }
    color.value = 64;
    grWriteCellString( surface, 0, 0, "just an example", color );
    
    do
    {
      listen_event((grXSurface*)surface, 0, &event);
    
      /* return if ESC was pressed */
      if ( event.key == grKeyEsc )
        return 0;
      
      /* otherwise, display key string */
      color.value = (color.value + 8) & 127;
      {
        int         count = sizeof(key_names)/sizeof(key_names[0]);
        grKeyName*  name  = key_names;
        grKeyName*  limit = name + count;
        const char* kname  = 0;
        char        kname_temp[16];
      
        while (name < limit)
        {
          if ( name->key == event.key )
          {
            kname = name->name;
            break;
          }
          name++;
        }
      
        if (!kname)
        {
          sprintf( kname_temp, "char '%c'", (char)event.key );
          kname = kname_temp;
        }
        
        grWriteCellString( surface, 30, 30, kname, color );
        grRefreshSurface(surface);
        paint_rectangle( surface, 0, 0, surface->bitmap.width, surface->bitmap.rows );
      }
    } while (1);
  }
    
  return 0;
  
  
}
#endif /* O */
#endif /* TEST */

