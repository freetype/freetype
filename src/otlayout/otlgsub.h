/***************************************************************************/
/*                                                                         */
/*  otlgsub.h                                                              */
/*                                                                         */
/*    OpenType layout support, GSUB table (specification).                 */
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


#ifndef __OTLGSUB_H__
#define __OTLGSUB_H__

#include "otlayout.h"

OTL_BEGIN_HEADER


  typedef OTL_UInt
  (*OTL_GSUB_AlternateFunc)( OTL_UInt     gindex,
                             OTL_UInt     count,
                             OTL_Bytes    alternates,
                             OTL_Pointer  data );

  typedef struct  OTL_GSUB_AlternateRec_
  {
    OTL_GSUB_AlternateFunc  handler_func;
    OTL_Pointer             handler_data;

  } OTL_GSUB_AlternateRec, *OTL_GSUB_Alternate;


  OTL_LOCAL( void )
  otl_gsub_validate( OTL_Bytes      table,
                     OTL_Validator  valid );


OTL_END_HEADER

#endif /* __OTLGSUB_H__ */


/* END */
