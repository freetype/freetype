/*******************************************************************
 *
 *  grwin32.c  graphics driver for Win32 platform.              0.1
 *
 *  This is the driver for displaying inside a window under Win32,
 *  used by the graphics utility of the FreeType test suite.
 *
 *  Written by Antoine Leca.
 *  Copyright 1999-2000 by Antoine Leca, David Turner
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  Borrowing liberally from the other FreeType drivers.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT. By continuing to use, modify or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "grwin32.h"
#include "grdevice.h"


/* logging facility */
#include <stdarg.h>

#define  DEBUGxxx

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
/*-------------------*/

/*  Size of the window. */
#define WIN_WIDTH   640u
#define WIN_HEIGHT  450u

/* These values can be changed, but WIN_WIDTH should remain for now a  */
/* multiple of 32 to avoid padding issues.                             */

  typedef struct  _Translator
  {
    ULONG   winkey;
	grKey   grkey;
	
  } Translator;
  
  static
  Translator  key_translators[] =
  {
    { VK_BACK,      grKeyBackSpace },
    { VK_TAB,       grKeyTab       },
    { VK_RETURN,    grKeyReturn    },
    { VK_ESCAPE,    grKeyEsc       },
    { VK_HOME,      grKeyHome      },
    { VK_LEFT,      grKeyLeft      },
    { VK_UP,        grKeyUp        },
    { VK_RIGHT,     grKeyRight     },
    { VK_DOWN,      grKeyDown      },
    { VK_PRIOR,     grKeyPageUp    },
    { VK_NEXT,      grKeyPageDown  },
    { VK_END,       grKeyEnd       },
    { VK_F1,        grKeyF1        },
    { VK_F2,        grKeyF2        },
    { VK_F3,        grKeyF3        },
    { VK_F4,        grKeyF4        },
    { VK_F5,        grKeyF5        },
    { VK_F6,        grKeyF6        },
    { VK_F7,        grKeyF7        },
    { VK_F8,        grKeyF8        },
    { VK_F9,        grKeyF9        },
    { VK_F10,       grKeyF10       },
    { VK_F11,       grKeyF11       },
    { VK_F12,       grKeyF12       }
  };

  static
  Translator  syskey_translators[] =
  {
    { VK_F1,        grKeyF1        }
  };

  /* This is a minimalist driver, it is only able to display */
  /* a _single_ window. Moreover, only monochrome and gray   */
  /* bitmaps are supported..                                 */
  
  /* handle of the window. */
  static HWND   hwndGraphic;

  static int window_width, window_height;

  /* the following variables are used to set the window title lazily */
  static int          title_set = 1;
  static const char*  the_title;

  /* bitmap information */
  static LPBITMAPINFO pbmi;
  static HBITMAP      hbm;

  /* local event to pass on */
  static grEvent  ourevent;
  static int      eventToProcess = 0;

