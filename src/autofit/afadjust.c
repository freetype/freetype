/****************************************************************************
 *
 * afadjust.c
 *
 *   Auto-fitter routines to adjust components based on charcode (body).
 *
 * Copyright (C) 2023-2024 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * Written by Craig White <gerzytet@gmail.com>.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

#include "afadjust.h"

#include <freetype/freetype.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftmemory.h>
#include <freetype/internal/ftdebug.h>

#define AF_ADJUSTMENT_DATABASE_LENGTH           \
          ( sizeof ( adjustment_database ) /    \
            sizeof ( adjustment_database[0] ) )

#undef  FT_COMPONENT
#define FT_COMPONENT  afadjust


  /*
    All entries in this list must be sorted by ascending Unicode code
    points.  The table entries are 3 numbers consisting of:

    - Unicode code point.
    - The vertical adjustment type.  This should be one of the enum
      constants in `AF_VerticalSeparationAdjustmentType`.
    - Value 1 if the topmost contour is a tilde and should be prevented from
      flattening, and 0 otherwise.
  */
  FT_LOCAL_ARRAY_DEF( AF_AdjustmentDatabaseEntry )
  adjustment_database[] =
  {
    { 0x21,  AF_ADJUST_UP, 0 }, /* ! */
    { 0x3F,  AF_ADJUST_UP, 0 }, /* ? */
    { 0x69,  AF_ADJUST_UP, 0 }, /* i */
    { 0x6A,  AF_ADJUST_UP, 0 }, /* j */
#if 0
    /* XXX TODO */
    { 0x7E,  AF_ADJUST_NONE, 1 }, /* ~ */
#endif

    { 0xA1,  AF_ADJUST_UP, 0 }, /* ¡ */
    { 0xA6,  AF_ADJUST_UP, 0 }, /* ¦ */
    { 0xAA,  AF_ADJUST_UP, 0 }, /* ª */
    { 0xBA,  AF_ADJUST_UP, 0 }, /* º */
    { 0xBF,  AF_ADJUST_UP, 0 }, /* ¿ */

    { 0xC0,  AF_ADJUST_UP, 0 }, /* À */
    { 0xC1,  AF_ADJUST_UP, 0 }, /* Á */
    { 0xC2,  AF_ADJUST_UP, 0 }, /* Â */
    { 0xC3,  AF_ADJUST_UP, 1 }, /* Ã */
    { 0xC4,  AF_ADJUST_UP, 0 }, /* Ä */
    { 0xC5,  AF_ADJUST_UP, 0 }, /* Å */
    { 0xC8,  AF_ADJUST_UP, 0 }, /* È */
    { 0xC9,  AF_ADJUST_UP, 0 }, /* É */
    { 0xCA,  AF_ADJUST_UP, 0 }, /* Ê */
    { 0xCB,  AF_ADJUST_UP, 0 }, /* Ë */
    { 0xCC,  AF_ADJUST_UP, 0 }, /* Ì */
    { 0xCD,  AF_ADJUST_UP, 0 }, /* Í */
    { 0xCE,  AF_ADJUST_UP, 0 }, /* Î */
    { 0xCF,  AF_ADJUST_UP, 0 }, /* Ï */

    { 0xD1,  AF_ADJUST_UP, 1 }, /* Ñ */
    { 0xD2,  AF_ADJUST_UP, 0 }, /* Ò */
    { 0xD3,  AF_ADJUST_UP, 0 }, /* Ó */
    { 0xD4,  AF_ADJUST_UP, 0 }, /* Ô */
    { 0xD5,  AF_ADJUST_UP, 1 }, /* Õ */
    { 0xD6,  AF_ADJUST_UP, 0 }, /* Ö */
    { 0xD9,  AF_ADJUST_UP, 0 }, /* Ù */
    { 0xDA,  AF_ADJUST_UP, 0 }, /* Ú */
    { 0xDB,  AF_ADJUST_UP, 0 }, /* Û */
    { 0xDC,  AF_ADJUST_UP, 0 }, /* Ü */
    { 0xDD,  AF_ADJUST_UP, 0 }, /* Ý */

    { 0xE0,  AF_ADJUST_UP, 0 }, /* à */
    { 0xE1,  AF_ADJUST_UP, 0 }, /* á */
    { 0xE2,  AF_ADJUST_UP, 0 }, /* â */
    { 0xE3,  AF_ADJUST_UP, 1 }, /* ã */
    { 0xE4,  AF_ADJUST_UP, 0 }, /* ä */
    { 0xE5,  AF_ADJUST_UP, 0 }, /* å */
    { 0xE8,  AF_ADJUST_UP, 0 }, /* è */
    { 0xE9,  AF_ADJUST_UP, 0 }, /* é */
    { 0xEA,  AF_ADJUST_UP, 0 }, /* ê */
    { 0xEB,  AF_ADJUST_UP, 0 }, /* ë */
    { 0xEC,  AF_ADJUST_UP, 0 }, /* ì */
    { 0xED,  AF_ADJUST_UP, 0 }, /* í */
    { 0xEE,  AF_ADJUST_UP, 0 }, /* î */
    { 0xEF,  AF_ADJUST_UP, 0 }, /* ï */

    { 0xF1,  AF_ADJUST_UP, 1 }, /* ñ */
    { 0xF2,  AF_ADJUST_UP, 0 }, /* ò */
    { 0xF3,  AF_ADJUST_UP, 0 }, /* ó */
    { 0xF4,  AF_ADJUST_UP, 0 }, /* ô */
    { 0xF5,  AF_ADJUST_UP, 1 }, /* õ */
    { 0xF6,  AF_ADJUST_UP, 0 }, /* ö */
    { 0xF9,  AF_ADJUST_UP, 0 }, /* ù */
    { 0xFA,  AF_ADJUST_UP, 0 }, /* ú */
    { 0xFB,  AF_ADJUST_UP, 0 }, /* û */
    { 0xFC,  AF_ADJUST_UP, 0 }, /* ü */
    { 0xFD,  AF_ADJUST_UP, 0 }, /* ý */
    { 0xFF,  AF_ADJUST_UP, 0 }, /* ÿ */

    { 0x100, AF_ADJUST_UP, 0 }, /* Ā */
    { 0x101, AF_ADJUST_UP, 0 }, /* ā */
    { 0x102, AF_ADJUST_UP, 0 }, /* Ă */
    { 0x103, AF_ADJUST_UP, 0 }, /* ă */
    { 0x106, AF_ADJUST_UP, 0 }, /* Ć */
    { 0x107, AF_ADJUST_UP, 0 }, /* ć */
    { 0x108, AF_ADJUST_UP, 0 }, /* Ĉ */
    { 0x109, AF_ADJUST_UP, 0 }, /* ĉ */
    { 0x10A, AF_ADJUST_UP, 0 }, /* Ċ */
    { 0x10B, AF_ADJUST_UP, 0 }, /* ċ */
    { 0x10C, AF_ADJUST_UP, 0 }, /* Č */
    { 0x10D, AF_ADJUST_UP, 0 }, /* č */
    { 0x10E, AF_ADJUST_UP, 0 }, /* Ď */

    { 0x112, AF_ADJUST_UP, 0 }, /* Ē */
    { 0x113, AF_ADJUST_UP, 0 }, /* ē */
    { 0x114, AF_ADJUST_UP, 0 }, /* Ĕ */
    { 0x115, AF_ADJUST_UP, 0 }, /* ĕ */
    { 0x116, AF_ADJUST_UP, 0 }, /* Ė */
    { 0x117, AF_ADJUST_UP, 0 }, /* ė */
    { 0x11A, AF_ADJUST_UP, 0 }, /* Ě */
    { 0x11B, AF_ADJUST_UP, 0 }, /* ě */
    { 0x11C, AF_ADJUST_UP, 0 }, /* Ĝ */
    { 0x11D, AF_ADJUST_UP, 0 }, /* ĝ */
    { 0x11E, AF_ADJUST_UP, 0 }, /* Ğ */
    { 0x11F, AF_ADJUST_UP, 0 }, /* ğ */

    { 0x120, AF_ADJUST_UP, 0 }, /* Ġ */
    { 0x121, AF_ADJUST_UP, 0 }, /* ġ */
    { 0x122, AF_ADJUST_DOWN, 0 }, /* Ģ */
    { 0x123, AF_ADJUST_UP, 0 }, /* ģ */
    { 0x124, AF_ADJUST_UP, 0 }, /* Ĥ */
    { 0x125, AF_ADJUST_UP, 0 }, /* ĥ */
    { 0x128, AF_ADJUST_UP, 1 }, /* Ĩ */
    { 0x129, AF_ADJUST_UP, 1 }, /* ĩ */
    { 0x12A, AF_ADJUST_UP, 0 }, /* Ī */
    { 0x12B, AF_ADJUST_UP, 0 }, /* ī */
    { 0x12C, AF_ADJUST_UP, 0 }, /* Ĭ */
    { 0x12D, AF_ADJUST_UP, 0 }, /* ĭ */
    { 0x12F, AF_ADJUST_UP, 0 }, /* į */

    { 0x130, AF_ADJUST_UP, 0 }, /* İ */
    { 0x133, AF_ADJUST_UP, 0 }, /* ĳ */
    { 0x134, AF_ADJUST_UP, 0 }, /* Ĵ */
    { 0x135, AF_ADJUST_UP, 0 }, /* ĵ */
    { 0x136, AF_ADJUST_DOWN, 0 }, /* Ķ */
    { 0x137, AF_ADJUST_DOWN, 0 }, /* ķ */
    { 0x139, AF_ADJUST_UP, 0 }, /* Ĺ */
    { 0x13A, AF_ADJUST_UP, 0 }, /* ĺ */
    { 0x13B, AF_ADJUST_DOWN, 0 }, /* Ļ */
    { 0x13C, AF_ADJUST_DOWN, 0 }, /* ļ */

    { 0x143, AF_ADJUST_UP, 0 }, /* Ń */
    { 0x144, AF_ADJUST_UP, 0 }, /* ń */
    { 0x145, AF_ADJUST_DOWN, 0 }, /* Ņ */
    { 0x146, AF_ADJUST_DOWN, 0 }, /* ņ */
    { 0x147, AF_ADJUST_UP, 0 }, /* Ň */
    { 0x148, AF_ADJUST_UP, 0 }, /* ň */
    { 0x14C, AF_ADJUST_UP, 0 }, /* Ō */
    { 0x14D, AF_ADJUST_UP, 0 }, /* ō */
    { 0x14E, AF_ADJUST_UP, 0 }, /* Ŏ */
    { 0x14F, AF_ADJUST_UP, 0 }, /* ŏ */

    { 0x150, AF_ADJUST_UP, 0 }, /* Ő */
    { 0x151, AF_ADJUST_UP, 0 }, /* ő */
    { 0x154, AF_ADJUST_UP, 0 }, /* Ŕ */
    { 0x155, AF_ADJUST_UP, 0 }, /* ŕ */
    { 0x156, AF_ADJUST_DOWN, 0 }, /* Ŗ */
    { 0x157, AF_ADJUST_DOWN, 0 }, /* ŗ */
    { 0x158, AF_ADJUST_UP, 0 }, /* Ř */
    { 0x159, AF_ADJUST_UP, 0 }, /* ř */
    { 0x15A, AF_ADJUST_UP, 0 }, /* Ś */
    { 0x15B, AF_ADJUST_UP, 0 }, /* ś */
    { 0x15C, AF_ADJUST_UP, 0 }, /* Ŝ */
    { 0x15D, AF_ADJUST_UP, 0 }, /* ŝ */

    { 0x160, AF_ADJUST_UP, 0 }, /* Š */
    { 0x161, AF_ADJUST_UP, 0 }, /* š */
    { 0x164, AF_ADJUST_UP, 0 }, /* Ť */
    { 0x168, AF_ADJUST_UP, 1 }, /* Ũ */
    { 0x169, AF_ADJUST_UP, 1 }, /* ũ */
    { 0x16A, AF_ADJUST_UP, 0 }, /* Ū */
    { 0x16B, AF_ADJUST_UP, 0 }, /* ū */
    { 0x16C, AF_ADJUST_UP, 0 }, /* Ŭ */
    { 0x16D, AF_ADJUST_UP, 0 }, /* ŭ */
    { 0x16E, AF_ADJUST_UP, 0 }, /* Ů */
    { 0x16F, AF_ADJUST_UP, 0 }, /* ů */

    { 0x170, AF_ADJUST_UP, 0 }, /* Ű */
    { 0x171, AF_ADJUST_UP, 0 }, /* ű */
    { 0x174, AF_ADJUST_UP, 0 }, /* Ŵ */
    { 0x175, AF_ADJUST_UP, 0 }, /* ŵ */
    { 0x176, AF_ADJUST_UP, 0 }, /* Ŷ */
    { 0x177, AF_ADJUST_UP, 0 }, /* ŷ */
    { 0x178, AF_ADJUST_UP, 0 }, /* Ÿ */
    { 0x179, AF_ADJUST_UP, 0 }, /* Ź */
    { 0x17A, AF_ADJUST_UP, 0 }, /* ź */
    { 0x17B, AF_ADJUST_UP, 0 }, /* Ż */
    { 0x17C, AF_ADJUST_UP, 0 }, /* ż */
    { 0x17D, AF_ADJUST_UP, 0 }, /* Ž */
    { 0x17E, AF_ADJUST_UP, 0 }  /* ž */
  };


  FT_LOCAL_DEF( const AF_AdjustmentDatabaseEntry* )
  af_adjustment_database_lookup( FT_UInt32  codepoint )
  {
    /* Binary search for database entry */
    FT_Int  low  = 0;
    FT_Int  high = AF_ADJUSTMENT_DATABASE_LENGTH - 1;


    while ( high >= low )
    {
      FT_Int     mid           = ( low + high ) / 2;
      FT_UInt32  mid_codepoint = adjustment_database[mid].codepoint;


      if ( mid_codepoint < codepoint )
        low = mid + 1;
      else if ( mid_codepoint > codepoint )
        high = mid - 1;
      else
        return &adjustment_database[mid];
    }

    return NULL;
  }


  /* `qsort` compare function for reverse character map. */
  FT_COMPARE_DEF( FT_Int )
  af_reverse_character_map_entry_compare( const void  *a,
                                          const void  *b )
  {
    const AF_ReverseMapEntry  entry_a = *((const AF_ReverseMapEntry *)a);
    const AF_ReverseMapEntry  entry_b = *((const AF_ReverseMapEntry *)b);


    return entry_a.glyph_index < entry_b.glyph_index
           ? -1
           : entry_a.glyph_index > entry_b.glyph_index
               ? 1
               : entry_a.codepoint < entry_b.codepoint
                   ? -1
                   : entry_a.codepoint > entry_b.codepoint
                       ? 1
                       : 0;
  }


  FT_LOCAL_DEF( const AF_ReverseMapEntry* )
  af_reverse_character_map_lookup( AF_ReverseCharacterMap  map,
                                   FT_Int                  glyph_index )
  {
    FT_Int   low, high;
    FT_Long  length;


    if ( !map )
      return NULL;

    length = map->length;

    /* Binary search for reverse character map entry. */
    low  = 0;
    high = length - 1;

    while ( high >= low )
    {
      FT_Int  mid             = ( high + low ) / 2;
      FT_Int  mid_glyph_index = map->entries[mid].glyph_index;


      if ( glyph_index < mid_glyph_index )
        high = mid - 1;
      else if ( glyph_index > mid_glyph_index )
        low = mid + 1;
      else if (low != mid)
        /* We want the first occurrence of `glyph_index` */
        /* (i.e., the one with the lowest array index).  */
        high = mid;
      else
        return &map->entries[mid];
    }

    return NULL;
  }


  /* Prepare to add one more entry to the reverse character map.   */
  /* This is a helper function for `af_reverse_character_map_new`. */
  static FT_Error
  af_reverse_character_map_expand( AF_ReverseCharacterMap  map,
                                   FT_Long                *capacity,
                                   FT_Memory               memory )
  {
    FT_Error  error;


    if ( map->length < *capacity )
      return FT_Err_Ok;

    if ( map->length == *capacity )
    {
      FT_Long  new_capacity = *capacity + *capacity / 2;


      if ( FT_RENEW_ARRAY( map->entries, map->length, new_capacity ) )
        return error;

      *capacity = new_capacity;
    }

    return FT_Err_Ok;
  }


