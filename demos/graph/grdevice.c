#include "grobjs.h"
#include "grdevice.h"
#include <stdlib.h>
#include <string.h>

  grDeviceChain   gr_device_chain[ GR_MAX_DEVICES ];
  int             gr_num_devices = 0;

  static
  grDevice*  find_device( const char*  device_name )
  {
    int  index = 0;
    
    if (device_name)
    {
      for ( index = gr_num_devices-1; index > 0; index-- )
        if ( strcmp( device_name, gr_device_chain[index].name ) == 0 )
          break;
    }
    
    if ( index < 0 || gr_num_devices <= 0 || !gr_device_chain[index].device )
    {
      grError = gr_err_invalid_device;
      return 0;
    }
    
    return gr_device_chain[index].device;
  }



 /**********************************************************************
  *
  * <Function>
  *    grGetDeviceModes
  *
  * <Description>
  *    queries the available pixel modes for a device.
  *
  * <Input>
  *    device_name  :: name of device to be used. 0 for the default
  *                    device. For a list of available devices, see
  *                    grInitDevices.
  *
  * <Output>
  *    num_modes    :: number of available modes. 0 in case of error,
  *                    which really is an invalid device name.
  *
  *    pixel_modes  :: array of available pixel modes for this device
  *                    this table is internal to the device and should
  *                    not be freed by client applications.
  *
  * <Return>
  *    error code. 0 means success. invalid device name otherwise
  *
  * <Note>
  *    All drivers are _required_ to support at least the following
  *    pixel formats :
  *
  *    - gr_pixel_mode_mono : i.e. monochrome bitmaps
  *    - gr_pixel_mode_gray : with any number of gray levels between
  *                           2 and 256.
  *
  *    the pixel modes do not provide the number of grays in the case
  *    of "gray" devices. You should try to create a surface with the 
  *    maximal number (256, that is) and see the value returned in
  *    the bitmap descriptor.
  *
  **********************************************************************/
 
  extern void grGetDeviceModes( const char*    device_name,
                                int           *num_modes,
                                grPixelMode*  *pixel_modes )
  {
    grDevice*  device;
    
    *num_modes   = 0;
    *pixel_modes = 0;

    device = find_device( device_name );
    if (device)
    {
      *num_modes   = device->num_pixel_modes;
      *pixel_modes = device->pixel_modes;
    }
  }


 /**********************************************************************
  *
  * <Function>
  *    grNewSurface
  *
  * <Description>
  *    creates a new device-specific surface. A surface is either
  *    a window or a screen, depending on the device.
  *
  * <Input>
  *    device  :: name of the device to use. A value of NULL means
  *               the default device (which depends on the system).
  *               for a list of available devices, see grInitDevices.
  *
  * <InOut>
  *    bitmap  :: handle to a bitmap descriptor containing the
  *               requested pixel mode, number of grays and dimensions
  *               for the surface. the bitmap's 'pitch' and 'buffer'
  *               fields are ignored on input.
  *
  *               On output, the bitmap describes the surface's image
  *               completely. It is possible to write directly in it
  *               with grBlitGlyphToBitmap, even though the use of
  *               grBlitGlyphToSurface is recommended.
  *
  * <Return>
  *    handle to the corresponding surface object. 0 in case of error
  *
  * <Note>
  *    All drivers are _required_ to support at least the following
  *    pixel formats :
  *
  *    - gr_pixel_mode_mono : i.e. monochrome bitmaps
  *    - gr_pixel_mode_gray : with any number of gray levels between
  *                           2 and 256.
  *
  *    This function might change the bitmap descriptor's fields. For
  *    example, when displaying a full-screen surface, the bitmap's
  *    dimensions will be set to those of the screen (e.g. 640x480
  *    or 800x600); also, the bitmap's 'buffer' field might point to
  *    the Video Ram depending on the mode requested..
  *
  *    The surface contains a copy of the returned bitmap descriptor,
  *    you can thus discard the 'bitmap' parameter after the call.
  *
  **********************************************************************/
    
  extern grSurface*  grNewSurface( const char*  device_name,
                                   grBitmap*    bitmap )
  {
    grDevice*   device;
    grSurface*  surface;

    /* Now find the device */
    device = find_device( device_name );
    if (!device) return 0;

    surface = (grSurface*)grAlloc( device->surface_objsize );
    if (!surface) return 0;
    
    if ( !device->init_surface( surface, bitmap ) )
    {
      grFree( surface );
      surface = 0;
    }
    return surface;
  }


 /**********************************************************************
  *
  * <Function>
  *    grRefreshRectangle
  *
  * <Description>
  *    this function is used to indicate that a given surface rectangle
  *    was modified and thus needs re-painting. It really is useful for
  *    windowed or gray surfaces.
  *
  * <Input>
  *    surface :: handle to target surface
  *    x       :: x coordinate of the top-left corner of the rectangle
  *    y       :: y coordinate of the top-left corner of the rectangle
  *    width   :: rectangle width in pixels
  *    height  :: rectangle height in pixels
  *
  **********************************************************************/
    
  extern void  grRefreshRectangle( grSurface*  surface,
                                   grPos       x,
                                   grPos       y,
                                   grPos       width,
                                   grPos       height )
  {
    if (surface->refresh_rect)
      surface->refresh_rect( surface, x, y, width, height );
  }


 
 /**********************************************************************
  *
  * <Function>
  *    grWriteSurfaceChar
  *
  * <Description>
  *    This function is equivalent to calling grWriteCellChar on the
  *    surface's bitmap, then invoking grRefreshRectangle.
  *
  *    The graphics sub-system contains an internal Latin1 8x8 font
  *    which can be used to display simple strings of text without
  *    using FreeType.
  *
  *    This function writes a single 8x8 character on the target bitmap.
  *
  * <Input>
  *    target   :: handle to target surface
  *    x        :: x pixel position of character cell's top left corner
  *    y        :: y pixel position of character cell's top left corner
  *    charcode :: Latin-1 character code
  *    color    :: color to be used to draw the character
  *
  **********************************************************************/
  
  extern
  void  grWriteSurfaceChar( grSurface* target,
                            int        x,
                            int        y,
                            int        charcode,
                            grColor    color )
  {
    grWriteCellChar( &target->bitmap, x, y, charcode, color );
    if (target->refresh_rect)
      target->refresh_rect( target, x, y, 8, 8 );
  }


 /**********************************************************************
  *
  * <Function>
  *    grWriteSurfaceString
  *
  * <Description>
  *    This function is equivalent to calling grWriteCellString on the
  *    surface's bitmap, then invoking grRefreshRectangle.
  *
  *    The graphics sub-system contains an internal Latin1 8x8 font
  *    which can be used to display simple strings of text without
  *    using FreeType.
  *
  *    This function writes a string with the internal font
  *
  * <Input>
  *    target       :: handle to target bitmap  
  *    x            :: x pixel position of string's top left corner
  *    y            :: y pixel position of string's top left corner
  *    string       :: Latin-1 text string
  *    color        :: color to be used to draw the character
  *
  **********************************************************************/
  
  extern
  void  grWriteSurfaceString( grSurface*  target,
                              int         x,
                              int         y,
                              const char* string,
                              grColor     color )
  {
    int  len;

    len = strlen(string);
    grWriteCellString( &target->bitmap, x, y, string, color );
    if (target->refresh_rect)
      target->refresh_rect( target, x, y, 8*len, 8 );
  }


 /**********************************************************************
  *
  * <Function>
  *    grRefreshSurface
  *
  * <Description>
  *    a variation of grRefreshRectangle which repaints the whole surface
  *    to the screen.                                                    
  *
  * <Input>
  *    surface :: handle to target surface
  *
  **********************************************************************/

  extern void  grRefreshSurface( grSurface*  surface )
  {
    if (surface->refresh_rect)
      surface->refresh_rect( surface, 0, 0,
                             surface->bitmap.width,
                             surface->bitmap.rows );
  }



 /**********************************************************************
  *
  * <Function>
  *    grSetTitle
  *
  * <Description>
  *    set the window title of a given windowed surface.
  *
  * <Input>
  *    surface      :: handle to target surface
  *    title_string :: the new title
  *
  **********************************************************************/

  extern void  grSetTitle( grSurface*  surface,
                           const char* title_string )
  {
    if (surface->set_title)
      surface->set_title( surface, title_string );
  }




 /**********************************************************************
  *
  * <Function>
  *    grListenSurface
  *
  * <Description>
  *    listen the events for a given surface
  *
  * <Input>
  *    surface    :: handle to target surface
  *    event_mask :: the event mask (mode)
  *
  * <Output>
  *    event  :: the returned event
  *
  * <Note>
  *    XXX : For now, only keypresses are supported.
  *
  **********************************************************************/
  
  extern
  int   grListenSurface( grSurface*  surface,
                         int         event_mask,
                         grEvent    *event )
  {
    return surface->listen_event( surface, event_mask, event );
  }


#if 0
  static
  void  gr_done_surface( grSurface*  surface )
  {
    if (surface)
    {
      /* first of all, call the device-specific destructor */
      surface->done(surface);
      
      /* then remove the bitmap if we're owner */
      if (surface->owner)
        grFree( surface->bitmap.buffer );
        
      surface->owner         = 0;  
      surface->bitmap.buffer = 0;
      grFree( surface );
    }
  }
#endif

