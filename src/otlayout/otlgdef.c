/***************************************************************************/
/*                                                                         */
/*  otlgdef.c                                                              */
/*                                                                         */
/*    OpenType layout support, GDEF table (body).                          */
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


#include "otlgdef.h"
#include "otlcommn.h"


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       ATTACHMENTS LIST                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_attach_point_validate( OTL_Bytes      table,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   count;


    OTL_CHECK( 2 );

    count = OTL_NEXT_USHORT( p );
    OTL_CHECK( count * 2 );
  }


  static void
  otl_attach_list_validate( OTL_Bytes      table,
                            OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_Bytes  coverage;
    OTL_UInt   num_glyphs;


    OTL_CHECK( 4 );

    coverage   = table + OTL_NEXT_USHORT( p );
    num_glyphs = OTL_NEXT_USHORT( p );

    otl_coverage_validate( coverage, valid );
    if ( num_glyphs != otl_coverage_get_count( coverage ) )
      OTL_INVALID_DATA;

    OTL_CHECK( num_glyphs * 2 );

    /* scan attach point records */
    for ( ; num_glyphs > 0; num_glyphs-- )
      otl_attach_point_validate( table + OTL_NEXT_USHORT( p ), valid );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       LIGATURE CARETS                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_caret_value_validate( OTL_Bytes      table,
                            OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_Int    format;


    OTL_CHECK( 4 );

    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      /* skip coordinate, no test */
      break;

    case 2:
      /* skip contour point index, no test */
      break;

    case 3:
      p += 2;           /* skip coordinate */

      OTL_CHECK( 2 );

      otl_device_table_validate( table + OTL_PEEK_USHORT( p ), valid );
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  static void
  otl_ligature_glyph_validate( OTL_Bytes      table,
                               OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_carets;


    OTL_CHECK( 2 );
    num_carets = OTL_NEXT_USHORT( p );
    OTL_CHECK( num_carets * 2 );

    /* scan caret value records */
    for ( ; num_carets > 0; num_carets-- )
      otl_caret_value_validate( table + OTL_NEXT_USHORT( p ), valid );
  }


  static void
  otl_ligature_caret_list_validate( OTL_Bytes      table,
                                    OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_Bytes  coverage;
    OTL_UInt   num_ligglyphs;


    OTL_CHECK( 4 );

    coverage      = table + OTL_NEXT_USHORT( p );
    num_ligglyphs = OTL_NEXT_USHORT( p );

    otl_coverage_validate( coverage, valid );
    if ( num_ligglyphs != otl_coverage_get_count( coverage ) )
      OTL_INVALID_DATA;

    OTL_CHECK( num_ligglyphs * 2 );

    /* scan ligature glyph records */
    for ( ; num_ligglyphs > 0; num_ligglyphs-- )
      otl_ligature_glyph_validate( table + OTL_NEXT_USHORT( p ), valid );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                         GDEF TABLE                            *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_APIDEF( void )
  otl_gdef_validate( OTL_Bytes      table,
                     OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   val;


    OTL_CHECK( 12 );

    /* check format */
    if ( OTL_NEXT_ULONG( p ) != 0x00010000UL )
      OTL_INVALID_FORMAT;

    /* validate glyph class definition table */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_class_definition_validate( table + val, valid );

    /* validate attachment point list */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_attach_list_validate( table + val, valid );

    /* validate ligature caret list */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_ligature_caret_list_validate( table + val, valid );

    /* validate mark attachment class definition table */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_class_definition_validate( table + val, valid );
  }


/* END */
