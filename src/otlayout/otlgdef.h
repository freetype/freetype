/***************************************************************************/
/*                                                                         */
/*  otlgdef.h                                                              */
/*                                                                         */
/*    OpenType layout support, GDEF table (specification).                 */
/*                                                                         */
/*  Copyright 2002, 2004 by                                                */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __OTLGDEF_H__
#define __OTLGDEF_H__

#include "otlayout.h"

#if 0
#include "otltable.h"
#endif

OTL_BEGIN_HEADER


  OTL_API( void )
  otl_gdef_validate( OTL_Bytes      table,
                     OTL_Validator  valid );


OTL_END_HEADER

#endif /* __OTLGDEF_H__ */


/* END */
