/***************************************************************************
 *
 *  graph.h
 *
 *    Graphics Subsystem interface
 *
 *  Copyright 1999 - The FreeType Development Team - www.freetype.org
 *
 *
 *
 *
 ***************************************************************************/

#ifndef GRAPH_H
#define GRAPH_H

#include "grevents.h"

 /*************************************************************************/
 /*************************************************************************/
 /*************************************************************************/
 /********                                                         ********/
 /********         GENERAL DEFINITIONS AND BLITTING ROUTINES       ********/
 /********                                                         ********/
 /********                                                         ********/
 /*************************************************************************/
 /*************************************************************************/
 /*************************************************************************/


  /* define the global error variable */
  extern int  grError;

  /* initialisation */
  extern int  grInit( void );
  
  /* finalisation */
  extern void grDone( void );


  /* pixel mode constants */
  typedef enum grPixelMode
  {
    gr_pixel_mode_none = 0,
    gr_pixel_mode_mono,        /* monochrome bitmaps               */
    gr_pixel_mode_pal4,        /* 4-bit paletted - 16 colors       */
    gr_pixel_mode_pal8,        /* 8-bit paletted - 256 colors      */
    gr_pixel_mode_gray,        /* 8-bit gray levels                */
    gr_pixel_mode_rgb555,      /* 15-bits mode - 32768 colors      */
    gr_pixel_mode_rgb565,      /* 16-bits mode - 65536 colors      */
    gr_pixel_mode_rgb24,       /* 24-bits mode - 16 million colors */
    gr_pixel_mode_rgb32,       /* 32-bits mode - 16 million colors */

    gr_pixel_mode_max          /* don't remove */
  
  } grPixelMode;


  /* forward declaration of the surface class */
  typedef struct grSurface_     grSurface;
  

 /*********************************************************************
  *
  * <Struct>
  *   grBitmap
  *
  * <Description>
  *   a simple bitmap descriptor
  *
  * <Fields>
  *   rows   :: height in pixels
  *   width  :: width in pixels
  *   pitch  :: + or - the number of bytes per row
  *   mode   :: pixel mode of bitmap buffer
  *   grays  :: number of grays in palette for PAL8 mode. 0 otherwise
  *   buffer :: pointer to pixel buffer
  *
  * <Note>
  *   the 'pitch' is positive for downward flows, and negative otherwise
  *   Its absolute value is always the number of bytes taken by each
  *   bitmap row.
  *
  *   All drawing operations will be performed within the first
  *   "width" pixels of each row (clipping is always performed).
  *
  ********************************************************************/

  typedef struct grBitmap_
  {
    int          rows;
    int          width;
    int          pitch;
    grPixelMode  mode;
    int          grays;
    char*        buffer;

  } grBitmap;



  typedef long   grPos;
  typedef char   grBool;

  typedef struct grVector_
  {
    grPos  x;
    grPos  y;
    
  } grVector;


  typedef union grColor_
  {
    long           value;
    unsigned char  chroma[4];
    
  } grColor;



 /**********************************************************************
  *
  * <Function>
  *    grNewBitmap
  *
  * <Description>
  *    creates a new bitmap    
  *
  * <Input>
  *    pixel_mode   :: the target surface's pixel_mode
  *    num_grays    :: number of grays levels for PAL8 pixel mode
  *    width        :: width in pixels
  *    height       :: height in pixels
  *
  * <Output>
  *    bit          :: descriptor of the new bitmap
  *
  * <Return>
  *    Error code. 0 means success.
  *
  * <Note>
  *    This function really allocates a pixel buffer, zero it, then
  *    returns a descriptor for it.
  *
  *    Call grDoneBitmap when you're done with it..
  *
  **********************************************************************/

  extern  int  grNewBitmap( grPixelMode  pixel_mode,
                            int          num_grays,
                            int          width,
                            int          height,
                            grBitmap    *bit );


 /**********************************************************************
  *
  * <Function>
  *    grBlitGlyphToBitmap
  *
  * <Description>
  *    writes a given glyph bitmap to a target surface.
  *
  * <Input>
  *    target  :: handle to target bitmap 
  *    glyph   :: handle to source glyph bitmap
  *    x       :: position of left-most pixel of glyph image in target surface
  *    y       :: position of top-most pixel of glyph image in target surface
  *    color   :: color to be used to draw a monochrome glyph
  *
  * <Return>
  *   Error code. 0 means success
  *
  * <Note>
  *   There are only two supported source pixel modes : monochrome
  *   and gray. The 8-bit images can have any number of grays between
  *   2 and 128, and conversions to the target surface is handled
  *   _automatically_.
  *
  *   Note however that you should avoid blitting a gray glyph to a gray
  *   bitmap with fewer levels of grays, as this would much probably
  *   give unpleasant results..
  *
  *   This function performs clipping
  *
  **********************************************************************/
    
  extern int   grBlitGlyphToBitmap( grBitmap*  target,
                                    grBitmap*  glyph,
                                    grPos      x,
                                    grPos      y,
                                    grColor    color );


 /**********************************************************************
  *
  * <Function>
  *    grFillRectangle
  *
  * <Description>
  *    this function is used to fill a given rectangle on a surface   
  *
  * <Input>
  *    surface :: handle to target surface
  *    x       :: x coordinate of the top-left corner of the rectangle
  *    y       :: y coordinate of the top-left corner of the rectangle
  *    width   :: rectangle width in pixels
  *    height  :: rectangle height in pixels
  *    color   :: fill color
  *
  **********************************************************************/
    
  extern void  grFillRectangle( grBitmap*  surface,
                                grPos      x,
                                grPos      y,
                                grPos      width,
                                grPos      height,
                                grColor    color );



 /**********************************************************************
  *
  * <Function>
  *    grWriteCellChar
  *
  * <Description>
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
  void  grWriteCellChar( grBitmap*  target,
                         int        x,
                         int        y,
                         int        charcode,
                         grColor    color );


 /**********************************************************************
  *
  * <Function>
  *    grWriteCellString
  *
  * <Description>
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
  void  grWriteCellString( grBitmap*   target,
                           int         x,
                           int         y,
                           const char* string,
                           grColor     color );

 /**********************************************************************
  *
  * <Function>
  *    grDoneBitmap
  *
  * <Description>
  *    destroys a bitmap    
  *
  * <Input>
  *    bitmap :: handle to bitmap descriptor
  *
  * <Note>
  *    This function does NOT release the bitmap descriptor, only
  *    the pixel buffer.
  *
  **********************************************************************/

  extern  void  grDoneBitmap( grBitmap*  bit );




 /*************************************************************************/
 /*************************************************************************/
 /*************************************************************************/
 /********                                                         ********/
 /********         DEVICE-SPECIFIC DEFINITIONS AND ROUTINES        ********/
 /********                                                         ********/
 /********                                                         ********/
 /*************************************************************************/
 /*************************************************************************/
 /*************************************************************************/


  /* forward declaration - the definition of grDevice is not visible */
  /* to clients..                                                    */
  typedef struct grDevice_  grDevice;


 /**********************************************************************
  *
  * <Struct>
  *    grDeviceChain
  *
  * <Description>
  *    a simple structure used to implement a linked list of
  *    graphics device descriptors. The list is called a
  *    "device chain"
  *
  * <Fields>
  *    name   :: ASCII name of the device, e.g. "x11", "os2pm", etc..
  *    device :: handle to the device descriptor.
  *    next   :: next element in chain
  *
  * <Note>
  *    the 'device' field is a blind pointer; it is thus unusable by
  *    client applications..
  *
  **********************************************************************/
    
  typedef struct grDeviceChain_  grDeviceChain;

  struct grDeviceChain_
  {
    const char*     name;
    grDevice*       device;
    grDeviceChain*  next;
  };


 /**********************************************************************
  *
  * <Function>
  *    grInitDevices
  *
  * <Description>
  *    This function is in charge of initialising all system-specific
  *    devices. A device is responsible for creating and managing one
  *    or more "surfaces". A surface is either a window or a screen,
  *    depending on the system.
  *
  * <Return>
  *    a pointer to the first element of a device chain. The chain can
  *    be parsed to find the available devices on the current system
  *
  * <Note>
  *    If a device cannot be initialised correctly, it is not part of
  *    the device chain returned by this function. For example, if an
  *    X11 device was compiled in the library, it will be part of
  *    the returned device chain only if a connection to the display
  *    could be establisged
  *
  *    If no driver could be initialised, this function returns NULL.
  *
  **********************************************************************/
    
  extern
  grDeviceChain*  grInitDevices( void );



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
 
  extern void  grGetDeviceModes( const char*    device_name,
                                 int           *num_modes,
                                 grPixelMode*  *pixel_modes );



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
    
  extern grSurface*  grNewSurface( const char*  device,
                                   grBitmap*    bitmap );



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
                                   grPos       height );


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

  extern void  grRefreshSurface( grSurface*  surface );



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
                            grColor    color );


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
                              grColor     color );


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
                           const char* title_string );




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
                         grEvent    *event );


#endif /* GRAPH_H */
