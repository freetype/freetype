#ifndef GRX11_H
#define GRX11_H

#include "grobjs.h"
#include "grdevice.h"

  extern
  grDevice  gr_x11_device;

#ifdef GR_INIT_BUILD
  static
  grDeviceChain  gr_x11_device_chain =
  {
    "x11",
    &gr_x11_device,
    GR_INIT_DEVICE_CHAIN
  };

#undef GR_INIT_DEVICE_CHAIN
#define GR_INIT_DEVICE_CHAIN  &gr_x11_device_chain

#endif  /* GR_INIT_BUILD */

#endif /* GRX11_H */
