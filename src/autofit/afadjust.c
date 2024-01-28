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
    { 0x21,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ! */
    { 0x69,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* i */
    { 0x6A,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* j */

    { 0xA1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ¡ */
    { 0xBF,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ¿ */

    { 0xC0,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* À */
    { 0xC1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Á */
    { 0xC2,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Â */
    { 0xC3,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1 }, /* Ã */
    { 0xC8,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* È */
    { 0xC9,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* É */
    { 0xCA,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ê */
    { 0xCC,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ì */
    { 0xCD,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Í */
    { 0xCE,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Î */

    { 0xD1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1 }, /* Ñ */
    { 0xD2,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ò */
    { 0xD3,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ó */
    { 0xD4,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ô */
    { 0xD5,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1 }, /* Õ */
    { 0xD9,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ù */
    { 0xDA,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ú */
    { 0xDB,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Û */
    { 0xDD,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ý */

    { 0xE0,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* à */
    { 0xE1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* á */
    { 0xE2,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* â */
    { 0xE3,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1 }, /* ã */
    { 0xE8,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* è */
    { 0xE9,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* é */
    { 0xEA,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ê */
    { 0xEC,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ì */
    { 0xED,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* í */
    { 0xEE,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* î */

    { 0xF1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1 }, /* ñ */
    { 0xF2,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ò */
    { 0xF3,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ó */
    { 0xF4,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ô */
    { 0xF5,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1 }, /* õ */
    { 0xF9,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ù */
    { 0xFA,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ú */
    { 0xFB,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* û */
    { 0xFD,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ý */

    { 0x100, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ā */
    { 0x101, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ā */
    { 0x102, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ă */
    { 0x103, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ă */
    { 0x106, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ć */
    { 0x107, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ć */
    { 0x108, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ĉ */
    { 0x109, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ĉ */
    { 0x10A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ċ */
    { 0x10B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ċ */
    { 0x10C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Č */
    { 0x10D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* č */
    { 0x10E, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ď */

    { 0x112, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ē */
    { 0x113, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ē */
    { 0x114, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ĕ */
    { 0x115, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ĕ */
    { 0x116, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ė */
    { 0x117, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ė */
    { 0x11A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ě */
    { 0x11B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ě */
    { 0x11C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ĝ */
    { 0x11D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ĝ */
    { 0x11E, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ğ */
    { 0x11F, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ğ */

    { 0x120, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ġ */
    { 0x121, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ġ */
    { 0x123, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ģ */
    { 0x124, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ĥ */
    { 0x125, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ĥ */
    { 0x128, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1 }, /* Ĩ */
    { 0x129, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1 }, /* ĩ */
    { 0x12A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ī */
    { 0x12B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ī */
    { 0x12C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ĭ */
    { 0x12D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ĭ */
    { 0x12F, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* į */

    { 0x130, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* İ */
    { 0x133, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ĳ */
    { 0x134, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ĵ */
    { 0x135, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ĵ */
    { 0x139, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ĺ */
    { 0x13A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ĺ */

    { 0x143, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ń */
    { 0x144, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ń */
    { 0x147, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ň */
    { 0x148, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ň */
    { 0x14C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ō */
    { 0x14D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ō */
    { 0x14E, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ŏ */
    { 0x14F, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ŏ */

    { 0x154, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ŕ */
    { 0x155, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ŕ */
    { 0x158, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ř */
    { 0x159, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ř */
    { 0x15A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ś */
    { 0x15B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ś */
    { 0x15C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ŝ */
    { 0x15D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ŝ */

    { 0x160, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Š */
    { 0x161, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* š */
    { 0x164, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ť */
    { 0x168, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1 }, /* Ũ */
    { 0x169, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1 }, /* ũ */
    { 0x16A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ū */
    { 0x16B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ū */
    { 0x16C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ŭ */
    { 0x16D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ŭ */

    { 0x174, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ŵ */
    { 0x175, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ŵ */
    { 0x176, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ŷ */
    { 0x177, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ŷ */
    { 0x179, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ź */
    { 0x17A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ź */
    { 0x17B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ż */
    { 0x17C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* ż */
    { 0x17D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }, /* Ž */
    { 0x17E, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0 }  /* ž */
  };


  /* Helper function: get the adjustment database entry for a codepoint. */
  static const AF_AdjustmentDatabaseEntry*
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
               : 0;
  }


  static FT_UInt32
  af_reverse_character_map_lookup( AF_ReverseCharacterMap  map,
                                   FT_Int                  glyph_index )
  {
    FT_Int   low, high;
    FT_Long  length;


    if ( !map )
      return 0;

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
      else
        return map->entries[mid].codepoint;
    }

    return 0;
  }


  FT_LOCAL_DEF( AF_VerticalSeparationAdjustmentType )
  af_lookup_vertical_separation_type( AF_ReverseCharacterMap  map,
                                      FT_Int                  glyph_index )
  {
    FT_UInt32  codepoint = af_reverse_character_map_lookup( map,
                                                            glyph_index );

    const AF_AdjustmentDatabaseEntry  *entry =
      af_adjustment_database_lookup( codepoint );


    if ( !entry )
      return AF_VERTICAL_ADJUSTMENT_NONE;

    return entry->vertical_separation_adjustment_type;
  }


  /* Return 1 if tilde correction should be applied to the topmost */
  /* contour, else 0.                                              */
  FT_LOCAL_DEF( FT_Bool )
  af_lookup_tilde_correction_type( AF_ReverseCharacterMap  map,
                                   FT_Int                  glyph_index )
  {
    FT_UInt32  codepoint = af_reverse_character_map_lookup( map,
                                                            glyph_index );

    const AF_AdjustmentDatabaseEntry  *entry =
      af_adjustment_database_lookup( codepoint );


    if ( !entry )
      return 0;

    return entry->apply_tilde;
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

    {
      FT_UInt  i;
#ifdef FT_DEBUG_LEVEL_TRACE
      int  failed_lookups = 0;
#endif


      for ( i = 0; i < AF_ADJUSTMENT_DATABASE_LENGTH; i++ )
      {
        FT_UInt32  codepoint = adjustment_database[i].codepoint;
        FT_Int     glyph     = FT_Get_Char_Index( face, codepoint );


        if ( glyph == 0 )
        {
#ifdef FT_DEBUG_LEVEL_TRACE
          failed_lookups++;
#endif
          continue;
        }

        error = af_reverse_character_map_expand( *map, &capacity, memory );
        if ( error )
          goto Exit;

        ( *map )->length++;
        ( *map )->entries[i].glyph_index = glyph;
        ( *map )->entries[i].codepoint   = codepoint;
      }
    }

    ft_qsort( ( *map )->entries,
              ( *map )->length,
              sizeof ( AF_ReverseMapEntry ),
              af_reverse_character_map_entry_compare );

    FT_TRACE4(( "    reverse character map built successfully"
                " with %ld entries\n", (*map)->length ));

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
