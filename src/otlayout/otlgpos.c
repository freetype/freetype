/***************************************************************************/
/*                                                                         */
/*  otlgpos.c                                                              */
/*                                                                         */
/*    OpenType layout support, GPOS table (body).                          */
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


#include "otlgpos.h"
#include "otlcommn.h"


 /* forward declaration */
  static OTL_ValidateFunc  otl_gpos_validate_funcs[];


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                        VALUE RECORDS                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static OTL_UInt
  otl_value_length( OTL_UInt  format )
  {
    OTL_UInt  count;


    count = ( ( format & 0xAA ) >> 1 ) + ( format & 0x55 );
    count = ( ( count  & 0xCC ) >> 2 ) + ( count  & 0x33 );
    count = ( ( count  & 0xF0 ) >> 4 ) + ( count  & 0x0F );

    return count * 2;
  }


  static void
  otl_value_validate( OTL_Bytes      table,
                      OTL_Bytes      pos_table,
                      OTL_UInt       format,
                      OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   count, device;


    if ( format >= 0x100 )
      OTL_INVALID_DATA;

    for ( count = 4; count > 0; count-- )
    {
      if ( format & 1 )
      {
        OTL_CHECK( 2 );
        p += 2;
      }

      format >>= 1;
    }

    for ( count = 4; count > 0; count-- )
    {
      if ( format & 1 )
      {
        OTL_CHECK( 2 );
        device = OTL_NEXT_USHORT( p );
        if ( device )
          otl_device_table_validate( pos_table + device, valid );
      }
      format >>= 1;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                           ANCHORS                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_anchor_validate( OTL_Bytes      table,
                       OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;


    OTL_CHECK( 6 );
    format = OTL_NEXT_USHORT( p );
    p += 4;     /* skip coordinates */

    switch ( format )
    {
    case 1:
      break;

    case 2:
      OTL_CHECK( 2 );  /* anchor point */
      break;

    case 3:
      {
        OTL_UInt  x_device, y_device;


        OTL_CHECK( 4 );
        x_device = OTL_NEXT_USHORT( p );
        y_device = OTL_NEXT_USHORT( p );

        if ( x_device )
          otl_device_table_validate( table + x_device, valid );

        if ( y_device )
          otl_device_table_validate( table + y_device, valid );
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                          MARK ARRAY                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_mark_array_validate( OTL_Bytes      table,
                           OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   count;

    OTL_CHECK( 2 );

    count = OTL_NEXT_USHORT( p );
    OTL_CHECK( count * 4 );
    for ( ; count > 0; count-- )
    {
      p += 2;  /* ignore class index */
      otl_anchor_validate( table + OTL_NEXT_USHORT( p ), valid );
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     GPOS LOOKUP TYPE 1                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_gpos_lookup1_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( lookup_count );
    OTL_UNUSED( glyph_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  coverage, value_format;


        OTL_CHECK( 4 );
        coverage     = OTL_NEXT_USHORT( p );
        value_format = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );
        otl_value_validate( p, table, value_format, valid );
      }
      break;

    case 2:
      {
        OTL_UInt  coverage, value_format, num_values, len_value;


        OTL_CHECK( 6 );
        coverage     = OTL_NEXT_USHORT( p );
        value_format = OTL_NEXT_USHORT( p );
        num_values   = OTL_NEXT_USHORT( p );

        len_value    = otl_value_length( value_format );

        otl_coverage_validate( table + coverage, valid );

        OTL_CHECK( num_values * len_value );

        /* scan value records */
        for ( ; num_values > 0; num_values-- )
        {
          otl_value_validate( p, table, value_format, valid );
          p += len_value;
        }
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     GPOS LOOKUP TYPE 2                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_gpos_pairset_validate( OTL_Bytes      table,
                             OTL_Bytes      pos_table,
                             OTL_UInt       format1,
                             OTL_UInt       format2,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   value_len1, value_len2, num_pairvalues;


    OTL_CHECK( 2 );
    num_pairvalues = OTL_NEXT_USHORT( p );
    value_len1     = otl_value_length( format1 );
    value_len2     = otl_value_length( format2 );

    OTL_CHECK( num_pairvalues * ( value_len1 + value_len2 + 2 ) );

    /* scan pair value records */
    for ( ; num_pairvalues > 0; num_pairvalues-- )
    {
      p += 2;       /* ignore glyph id */

      otl_value_validate( p, pos_table, format1, valid );
      p += value_len1;

      otl_value_validate( p, pos_table, format2, valid );
      p += value_len2;
    }
  }


  static void
  otl_gpos_lookup2_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( lookup_count );
    OTL_UNUSED( glyph_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  coverage, value1, value2, num_pairsets;


        OTL_CHECK( 8 );
        coverage     = OTL_NEXT_USHORT( p );
        value1       = OTL_NEXT_USHORT( p );
        value2       = OTL_NEXT_USHORT( p );
        num_pairsets = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        OTL_CHECK( num_pairsets * 2 );

        for ( ; num_pairsets > 0; num_pairsets-- )
          otl_gpos_pairset_validate( table + OTL_NEXT_USHORT( p ),
                                     table, value1, value2, valid );
      }
      break;

    case 2:
      {
        OTL_UInt  coverage, value1, value2, class1, class2;
        OTL_UInt  num_classes1, num_classes2, len_value1, len_value2, count;


        OTL_CHECK( 14 );
        coverage     = OTL_NEXT_USHORT( p );
        value1       = OTL_NEXT_USHORT( p );
        value2       = OTL_NEXT_USHORT( p );
        class1       = OTL_NEXT_USHORT( p );
        class2       = OTL_NEXT_USHORT( p );
        num_classes1 = OTL_NEXT_USHORT( p );
        num_classes2 = OTL_NEXT_USHORT( p );

        len_value1 = otl_value_length( value1 );
        len_value2 = otl_value_length( value2 );

        otl_coverage_validate( table + coverage, valid );
        otl_class_definition_validate( table + class1, valid );
        otl_class_definition_validate( table + class2, valid );

        OTL_CHECK( num_classes1 * num_classes2 *
                     ( len_value1 + len_value2 ) );

        for ( ; num_classes1 > 0; num_classes1-- )
        {
          for ( count = num_classes2; count > 0; count-- )
          {
            otl_value_validate( p, table, value1, valid );
            p += len_value1;

            otl_value_validate( p, table, value2, valid );
            p += len_value2;
          }
        }
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     GPOS LOOKUP TYPE 3                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_gpos_lookup3_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( lookup_count );
    OTL_UNUSED( glyph_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  coverage, num_entryexit, anchor1, anchor2;


        OTL_CHECK( 4 );
        coverage      = OTL_NEXT_USHORT( p );
        num_entryexit = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        OTL_CHECK( num_entryexit * 4 );

        /* scan entry-exit records */
        for ( ; num_entryexit > 0; num_entryexit-- )
        {
          anchor1 = OTL_NEXT_USHORT( p );
          anchor2 = OTL_NEXT_USHORT( p );

          if ( anchor1 )
            otl_anchor_validate( table + anchor1, valid );

          if ( anchor2 )
            otl_anchor_validate( table + anchor2, valid );
        }
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     GPOS LOOKUP TYPE 4                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_base_array_validate( OTL_Bytes      table,
                           OTL_UInt       class_count,
                           OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_bases, count;


    OTL_CHECK( 2 );
    num_bases = OTL_NEXT_USHORT( p );

    OTL_CHECK( num_bases * class_count * 2 );

    /* scan base array records */
    for ( ; num_bases > 0; num_bases-- )
      /* scan base records */
      for ( count = class_count; count > 0; count-- )
        otl_anchor_validate( table + OTL_NEXT_USHORT( p ), valid );
  }


  static void
  otl_gpos_lookup4_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( lookup_count );
    OTL_UNUSED( glyph_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  mark_coverage, base_coverage, num_classes;
        OTL_UInt  mark_array, base_array;


        OTL_CHECK( 10 );
        mark_coverage = OTL_NEXT_USHORT( p );
        base_coverage = OTL_NEXT_USHORT( p );
        num_classes   = OTL_NEXT_USHORT( p );
        mark_array    = OTL_NEXT_USHORT( p );
        base_array    = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + mark_coverage, valid );
        otl_coverage_validate( table + base_coverage, valid );

        otl_mark_array_validate( table + mark_array, valid );
        otl_base_array_validate( table + base_array, num_classes, valid );
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     GPOS LOOKUP TYPE 5                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* used by lookup type 5 and 6 */
  static void
  otl_liga_mark2_validate( OTL_Bytes      table,
                           OTL_UInt       class_count,
                           OTL_Bool       maybe_zero,
                           OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_components, count;


    OTL_CHECK( 2 );
    num_components = OTL_NEXT_USHORT( p );

    OTL_CHECK( num_components * class_count * 2 );

    /* scan component records */
    for ( ; num_components > 0; num_components-- )
      /* scan ligature anchor records */
      for ( count = class_count; count > 0; count-- )
      {
        OTL_UInt  offset = OTL_NEXT_USHORT( p );


        if ( !offset && maybe_zero )
          continue;
  
        otl_anchor_validate( table + offset, valid );
      }
  }


  static void
  otl_liga_array_validate( OTL_Bytes      table,
                           OTL_UInt       class_count,
                           OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   ligature_count;


    OTL_CHECK( 2 );
    ligature_count = OTL_NEXT_USHORT( p );

    OTL_CHECK( ligature_count * 2 );

    /* scan ligature attach records */
    for ( ; ligature_count > 0; ligature_count-- )
      otl_liga_mark2_validate( table + OTL_NEXT_USHORT( p ), class_count, 1,
                               valid );
  }


  static void
  otl_gpos_lookup5_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( lookup_count );
    OTL_UNUSED( glyph_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  mark_coverage, liga_coverage, num_classes;
        OTL_UInt  mark_array, liga_array;


        OTL_CHECK( 10 );
        mark_coverage = OTL_NEXT_USHORT( p );
        liga_coverage = OTL_NEXT_USHORT( p );
        num_classes   = OTL_NEXT_USHORT( p );
        mark_array    = OTL_NEXT_USHORT( p );
        liga_array    = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + mark_coverage, valid );
        otl_coverage_validate( table + liga_coverage, valid );

        otl_mark_array_validate( table + mark_array, valid );
        otl_liga_array_validate( table + liga_array, num_classes, valid );
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     GPOS LOOKUP TYPE 6                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_gpos_lookup6_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( lookup_count );
    OTL_UNUSED( glyph_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  coverage1, coverage2, num_classes, array1, array2;


        OTL_CHECK( 10 );
        coverage1   = OTL_NEXT_USHORT( p );
        coverage2   = OTL_NEXT_USHORT( p );
        num_classes = OTL_NEXT_USHORT( p );
        array1      = OTL_NEXT_USHORT( p );
        array2      = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage1, valid );
        otl_coverage_validate( table + coverage2, valid );

        otl_mark_array_validate( table + array1, valid );
        otl_liga_mark2_validate( table + array2, num_classes, 0, valid );
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     GPOS LOOKUP TYPE 7                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* used for both format 1 and 2 */
  static void
  otl_pos_rule_validate( OTL_Bytes      table,
                         OTL_UInt       lookup_count,
                         OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_glyphs, num_pos;


    OTL_CHECK( 4 );
    num_glyphs = OTL_NEXT_USHORT( p );
    num_pos    = OTL_NEXT_USHORT( p );

    if ( num_glyphs == 0 )
      OTL_INVALID_DATA;

    OTL_CHECK( ( num_glyphs - 1 ) * 2 + num_pos * 4 );

    for ( ; num_pos > 0; num_pos-- )
    {
      if ( OTL_NEXT_USHORT( p ) >= num_glyphs )
        OTL_INVALID_DATA;

      if ( OTL_NEXT_USHORT( p ) >= lookup_count )
        OTL_INVALID_DATA;
    }

    /* no need to check glyph indices/classes used as input for this  */
    /* context rule since even invalid glyph indices/classes return a */
    /* meaningful result                                              */
  }


  /* used for both format 1 and 2 */
  static void
  otl_pos_rule_set_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_posrules;


    OTL_CHECK( 2 );
    num_posrules = OTL_NEXT_USHORT( p );

    OTL_CHECK( num_posrules * 2 );

    /* scan posrule records */
    for ( ; num_posrules > 0; num_posrules-- )
      otl_pos_rule_validate( table + OTL_NEXT_USHORT( p ), lookup_count,
                             valid );
  }


  static void
  otl_gpos_lookup7_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( glyph_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  coverage, num_posrule_sets;


        OTL_CHECK( 4 );
        coverage         = OTL_NEXT_USHORT( p );
        num_posrule_sets = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        OTL_CHECK( num_posrule_sets * 2 );

        /* scan posrule set records */
        for ( ; num_posrule_sets > 0; num_posrule_sets-- )
          otl_pos_rule_set_validate( table + OTL_NEXT_USHORT( p ),
                                     lookup_count, valid );
      }
      break;

    case 2:
      {
        OTL_UInt  coverage, class_def, num_posclass_sets;


        OTL_CHECK( 6 );
        coverage          = OTL_NEXT_USHORT( p );
        class_def         = OTL_NEXT_USHORT( p );
        num_posclass_sets = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );
        otl_class_definition_validate( table + class_def, valid );

        OTL_CHECK( num_posclass_sets * 2 );

        /* scan pos class set rules */
        for ( ; num_posclass_sets > 0; num_posclass_sets-- )
        {
          OTL_UInt  offset = OTL_NEXT_USHORT( p );


          if ( offset )
            otl_pos_rule_set_validate( table + offset, lookup_count, valid );
        }
      }
      break;

    case 3:
      {
        OTL_UInt  num_glyphs, num_pos, count;


        OTL_CHECK( 4 );
        num_glyphs = OTL_NEXT_USHORT( p );
        num_pos    = OTL_NEXT_USHORT( p );

        OTL_CHECK( num_glyphs * 2 + num_pos * 4 );

        for ( count = num_glyphs; count > 0; count-- )
          otl_coverage_validate( table + OTL_NEXT_USHORT( p ), valid );

        for ( ; num_pos > 0; num_pos-- )
        {
          if ( OTL_NEXT_USHORT( p ) >= num_glyphs )
            OTL_INVALID_DATA;

          if ( OTL_NEXT_USHORT( p ) >= lookup_count )
            OTL_INVALID_DATA;
        }
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     GPOS LOOKUP TYPE 8                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* used for both format 1 and 2 */
  static void
  otl_chain_pos_rule_validate( OTL_Bytes      table,
                               OTL_UInt       lookup_count,
                               OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_backtrack_glyphs, num_input_glyphs, num_lookahead_glyphs;
    OTL_UInt   num_pos;


    OTL_CHECK( 2 );
    num_backtrack_glyphs = OTL_NEXT_USHORT( p );

    OTL_CHECK( num_backtrack_glyphs * 2 + 2 );
    p += num_backtrack_glyphs * 2;

    num_input_glyphs = OTL_NEXT_USHORT( p );
    if ( num_input_glyphs == 0 )
      OTL_INVALID_DATA;

    OTL_CHECK( num_input_glyphs * 2 );
    p += ( num_input_glyphs - 1 ) * 2;

    num_lookahead_glyphs = OTL_NEXT_USHORT( p );
    OTL_CHECK( num_lookahead_glyphs * 2 + 2 );
    p += num_lookahead_glyphs * 2;

    num_pos = OTL_NEXT_USHORT( p );
    OTL_CHECK( num_pos * 4 );

    for ( ; num_pos > 0; num_pos-- )
    {
      if ( OTL_NEXT_USHORT( p ) >= num_input_glyphs )
        OTL_INVALID_DATA;

      if ( OTL_NEXT_USHORT( p ) >= lookup_count )
        OTL_INVALID_DATA;
    }

    /* no need to check glyph indices/classes used as input for this  */
    /* context rule since even invalid glyph indices/classes return a */
    /* meaningful result                                              */
  }


  /* used for both format 1 and 2 */
  static void
  otl_chain_pos_rule_set_validate( OTL_Bytes      table,
                                   OTL_UInt       lookup_count,
                                   OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_chain_subrules;


    OTL_CHECK( 2 );
    num_chain_subrules = OTL_NEXT_USHORT( p );

    OTL_CHECK( num_chain_subrules * 2 );

    /* scan chain pos rule records */
    for ( ; num_chain_subrules > 0; num_chain_subrules-- )
      otl_chain_pos_rule_validate( table + OTL_NEXT_USHORT( p ),
                                   lookup_count, valid );
  }


  static void
  otl_gpos_lookup8_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( glyph_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  coverage, num_chain_pos_rulesets;


        OTL_CHECK( 4 );
        coverage               = OTL_NEXT_USHORT( p );
        num_chain_pos_rulesets = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        OTL_CHECK( num_chain_pos_rulesets * 2 );

        /* scan chain pos ruleset records */
        for ( ; num_chain_pos_rulesets > 0; num_chain_pos_rulesets-- )
          otl_chain_pos_rule_set_validate( table + OTL_NEXT_USHORT( p ),
                                           lookup_count, valid );
      }
      break;

    case 2:
      {
        OTL_UInt  coverage, back_class, input_class, ahead_class;
        OTL_UInt  num_chainpos_class_sets;


        OTL_CHECK( 10 );
        coverage                = OTL_NEXT_USHORT( p );
        back_class              = OTL_NEXT_USHORT( p );
        input_class             = OTL_NEXT_USHORT( p );
        ahead_class             = OTL_NEXT_USHORT( p );
        num_chainpos_class_sets = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        otl_class_definition_validate( table + back_class,  valid );
        otl_class_definition_validate( table + input_class, valid );
        otl_class_definition_validate( table + ahead_class, valid );

        OTL_CHECK( num_chainpos_class_sets * 2 );

        /* scan chainpos class set records */
        for ( ; num_chainpos_class_sets > 0; num_chainpos_class_sets-- )
        {
          OTL_UInt  offset = OTL_NEXT_USHORT( p );


          if ( offset )
            otl_chain_pos_rule_set_validate( table + offset, lookup_count,
                                             valid );
        }
      }
      break;

    case 3:
      {
        OTL_UInt  num_backtrack_glyphs, num_input_glyphs;
        OTL_UInt  num_lookahead_glyphs, num_pos, count;


        OTL_CHECK( 2 );

        num_backtrack_glyphs = OTL_NEXT_USHORT( p );
        OTL_CHECK( num_backtrack_glyphs * 2 + 2 );

        for ( ; num_backtrack_glyphs > 0; num_backtrack_glyphs-- )
          otl_coverage_validate( table + OTL_NEXT_USHORT( p ), valid );

        num_input_glyphs = OTL_NEXT_USHORT( p );
        OTL_CHECK( num_input_glyphs * 2 + 2 );

        for ( count = num_input_glyphs; count > 0; count-- )
          otl_coverage_validate( table + OTL_NEXT_USHORT( p ), valid );

        num_lookahead_glyphs = OTL_NEXT_USHORT( p );
        OTL_CHECK( num_lookahead_glyphs * 2 + 2 );

        for ( ; num_lookahead_glyphs > 0; num_lookahead_glyphs-- )
          otl_coverage_validate( table + OTL_NEXT_USHORT( p ), valid );

        num_pos = OTL_NEXT_USHORT( p );
        OTL_CHECK( num_pos * 4 );

        for ( ; num_pos > 0; num_pos-- )
        {
          if ( OTL_NEXT_USHORT( p ) >= num_input_glyphs )
            OTL_INVALID_DATA;

          if ( OTL_NEXT_USHORT( p ) >= lookup_count )
            OTL_INVALID_DATA;
        }
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                     GPOS LOOKUP TYPE 9                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_gpos_lookup9_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt          lookup_type, lookup_offset;
        OTL_ValidateFunc  validate;


        OTL_CHECK( 6 );
        lookup_type   = OTL_NEXT_USHORT( p );
        lookup_offset = OTL_NEXT_ULONG( p );

        if ( lookup_type == 0 || lookup_type >= 9 )
          OTL_INVALID_DATA;

        validate = otl_gpos_validate_funcs[lookup_type - 1];
        validate( table + lookup_offset, lookup_count, glyph_count, valid );
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  static OTL_ValidateFunc  otl_gpos_validate_funcs[9] =
  {
    otl_gpos_lookup1_validate,
    otl_gpos_lookup2_validate,
    otl_gpos_lookup3_validate,
    otl_gpos_lookup4_validate,
    otl_gpos_lookup5_validate,
    otl_gpos_lookup6_validate,
    otl_gpos_lookup7_validate,
    otl_gpos_lookup8_validate,
    otl_gpos_lookup9_validate
  };


  OTL_LOCALDEF( void )
  otl_gpos_subtable_validate( OTL_Bytes      table,
                              OTL_UInt       lookup_count,
                              OTL_UInt       glyph_count,
                              OTL_Validator  valid )
  {
    otl_lookup_validate( table, 9, otl_gpos_validate_funcs,
                         lookup_count, glyph_count, valid );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                          GPOS TABLE                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCALDEF( void )
  otl_gpos_validate( OTL_Bytes      table,
                     OTL_UInt       glyph_count,
                     OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   scripts, features, lookups;


    OTL_CHECK( 10 );

    if ( OTL_NEXT_USHORT( p ) != 0x10000UL )
      OTL_INVALID_DATA;

    scripts  = OTL_NEXT_USHORT( p );
    features = OTL_NEXT_USHORT( p );
    lookups  = OTL_NEXT_USHORT( p );

    otl_lookup_list_validate( table + lookups, 9, otl_gpos_validate_funcs,
                              glyph_count, valid );
    otl_feature_list_validate( table + features, table + lookups, valid );
    otl_script_list_validate( table + scripts, table + features, valid );
  }


/* END */
