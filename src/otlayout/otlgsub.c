/***************************************************************************/
/*                                                                         */
/*  otlgsub.c                                                              */
/*                                                                         */
/*    OpenType layout support, GSUB table (body).                          */
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


#include "otlgsub.h"
#include "otlcommn.h"
#include "otlparse.h"


  /* forward declaration */
  static OTL_ValidateFunc  otl_gsub_validate_funcs[];


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                  GSUB LOOKUP TYPE 1                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /*
   * 1: Single Substitution - Table format(s)
   *
   * This table is used to substitute individual glyph indices
   * with another one.  There are only two sub-formats:
   *
   *   Name         Offset    Size       Description
   *   --------------------------------------------------------------
   *   format       0         2          sub-table format (1)
   *   offset       2         2          offset to coverage table
   *   delta        4         2          16-bit delta to apply on all
   *                                     covered glyph indices
   *
   *   Name         Offset    Size       Description
   *   --------------------------------------------------------------
   *   format       0         2          sub-table format (2)
   *   offset       2         2          offset to coverage table
   *   count        4         2          coverage table count
   *   substs[]     6         2*count    substituted glyph indices,
   *
   */

  static void
  otl_gsub_lookup1_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( lookup_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );

    switch ( format )
    {
    case 1:
      {
        OTL_Bytes  coverage;
        OTL_Int    delta;
        OTL_Long   idx;


        OTL_CHECK( 4 );
        coverage = table + OTL_NEXT_USHORT( p );
        delta    = OTL_NEXT_SHORT( p );

        otl_coverage_validate( coverage, valid );

        idx = otl_coverage_get_first( coverage ) + delta;
        if ( idx < 0 )
          OTL_INVALID_DATA;

        idx = otl_coverage_get_last( coverage ) + delta;
        if ( (OTL_UInt)idx >= glyph_count )
          OTL_INVALID_DATA;
      }
      break;

    case 2:
      {
        OTL_UInt  coverage, num_glyphs;


        OTL_CHECK( 4 );
        coverage   = OTL_NEXT_USHORT( p );
        num_glyphs = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        OTL_CHECK( num_glyphs * 2 );

        for ( ; num_glyphs > 0; num_glyphs-- )
          if ( OTL_NEXT_USHORT( p ) >= glyph_count )
            OTL_INVALID_DATA;
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


#if 0
  static OTL_Bool
  otl_gsub_lookup1_apply( OTL_Bytes   table,
                          OTL_Parser  parser )
  {
    OTL_Bytes  p = table;
    OTL_Bytes  coverage;
    OTL_UInt   format, gindex, property;
    OTL_Long   index;
    OTL_Bool   subst = 0;


    if ( parser->context_len != 0xFFFFU && parser->context_len < 1 )
      goto Exit;

    gindex = otl_parser_get_gindex( parser );

    otl_parser_check_property( parser, gindex, &property );
    if ( parser->error )
      goto Exit;

    format   = OTL_NEXT_USHORT(p);
    coverage = table + OTL_NEXT_USHORT(p);
    index    = otl_coverage_get_index( coverage, gindex );

    if ( index >= 0 )
    {
      switch ( format )
      {
      case 1:
        {
          OTL_Int  delta = OTL_NEXT_SHORT( p );


          gindex = ( gindex + delta ) & 0xFFFFU;
          otl_parser_replace_1( parser, gindex );
          subst = 1;
        }
        break;

      case 2:
        {
          OTL_UInt  count = OTL_NEXT_USHORT( p );


          if ( (OTL_UInt)index < count )
          {
            p += index * 2;
            otl_parser_replace_1( parser, OTL_PEEK_USHORT( p ) );
            subst = 1;
          }
        }
        break;

      default:
        ;
      }
    }

  Exit:
    return subst;
  }
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                  GSUB LOOKUP TYPE 2                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /*
   * 2: Multiple Substitution - Table format(s)
   *
   * Replaces a single glyph with one or more glyphs.
   *
   *   Name         Offset    Size       Description
   *   --------------------------------------------------------------
   *   format       0         2          sub-table format (1)
   *   offset       2         2          offset to coverage table
   *   count        4         2          coverage table count
   *   sequencess[] 6         2*count    offsets to sequence items
   *
   * each sequence item has the following format:
   *
   *   Name         Offset    Size       Description
   *   --------------------------------------------------------------
   *   count        0         2          number of replacement glyphs
   *   gindices[]   2         2*count    string of glyph indices
   *
   */

  static void
  otl_sequence_validate( OTL_Bytes      table,
                         OTL_UInt       glyph_count,
                         OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_glyphs;


    OTL_CHECK( 2 );
    num_glyphs = OTL_NEXT_USHORT( p );

    /* according to the specification, `num_glyphs' should be > 0; */
    /* we can deal with these cases pretty well, however           */

    OTL_CHECK( num_glyphs * 2 );

    for ( ; num_glyphs > 0; num_glyphs-- )
      if ( OTL_NEXT_USHORT( p ) >= glyph_count )
        OTL_INVALID_DATA;
  }


  static void
  otl_gsub_lookup2_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( lookup_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  coverage, num_sequences;


        OTL_CHECK( 4 );
        coverage      = OTL_NEXT_USHORT( p );
        num_sequences = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        OTL_CHECK( num_sequences * 2 );

        /* scan sequence records */
        for ( ; num_sequences > 0; num_sequences-- )
          otl_sequence_validate( table + OTL_NEXT_USHORT( p ), glyph_count,
                                 valid );
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


#if 0
  static OTL_Bool
  otl_gsub_lookup2_apply( OTL_Bytes    table,
                          OTL_Parser   parser )
  {
    OTL_Bytes  p = table;
    OTL_Bytes  coverage, sequence;
    OTL_UInt   format, gindex, property, context_len, seq_count, count;
    OTL_Long   index;
    OTL_Bool   subst = 0;


    if ( parser->context_len != 0xFFFFU && parser->context_len < 1 )
      goto Exit;

    gindex = otl_parser_get_gindex( parser );

    otl_parser_check_property( parser, gindex, &property );
    if ( parser->error )
      goto Exit;

    p        += 2;                              /* skip format */
    coverage  = table + OTL_NEXT_USHORT( p );
    seq_count = OTL_NEXT_USHORT( p );
    index     = otl_coverage_get_index( coverage, gindex );

    if ( (OTL_UInt)index >= seq_count || index < 0 )
      goto Exit;

    p       += index * 2;
    sequence = table + OTL_PEEK_USHORT( p );
    p        = sequence;
    count    = OTL_NEXT_USHORT( p );

    otl_parser_replace_n( parser, count, p );
    subst = 1;

   Exit:
    return subst;
  }
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    GSUB LOOKUP TYPE 3                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /*
   * 3: Alternate Substitution - Table format(s)
   *
   * Replaces a single glyph by another one taken liberally
   * in a list of alternatives.
   *
   *   Name         Offset    Size       Description
   *   ---------------------------------------------------------------
   *   format       0         2          sub-table format (1)
   *   offset       2         2          offset to coverage table
   *   count        4         2          coverage table count
   *   alternates[] 6         2*count    offsets to alternate items
   *
   * each alternate item has the following format:
   *
   *   Name         Offset    Size       Description
   *   ---------------------------------------------------------------
   *   count        0         2          number of replacement glyphs
   *   gindices[]   2         2*count    string of glyph indices, each
   *                                     one is a valid alternative
   *
   */

  static void
  otl_alternate_set_validate( OTL_Bytes      table,
                              OTL_UInt       glyph_count,
                              OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_glyphs;


    OTL_CHECK( 2 );
    num_glyphs = OTL_NEXT_USHORT( p );

    OTL_CHECK( num_glyphs * 2 );

    for ( ; num_glyphs > 0; num_glyphs-- )
      if ( OTL_NEXT_USHORT( p ) >= glyph_count )
        OTL_INVALID_DATA;
  }


  static void
  otl_gsub_lookup3_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( lookup_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  coverage, num_alternate_sets;


        OTL_CHECK( 4 );
        coverage           = OTL_NEXT_USHORT( p );
        num_alternate_sets = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        OTL_CHECK( num_alternate_sets * 2 );

        /* scan alternate set records */
        for ( ; num_alternate_sets > 0; num_alternate_sets-- )
          otl_alternate_set_validate( table + OTL_NEXT_USHORT( p ),
                                      glyph_count, valid );
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


#if 0
  static OTL_Bool
  otl_gsub_lookup3_apply( OTL_Bytes    table,
                          OTL_Parser   parser )
  {
    OTL_Bytes  p = table;
    OTL_Bytes  coverage, alternates;
    OTL_UInt   format, gindex, property, seq_count, count;
    OTL_Long   index;
    OTL_Bool   subst = 0;

    OTL_GSUB_Alternate  alternate = parser->alternate;


    if ( parser->context_len != 0xFFFFU && parser->context_len < 1 )
      goto Exit;

    if ( alternate == 0 )
      goto Exit;

    gindex = otl_parser_get_gindex( parser );

    otl_parser_check_property( parser, gindex, &property );
    if ( parser->error )
      goto Exit;

    p        += 2;                              /* skip format */
    coverage  = table + OTL_NEXT_USHORT( p );
    seq_count = OTL_NEXT_USHORT( p );
    index     = otl_coverage_get_index( coverage, gindex );

    if ( (OTL_UInt)index >= seq_count || index < 0 )
      goto Exit;

    p         += index * 2;
    alternates = table + OTL_PEEK_USHORT( p );
    p          = alternates;
    count      = OTL_NEXT_USHORT( p );

    gindex = alternate->handler_func(
                 gindex, count, p, alternate->handler_data );

    otl_parser_replace_1( parser, gindex );
    subst = 1;

   Exit:
    return subst;
  }
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    GSUB LOOKUP TYPE 4                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_ligature_validate( OTL_Bytes      table,
                         OTL_UInt       glyph_count,
                         OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   glyph_id, num_components;


    OTL_CHECK( 4 );
    glyph_id       = OTL_NEXT_USHORT( p );
    num_components = OTL_NEXT_USHORT( p );

    if ( num_components == 0 )
      OTL_INVALID_DATA;

    num_components--;

    OTL_CHECK( num_components * 2 );

    for ( ; num_components > 0; num_components-- )
      if ( OTL_NEXT_USHORT( p ) >= glyph_count )
        OTL_INVALID_DATA;
  }


  static void
  otl_ligature_set_validate( OTL_Bytes      table,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_ligatures;


    OTL_CHECK( 2 );
    num_ligatures = OTL_NEXT_USHORT( p );

    OTL_CHECK( num_ligatures * 2 );

    /* scan ligature records */
    for ( ; num_ligatures > 0; num_ligatures-- )
      otl_ligature_validate( table + OTL_NEXT_USHORT( p ), glyph_count,
                             valid );
  }


  static void
  otl_gsub_lookup4_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   format;

    OTL_UNUSED( lookup_count);


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      {
        OTL_UInt  coverage, num_ligsets;


        OTL_CHECK( 4 );
        coverage    = OTL_NEXT_USHORT( p );
        num_ligsets = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        OTL_CHECK( num_ligsets * 2 );

        /* scan ligature set records */
        for ( ; num_ligsets > 0; num_ligsets-- )
          otl_ligature_set_validate( table + OTL_NEXT_USHORT( p ),
                                     glyph_count, valid );
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                  GSUB LOOKUP TYPE 5                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* used for both format 1 and 2 */
  static void
  otl_sub_rule_validate( OTL_Bytes      table,
                         OTL_UInt       lookup_count,
                         OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_glyphs, num_subst;


    OTL_CHECK( 4 );
    num_glyphs = OTL_NEXT_USHORT( p );
    num_subst  = OTL_NEXT_USHORT( p );

    if ( num_glyphs == 0 )
      OTL_INVALID_DATA;

    OTL_CHECK( ( num_glyphs - 1 ) * 2 + num_subst * 4 );

    for ( ; num_subst > 0; num_subst-- )
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
  otl_sub_rule_set_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_subrules;


    OTL_CHECK( 2 );
    num_subrules = OTL_NEXT_USHORT( p );

    OTL_CHECK( num_subrules * 2 );

    /* scan subrule records */
    for ( ; num_subrules > 0; num_subrules-- )
      otl_sub_rule_validate( table + OTL_NEXT_USHORT( p ), lookup_count,
                             valid );
  }


  static void
  otl_gsub_lookup5_validate( OTL_Bytes      table,
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
        OTL_UInt  coverage, num_subrulesets;


        OTL_CHECK( 4 );
        coverage        = OTL_NEXT_USHORT( p );
        num_subrulesets = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        OTL_CHECK( num_subrulesets * 2 );

        /* scan subrule set records */
        for ( ; num_subrulesets > 0; num_subrulesets-- )
          otl_sub_rule_set_validate( table + OTL_NEXT_USHORT( p ),
                                     lookup_count, valid );
      }
      break;

    case 2:
      {
        OTL_UInt  coverage, class_def, num_subclass_sets;


        OTL_CHECK( 6 );
        coverage          = OTL_NEXT_USHORT( p );
        class_def         = OTL_NEXT_USHORT( p );
        num_subclass_sets = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );
        otl_class_definition_validate( table + class_def, valid );

        OTL_CHECK( num_subclass_sets * 2 );

        /* scan subclass set records */
        for ( ; num_subclass_sets > 0; num_subclass_sets-- )
        {
          OTL_UInt  offset = OTL_NEXT_USHORT( p );


          if ( offset )
            otl_sub_rule_set_validate( table + offset, lookup_count, valid );
        }
      }
      break;

    case 3:
      {
        OTL_UInt  num_glyphs, num_subst, count;


        OTL_CHECK( 4 );
        num_glyphs = OTL_NEXT_USHORT( p );
        num_subst  = OTL_NEXT_USHORT( p );

        OTL_CHECK( num_glyphs * 2 + num_subst * 4 );

        for ( count = num_glyphs; count > 0; count-- )
          otl_coverage_validate( table + OTL_NEXT_USHORT( p ), valid );

        for ( ; num_subst > 0; num_subst-- )
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
  /*****                    GSUB LOOKUP TYPE 6                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* used for both format 1 and 2 */
  static void
  otl_chain_sub_rule_validate( OTL_Bytes      table,
                               OTL_UInt       lookup_count,
                               OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_backtrack_glyphs, num_input_glyphs, num_lookahead_glyphs;
    OTL_UInt   num_subst;


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

    num_subst = OTL_NEXT_USHORT( p );
    OTL_CHECK( num_subst * 4 );

    for ( ; num_subst > 0; num_subst-- )
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
  otl_chain_sub_rule_set_validate( OTL_Bytes      table,
                                   OTL_UInt       lookup_count,
                                   OTL_Validator  valid )
  {
    OTL_Bytes  p = table;
    OTL_UInt   num_chain_subrules;


    OTL_CHECK( 2 );
    num_chain_subrules = OTL_NEXT_USHORT( p );

    OTL_CHECK( num_chain_subrules * 2 );

    /* scan chain subst rule records */
    for ( ; num_chain_subrules > 0; num_chain_subrules-- )
      otl_chain_sub_rule_validate( table + OTL_NEXT_USHORT( p ),
                                   lookup_count, valid );
  }


  static void
  otl_gsub_lookup6_validate( OTL_Bytes      table,
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
        OTL_UInt  coverage, num_chain_subrulesets;


        OTL_CHECK( 4 );
        coverage              = OTL_NEXT_USHORT( p );
        num_chain_subrulesets = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        OTL_CHECK( num_chain_subrulesets * 2 );

        /* scan chain subrule set records */
        for ( ; num_chain_subrulesets > 0; num_chain_subrulesets-- )
          otl_chain_sub_rule_set_validate( table + OTL_NEXT_USHORT( p ),
                                           lookup_count, valid );
      }
      break;

    case 2:
      {
        OTL_UInt  coverage, back_class, input_class, ahead_class;
        OTL_UInt  num_chain_subclass_sets;


        OTL_CHECK( 10 );
        coverage                = OTL_NEXT_USHORT( p );
        back_class              = OTL_NEXT_USHORT( p );
        input_class             = OTL_NEXT_USHORT( p );
        ahead_class             = OTL_NEXT_USHORT( p );
        num_chain_subclass_sets = OTL_NEXT_USHORT( p );

        otl_coverage_validate( table + coverage, valid );

        otl_class_definition_validate( table + back_class,  valid );
        otl_class_definition_validate( table + input_class, valid );
        otl_class_definition_validate( table + ahead_class, valid );

        OTL_CHECK( num_chain_subclass_sets * 2 );

        /* scan chain subclass set records */
        for ( ; num_chain_subclass_sets > 0; num_chain_subclass_sets-- )
        {
          OTL_UInt  offset = OTL_NEXT_USHORT( p );


          if ( offset )
            otl_chain_sub_rule_set_validate( table + offset, lookup_count,
                                             valid );
        }
      }
      break;

    case 3:
      {
        OTL_UInt  num_backtrack_glyphs, num_input_glyphs;
        OTL_UInt  num_lookahead_glyphs, num_subst, count;


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

        num_subst = OTL_NEXT_USHORT( p );
        OTL_CHECK( num_subst * 4 );

        for ( ; num_subst > 0; num_subst-- )
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
  /*****                    GSUB LOOKUP TYPE 7                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_gsub_lookup7_validate( OTL_Bytes      table,
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

        if ( lookup_type == 0 || lookup_type == 7 || lookup_type > 8 )
          OTL_INVALID_DATA;

        validate = otl_gsub_validate_funcs[lookup_type - 1];
        validate( table + lookup_offset, lookup_count, glyph_count, valid );
      }
      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    GSUB LOOKUP TYPE 8                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  otl_gsub_lookup8_validate( OTL_Bytes      table,
                             OTL_UInt       lookup_count,
                             OTL_UInt       glyph_count,
                             OTL_Validator  valid )
  {
    OTL_Bytes  p = table, coverage;
    OTL_UInt   format;
    OTL_UInt   num_backtrack_glyphs, num_lookahead_glyphs, num_glyphs;

    OTL_UNUSED( lookup_count );


    OTL_CHECK( 2 );
    format = OTL_NEXT_USHORT( p );
    switch ( format )
    {
    case 1:
      OTL_CHECK( 4 );

      coverage = table + OTL_NEXT_USHORT( p );
      num_backtrack_glyphs = OTL_NEXT_USHORT( p );

      otl_coverage_validate( coverage, valid );

      OTL_CHECK( num_backtrack_glyphs * 2 + 2 );

      for ( ; num_backtrack_glyphs > 0; num_backtrack_glyphs-- )
        otl_coverage_validate( table + OTL_NEXT_USHORT( p ), valid );

      num_lookahead_glyphs = OTL_NEXT_USHORT( p );
      OTL_CHECK( num_lookahead_glyphs * 2 + 2 );

      for ( ; num_lookahead_glyphs > 0; num_lookahead_glyphs-- )
        otl_coverage_validate( table + OTL_NEXT_USHORT( p ), valid );

      num_glyphs = OTL_NEXT_USHORT( p );
      if ( num_glyphs != otl_coverage_get_count( coverage ) )
        OTL_INVALID_DATA;

      OTL_CHECK( num_glyphs * 2 );

      for ( ; num_glyphs > 0; num_glyphs-- )
        if ( OTL_NEXT_USHORT( p ) >= glyph_count )
          OTL_INVALID_DATA;

      break;

    default:
      OTL_INVALID_DATA;
    }
  }


  static OTL_ValidateFunc  otl_gsub_validate_funcs[8] =
  {
    otl_gsub_lookup1_validate,
    otl_gsub_lookup2_validate,
    otl_gsub_lookup3_validate,
    otl_gsub_lookup4_validate,
    otl_gsub_lookup5_validate,
    otl_gsub_lookup6_validate,
    otl_gsub_lookup7_validate,
    otl_gsub_lookup8_validate
  };


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                          GSUB TABLE                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  OTL_LOCALDEF( void )
  otl_gsub_validate( OTL_Bytes      table,
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

    otl_lookup_list_validate( table + lookups, 8, otl_gsub_validate_funcs,
                              glyph_count, valid );
    otl_feature_list_validate( table + features, table + lookups, valid );
    otl_script_list_validate( table + scripts, table + features, valid );
  }


/* END */
