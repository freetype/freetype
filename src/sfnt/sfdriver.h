/***************************************************************************/
/*                                                                         */
/*  sfdriver.h                                                             */
/*                                                                         */
/*    High-level SFNT driver interface (specification).                    */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef SFDRIVER_H
#define SFDRIVER_H

#include <freetype.h>
#include <ftdriver.h>

  EXPORT_DEF
  const FT_DriverInterface  sfnt_driver_interface;



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
  const FT_DriverChain  ftinit_sfnt_driver_chain =
  {
    FT_INIT_LAST_DRIVER_CHAIN,
    &sfnt_driver_interface
  };

#undef  FT_INIT_LAST_DRIVER_CHAIN
#define FT_INIT_LAST_DRIVER_CHAIN   &ftinit_sfnt_driver_chain

#endif /* FTINIT_DRIVER_CHAIN */ 



#endif /* SFDRIVER_H */


/* END */
