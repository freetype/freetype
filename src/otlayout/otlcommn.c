/***************************************************************************/
/*                                                                         */
/*  otlcommn.c                                                             */
/*                                                                         */
/*    OpenType layout support, common tables (body).                       */
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


#include "otlayout.h"


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       COVERAGE TABLE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCALDEF( void )
  otl_coverage_validate( OTL_Bytes      table,
                         OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;


    OTL_CHECK( 4 );

    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  num_glyphs = OTL_NEXT_USHORT( p );


        OTL_CHECK( num_glyphs * 2 );

      }
      break;

    case 2:
      {
        OTL_UInt  n, num_ranges = OTL_NEXT_USHORT( p );
        OTL_UInt  start, end, start_coverage, total = 0, last = 0;


        OTL_CHECK( num_ranges * 6 );

        /* scan range records */
        for ( n = 0; n < num_ranges; n++ )
        {
          start          = OTL_NEXT_USHORT( p );
          end            = OTL_NEXT_USHORT( p );
          start_coverage = OTL_NEXT_USHORT( p );

          if ( start > end || start_coverage != total )
            OTL_INVALID_DATA;

          if ( n > 0 && start <= last )
            OTL_INVALID_DATA;

          total += end - start + 1;
          last   = end;
        }
      }
      break;

    default:
      OTL_INVALID_FORMAT;
    }

    /* no need to check glyph indices used as input to coverage tables */
    /* since even invalid glyph indices return a meaningful result     */
  }


  OTL_LOCALDEF( OTL_UInt )
  otl_coverage_get_first( OTL_Bytes  table )
  {
    OTL_Bytes  p = table;


    p += 4;     /* skip format and count */

    return OTL_NEXT_USHORT( p );
  }


  OTL_LOCALDEF( OTL_UInt )
  otl_coverage_get_last( OTL_Bytes  table )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format = OTL_NEXT_USHORT( p );
    OTL_UInt   count  = OTL_NEXT_USHORT( p );
    OTL_UInt   result;


    switch ( format )
    {
    case 1:
      p += ( count - 1 ) * 2;
      result = OTL_NEXT_USHORT( p );
      break;

    case 2:
      p += ( count - 1 ) * 6 + 2;
      result = OTL_NEXT_USHORT( p );
      break;

    default:
      ;
    }

    return result;
  }


  OTL_LOCALDEF( OTL_UInt )
  otl_coverage_get_count( OTL_Bytes  table )
  {
    OTL_Bytes  p      = table;
    OTL_UInt   format = OTL_NEXT_USHORT( p );
    OTL_UInt   count  = OTL_NEXT_USHORT( p );
    OTL_UInt   result = 0;


    switch ( format )
    {
    case 1:
      return count;

    case 2:
      {
        OTL_UInt  start, end;


        for ( ; count > 0; count-- )
        {
          start = OTL_NEXT_USHORT( p );
          end   = OTL_NEXT_USHORT( p );
          p    += 2;                    /* skip start_index */

          result += end - start + 1;
        }
      }
      break;

    default:
      ;
    }

    return result;
  }


