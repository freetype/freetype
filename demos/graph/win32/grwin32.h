#ifndef GRWIN32_H
#define GRWIN32_H

#include "grobjs.h"

  extern
  grDevice  gr_win32_device;

#ifdef GR_INIT_BUILD
  static
  grDeviceChain  gr_win32_device_chain =
  {
    "win32",
    &gr_win32_device,
    GR_INIT_DEVICE_CHAIN
  };

#undef GR_INIT_DEVICE_CHAIN
#define GR_INIT_DEVICE_CHAIN  &gr_win32_device_chain

#endif  /* GR_INIT_BUILD */

#endif /* GRWIN32_H */
