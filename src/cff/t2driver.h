/***************************************************************************/
/*                                                                         */
/*  t2driver.h                                                             */
/*                                                                         */
/*    High-level OpenType driver interface (specification).                */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef T2DRIVER_H
#define T2DRIVER_H

#include <freetype/internal/ftdriver.h>

#ifdef __cplusplus
  extern "C" {
#endif


  FT_CALLBACK_TABLE
  const FT_Driver_Class  cff_driver_class;


#ifdef __cplusplus
  }
#endif


#endif /* T2DRIVER_H */


/* END */