#if 0
  OTL_LOCALDEF( OTL_Long )
  otl_coverage_get_index( OTL_Bytes  table,
                          OTL_UInt   glyph_index )
  {
    OTL_Bytes  p      = table;
    OTL_UInt   format = OTL_NEXT_USHORT( p );
    OTL_UInt   count  = OTL_NEXT_USHORT( p );


    switch ( format )
    {
    case 1:
      {
        OTL_UInt  min = 0, max = count, mid, gindex;


        table += 4;
        while ( min < max )
        {
          mid    = ( min + max ) >> 1;
          p      = table + 2 * mid;
          gindex = OTL_PEEK_USHORT( p );

          if ( glyph_index == gindex )
            return (OTL_Long)mid;

          if ( glyph_index < gindex )
            max = mid;
          else
            min = mid + 1;
        }
      }
      break;

    case 2:
      {
        OTL_UInt  min = 0, max = count, mid;
        OTL_UInt  start, end;


        table += 4;
        while ( min < max )
        {
          mid    = ( min + max ) >> 1;
          p      = table + 6 * mid;
          start  = OTL_NEXT_USHORT( p );
          end    = OTL_NEXT_USHORT( p );

          if ( glyph_index < start )
            max = mid;
          else if ( glyph_index > end )
            min = mid + 1;
          else
            return (OTL_Long)( glyph_index + OTL_NEXT_USHORT( p ) - start );
        }
      }
      break;

    default:
      ;
    }

    return -1;
  }
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                  CLASS DEFINITION TABLE                       *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCALDEF( void )
  otl_class_definition_validate( OTL_Bytes      table,
                                 OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;


    OTL_CHECK( 4 );

    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  num_glyphs;


        p += 2;         /* skip start_glyph */

        OTL_CHECK( 2 );
        num_glyphs = OTL_NEXT_USHORT( p );

        OTL_CHECK( num_glyphs * 2 );
      }
      break;

    case 2:
      {
        OTL_UInt  n, num_ranges = OTL_NEXT_USHORT( p );
        OTL_UInt  start, end, last = 0;


        OTL_CHECK( num_ranges * 6 );

        /* scan class range records */
        for ( n = 0; n < num_ranges; n++ )
        {
          start = OTL_NEXT_USHORT( p );
          end   = OTL_NEXT_USHORT( p );
          p    += 2;                        /* ignored */

          if ( start > end || ( n > 0 && start <= last ) )
            OTL_INVALID_DATA;

          last = end;
        }
      }
      break;

    default:
      OTL_INVALID_FORMAT;
    }

    /* no need to check glyph indices used as input to class definition   */
    /* tables since even invalid glyph indices return a meaningful result */
  }


#if 0
  OTL_LOCALDEF( OTL_UInt )
  otl_class_definition_get_value( OTL_Bytes  table,
                                  OTL_UInt   glyph_index )
  {
    OTL_Bytes  p      = table;
    OTL_UInt   format = OTL_NEXT_USHORT( p );


    switch ( format )
    {
    case 1:
      {
        OTL_UInt  start = OTL_NEXT_USHORT( p );
        OTL_UInt  count = OTL_NEXT_USHORT( p );
        OTL_UInt  idx   = (OTL_UInt)( glyph_index - start );


        if ( idx < count )
        {
          p += 2 * idx;
          return OTL_PEEK_USHORT( p );
        }
      }
      break;

    case 2:
      {
        OTL_UInt  count = OTL_NEXT_USHORT( p );
        OTL_UInt  min = 0, max = count, mid, gindex, start, end;


        table += 4;
        while ( min < max )
        {
          mid   = ( min + max ) >> 1;
          p     = table + 6 * mid;
          start = OTL_NEXT_USHORT( p );
          end   = OTL_NEXT_USHORT( p );

          if ( glyph_index < start )
            max = mid;
          else if ( glyph_index > end )
            min = mid + 1;
          else
            return OTL_PEEK_USHORT( p );
        }
      }
      break;

    default:
      ;
    }

    return 0;
  }
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      DEVICE TABLE                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCALDEF( void )
  otl_device_table_validate( OTL_Bytes      table,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   start, end, count, format;


    OTL_CHECK( 8 );

    start  = OTL_NEXT_USHORT( p );
    end    = OTL_NEXT_USHORT( p );
    format = OTL_NEXT_USHORT( p );

    if ( format < 1 || format > 3 || end < start )
      OTL_INVALID_DATA;

    count = end - start + 1;

    OTL_CHECK( ( 1 << format ) * count / 8 );
  }


#if 0
  OTL_LOCALDEF( OTL_UInt )
  otl_device_table_get_start( OTL_Bytes  table )
  {
    OTL_Bytes  p = table;


    return OTL_PEEK_USHORT( p );
  }
#endif


#if 0
  OTL_LOCALDEF( OTL_UInt )
  otl_device_table_get_end( OTL_Bytes  table )
  {
    OTL_Bytes  p = table + 2;


    return OTL_PEEK_USHORT( p );
  }
#endif