/* destroys the surface*/
static
void done_surface( grSurface*  surface )
{
  /* The graphical window has perhaps already destroyed itself */
  if ( hwndGraphic )
  {
    DestroyWindow ( hwndGraphic );
    PostMessage( hwndGraphic, WM_QUIT, 0, 0 );
  }
  grDoneBitmap( &surface->bitmap );
  if ( pbmi ) free ( pbmi );
}

  static
  const int  pixel_mode_bit_count[] =
  {
    0,
    1,   /* mono  */
    4,   /* pal4  */
    8,   /* pal8  */
	8,   /* grays */
    15,  /* rgb15 */
    16,  /* rgb16 */
    24,  /* rgb24 */
    32   /* rgb32 */
  };

  static
  void  refresh_rectangle( grSurface*  surface,
  						   int         x,
						   int		   y,
						   int         w,
						   int         h )
  {
    HDC     hDC;
    int     row_bytes;

    LOG(( "Win32: refresh_rectangle( %08lx, %d, %d, %d, %d )\n",
           (long)surface, x, y, w, h ));
    (void)x;
    (void)y;
    (void)w;
    (void)h;

    row_bytes = surface->bitmap.pitch;
    if (row_bytes < 0) row_bytes = -row_bytes;
    
    if ( row_bytes*8 != pbmi->bmiHeader.biWidth * pbmi->bmiHeader.biBitCount )
      pbmi->bmiHeader.biWidth  = row_bytes * 8 / pbmi->bmiHeader.biBitCount;

    hDC = GetDC ( hwndGraphic );
    SetDIBits ( hDC, hbm,
                0,
                surface->bitmap.rows,
                surface->bitmap.buffer,
                pbmi,
                DIB_RGB_COLORS );
                
    ReleaseDC ( hwndGraphic, hDC );

    ShowWindow( hwndGraphic, SW_SHOW );
    InvalidateRect ( hwndGraphic, NULL, FALSE );
    UpdateWindow ( hwndGraphic );
  }

  static
  void set_title( grSurface* surface, const char* title )
  {
    (void)surface;
    
    /* the title will be set on the next listen_event, just */
    /* record it there..                                    */
    the_title = title;
    title_set = 0;
  }

  static
  void listen_event( grSurface*  surface,
                     int         event_mask,
                     grEvent*    grevent )
  {
    MSG  msg;
    
    (void)surface;
    (void)event_mask;
    
    if ( hwndGraphic && !title_set )
    {
      SetWindowText( hwndGraphic, the_title );
      title_set = 1;
    }

    eventToProcess = 0;    
    while (GetMessage( &msg, 0, 0, 0 ))
    {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
      if (eventToProcess)
        break;
    }
    
    *grevent = ourevent;
  }

/*
 * set graphics mode
 * and create the window class and the message handling.
 */

/* Declarations of the Windows-specific functions that are below. */
static BOOL RegisterTheClass ( void );
static BOOL CreateTheWindow  ( int width, int height );

static
grSurface*  init_surface( grSurface*  surface,
                          grBitmap*   bitmap )
{
  static RGBQUAD  black = {    0,    0,    0, 0 };
  static RGBQUAD  white = { 0xFF, 0xFF, 0xFF, 0 };
  
  if( ! RegisterTheClass() ) return 0;  /* if already running, fails. */

  /* find some memory for the bitmap header */
  if ( (pbmi = malloc ( sizeof ( BITMAPINFO ) + sizeof ( RGBQUAD ) * 256 ) )
                              /* 256 should really be 2 if not grayscale */
             == NULL )
    /* lack of memory; fails the process */
    return 0;

  LOG(( "Win32: init_surface( %08lx, %08lx )\n",
        (long)surface, (long)bitmap ));

  LOG(( "       -- input bitmap =\n" ));
  LOG(( "       --   mode   = %d\n", bitmap->mode ));
  LOG(( "       --   grays  = %d\n", bitmap->grays ));
  LOG(( "       --   width  = %d\n", bitmap->width ));
  LOG(( "       --   height = %d\n", bitmap->rows ));

  /* create the bitmap - under Win32, we support all modes as the GDI */
  /* handles all conversions automatically..                          */
  if ( grNewBitmap( bitmap->mode,
                    bitmap->grays,
  			        bitmap->width,
			        bitmap->rows,
                    bitmap ) )
    return 0;

  LOG(( "       -- output bitmap =\n" ));
  LOG(( "       --   mode   = %d\n", bitmap->mode ));
  LOG(( "       --   grays  = %d\n", bitmap->grays ));
  LOG(( "       --   width  = %d\n", bitmap->width ));
  LOG(( "       --   height = %d\n", bitmap->rows ));

  bitmap->pitch   = -bitmap->pitch;
  surface->bitmap = *bitmap;

  /* initialize the header to appropriate values */
  memset( pbmi, 0, sizeof ( BITMAPINFO ) + sizeof ( RGBQUAD ) * 256 );

  switch ( bitmap->mode )
  {
  case gr_pixel_mode_mono:
    pbmi->bmiHeader.biBitCount = 1;
    pbmi->bmiColors[0] = white;
    pbmi->bmiColors[1] = black;
    break;

  case gr_pixel_mode_gray:
    pbmi->bmiHeader.biBitCount = 8;
    pbmi->bmiHeader.biClrUsed  = bitmap->grays;
    {
      int   count = bitmap->grays;
      int   x;
      RGBQUAD*  color = pbmi->bmiColors;
      
      for ( x = 0; x < count; x++, color++ )
      {
        color->rgbRed   =
        color->rgbGreen =
        color->rgbBlue  = (((count-x)*255)/count);
        color->rgbReserved = 0;
      }
    }
    break;

  default:
    free ( pbmi );
    return 0;         /* Unknown mode */
  }

  pbmi->bmiHeader.biSize   = sizeof ( BITMAPINFOHEADER );
  pbmi->bmiHeader.biWidth  = bitmap->width;
  pbmi->bmiHeader.biHeight = bitmap->rows;
  pbmi->bmiHeader.biPlanes = 1;

  if( ! CreateTheWindow( bitmap->width, bitmap->rows ) )
  {
    free ( pbmi );
    return 0;
  }
  
  surface->done         = (grDoneSurfaceFunc) done_surface;
  surface->refresh_rect = (grRefreshRectFunc) refresh_rectangle;
  surface->set_title    = (grSetTitleFunc)    set_title;
  surface->listen_event = (grListenEventFunc) listen_event;

  return surface;
}


