/***************************************************************************/
/*                                                                         */
/*  otlbase.c                                                              */
/*                                                                         */
/*    OpenType layout support, BASE table (body).                          */
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


#include "otlbase.h"
#include "otlcommn.h"


  static void
  otl_base_coord_validate( OTL_Bytes      table,
                           OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;


    OTL_CHECK( 4 );

    format = OTL_NEXT_USHORT( p );

    p += 2;     /* skip coordinate */

    switch ( format )
    {
    case 1:
      break;

    case 2:
      OTL_CHECK( 4 );
      break;

    case 3:
      OTL_CHECK( 2 );

      otl_device_table_validate( table + OTL_PEEK_USHORT( p ), valid );
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  static void
  otl_base_tag_list_validate( OTL_Bytes      table,
                              OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   count;


    OTL_CHECK( 2 );

    count = OTL_NEXT_USHORT( p );
    OTL_CHECK( count * 4 );
  }


  static void
  otl_base_values_validate( OTL_Bytes      table,
                            OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_coord;


    OTL_CHECK( 4 );

    p        += 2;              /* skip default index */
    num_coord = OTL_NEXT_USHORT( p );

    OTL_CHECK( num_coord * 2 );

    /* scan base coordinate records */
    for ( ; num_coord > 0; num_coord-- )
      otl_base_coord_validate( table + OTL_NEXT_USHORT( p ), valid );
  }


  static void
  otl_base_minmax_validate( OTL_Bytes      table,
                            OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   min_coord, max_coord, num_minmax;


    OTL_CHECK( 6 );

    min_coord  = OTL_NEXT_USHORT( p );
    max_coord  = OTL_NEXT_USHORT( p );
    num_minmax = OTL_NEXT_USHORT( p );

    if ( min_coord )
      otl_base_coord_validate( table + min_coord, valid );

    if ( max_coord )
      otl_base_coord_validate( table + max_coord, valid );

    OTL_CHECK( num_minmax * 8 );

    /* scan feature minmax records */
    for ( ; num_minmax > 0; num_minmax-- )
    {
      p += 4;       /* skip tag */

      min_coord = OTL_NEXT_USHORT( p );
      max_coord = OTL_NEXT_USHORT( p );

      if ( min_coord )
        otl_base_coord_validate( table + min_coord, valid );

      if ( max_coord )
        otl_base_coord_validate( table + max_coord, valid );
    }
  }


  static void
  otl_base_script_validate( OTL_Bytes      table,
                            OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   values, default_minmax, num_langsys;


    OTL_CHECK( 6 );

    values         = OTL_NEXT_USHORT( p );
    default_minmax = OTL_NEXT_USHORT( p );
    num_langsys    = OTL_NEXT_USHORT( p );

    if ( values )
      otl_base_values_validate( table + values, valid );

    if ( default_minmax )
      otl_base_minmax_validate( table + default_minmax, valid );

    OTL_CHECK( num_langsys * 6 );

    /* scan base langsys records */
    for ( ; num_langsys > 0; num_langsys-- )
    {
      p += 4;       /* skip tag */

      otl_base_minmax_validate( table + OTL_NEXT_USHORT( p ), valid );
    }
  }


  static void
  otl_base_script_list_validate( OTL_Bytes      table,
                                 OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_scripts;


    OTL_CHECK( 2 );

    num_scripts = OTL_NEXT_USHORT( p );
    OTL_CHECK( num_scripts * 6 );

    /* scan base script records */
    for ( ; num_scripts > 0; num_scripts-- )
    {
      p += 4;       /* skip tag */

      otl_base_script_validate( table + OTL_NEXT_USHORT( p ), valid );
    }
  }


  static void
  otl_axis_table_validate( OTL_Bytes      table,
                           OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   tags;


    OTL_CHECK( 4 );

    tags = OTL_NEXT_USHORT( p );
    if ( tags )
      otl_base_tag_list_validate( table + tags, valid );

    otl_base_script_list_validate( table + OTL_NEXT_USHORT( p ), valid );
  }


  OTL_LOCALDEF( void )
  otl_base_validate( OTL_Bytes      table,
                     OTL_Validator  valid )
  {
    OTL_Bytes p = table;
    OTL_UInt  val;


    OTL_CHECK( 6 );

    if ( OTL_NEXT_ULONG( p ) != 0x10000UL )
      OTL_INVALID_DATA;

    /* validate horizontal axis table */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_axis_table_validate( table + val, valid );

    /* validate vertical axis table */
    val = OTL_NEXT_USHORT( p );
    if ( val )
      otl_axis_table_validate( table + val, valid );
  }


/* END */