#if 0
  OTL_LOCALDEF( OTL_Int )
  otl_device_table_get_delta( OTL_Bytes  table,
                              OTL_UInt   size )
  {
    OTL_Bytes  p = table;
    OTL_Int    result = 0;
    OTL_UInt   start, end, format, idx, value, shift;


    start  = OTL_NEXT_USHORT( p );
    end    = OTL_NEXT_USHORT( p );
    format = OTL_NEXT_USHORT( p );

    if ( size >= start && size <= end )
    {
      /* we could do that with clever bit operations, but a switch is */
      /* much simpler to understand and maintain                      */
      /*                                                              */
      switch ( format )
      {
      case 1:
        idx    = (OTL_UInt)( ( size - start ) * 2 );
        p     += idx / 16;
        value  = OTL_PEEK_USHORT( p );
        shift  = idx & 15;
        result = (OTL_Int16)( value << shift ) >> ( 14 - shift );

        break;

      case 2:
        idx    = (OTL_UInt)( ( size - start ) * 4 );
        p     += idx / 16;
        value  = OTL_PEEK_USHORT( p );
        shift  = idx & 15;
        result = (OTL_Int16)( value << shift ) >> ( 12 - shift );

        break;

      case 3:
        idx    = (OTL_UInt)( ( size - start ) * 8 );
        p     += idx / 16;
        value  = OTL_PEEK_USHORT( p );
        shift  = idx & 15;
        result = (OTL_Int16)( value << shift ) >> ( 8 - shift );

        break;

      default:
        ;
      }
    }

    return result;
  }
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                         LOOKUPS                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCALDEF( void )
  otl_lookup_validate( OTL_Bytes          table,
                       OTL_UInt           type_count,
                       OTL_ValidateFunc*  type_funcs,
                       OTL_UInt           lookup_count,
                       OTL_UInt           glyph_count,
                       OTL_Validator      valid )
  {
    OTL_Bytes         p = table;
    OTL_UInt          lookup_type, lookup_flag, num_subtables;
    OTL_ValidateFunc  validate;


    OTL_CHECK( 6 );
    lookup_type   = OTL_NEXT_USHORT( p );
    lookup_flag   = OTL_NEXT_USHORT( p );
    num_subtables = OTL_NEXT_USHORT( p );

    if ( lookup_type == 0 || lookup_type >= type_count )
      OTL_INVALID_DATA;

    validate = type_funcs[lookup_type - 1];

    OTL_CHECK( 2 * num_subtables );

    /* scan subtables */
    for ( ; num_subtables > 0; num_subtables-- )
      validate( table + OTL_NEXT_USHORT( p ), lookup_count, glyph_count,
                valid );
  }


#if 0
  OTL_LOCALDEF( OTL_UInt )
  otl_lookup_get_count( OTL_Bytes  table )
  {
    OTL_Bytes  p = table + 4;


    return OTL_PEEK_USHORT( p );
  }
#endif


#if 0
  OTL_LOCALDEF( OTL_Bytes )
  otl_lookup_get_table( OTL_Bytes  table,
                        OTL_UInt   idx )
  {
    OTL_Bytes  p, result = 0;
    OTL_UInt   count;


    p     = table + 4;
    count = OTL_NEXT_USHORT( p );
    if ( idx < count )
    {
      p     += idx * 2;
      result = table + OTL_PEEK_USHORT( p );
    }

    return result;
  }
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      LOOKUP LISTS                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCALDEF( void )
  otl_lookup_list_validate( OTL_Bytes          table,
                            OTL_UInt           type_count,
                            OTL_ValidateFunc*  type_funcs,
                            OTL_UInt           glyph_count,
                            OTL_Validator      valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_lookups, count;


    OTL_CHECK( 2 );
    num_lookups = OTL_NEXT_USHORT( p );
    OTL_CHECK( 2 * num_lookups );

    /* scan lookup records */
    for ( count = num_lookups; count > 0; count-- )
      otl_lookup_validate( table + OTL_NEXT_USHORT( p ),
                           type_count, type_funcs,
                           num_lookups, glyph_count, valid );
  }


  OTL_LOCALDEF( OTL_UInt )
  otl_lookup_list_get_count( OTL_Bytes  table )
  {
    OTL_Bytes  p = table;


    return OTL_PEEK_USHORT( p );
  }


