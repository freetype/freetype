#include "gros2pm.h"
#include "grdevice.h"

#define INCL_DOS
#define INCL_WIN
#define INCL_GPI
#define INCL_SUB

#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

  typedef struct Translator
  {
    ULONG   os2key;
    grKey   grkey;
    
  } Translator;
  

  static
  Translator  key_translators[] =
  {
    { VK_BACKSPACE, grKeyBackSpace },
    { VK_TAB,       grKeyTab       },
    { VK_ENTER,     grKeyReturn    },
    { VK_ESC,       grKeyEsc       },
    { VK_HOME,      grKeyHome      },
    { VK_LEFT,      grKeyLeft      },
    { VK_UP,        grKeyUp        },
    { VK_RIGHT,     grKeyRight     },
    { VK_DOWN,      grKeyDown      },
    { VK_PAGEUP,    grKeyPageUp    },
    { VK_PAGEDOWN,  grKeyPageDown  },
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


#define MAX_PIXEL_MODES  32

  static  HAB   gr_anchor;   /* device anchor block */

  typedef POINTL  PMBlitPoints[4];

  typedef struct grPMSurface_
  {
    grSurface  root;
    grBitmap   image;

    HAB        anchor;         /* handle to anchor block for surface's window */
    HWND       frame_window;   /* handle to window's frame                    */
    HWND       client_window;  /* handle to window's client                   */
    HWND       title_window;   /* handle to window's title bar                */

    HPS        image_ps;       /* memory presentation space used to hold */
                               /* the surface's content under PM         */
    HDC        image_dc;       /* memory device context for the image    */

    HEV        event_lock;     /* semaphore used in listen_surface   */
    HMTX       image_lock;     /* a mutex used to synchronise access */
                               /* to the memory presentation space   */
                               /* used to hold the surface           */

    TID        message_thread; /* thread used to process this surface's */
                               /* messages..                            */

    PBITMAPINFO2 bitmap_header;/* os/2 bitmap descriptor                   */
    HBITMAP      os2_bitmap;   /* Handle to OS/2 bitmap contained in image */
    BOOL         ready;        /* ??? */

    long         shades[256];  /* indices of gray levels in pixel_mode_gray */

    POINTL       surface_blit[4];  /* surface blitting table   */
    POINTL       magnify_blit[4];  /* magnifier blitting table */
    int          magnification;    /* level of magnification   */
    POINTL       magnify_center;
    SIZEL        magnify_size;

    grEvent      event;

    PMBlitPoints blit_points;

  } grPMSurface;

  /* we use a static variable to pass a pointer to the PM Surface  */
  /* to the client window. This is a bit ugly, but it makes things */
  /* a lot more simple..                                           */
  static  grPMSurface*  the_surface;

  static  int window_created = 0;


  static
  void  enable_os2_iostreams( void )
  {
    PTIB  thread_block;
    PPIB  process_block;

    /* XXX : This is a very nasty hack, it fools OS/2 and let the program */
    /*       call PM functions, even though stdin/stdout/stderr are still */
    /*       directed to the standard i/o streams..                       */
    /*       The program must be compiled with WINDOWCOMPAT               */
    /*                                                                    */
    /*   Credits go to Michal for finding this !!                         */
    /*                                                                    */
    DosGetInfoBlocks( &thread_block, &process_block );
    process_block->pib_ultype = 3;
  }



  static
  int  init_device( void )
  {
    enable_os2_iostreams();

    /* create an anchor block. This will allow this thread (i.e. the */
    /* main one) to call Gpi functions..                             */
    gr_anchor = WinInitialize(0);
    if (!gr_anchor)
    {
      /* could not initialise Presentation Manager */
      return -1;
    }

    return 0;
  }



  static
  void  done_device( void )
  {
    /* Indicates that we do not use the Presentation Manager, this */
    /* will also release all associated resources..                */
    WinTerminate( gr_anchor );
  }



  /* close a given window */
  static
  void  done_surface( grPMSurface*  surface )
  {
    LOG(( "Os2PM: done_surface(%08lx)\n", (long)surface ));

    if ( surface->frame_window )
      WinDestroyWindow( surface->frame_window );

    WinReleasePS( surface->image_ps );

    grDoneBitmap( &surface->image );
    grDoneBitmap( &surface->root.bitmap );
  }





#define LOCK(x)    DosRequestMutexSem( x, SEM_INDEFINITE_WAIT );
#define UNLOCK(x)  DosReleaseMutexSem( x )


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


 /************************************************************************
  *
  * Technical note : how the OS/2 Presntation Manager driver works
  *
  * PM is, in my opinion, a bloated and over-engineered graphics
  * sub-system, even though it has lots of nice features. Here are
  * a few tidbits about it :
  *
  *
  * - under PM, a "bitmap" is a device-specific object whose bits are
  *   not directly accessible to the client application. This means
  *   that we must use a scheme like the following to display our
  *   surfaces :
  *
  *     - hold, for each surface, its own bitmap buffer where the
  *       rest of the graph library writes directly.
  *
  *     - create a PM bitmap object with the same dimensions (and
  *       possibly format).
  *
  *     - copy the content of each updated rectangle into the
  *       PM bitmap with the function 'GpiSetBitmapBits'.
  *
  *     - finally, "blit" the PM bitmap to the screen calling
  *       'GpiBlitBlt'
  *
  * - but there is more : you cannot directly blit a PM bitmap to the
  *   screen with PM. The 'GpiBlitBlt' only works with presentation
  *   spaces. This means that we also need to create, for each surface :
  *
  *     - a memory presentation space, used to hold the PM bitmap
  *     - a "memory device context" for the presentation space
  *
  *   The blit is then performed from the memory presentation space
  *   to the screen's presentation space..
  *
  *
  * - because each surface creates its own event-handling thread,
  *   we must protect the surface's presentation space from concurrent
  *   accesses (i.e. calls to 'GpiSetBitmapBits' when drawing to the
  *   surface, and calls to 'GpiBlitBlt' when drawing it on the screen
  *   are performed in two different threads).
  *
  *   we use a simple mutex to do this.
  *
  *
  * - we also use a semaphore to perform a rendez-vous between the
  *   main and event-handling threads (needed in "listen_event").
  *
  ************************************************************************/

  static
  void  RunPMWindow( grPMSurface*  surface );




  static
  void  refresh_rectangle( grPMSurface* surface,
                           int          x,
                           int          y,
                           int          w,
                           int          h )
  {
    LOG(( "Os2PM: refresh_rectangle( %08lx, %d, %d, %d, %d )\n",
          (long)surface, x, y, w, h ));

    (void)x;
    (void)y;
    (void)w;
    (void)h;

    /*
    convert_rectangle( surface, x, y, w, h );
    */
    DosRequestMutexSem( surface->image_lock, SEM_INDEFINITE_WAIT );
    GpiSetBitmapBits( surface->image_ps,
                      0,
                      surface->root.bitmap.rows,
                      surface->root.bitmap.buffer,
                      surface->bitmap_header );
    DosReleaseMutexSem( surface->image_lock );

    WinInvalidateRect( surface->client_window, NULL, FALSE );
    WinUpdateWindow( surface->frame_window );
  }

  
  static
  void  set_title( grPMSurface* surface,
                   const char*  title )
  {
    ULONG  rc;

#if 1
    LOG(( "Os2PM: set_title( %08lx == %08lx, %s )\n",
             (long)surface, surface->client_window, title ));
#endif
    LOG(( "      -- frame         = %08lx\n",
          (long)surface->frame_window ));

    LOG(( "      -- client parent = %08lx\n",
          (long)WinQueryWindow( surface->client_window, QW_PARENT ) ));

    rc = WinSetWindowText( surface->client_window, (PSZ)title );
    LOG(( "      -- returned rc = %ld\n",rc ));
  }



  static  
  void  listen_event( grPMSurface* surface,
                      int          event_mask,
                      grEvent*     grevent )
  {
    ULONG  ulRequestCount;

    (void) event_mask;   /* ignored for now */

    /* the listen_event function blocks until there is an event to process */
    DosWaitEventSem( surface->event_lock, SEM_INDEFINITE_WAIT );
    DosQueryEventSem( surface->event_lock, &ulRequestCount );
    *grevent = surface->event;
    DosResetEventSem( surface->event_lock, &ulRequestCount );

    return;
  }


  static
  grPMSurface*  init_surface( grPMSurface*  surface,
                              grBitmap*     bitmap )
  {
    PBITMAPINFO2  bit;
    SIZEL         sizl = { 0, 0 };
    LONG          palette[256];

    LOG(( "Os2PM: init_surface( %08lx, %08lx )\n",
          (long)surface, (long)bitmap ));

    LOG(( "       -- input bitmap =\n" ));
    LOG(( "       --   mode   = %d\n", bitmap->mode ));
    LOG(( "       --   grays  = %d\n", bitmap->grays ));
    LOG(( "       --   width  = %d\n", bitmap->width ));
    LOG(( "       --   height = %d\n", bitmap->rows ));

    /* create the bitmap - under OS/2, we support all modes as PM */
    /* handles all conversions automatically..                    */
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

    bitmap->pitch = -bitmap->pitch;
    surface->root.bitmap = *bitmap;

    /* create the image and event lock */
    DosCreateEventSem( NULL, &surface->event_lock, 0, TRUE  );
    DosCreateMutexSem( NULL, &surface->image_lock, 0, FALSE );

    /* create the image's presentation space */
    surface->image_dc = DevOpenDC( gr_anchor,
                                   OD_MEMORY, (PSZ)"*", 0L, 0L, 0L );

    surface->image_ps = GpiCreatePS( gr_anchor,
                                     surface->image_dc,
                                     &sizl,
                                     PU_PELS    | GPIT_MICRO |
                                     GPIA_ASSOC | GPIF_DEFAULT );

    GpiSetBackMix( surface->image_ps, BM_OVERPAINT );

    /* create the image's PM bitmap */
    bit = (PBITMAPINFO2)grAlloc( sizeof(BITMAPINFO2) + 256*sizeof(RGB2) );
    surface->bitmap_header = bit;

    bit->cbFix   = sizeof( BITMAPINFOHEADER2 );
    bit->cx      = surface->root.bitmap.width;
    bit->cy      = surface->root.bitmap.rows;
    bit->cPlanes = 1;

    bit->argbColor[0].bBlue  = 255;
    bit->argbColor[0].bGreen = 0;
    bit->argbColor[0].bRed   = 0;

    bit->argbColor[1].bBlue  = 0;
    bit->argbColor[1].bGreen = 255;
    bit->argbColor[1].bRed   = 0;

    bit->cBitCount = (bitmap->mode == gr_pixel_mode_gray ? 8 : 1 );

    if (bitmap->mode == gr_pixel_mode_gray)
    {
      RGB2*  color = bit->argbColor;
      int    x, count;

      count = bitmap->grays;
      for ( x = 0; x < count; x++, color++ )
      {
        color->bBlue  =
        color->bGreen =
        color->bRed   = (((count-x)*255)/count);
      }
    }
    else
    {
      RGB2*  color = bit->argbColor;

      color[0].bBlue  =
      color[0].bGreen =
      color[0].bRed   = 0;

      color[1].bBlue  =
      color[1].bGreen =
      color[1].bRed   = 255;
    }

    surface->os2_bitmap = GpiCreateBitmap( surface->image_ps,
                                           (PBITMAPINFOHEADER2)bit,
                                           0L, NULL, NULL );

    GpiSetBitmap( surface->image_ps, surface->os2_bitmap );

    bit->cbFix = sizeof( BITMAPINFOHEADER2 );
    GpiQueryBitmapInfoHeader( surface->os2_bitmap,
                              (PBITMAPINFOHEADER2)bit );
    surface->bitmap_header = bit;

    /* for gr_pixel_mode_gray, create a gray-levels logical palette */
    if ( bitmap->mode == gr_pixel_mode_gray )
    {
      int     x, count;

      count = bitmap->grays;
      for ( x = 0; x < count; x++ )
        palette[x] = (((count-x)*255)/count) * 0x010101;

      /* create logical color table */
      GpiCreateLogColorTable( surface->image_ps,
                              (ULONG) LCOL_PURECOLOR,
                              (LONG)  LCOLF_CONSECRGB,
                              (LONG)  0L,
                              (LONG)  count,
                              (PLONG) palette );

      /* now, copy the color indexes to surface->shades */
      for ( x = 0; x < count; x++ )
        surface->shades[x] = GpiQueryColorIndex( surface->image_ps,
                                                 0, palette[x] );
    }

    /* set up the blit points array */
    surface->blit_points[1].x = surface->root.bitmap.width;
    surface->blit_points[1].y = surface->root.bitmap.rows;
    surface->blit_points[3]   = surface->blit_points[1];

    window_created = 0;

    /* Finally, create the event handling thread for the surface's window */
    DosCreateThread( &surface->message_thread,
                     (PFNTHREAD) RunPMWindow,
                     (ULONG)     surface,
                     0UL,
                     32920 );

    /* wait for the window creation */
    for ( ; window_created == 0; )

    surface->root.done         = (grDoneSurfaceFunc) done_surface;
    surface->root.refresh_rect = (grRefreshRectFunc) refresh_rectangle;
    surface->root.set_title    = (grSetTitleFunc)    set_title;
    surface->root.listen_event = (grListenEventFunc) listen_event;

    /* convert_rectangle( surface, 0, 0, bitmap->width, bitmap->rows ); */
    return surface;
  }



  MRESULT EXPENTRY  Message_Process( HWND    handle,
                                     ULONG   mess,
                                     MPARAM  parm1,
                                     MPARAM  parm2 );


  static
  void  RunPMWindow( grPMSurface*  surface )
  {
    unsigned char   class_name[] = "DisplayClass";
             ULONG  class_flags;

    static   HMQ    queue;
             QMSG   message;

    /* store the current surface pointer in "the_surface". It is a static */
    /* variable that is only used to retrieve the pointer in the client   */
    /* window procedure the first time is is called..                     */
    the_surface = surface;

    LOG(( "Os2PM: RunPMWindow( %08lx )\n", (long)surface ));
     
    /* create an anchor to allow this thread to use PM */
    surface->anchor = WinInitialize(0);
    if (!surface->anchor)
    {
      printf( "Error doing WinInitialize()\n" );
      return;
    }

    /* create a message queue */
    queue = WinCreateMsgQueue( surface->anchor, 0 );
    if (!queue)
    {
      printf( "Error doing >inCreateMsgQueue()\n" );
      return;
    }

    /* register the window class */
    if ( !WinRegisterClass( surface->anchor,
                            (PSZ)   class_name,
                            (PFNWP) Message_Process,
                            CS_SIZEREDRAW,
                            0 ) )
    {
      printf( "Error doing WinRegisterClass()\n" );
      return;
    }

    /* create the PM window */
    class_flags = FCF_TITLEBAR | FCF_MINBUTTON | FCF_DLGBORDER | 
                  FCF_TASKLIST | FCF_SYSMENU; 

    LOG(( "Os2PM: RunPMWindow: Creating window\n" ));
    surface->frame_window = WinCreateStdWindow(
                                HWND_DESKTOP,
                                WS_VISIBLE,
                                &class_flags,
                                (PSZ) class_name,
                                (PSZ) "FreeType Viewer - press F1 for help",
                                WS_VISIBLE,
                                0, 0,
                                &surface->client_window );
    if (!surface->frame_window)
    {
      printf( "Error doing WinCreateStdWindow()\n" );
      return;
    }

    /* find the title window handle */
    surface->title_window = WinWindowFromID( surface->frame_window,
                                             FID_TITLEBAR );
    LOG (( "Os2PM: RunPMWIndow: Creation succeeded\n" ));
    LOG (( "    -- frame  = %08lx\n", surface->frame_window ));
    LOG (( "    -- client = %08lx\n", surface->client_window ));

    /* set Window size and position */
    WinSetWindowPos( surface->frame_window,
                     0L,
                     (SHORT) 60,

                     (SHORT) WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN ) -
                             (surface->root.bitmap.rows + 100),

                     (SHORT) WinQuerySysValue( HWND_DESKTOP, SV_CYDLGFRAME )*2 +
                             surface->root.bitmap.width,

                     (SHORT) WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR ) +
                             WinQuerySysValue( HWND_DESKTOP, SV_CYDLGFRAME )*2 +
                             surface->root.bitmap.rows,

                     SWP_SIZE | SWP_MOVE );

