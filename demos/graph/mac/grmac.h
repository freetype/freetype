#ifndef GRMAC_H
#define GRMAC_H

#include "grobjs.h"

  extern
  grDevice  gr_mac_device;

#ifdef GR_INIT_BUILD
  static
  grDeviceChain  gr_mac_device_chain =
  {
    "mac",
    &gr_mac_device,
    GR_INIT_DEVICE_CHAIN
  };

#undef GR_INIT_DEVICE_CHAIN
#define GR_INIT_DEVICE_CHAIN  &gr_mac_device_chain

#endif  /* GR_INIT_BUILD */

#endif /* GRMAC_H */
