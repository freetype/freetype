#include "grobjs.h"
#include "grdevice.h"
#include <stdio.h>

#define GR_INIT_DEVICE_CHAIN   ((grDeviceChain*)0)
#define GR_INIT_BUILD

#ifdef DEVICE_X11
#include "grx11.h"
#endif

#ifdef DEVICE_OS2_PM
#include "gros2pm.h"
#endif

#ifdef DEVICE_WIN32
#include "grwin32.h"
#endif

#ifdef macintosh
#include "grmac.h"
#endif


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
  grDeviceChain*  grInitDevices( void )
  {
    grDeviceChain*  chain = GR_INIT_DEVICE_CHAIN;
    grDeviceChain*  cur   = gr_device_chain;

    while (chain)
    {
      /* initialize the device */
      grDevice*  device;

      device = chain->device;
      if ( device->init() == 0             &&
           gr_num_devices < GR_MAX_DEVICES )
          
      {
        /* successful device initialisation - add it to our chain */
        cur->next   = 0;
        cur->device = device;
        cur->name   = device->device_name;

        if (cur > gr_device_chain)
          cur[-1].next = cur;

        cur++;
        gr_num_devices++;
      }
      chain = chain->next;
    }

    return (gr_num_devices > 0 ? gr_device_chain : 0 );
  }



