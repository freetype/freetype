/***************************************************************************/
/*                                                                         */
/*  otlgpos.h                                                              */
/*                                                                         */
/*    OpenType layout support, GPOS table (specification).                 */
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


#ifndef __OTLGPOS_H__
#define __OTLGPOS_H__

#include "otlayout.h"

OTL_BEGIN_HEADER


  OTL_LOCAL( void )
  otl_gpos_subtable_validate( OTL_Bytes      table,
                              OTL_UInt       glyph_count,
                              OTL_Validator  valid );

  OTL_LOCAL( void )
  otl_gpos_validate( OTL_Bytes      table,
                     OTL_UInt       glyph_count,
                     OTL_Validator  valid );


OTL_END_HEADER

#endif /* __OTLGPOS_H__ */


/* END */
