/***************************************************************************/
/*                                                                         */
/*  otljstf.h                                                              */
/*                                                                         */
/*    OpenType layout support, JSTF table (specification).                 */
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


#ifndef __OTLJSTF_H__
#define __OTLJSTF_H__

#include "otlayout.h"


OTL_BEGIN_HEADER


  /* validate JSTF table                                         */
  /* GSUB and GPOS tables must already be validated; if table is */
  /* missing, set value to 0                                     */
  OTL_LOCAL( void )
  otl_jstf_validate( OTL_Bytes      table,
                     OTL_Bytes      gsub,
                     OTL_Bytes      gpos,
                     OTL_Validator  valid );


OTL_END_HEADER

#endif /* __OTLJSTF_H__ */


/* END */