#if 0
  OTL_LOCALDEF( OTL_Bytes )
  otl_lookup_list_get_lookup( OTL_Bytes  table,
                              OTL_UInt   idx )
  {
    OTL_Bytes  p, result = 0;
    OTL_UInt   count;


    p     = table;
    count = OTL_NEXT_USHORT( p );
    if ( idx < count )
    {
      p     += idx * 2;
      result = table + OTL_PEEK_USHORT( p );
    }

    return result;
  }
#endif


#if 0
  OTL_LOCALDEF( OTL_Bytes )
  otl_lookup_list_get_table( OTL_Bytes  table,
                             OTL_UInt   lookup_index,
                             OTL_UInt   table_index )
  {
    OTL_Bytes  result = 0;


    result = otl_lookup_list_get_lookup( table, lookup_index );
    if ( result )
      result = otl_lookup_get_table( result, table_index );

    return result;
  }
#endif


#if 0
  OTL_LOCALDEF( void )
  otl_lookup_list_foreach( OTL_Bytes        table,
                           OTL_ForeachFunc  func,
                           OTL_Pointer      func_data )
  {
    OTL_Bytes  p     = table;
    OTL_UInt   count = OTL_NEXT_USHORT( p );


    for ( ; count > 0; count-- )
      func( table + OTL_NEXT_USHORT( p ), func_data );
  }
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                        FEATURES                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCALDEF( void )
  otl_feature_validate( OTL_Bytes      table,
                        OTL_UInt       lookup_count,
                        OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_lookups;


    OTL_CHECK( 4 );
    p           += 2;                       /* unused */
    num_lookups  = OTL_NEXT_USHORT( p );
    OTL_CHECK( 2 * num_lookups );

    for ( ; num_lookups > 0; num_lookups-- )
      if ( OTL_NEXT_USHORT( p ) >= lookup_count )
        OTL_INVALID_DATA;
  }


#if 0
  OTL_LOCALDEF( OTL_UInt )
  otl_feature_get_count( OTL_Bytes  table )
  {
    OTL_Bytes  p = table + 4;


    return OTL_PEEK_USHORT( p );
  }
#endif


#if 0
  OTL_LOCALDEF( OTL_UInt )
  otl_feature_get_lookups( OTL_Bytes  table,
                           OTL_UInt   start,
                           OTL_UInt   count,
                           OTL_UInt  *lookups )
  {
    OTL_Bytes  p;
    OTL_UInt   num_features, result = 0;


    p            = table + 4;
    num_features = OTL_NEXT_USHORT( p );

    p += start * 2;

    for ( ; count > 0 && start < num_features; count--, start++ )
    {
      lookups[0] = OTL_NEXT_USHORT(p);
      lookups++;
      result++;
    }

    return result;
  }
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                        FEATURE LIST                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCALDEF( void )
  otl_feature_list_validate( OTL_Bytes      table,
                             OTL_Bytes      lookups,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_features, lookup_count;


    OTL_CHECK( 2 );
    num_features = OTL_NEXT_USHORT( p );
    OTL_CHECK( 2 * num_features );

    lookup_count = otl_lookup_list_get_count( lookups );

    /* scan feature records */
    for ( ; num_features > 0; num_features-- )
    {
      p += 4;       /* skip tag */

      otl_feature_validate( table + OTL_NEXT_USHORT( p ), lookup_count,
                            valid );
    }
  }


  OTL_LOCALDEF( OTL_UInt )
  otl_feature_list_get_count( OTL_Bytes  table )
  {
    OTL_Bytes  p = table;


    return OTL_PEEK_USHORT( p );
  }


#if 0
  OTL_LOCALDEF( OTL_Bytes )
  otl_feature_list_get_feature( OTL_Bytes  table,
                                OTL_UInt   idx )
  {
    OTL_Bytes  p, result = 0;
    OTL_UInt   count;


    p     = table;
    count = OTL_NEXT_USHORT( p );

    if ( idx < count )
    {
      p     += idx * 2;
      result = table + OTL_PEEK_USHORT( p );
    }

    return result;
  }
#endif


