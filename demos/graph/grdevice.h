/***************************************************************************
 *
 *  grdevice.h
 *
 *    Graphics device interface
 *
 *  Copyright 1999 - The FreeType Development Team - www.freetype.org
 *
 *
 ***************************************************************************/

#ifndef GRDEVICE_H
#define GRDEVICE_H

#include "graph.h"


 /********************************************************************
  *
  * <FuncType>
  *   grDeviceInitFunc
  *
  * <Description>
  *   Simple device initialiser function
  *
  * <Return>
  *   error code. 0 means success
  *
  ********************************************************************/

  typedef int  (*grDeviceInitFunc)( void );


 /********************************************************************
  *
  * <FuncType>
  *   grDeviceDoneFunc
  *
  * <Description>
  *   Simple device finaliser function
  *
  * <Return>
  *   error code. 0 means success
  *
  ********************************************************************/

  typedef void (*grDeviceDoneFunc)( void );


 /********************************************************************
  *
  * <FuncType>
  *   grDeviceInitSurfaceFunc
  *
  * <Description>
  *   initializes a new surface for the device. This may be a window
  *   or a video screen, depending on the device.
  *
  * <Input>
  *   surface  :: handle to target surface
  *
  * <InOut>
  *   bitmap   :: handle to bitmap descriptor
  *
  ********************************************************************/

  typedef int  (*grDeviceInitSurfaceFunc)( grSurface*   surface,
                                           grBitmap*    bitmap );


 /********************************************************************
  *
  * <Struct>
  *   grDevice
  *
  * <Description>
  *   Simple device interface structure
  *
  * <Fields>
  *   surface_objsize :: size in bytes of a single surface object for
  *                      this device.
  *
  *   device_name :: name of device, e.g. "x11", "os2pm", "directx" etc..
  *   init        :: device initialisation routine
  *   done        :: device finalisation
  *   new_surface :: function used to create a new surface (screen or
  *                  window) from the device
  *
  *   num_pixel_modes :: the number of pixel modes supported by this
  *                      device. This value _must_ be set to -1
  *                      default, unless the device provides a
  *                      static set of pixel modes (fullscreen).
  *
  *   pixel_modes     :: an array of pixel modes supported by this
  *                      device
  *
  * <Note>
  *   the fields "num_pixel_modes" and "pixel_modes" must be
  *   set by the "init" function.
  *
  *   This allows windowed devices to "discover" at run-time the
  *   available pixel modes they can provide depending on the
  *   current screen depth.
  *
  ********************************************************************/

  struct grDevice_
  {
    int          surface_objsize;
    const char*  device_name;  /* name of device                 */
  
    grDeviceInitFunc        init;
    grDeviceDoneFunc        done;

    grDeviceInitSurfaceFunc init_surface;

    int                     num_pixel_modes;
    grPixelMode*            pixel_modes; 
  };


  extern grDeviceChain   gr_device_chain[];
  extern int             gr_num_devices;


#endif /* GRDEVICE_H */
