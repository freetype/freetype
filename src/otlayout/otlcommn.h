/***************************************************************************/
/*                                                                         */
/*  otlcommn.h                                                             */
/*                                                                         */
/*    OpenType layout support, common tables (specification).              */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __OTLCOMMN_H__
#define __OTLCOMMN_H__

#include "otlayout.h"

OTL_BEGIN_HEADER


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       COVERAGE TABLE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* validate coverage table */
  OTL_LOCAL( void )
  otl_coverage_validate( OTL_Bytes      base,
                         OTL_Validator  valid );

  /* return number of covered glyphs */
  OTL_LOCAL( OTL_UInt )
  otl_coverage_get_count( OTL_Bytes  base );

  /* Return the coverage index corresponding to a glyph glyph index. */
  /* Return -1 if the glyph isn't covered.                           */
  OTL_LOCAL( OTL_Long )
  otl_coverage_get_index( OTL_Bytes  base,
                          OTL_UInt   glyph_index );


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                  CLASS DEFINITION TABLE                       *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* validate class definition table */
  OTL_LOCAL( void )
  otl_class_definition_validate( OTL_Bytes      table,
                                 OTL_Validator  valid );

#if 0
  /* return class value for a given glyph index */
  OTL_LOCAL( OTL_UInt )
  otl_class_definition_get_value( OTL_Bytes  table,
                                  OTL_UInt   glyph_index );
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      DEVICE TABLE                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* validate a device table */
  OTL_LOCAL( void )
  otl_device_table_validate( OTL_Bytes      table,
                             OTL_Validator  valid );

#if 0
  /* return a device table's first size */
  OTL_LOCAL( OTL_UInt )
  otl_device_table_get_start( OTL_Bytes  table );
#endif

#if 0
  /* return a device table's last size */
  OTL_LOCAL( OTL_UInt )
  otl_device_table_get_end( OTL_Bytes  table );
#endif

#if 0
  /* return pixel adjustment for a given size */
  OTL_LOCAL( OTL_Int )
  otl_device_table_get_delta( OTL_Bytes  table,
                              OTL_UInt   size );
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                           LOOKUPS                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCAL( void )
  otl_lookup_validate( OTL_Bytes          table,
                       OTL_UInt           type_count,
                       OTL_ValidateFunc*  type_funcs,
                       OTL_Validator      valid );

  /* return number of sub-tables in a lookup */
  OTL_LOCAL( OTL_UInt )
  otl_lookup_get_count( OTL_Bytes  table );

#if 0
  /* return lookup sub-table */
  OTL_LOCAL( OTL_Bytes )
  otl_lookup_get_table( OTL_Bytes  table,
                        OTL_UInt   idx );
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      LOOKUP LISTS                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* validate lookup list */
  OTL_LOCAL( void )
  otl_lookup_list_validate( OTL_Bytes          list,
                            OTL_UInt           type_count,
                            OTL_ValidateFunc*  type_funcs,
                            OTL_Validator      valid );

#if 0
  /* return number of lookups in list */
  OTL_LOCAL( OTL_UInt )
  otl_lookup_list_get_count( OTL_Bytes  table );
#endif

#if 0
  /* return a given lookup from a list */
  OTL_LOCAL( OTL_Bytes )
  otl_lookup_list_get_lookup( OTL_Bytes  table,
                              OTL_UInt   idx );
#endif

#if 0
  /* return lookup sub-table from a list */
  OTL_LOCAL( OTL_Bytes )
  otl_lookup_list_get_table( OTL_Bytes  table,
                             OTL_UInt   lookup_index,
                             OTL_UInt   table_index );
#endif

#if 0
  /* iterate over lookup list */
  OTL_LOCAL( void )
  otl_lookup_list_foreach( OTL_Bytes        table,
                           OTL_ForeachFunc  func,
                           OTL_Pointer      func_data );
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                        FEATURES                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* validate feature table */
  OTL_LOCAL( void )
  otl_feature_validate( OTL_Bytes      table,
                        OTL_UInt       lookup_count,
                        OTL_Validator  valid );

  /* return feature's lookup count */
  OTL_LOCAL( OTL_UInt )
  otl_feature_get_count( OTL_Bytes  table );

#if 0
  /* get several lookups indices from a feature. returns the number of */
  /* lookups grabbed                                                   */
  OTL_LOCAL( OTL_UInt )
  otl_feature_get_lookups( OTL_Bytes  table,
                           OTL_UInt   start,
                           OTL_UInt   count,
                           OTL_UInt  *lookups );
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                        FEATURE LIST                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* validate a feature list           */
  /* lookups must already be validated */
  OTL_LOCAL( void )
  otl_feature_list_validate( OTL_Bytes      table,
                             OTL_Bytes      lookups,
                             OTL_Validator  valid );

#if 0
  /* return number of features in list */
  OTL_LOCAL( OTL_UInt )
  otl_feature_list_get_count( OTL_Bytes  table );
#endif

#if 0
  /* return a given feature from a list */
  OTL_LOCAL( OTL_Bytes )
  otl_feature_list_get_feature( OTL_Bytes  table,
                                OTL_UInt   idx );
#endif

#if 0
  /* iterate over all features in a list */
  OTL_LOCAL( void )
  otl_feature_list_foreach( OTL_Bytes        table,
                            OTL_ForeachFunc  func,
                            OTL_Pointer      func_data );
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       LANGUAGE SYSTEM                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCAL( void )
  otl_lang_validate( OTL_Bytes      table,
                     OTL_UInt       feature_count,
                     OTL_Validator  valid );

#if 0
  OTL_LOCAL( OTL_UInt )
  otl_lang_get_req_feature( OTL_Bytes  table );
#endif

#if 0
  OTL_LOCAL( OTL_UInt )
  otl_lang_get_count( OTL_Bytes  table );
#endif

#if 0
  OTL_LOCAL( OTL_UInt )
  otl_lang_get_features( OTL_Bytes  table,
                         OTL_UInt   start,
                         OTL_UInt   count,
                         OTL_UInt  *features );
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                           SCRIPTS                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCAL( void )
  otl_script_validate( OTL_Bytes      table,
                       OTL_UInt       feature_count,
                       OTL_Validator  valid );

  /* validate a script list             */
  /* features must already be validated */
  OTL_LOCAL( void )
  otl_script_list_validate( OTL_Bytes          list,
                            OTL_Bytes          features,
                            OTL_Validator      valid );


 /* */

OTL_END_HEADER

#endif /* __OTLCOMMN_H__ */


/* END */
