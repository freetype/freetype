#include "afadjust.h"
#include <freetype/freetype.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftmemory.h>
#include <freetype/internal/ftdebug.h>

#define AF_ADJUSTMENT_DATABASE_LENGTH ( sizeof(adjustment_database)/sizeof(adjustment_database[0]) )
#undef  FT_COMPONENT
#define FT_COMPONENT  afadjust

/*
  All entries in this list must be sorted by unicode codepoint ascending
  The table entries are 3 numbers consisting of:
  - unicode codepoint.  THESE MUST BE LISTED IN ASCENDING ORDER
  - the vertical adjustment type.  This should be one of the enum constants in AF_VerticalSeparationAdjustmentType
  - 1 if the topmost contour is a tilde and should be prevented from flattening.
*/
FT_LOCAL_ARRAY_DEF( AF_AdjustmentDatabaseEntry )
adjustment_database[] =
{
    {0x21,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /* ! */
    {0x69,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /* i */
    {0x6A,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /* j */
    {0xA1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /*Inverted Exclamation Mark*/
    {0xBF,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /* j */
    {0xA1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /*Inverted Exclamation Mark*/
    {0xBF,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /*Inverted Question Mark*/
    {0xC0,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /*A with grave*/
    {0xC1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /*A with acute*/
    {0xC2,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /*A with circumflex*/
    {0xC3,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /*A with tilde*/
    {0xC8,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /*E with grave*/
    {0xCC,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /*I with grave*/
    {0xCD,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xCE,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xD1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xD2,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xD3,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xD4,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xD5,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xD9,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xD1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xDB,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xDD,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xE0,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xE1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xE2,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xE3,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1}, /*a with tilde*/
    {0xE8,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xE9,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xEA,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xEC,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xED,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xEE,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xF1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}, /*n with tilde*/
    {0xF2,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xF3,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xF4,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xF5,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1}, /*o with tilde*/
    {0xF9,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xFA,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xFB,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0xFD,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x100, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x101, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x102, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x103, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x106, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x108, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x109, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x10A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x10B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x10C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x10D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x10E, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x112, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x113, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x114, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x115, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x116, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x117, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x11B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x11C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x11D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x11E, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x11F, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x120, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x121, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x123, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x124, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x125, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x128, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x129, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1}, /*i with tilde*/
    {0x12A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x12B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x12C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x12D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x12F, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x130, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x134, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x135, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x139, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x13A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x143, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x144, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x147, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x14C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x14D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x14E, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x14F, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x154, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x155, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x158, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x159, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x15A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x15B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x15C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x15D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x160, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x161, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x164, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x168, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x169, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 1}, /*u with tilde*/
    {0x16A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x16B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x16C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x16D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x174, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x175, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x176, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x177, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x179, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x17A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x17B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x17C, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x17D, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0},
    {0x17E, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP, 0}
};

/*Helper function: get the adjustment database entry for a codepoint*/
FT_LOCAL_DEF( const AF_AdjustmentDatabaseEntry* )
af_adjustment_database_lookup( FT_UInt32 codepoint ) {
    /* Binary search for database entry */
    FT_Int low = 0;
    FT_Int high = AF_ADJUSTMENT_DATABASE_LENGTH - 1;
    while ( high >= low )
    {
        FT_Int mid = ( low + high ) / 2;
        FT_UInt32 mid_codepoint = adjustment_database[mid].codepoint;
        if ( mid_codepoint < codepoint )
        {
            low = mid + 1;
        }
        else if ( mid_codepoint > codepoint )
        {
            high = mid - 1;
        }
        else
        {
            return &adjustment_database[mid];
        }
    }

    return NULL;
}

FT_LOCAL_DEF( AF_VerticalSeparationAdjustmentType )
af_lookup_vertical_seperation_type( AF_ReverseCharacterMap map, FT_Int glyph_index ) {
    FT_UInt32 codepoint = af_reverse_character_map_lookup( map, glyph_index );
    const AF_AdjustmentDatabaseEntry *entry = af_adjustment_database_lookup( codepoint );
    if ( entry == NULL )
    {
        return AF_VERTICAL_ADJUSTMENT_NONE;
    }
    return entry->vertical_separation_adjustment_type;
}

/*1 if tilde correction should be applied to the topmost contour
else 0*/
FT_LOCAL_DEF( FT_Bool )
af_lookup_tilde_correction_type( AF_ReverseCharacterMap map, FT_Int glyph_index ) {
    FT_UInt32 codepoint = af_reverse_character_map_lookup( map, glyph_index );
    const AF_AdjustmentDatabaseEntry *entry = af_adjustment_database_lookup( codepoint );
    if ( entry == NULL )
    {
        return 0;
    }
    return entry->apply_tilde;
}

typedef struct AF_ReverseMapEntry_
{
    FT_Int glyph_index;
    FT_UInt32 codepoint;
} AF_ReverseMapEntry;

typedef struct AF_ReverseCharacterMap_
{
    FT_Long length;
    AF_ReverseMapEntry *entries;
} AF_ReverseCharacterMap_Rec;

/* qsort compare function for reverse character map */
FT_LOCAL_DEF( FT_Int )
af_reverse_character_map_entry_compare( const void *a, const void *b ) {
    const AF_ReverseMapEntry entry_a = *((const AF_ReverseMapEntry *)a);
    const AF_ReverseMapEntry entry_b = *((const AF_ReverseMapEntry *)b);
    return entry_a.glyph_index < entry_b.glyph_index ? -1 : entry_a.glyph_index > entry_b.glyph_index ? 1 : 0;
}

FT_LOCAL_DEF( FT_UInt32 )
af_reverse_character_map_lookup_( AF_ReverseCharacterMap map, FT_Int glyph_index, FT_Long length )
{
    if ( map == NULL )
    {
        return 0;
    }
    /* Binary search for reverse character map entry */
    FT_Int low = 0;
    FT_Int high = length - 1;
    while ( high >= low )
    {
        FT_Int mid = ( high + low ) / 2;
        FT_Int mid_glyph_index = map->entries[mid].glyph_index;
        if ( glyph_index < mid_glyph_index )
        {
            high = mid - 1;
        }
        else if ( glyph_index > mid_glyph_index )
        {
            low = mid + 1;
        }
        else
        {
            return map->entries[mid].codepoint;
        }
    }

    return 0;
}

FT_LOCAL_DEF( FT_UInt32 )
af_reverse_character_map_lookup( AF_ReverseCharacterMap map, FT_Int glyph_index )
{
    return af_reverse_character_map_lookup_( map, glyph_index, map->length );
}

/*prepare to add one more entry to the reverse character map
  this is a helper for af_reverse_character_map_new*/
FT_LOCAL_DEF( FT_Error )
af_reverse_character_map_expand( AF_ReverseCharacterMap map, FT_Long *capacity, FT_Memory memory )
{
    FT_Error error;
    if ( map->length < *capacity )
    {
        return FT_Err_Ok;
    }

    if ( map->length == *capacity )
    {
        FT_Long new_capacity = *capacity + *capacity / 2;
        if ( FT_RENEW_ARRAY( map->entries, map->length, new_capacity ) ) {
            return error;
        }
        *capacity = new_capacity;
    }

    return FT_Err_Ok;
}

FT_LOCAL_DEF( FT_Error )
af_reverse_character_map_new( AF_ReverseCharacterMap *map, AF_FaceGlobals globals )
{
    FT_Face face = globals->face;
    FT_Memory memory = face->memory;
    /* Search for a unicode charmap */
    /* If there isn't one, create a blank map */

    FT_TRACE4(( "af_reverse_character_map_new: building reverse character map\n" ));

    FT_Error error;
    /* backup face->charmap because find_unicode_charmap sets it */
    FT_CharMap old_charmap = face->charmap;
    if ( ( error = find_unicode_charmap( face ) ) )
    {
        *map = NULL;
        goto Exit;
    }

    if ( FT_NEW( *map ) )
    {
        goto Exit;
    }

    FT_Long capacity = 10;
    ( *map )->length = 0;

    if ( FT_NEW_ARRAY( ( *map )->entries, capacity) )
    {
        goto Exit;
    }

#ifdef FT_DEBUG_LEVEL_TRACE
    int failed_lookups = 0;
#endif
    for ( FT_Int i = 0; i < AF_ADJUSTMENT_DATABASE_LENGTH; i++ )
    {
        FT_UInt32 codepoint = adjustment_database[i].codepoint;
        FT_Int glyph = FT_Get_Char_Index( face, codepoint );
        if ( glyph == 0 )
        {
#ifdef FT_DEBUG_LEVEL_TRACE
            failed_lookups++;
#endif
            continue;
        }
        error = af_reverse_character_map_expand( *map, &capacity, memory );
        if ( error ) {
            goto Exit;
        }

        ( *map )->length++;
        ( *map )->entries[i].glyph_index = glyph;
        ( *map )->entries[i].codepoint = codepoint;
    }

    ft_qsort(
        ( *map )->entries,
        ( *map )->length,
        sizeof( AF_ReverseMapEntry ),
        af_reverse_character_map_entry_compare
    );

    FT_TRACE4(( "    reverse character map built successfully"\
                " with %d entries\n", (*map)->length ));

Exit:
    face->charmap = old_charmap;
    if ( error )
    {
        FT_TRACE4(( "    error while building reverse character map.  Using blank map.\n" ));
        if ( *map )
        {
            FT_FREE( ( *map )->entries );
        }
        FT_FREE( *map );
        return error;
    }

    return FT_Err_Ok;
}

FT_LOCAL_DEF( FT_Error )
af_reverse_character_map_done( AF_ReverseCharacterMap map, FT_Memory memory ) {
    FT_FREE( map->entries );
    return FT_Err_Ok;
}
