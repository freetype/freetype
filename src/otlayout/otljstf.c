/***************************************************************************/
/*                                                                         */
/*  otljstf.c                                                              */
/*                                                                         */
/*    OpenType layout support, JSTF table (body).                          */
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


#include "otljstf.h"
#include "otlcommn.h"
#include "otlgpos.h"


  static void
  otl_jstf_extender_validate( OTL_Bytes      table,
                              OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_glyphs;


    OTL_CHECK( 2 );

    num_glyphs = OTL_NEXT_USHORT( p );

    OTL_CHECK( num_glyphs * 2 );

    /* XXX: check glyph indices */
  }


  static void
  otl_jstf_gsubgpos_mods_validate( OTL_Bytes      table,
                                   OTL_UInt       lookup_count,
                                   OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_lookups;


    OTL_CHECK( 2 );
    num_lookups = OTL_NEXT_USHORT( p );
    OTL_CHECK( num_lookups * 2 );

    if ( lookup_count )
    {
      for ( ; num_lookups > 0; num_lookups-- )
        if ( OTL_NEXT_USHORT( p ) >= lookup_count )
          OTL_INVALID_DATA;
    }
  }


  static void
  otl_jstf_max_validate( OTL_Bytes      table,
                         OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_lookups;


    OTL_CHECK( 2 );
    num_lookups = OTL_NEXT_USHORT( p );
    OTL_CHECK( num_lookups * 2 );

    /* scan subtable records */
    for ( ; num_lookups > 0; num_lookups-- )
      /* XXX: check lookup types? */
      otl_gpos_subtable_validate( table + OTL_NEXT_USHORT( p ), valid );
  }


  static void
  otl_jstf_priority_validate( OTL_Bytes      table,
                              OTL_UInt       gsub_lookup_count,
                              OTL_UInt       gpos_lookup_count,
                              OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   val;


    OTL_CHECK( 20 );

    /* shrinkage GSUB enable/disable */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_jstf_gsubgpos_mods_validate( table + val, gsub_lookup_count,
                                       valid );

    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_jstf_gsubgpos_mods_validate( table + val, gsub_lookup_count,
                                       valid );

    /* shrinkage GPOS enable/disable */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_jstf_gsubgpos_mods_validate( table + val, gpos_lookup_count,
                                       valid );

    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_jstf_gsubgpos_mods_validate( table + val, gpos_lookup_count,
                                       valid );

    /* shrinkage JSTF max */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_jstf_max_validate( table + val, valid );

    /* extension GSUB enable/disable */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_jstf_gsubgpos_mods_validate( table + val, gsub_lookup_count,
                                       valid );

    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_jstf_gsubgpos_mods_validate( table + val, gsub_lookup_count,
                                       valid );

    /* extension GPOS enable/disable */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_jstf_gsubgpos_mods_validate( table + val, gpos_lookup_count,
                                       valid );

    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_jstf_gsubgpos_mods_validate( table + val, gpos_lookup_count,
                                       valid );

    /* extension JSTF max */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_jstf_max_validate( table + val, valid );
  }


  static void
  otl_jstf_lang_validate( OTL_Bytes      table,
                          OTL_UInt       gsub_lookup_count,
                          OTL_UInt       gpos_lookup_count,
                          OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_priorities;


    OTL_CHECK( 2 );
    num_priorities = OTL_NEXT_USHORT( p );
    OTL_CHECK( num_priorities * 2 );

    /* scan priority records */
    for ( ; num_priorities > 0; num_priorities-- )
      otl_jstf_priority_validate( table + OTL_NEXT_USHORT( p ),
                                  gsub_lookup_count, gpos_lookup_count,
                                  valid );
  }


  static void
  otl_jstf_script_validate( OTL_Bytes      table,
                            OTL_UInt       gsub_lookup_count,
                            OTL_UInt       gpos_lookup_count,
                            OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_langsys, extender, default_lang;


    OTL_CHECK( 6 );
    extender     = OTL_NEXT_USHORT( p );
    default_lang = OTL_NEXT_USHORT( p );
    num_langsys  = OTL_NEXT_USHORT( p );

    if ( extender )
      otl_jstf_extender_validate( table + extender, valid );

    if ( default_lang )
      otl_jstf_lang_validate( table + default_lang,
                              gsub_lookup_count, gpos_lookup_count, valid );

    OTL_CHECK( 6 * num_langsys );

    /* scan langsys records */
    for ( ; num_langsys > 0; num_langsys-- )
    {
      p += 4;       /* skip tag */

      otl_jstf_lang_validate( table + OTL_NEXT_USHORT( p ),
                              gsub_lookup_count, gpos_lookup_count, valid );
    }
  }


  OTL_LOCALDEF( void )
  otl_jstf_validate( OTL_Bytes      table,
                     OTL_Bytes      gsub,
                     OTL_Bytes      gpos,
                     OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_scripts, gsub_lookup_count, gpos_lookup_count;


    OTL_CHECK( 6 );

    if ( OTL_NEXT_ULONG( p ) != 0x10000UL )
      OTL_INVALID_DATA;

    num_scripts = OTL_NEXT_USHORT( p );
    OTL_CHECK( num_scripts * 6 );

    if ( gsub )
      gsub_lookup_count = otl_gsubgpos_get_lookup_count( gsub );
    else
      gsub_lookup_count = 0;

    if ( gpos )
      gpos_lookup_count = otl_gsubgpos_get_lookup_count( gpos );
    else
      gpos_lookup_count = 0;

    /* scan script records */
    for ( ; num_scripts > 0; num_scripts-- )
    {
      p += 4;       /* skip tag */

      otl_jstf_script_validate( table + OTL_NEXT_USHORT( p ),
                                gsub_lookup_count, gpos_lookup_count, valid );
    }
  }


/* END */
