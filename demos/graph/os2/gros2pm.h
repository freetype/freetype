#ifndef GROS2PM_H
#define GROS2PM_H

#include "grobjs.h"

  extern
  grDevice  gr_os2pm_device;

#ifdef GR_INIT_BUILD
  static
  grDeviceChain  gr_os2pm_device_chain =
  {
    "os2pm",
    &gr_os2pm_device,
    GR_INIT_DEVICE_CHAIN
  };

#undef GR_INIT_DEVICE_CHAIN
#define GR_INIT_DEVICE_CHAIN  &gr_os2pm_device_chain

#endif  /* GR_INIT_BUILD */

#endif /* GROS2PM_H */
