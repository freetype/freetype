/***************************************************************************/
/*                                                                         */
/*  otlbase.h                                                              */
/*                                                                         */
/*    OpenType layout support, BASE table (specification).                 */
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


#ifndef __OTLBASE_H__
#define __OTLBASE_H__

#include "otlayout.h"

OTL_BEGIN_HEADER


  OTL_LOCAL( void )
  otl_base_validate( OTL_Bytes      table,
                     OTL_Validator  valid );


OTL_END_HEADER

#endif /* __OTLBASE_H__ */


/* END */