#ifdef FT_CONFIG_OPTION_USE_HARFBUZZ
#  if HB_VERSION_ATLEAST( 7, 2, 0 )

  /*
    Find all glyphs that a code point could turn into from the OpenType
    'GSUB' table.

    The algorithm first gets the glyph index from the code point (using the
    'cmap' table) and puts it into the result set.  It then calls function
    `hb_ot_layout_lookup_get_glyph_alternates` on each OpenType lookup (only
    handling 'SingleSubst' and 'AlternateSubst' lookups, ignoring all other
    types) to check which ones map the glyph in question onto something
    different.  These alternate glyph indices are then added to the result
    set.

    If there are results, `hb_ot_layout_lookup_get_glyph_alternates` is
    tried again on each of them to find out whether these glyphs in turn
    have also alternates, which are eventually added to the result set, too.
    This gets repeated in a loop until no more additional glyphs are found.

    Example:

      Suppose we have the following lookups in the GSUB table:

        L1: a -> b
        L2: b -> c
        L3: d -> e

      The algorithm takes the following steps to find all variants of 'a'.

      - Add 'a' to the result.
      - Check lookup L1 for 'a', yielding {b}.
      - Check lookups L2 and L3 for 'a', yielding nothing.
      => Add 'b' to the result list, try again.
        - Check lookup L1 for 'b', yielding nothing.
        - Check lookup L2 for 'b', yielding {c}.
        - Check lookup L3 for 'b', yielding nothing.
        => Add 'c' to the result list, try again.
          - Check lookups L1 to L3 for 'c', yielding nothing.
          => Done.
  */


  /* The chunk size used for retrieving results of */
  /* `hb_ot_layout_lookup_get_glyph_alternates`.   */