/* ---- Windows-specific stuff ------------------------------------------- */

LRESULT CALLBACK Message_Process( HWND, UINT, WPARAM, LPARAM );

static
BOOL RegisterTheClass ( void )
  {
  WNDCLASS ourClass = {
      /* UINT    style        */ 0,
      /* WNDPROC lpfnWndProc  */ Message_Process,
      /* int     cbClsExtra   */ 0,
      /* int     cbWndExtra   */ 0,
      /* HANDLE  hInstance    */ 0,
      /* HICON   hIcon        */ 0,
      /* HCURSOR hCursor      */ 0,
      /* HBRUSH  hbrBackground*/ 0,
      /* LPCTSTR lpszMenuName */ NULL,
      /* LPCTSTR lpszClassName*/ "FreeTypeTestGraphicDriver"
  };

    ourClass.hInstance    = GetModuleHandle( NULL );
    ourClass.hIcon        = LoadIcon(0, IDI_APPLICATION);
    ourClass.hCursor      = LoadCursor(0, IDC_ARROW);
    ourClass.hbrBackground= GetStockObject(BLACK_BRUSH);

    return RegisterClass(&ourClass) != 0;  /* return False if it fails. */
  }

static
BOOL CreateTheWindow ( int width, int height )
  {
    window_width  = width;
    window_height = height;

    if ( ! (hwndGraphic = CreateWindow(
        /* LPCSTR lpszClassName;    */ "FreeTypeTestGraphicDriver",
        /* LPCSTR lpszWindowName;   */ "FreeType Test Graphic Driver",
        /* DWORD dwStyle;           */  WS_OVERLAPPED | WS_SYSMENU,
        /* int x;                   */  CW_USEDEFAULT,
        /* int y;                   */  CW_USEDEFAULT,
        /* int nWidth;              */  width + 2*GetSystemMetrics(SM_CXBORDER),
        /* int nHeight;             */  height+ GetSystemMetrics(SM_CYBORDER)
                                              + GetSystemMetrics(SM_CYCAPTION),
        /* HWND hwndParent;         */  HWND_DESKTOP,
        /* HMENU hmenu;             */  0,
        /* HINSTANCE hinst;         */  GetModuleHandle( NULL ),
        /* void FAR* lpvParam;      */  NULL))
       )
         /*  creation failed... */
         return 0;

    return 1;
  }

  /* Message processing for our Windows class */
