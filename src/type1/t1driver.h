/*******************************************************************
 *
 *  t1driver.h
 *
 *    High-level Type1 driver interface for FreeType 2.0
 *
 *  Copyright 1996-1998 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used,
 *  modified, and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute 
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 ******************************************************************/

#ifndef T1DRIVER_H
#define T1DRIVER_H

#include <t1objs.h>
#include <t1errors.h>

  EXPORT_DEF
  const  FT_DriverInterface  t1_driver_interface;


/*************************************************************************
 *
 *  Here is a template of the code that should appear in each
 *  font driver's _interface_ file (the one included by "ftinit.c").
 *
 *  It is used to build, at compile time, a simple linked list of
 *  the interfaces of the drivers which have been #included in 
 *  "ftinit.c". See the source code of the latter file for details
 *
 *  (Note that this is only required when you want your driver included
 *   in the set of default drivers loaded by FT_Init_FreeType. Other
 *   drivers can still be added manually at runtime with FT_Add_Driver.
 *
 * {
 *   #ifdef FTINIT_DRIVER_CHAIN
 *
 *   static
 *   const FT_DriverChain  ftinit_<FORMAT>_driver_chain =
 *   {
 *     FT_INIT_LAST_DRIVER_CHAIN,
 *     &<FORMAT>_driver_interface
 *   };
 * 
 *   #undef  FT_INIT_LAST_DRIVER_CHAIN
 *   #define FT_INIT_LAST_DRIVER_CHAIN   &ftinit_<FORMAT>_driver_chain
 *
 *   #endif 
 * }
 *
 *  replace <FORMAT> with your driver's prefix
 *
 *************************************************************************/

#ifdef FTINIT_DRIVER_CHAIN

  static
  const FT_DriverChain  ftinit_t1_driver_chain =
  {
    FT_INIT_LAST_DRIVER_CHAIN,
    &t1_driver_interface
  };

#undef  FT_INIT_LAST_DRIVER_CHAIN
#define FT_INIT_LAST_DRIVER_CHAIN   &ftinit_t1_driver_chain

#endif /* FTINIT_DRIVER_CHAIN */ 


#endif /* T1DRIVER_H */