#define ALTERNATE_CHUNK  20


  /* Get all alternates for a given glyph index. */
  static void
  af_get_glyph_alternates_helper( hb_face_t      *hb_face,
                                  hb_codepoint_t  glyph,
                                  hb_set_t       *gsub_lookups,
                                  hb_set_t       *result )
  {
    hb_codepoint_t  lookup_index = HB_SET_VALUE_INVALID;


    /* Iterate over all lookups. */
    while ( hb_set_next( gsub_lookups, &lookup_index ) )
    {
      FT_Bool       lookup_done  = FALSE;
      unsigned int  start_offset = 0;


      while ( !lookup_done )
      {
        unsigned int    alternates_count = ALTERNATE_CHUNK;
        hb_codepoint_t  alternates[ALTERNATE_CHUNK];

        unsigned int  n;


        (void)hb_ot_layout_lookup_get_glyph_alternates( hb_face,
                                                        lookup_index,
                                                        glyph,
                                                        start_offset,
                                                        &alternates_count,
                                                        alternates );

        start_offset += ALTERNATE_CHUNK;
        if ( alternates_count < ALTERNATE_CHUNK )
          lookup_done = TRUE;

        for ( n = 0; n < alternates_count; n++ )
          hb_set_add( result, alternates[n] );
      }
    }
  }


  /* Get all alternates (including alternates of alternates) */
  /* for a given glyph index.                                */
  static void
  af_get_glyph_alternates( hb_font_t      *hb_font,
                           hb_codepoint_t  glyph,
                           hb_set_t       *gsub_lookups,
                           hb_set_t       *result )
  {
    hb_face_t  *hb_face       = hb_font_get_face( hb_font );
    hb_set_t   *helper_result = hb_set_create();


    /* Seed `helper_result` with `glyph` itself, then get all possible */
    /* values.  Note that we can't use `hb_set_next` to control the    */
    /* loop because we modify `helper_result` during iteration.        */
    hb_set_add( helper_result, glyph );
    while ( !hb_set_is_empty( helper_result ) )
    {
      hb_codepoint_t  elem;


      /* Always get the smallest element of the set. */
      elem = HB_SET_VALUE_INVALID;
      hb_set_next( helper_result, &elem );

      /* Don't process already handled glyphs again. */
      if ( !hb_set_has( result, elem ) )
      {
        /* This call updates the glyph set in `helper_result`. */
        af_get_glyph_alternates_helper( hb_face,
                                        elem,
                                        gsub_lookups,
                                        helper_result );
        hb_set_add( result, elem );
      }

      hb_set_del( helper_result, elem );
    }

    hb_set_destroy( helper_result );
  }

