/***************************************************************************/
/*                                                                         */
/*  cidriver.h                                                             */
/*                                                                         */
/*    High-level CID driver interface (specification).                     */
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


#ifndef CIDRIVER_H
#define CIDRIVER_H

#include <freetype/internal/ftdriver.h>

#ifdef __cplusplus
  extern "C" {
#endif


  FT_CALLBACK_TABLE
  const  FT_Driver_Class  t1cid_driver_class;

#ifdef __cplusplus
  }
#endif


#endif /* CIDRIVER_H */


/* END */