#if 0
    /* save the handle to the current surface within the window words */
    WinSetWindowPtr( surface->client_window,QWL_USER, surface );
#endif

    window_created = 1;

    /* run the message queue till the end */
    while ( WinGetMsg( surface->anchor, &message, (HWND)NULL, 0, 0 ) )
    {
      WinDispatchMsg( surface->anchor, &message );
    }

    /* clean-up */
    WinDestroyWindow( surface->frame_window );
    surface->frame_window = 0;

    WinDestroyMsgQueue( queue );
    WinTerminate( surface->anchor );

    /* await death... */
    while ( 1 )
      DosSleep( 100 );
  }




  /* Message processing for our PM Window class */
  MRESULT EXPENTRY  Message_Process( HWND    handle,
                                     ULONG   mess,
                                     MPARAM  parm1,
                                     MPARAM  parm2 )
  {
     static HDC     screen_dc;
     static HPS     screen_ps;
     static BOOL    minimized;

     SIZEL   sizl;
     SWP     swp;

     grPMSurface*  surface;

    /* get the handle to the window's surface -- note that this */
    /* value will be null when the window is created            */
    surface = (grPMSurface*)WinQueryWindowPtr( handle, QWL_USER );
    if (!surface)
    {
      surface = the_surface;
      WinSetWindowPtr( handle, QWL_USER, surface );
    }

    switch( mess )
    {
    case WM_DESTROY:
      /* warn the main thread to quit if it didn't know */
      surface->event.type = gr_event_key;
      surface->event.key  = grKeyEsc;
      DosPostEventSem( surface->event_lock );
      break;

    case WM_CREATE:
      /* set original magnification */
      minimized = FALSE;

      /* create Device Context and Presentation Space for screen. */
      screen_dc = WinOpenWindowDC( handle );
      screen_ps = GpiCreatePS( surface->anchor,
                               screen_dc,
                               &sizl,
                               PU_PELS | GPIT_MICRO |
                               GPIA_ASSOC | GPIF_DEFAULT );
      /* take the input focus */
      WinFocusChange( HWND_DESKTOP, handle, 0L );
      LOG(( "screen_dc and screen_ps have been created\n" ));
      break;

    case WM_MINMAXFRAME:
      /* to update minimized if changed */
      swp = *((PSWP) parm1);
      if ( swp.fl & SWP_MINIMIZE ) 
        minimized = TRUE;
      if ( swp.fl & SWP_RESTORE )
        minimized = FALSE;
      return WinDefWindowProc( handle, mess, parm1, parm2 );
      break;

    case WM_ERASEBACKGROUND:
    case WM_PAINT:  
      /* copy the memory image of the screen out to the real screen */
      DosRequestMutexSem( surface->image_lock, SEM_INDEFINITE_WAIT );
      WinBeginPaint( handle, screen_ps, NULL );
      
      /* main image and magnified picture */
      GpiBitBlt( screen_ps,
                 surface->image_ps,
                 4L,
                 surface->blit_points,
                 ROP_SRCCOPY, BBO_AND );

      WinEndPaint( screen_ps );
      DosReleaseMutexSem( surface->image_lock );   
      break;

    case WM_HELP:  /* this really is a F1 Keypress !! */
      surface->event.key = grKeyF1;
      goto Do_Key_Event;

    case WM_CHAR:
      if ( CHARMSG( &mess )->fs & KC_KEYUP )
        break;

      /* look for a specific vkey */
      {
        int          count = sizeof( key_translators )/sizeof( key_translators[0] );
        Translator*  trans = key_translators;
        Translator*  limit = trans + count;

        for ( ; trans < limit; trans++ )
          if ( CHARMSG(&mess)->vkey == trans->os2key )
          {
            surface->event.key = trans->grkey;
            goto Do_Key_Event;
          }
      }

      /* otherwise, simply record the character code */
      if ( (CHARMSG( &mess )->fs & KC_CHAR) == 0 )
        break;

      surface->event.key = CHARMSG(&mess)->chr;

    Do_Key_Event:
      surface->event.type = gr_event_key;
      DosPostEventSem( surface->event_lock );
      break;

    default:
      return WinDefWindowProc( handle, mess, parm1, parm2 );
    }

    return (MRESULT) FALSE;
  }




  grDevice  gr_os2pm_device =
  {
    sizeof( grPMSurface ),
    "os2pm",
    
    init_device,
    done_device,
    
    (grDeviceInitSurfaceFunc) init_surface,
    
    0,
    0
    
  };