#  endif /* HB_VERSION_ATLEAST */
#endif /* FT_CONFIG_OPTION_USE_HARFBUZZ */


  FT_LOCAL_DEF( FT_Error )
  af_reverse_character_map_new( AF_ReverseCharacterMap  *map,
                                AF_FaceGlobals           globals )
  {
    FT_Error  error;

    FT_Face    face   = globals->face;
    FT_Memory  memory = face->memory;

    FT_CharMap  old_charmap;

    FT_Long  capacity;


    /* Search for a unicode charmap.           */
    /* If there isn't one, create a blank map. */

    FT_TRACE4(( "af_reverse_character_map_new:"
                " building reverse character map\n" ));

    /* Back up `face->charmap` because `find_unicode_charmap` sets it. */
    old_charmap = face->charmap;

    if ( ( error = find_unicode_charmap( face ) ) )
      goto Exit;

    *map = NULL;
    if ( FT_NEW( *map ) )
      goto Exit;

    /* Start with a capacity of 10 entries. */
    capacity         = 10;
    ( *map )->length = 0;

    if ( FT_NEW_ARRAY( ( *map )->entries, capacity ) )
      goto Exit;

#ifdef FT_CONFIG_OPTION_USE_HARFBUZZ
#  if HB_VERSION_ATLEAST( 7, 2, 0 )

    {
      /* No need to check whether HarfBuzz has allocation issues; */
      /* it continues to work in such cases and simply returns    */
      /* 'empty' objects that do nothing.                         */

      hb_font_t  *hb_font = globals->hb_font;
      hb_face_t  *hb_face = hb_font_get_face( hb_font );

      hb_set_t  *result_set   = hb_set_create();
      hb_set_t  *gsub_lookups = hb_set_create();

      FT_UInt32  codepoint;
      FT_UInt    glyph_index;

      FT_ULong  i;

      AF_ReverseMapEntry  *map_limit;


      /* Compute set of all GSUB lookups. */
      hb_ot_layout_collect_lookups( hb_face,
                                    HB_OT_TAG_GSUB,
                                    NULL, NULL, NULL,
                                    gsub_lookups );

      /* Find all glyph alternates of the code points in  */
      /* the adjustment database and put them into `map`. */
      for ( i = 0; i < AF_ADJUSTMENT_DATABASE_LENGTH; i++ )
      {
        FT_UInt  cmap_glyph;

        hb_codepoint_t  glyph;


        codepoint  = adjustment_database[i].codepoint;
        cmap_glyph = FT_Get_Char_Index( face, codepoint );

        af_get_glyph_alternates( hb_font,
                                 cmap_glyph,
                                 gsub_lookups,
                                 result_set );

        glyph = HB_SET_VALUE_INVALID;
        while ( hb_set_next( result_set, &glyph ) )
        {
          FT_Long  insert_point;


          error = af_reverse_character_map_expand( *map, &capacity, memory );
          if ( error )
            goto Exit;

          insert_point = ( *map )->length;

          ( *map )->length++;
          ( *map )->entries[insert_point].glyph_index = glyph;
          ( *map )->entries[insert_point].codepoint   = codepoint;
        }

        hb_set_clear( result_set );
      }

      hb_set_destroy( result_set );
      hb_set_destroy( gsub_lookups );

      ft_qsort( ( *map )->entries,
                ( *map )->length,
                sizeof ( AF_ReverseMapEntry ),
                af_reverse_character_map_entry_compare );

      /* OpenType features like 'unic' map lowercase letter glyphs to    */
      /* uppercase forms (and vice versa), which could lead to the use   */
      /* of a wrong entry in the adjustment database.  For this reason   */
      /* we prioritize cmap entries.                                     */
      /*                                                                 */
      /* XXX Note, however, that this cannot cover all cases since there */
      /* might be contradictory entries for glyphs not in the cmap.  A   */
      /* possible solution might be to specially mark pairs of related   */
      /* lowercase and uppercase characters in the adjustment database   */
      /* that have diacritics on different vertical sides (for example,  */
      /* U+0122 'Ģ' and U+0123 'ģ').  The auto-hinter could then perform */
      /* a topological analysis to do the right thing.                   */

      codepoint = FT_Get_First_Char( face, &glyph_index );
      map_limit = ( *map )->entries + ( *map )->length;
      while ( glyph_index )
      {
        AF_ReverseMapEntry  *entry;


        entry = (AF_ReverseMapEntry*)
                  af_reverse_character_map_lookup( *map, glyph_index );
        if ( entry )
        {
          FT_Int  idx = entry->glyph_index;


          while ( entry < map_limit         &&
                  entry->glyph_index == idx )
          {
            entry->codepoint = codepoint;
            entry++;
          }
        }

        codepoint = FT_Get_Next_Char( face, codepoint, &glyph_index );
      }
    }

#  endif /* HB_VERSION_ATLEAST */

#else /* !FT_CONFIG_OPTION_USE_HARFBUZZ */

    {
      FT_UInt  i;
      FT_Long  insert_point;


      for ( i = 0; i < AF_ADJUSTMENT_DATABASE_LENGTH; i++ )
      {
        FT_UInt32  codepoint = adjustment_database[i].codepoint;
        FT_Int     glyph     = FT_Get_Char_Index( face, codepoint );


        if ( glyph == 0 )
          continue;

        error = af_reverse_character_map_expand( *map, &capacity, memory );
        if ( error )
          goto Exit;

        insert_point = ( *map )-> length;

        ( *map )->length++;
        ( *map )->entries[insert_point].glyph_index = glyph;
        ( *map )->entries[insert_point].codepoint   = codepoint;
      }

      ft_qsort( ( *map )->entries,
                ( *map )->length,
                sizeof ( AF_ReverseMapEntry ),
                af_reverse_character_map_entry_compare );
    }

#endif /* !FT_CONFIG_OPTION_USE_HARFBUZZ */

    FT_TRACE4(( "    reverse character map built successfully"
                " with %ld entries\n", ( *map )->length ));

#ifdef FT_DEBUG_LEVEL_TRACE
    {
      FT_Long  i;


      FT_TRACE7(( "       gidx   code    adj  tilde\n" ));
               /* "      XXXXX  0xXXXX  XXXX   XXX" */
      FT_TRACE7(( "     ----------------------------\n" ));

      for ( i = 0; i < ( *map )->length; i++ )
      {
        FT_Long  glyph_index = ( *map )->entries[i].glyph_index;
        FT_Int   codepoint   = ( *map )->entries[i].codepoint;

        const AF_AdjustmentDatabaseEntry    *db_entry =
          af_adjustment_database_lookup( codepoint );
        AF_VerticalSeparationAdjustmentType  adj_type;


        if ( !db_entry )
          continue;

        adj_type = db_entry->vertical_separation_adjustment_type;

        FT_TRACE7(( "      %5ld  0x%04X  %4s   %3s\n",
                    glyph_index,
                    codepoint,
                    adj_type == AF_ADJUST_DOWN
                      ? "down"
                      : adj_type == AF_ADJUST_UP
                        ? "up"
                        : "",
                    db_entry->apply_tilde ? "yes" : "no" ));
      }
    }
#endif


  Exit:
    face->charmap = old_charmap;

    if ( error )
    {
      FT_TRACE4(( "    error while building reverse character map."
                  " Using blank map.\n" ));

      if ( *map )
        FT_FREE( ( *map )->entries );

      FT_FREE( *map );
      *map = NULL;
      return error;
    }

    return FT_Err_Ok;
  }


  FT_LOCAL_DEF( FT_Error )
  af_reverse_character_map_done( AF_ReverseCharacterMap  map,
                                 FT_Memory               memory )
  {
    if ( map )
      FT_FREE( map->entries );
    FT_FREE( map );

    return FT_Err_Ok;
  }


/* END */
