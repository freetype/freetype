#ifndef __OTLGPOS_H__
#define __OTLGPOS_H__

#include "otlayout.h"

OTL_BEGIN_HEADER

  OTL_LOCAL( void )
  otl_gpos_subtable_validate( OTL_Bytes      table,
                              OTL_Validator  valid );

  OTL_LOCAL( void )
  otl_gpos_validate( OTL_Bytes      table,
                     OTL_Validator  valid );

OTL_END_HEADER

#endif /* __OTLGPOS_H__ */
