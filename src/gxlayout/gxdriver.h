/***************************************************************************/
/*                                                                         */
/*  gxdriver.h                                                             */
/*                                                                         */
/*    High-level AAT/TrueTypeGX driver interface (specification).          */
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/

#ifndef __GXDRIVER_H__
#define __GXDRIVER_H__


#include <ft2build.h>
#include FT_INTERNAL_DRIVER_H


FT_BEGIN_HEADER


  FT_CALLBACK_TABLE
  const FT_Driver_ClassRec  gx_driver_class;

FT_END_HEADER

#endif /* __GXDRIVER_H__ */


/* END */
