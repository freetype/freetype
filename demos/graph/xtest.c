#include "graph.h"
#include "grfont.h"  /* dispara^itra bientot */
#include <stdio.h>


static
void Panic( const char*  message )
{
  fprintf( stderr, "PANIC: %s\n", message );
  exit(1);
}


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
      grListenSurface( surface, 0, &event);

      /* return if ESC was pressed */
      if ( event.key == grKeyEsc )
        return 0;

      /* otherwise, display key string */
      color.value = (color.value + 8) & 127;
      {
        int         count = sizeof(key_names)/sizeof(key_names[0]);
        grKeyName*  name  = (grKeyName*)key_names;
        grKeyName*  limit = name + count;
        const char* kname  = 0;
        char        kname_temp[16];

        while (name < limit)
        {
          if ( name->key == event.key )
          {
            kname = (const char*)name->name;
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
      }
    } while (1);
  }

  return 0;
}