#if 0
  OTL_LOCALDEF( void )
  otl_feature_list_foreach( OTL_Bytes        table,
                            OTL_ForeachFunc  func,
                            OTL_Pointer      func_data )
  {
    OTL_Bytes  p;
    OTL_UInt   count;


    p = table;
    count = OTL_NEXT_USHORT( p );

    for ( ; count > 0; count-- )
      func( table + OTL_NEXT_USHORT( p ), func_data );
  }
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       LANGUAGE SYSTEM                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  OTL_LOCALDEF( void )
  otl_lang_validate( OTL_Bytes      table,
                     OTL_UInt       feature_count,
                     OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   req_feature;
    OTL_UInt   num_features;


    OTL_CHECK( 6 );

    p           += 2;                       /* unused */
    req_feature  = OTL_NEXT_USHORT( p );
    num_features = OTL_NEXT_USHORT( p );

    if ( req_feature != 0xFFFFU && req_feature >= feature_count )
      OTL_INVALID_DATA;

    OTL_CHECK( 2 * num_features );

    for ( ; num_features > 0; num_features-- )
      if ( OTL_NEXT_USHORT( p ) >= feature_count )
        OTL_INVALID_DATA;
  }


#if 0
  OTL_LOCALDEF( OTL_UInt )
  otl_lang_get_count( OTL_Bytes  table )
  {
    OTL_Bytes  p = table + 4;


    return OTL_PEEK_USHORT( p );
  }
#endif


#if 0
  OTL_LOCALDEF( OTL_UInt )
  otl_lang_get_req_feature( OTL_Bytes  table )
  {
    OTL_Bytes  p = table + 2;


    return OTL_PEEK_USHORT( p );
  }
#endif


#if 0
  OTL_LOCALDEF( OTL_UInt )
  otl_lang_get_features( OTL_Bytes  table,
                         OTL_UInt   start,
                         OTL_UInt   count,
                         OTL_UInt  *features )
  {
    OTL_Bytes  p            = table + 4;
    OTL_UInt   num_features = OTL_NEXT_USHORT( p );
    OTL_UInt   result       = 0;


    p += start * 2;

    for ( ; count > 0 && start < num_features; start++, count-- )
    {
      features[0] = OTL_NEXT_USHORT( p );
      features++;
      result++;
    }

    return result;
  }
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                           SCRIPTS                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCALDEF( void )
  otl_script_validate( OTL_Bytes      table,
                       OTL_UInt       feature_count,
                       OTL_Validator  valid )
  {
    OTL_UInt   default_lang, num_langs;
    OTL_Bytes  p = table;


    OTL_CHECK( 4 );

    default_lang = OTL_NEXT_USHORT( p );
    num_langs    = OTL_NEXT_USHORT( p );

    if ( default_lang != 0 )
      otl_lang_validate( table + default_lang, feature_count, valid );

    OTL_CHECK( num_langs * 6 );

    /* scan langsys records */
    for ( ; num_langs > 0; num_langs-- )
    {
      p += 4;       /* skip tag */

      otl_lang_validate( table + OTL_NEXT_USHORT( p ), feature_count, valid );
    }
  }


  OTL_LOCALDEF( void )
  otl_script_list_validate( OTL_Bytes      table,
                            OTL_Bytes      features,
                            OTL_Validator  valid )
  {
    OTL_UInt   num_scripts, feature_count;
    OTL_Bytes  p = table;


    OTL_CHECK( 2 );
    num_scripts = OTL_NEXT_USHORT( p );
    OTL_CHECK( num_scripts * 6 );

    feature_count = otl_feature_list_get_count( features );

    /* scan script records */
    for ( ; num_scripts > 0; num_scripts-- )
    {
      p += 4;       /* skip tag */

      otl_script_validate( table + OTL_NEXT_USHORT( p ), feature_count,
                           valid );
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      UTILITY FUNCTIONS                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCALDEF( OTL_UInt )
  otl_gsubgpos_get_lookup_count( OTL_Bytes  table )
  {
    OTL_Bytes  p = table + 8;


    return otl_lookup_list_get_count( table + OTL_PEEK_USHORT( p ) );
  }


/* END */