LRESULT CALLBACK Message_Process( HWND handle, UINT mess,
                                  WPARAM wParam, LPARAM lParam )
  {

    switch( mess )
    {
    case WM_DESTROY:
        /* warn the main thread to quit if it didn't know */
      ourevent.type  = gr_event_key;
      ourevent.key   = grKeyEsc;
      eventToProcess = 1;
      hwndGraphic    = 0;
      PostQuitMessage ( 0 );
      DeleteObject ( hbm );
      return 0;

    case WM_CREATE:
      {
        HDC     hDC;

        hDC = GetDC ( handle );
        hbm = CreateDIBitmap (
          /* HDC hdc;     handle of device context        */ hDC,
          /* BITMAPINFOHEADER FAR* lpbmih;  addr.of header*/ &pbmi->bmiHeader,
          /* DWORD dwInit;  CBM_INIT to initialize bitmap */ 0,
          /* const void FAR* lpvBits;   address of values */ NULL,
          /* BITMAPINFO FAR* lpbmi;   addr.of bitmap data */ pbmi,
          /* UINT fnColorUse;      RGB or palette indices */ DIB_RGB_COLORS);
        ReleaseDC ( handle, hDC );
        break;
      }

    case WM_PAINT:
      {
      HDC     hDC, memDC;
      HANDLE  oldbm;
      PAINTSTRUCT ps;

      hDC = BeginPaint ( handle, &ps );
      memDC = CreateCompatibleDC(hDC);
      oldbm = SelectObject(memDC, hbm);
      BitBlt ( hDC, 0, 0, window_width, window_height, memDC, 0, 0, SRCCOPY);
      ReleaseDC ( handle, hDC );
      SelectObject ( memDC, oldbm );
      DeleteObject ( memDC );
      EndPaint ( handle, &ps );
      return 0;
      }

    case WM_SYSKEYDOWN:
      {
        int          count = sizeof( syskey_translators )/sizeof( syskey_translators[0] );
        Translator*  trans = syskey_translators;
        Translator*  limit = trans + count;
        for ( ; trans < limit; trans++ )
          if ( wParam == trans->winkey )
          {
            ourevent.key = trans->grkey;
            goto Do_Key_Event;
          }
        return DefWindowProc( handle, mess, wParam, lParam );
      }

      
    case WM_KEYDOWN:
      switch ( wParam )
      {
      case VK_ESCAPE:
        ourevent.type  = gr_event_key;
        ourevent.key   = grKeyEsc;
        eventToProcess = 1;
        PostQuitMessage ( 0 );
        return 0;

      default:
        /* lookup list of translated keys */
        {
          int          count = sizeof( key_translators )/sizeof( key_translators[0] );
          Translator*  trans = key_translators;
          Translator*  limit = trans + count;
          for ( ; trans < limit; trans++ )
            if ( wParam == trans->winkey )
            {
              ourevent.key = trans->grkey;
              goto Do_Key_Event;
            }
        }
        
        /* the key isn't found, default processing               */
        /* return DefWindowProc( handle, mess, wParam, lParam ); */
        return DefWindowProc( handle, mess, wParam, lParam );
    }

    case WM_CHAR:
      {
        ourevent.key = wParam;
        
    Do_Key_Event:
        ourevent.type  = gr_event_key;
        eventToProcess = 1;
      }
      break;

    default:
       return DefWindowProc( handle, mess, wParam, lParam );
    }
    return 0;
  }

  static int init_device( void )
  {
    return 0;
  }

  static void done_device( void )
  {
  }

  grDevice  gr_win32_device =
  {
    sizeof( grSurface ),
    "win32",
    
    init_device,
    done_device,
    
    (grDeviceInitSurfaceFunc) init_surface,
    
    0,
    0
  };


/* End */
