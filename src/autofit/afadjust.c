#include "afadjust.h"
#include <freetype/freetype.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftmemory.h>
#include <freetype/internal/ftdebug.h>

#define AF_ADJUSTMENT_DATABASE_LENGTH (sizeof(adjustment_database)/sizeof(adjustment_database[0]))
#undef  FT_COMPONENT
#define FT_COMPONENT  afadjust

/*TODO: find out whether capital u/U with accent entries are needed*/
/*the accent won't merge with the rest of the glyph because the accent mark is sitting above empty space*/
FT_LOCAL_ARRAY_DEF( AF_AdjustmentDatabaseEntry )
adjustment_database[] =
{
    {0x21,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /* ! */
    {0x69,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /* i */
    {0x6A,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /* j */
    {0xA1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*Inverted Exclamation Mark*/
    {0xBF,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*Inverted Question Mark*/
    {0xC0,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*A with grave*/
    {0xC1,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*A with acute*/
    {0xC2,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*A with circumflex*/
    {0xC3,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*A with tilde*/
    {0xC8,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*E with grave*/
    {0xCC,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*I with grave*/
    {0xD9,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*U with grave*/
    {0xE0,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*a with grave*/
    {0xEC,  AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*i with grave*/
    {0x114, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*E with macron*/
    {0x12A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*I with macron*/
    {0x12B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*i with macron*/
    {0x16A, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}, /*U with macron*/
    {0x16B, AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP}  /*u with macron*/
};

/*Helper function: get the adjustment database entry for a codepoint*/
FT_LOCAL_DEF( const AF_AdjustmentDatabaseEntry* )
af_adjustment_database_lookup( FT_UInt32 codepoint ) {
    /* Binary search for database entry */
    FT_UInt low = 0;
    FT_UInt high = AF_ADJUSTMENT_DATABASE_LENGTH - 1;
    while (high > low) {
        FT_UInt mid = (low + high) / 2;
        if (adjustment_database[mid].codepoint < codepoint) {
            low = mid + 1;
        } else if (adjustment_database[mid].codepoint > codepoint) {
            high = mid - 1;
        } else {
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

typedef struct AF_ReverseMapEntry_
{
    FT_Int glyph_index;
    FT_UInt32 codepoint;
} AF_ReverseMapEntry;

typedef struct AF_ReverseCharacterMap_
{
    FT_UInt length;
    AF_ReverseMapEntry *entries;
} AF_ReverseCharacterMap_Rec;

FT_LOCAL_DEF(FT_UInt32)
af_reverse_character_map_lookup( AF_ReverseCharacterMap map, FT_Int glyph_index )
{
    if ( map == NULL )
    {
        return 0;
    }

    for ( FT_UInt entry = 0; entry < map->length; entry++ )
    {
        if ( map->entries[entry].glyph_index == glyph_index )
        {
            return map->entries[entry].codepoint;
        }
    }

    return 0;
}

FT_LOCAL_DEF( FT_Error )
af_reverse_character_map_new( FT_Face face, AF_ReverseCharacterMap *map, FT_Memory memory )
{
    /* Search for a unicode charmap */
    /* If there isn't one, create a blank map */

    /*TODO: change this to logic that searches for a "preferred" unicode charmap that maps the most codepoints*/
    /*see find_unicode_charmap*/
    /*TODO: use GSUB lookups    */
    FT_TRACE4(( "af_reverse_character_map_new: building reverse character map\n" ));
    FT_CMap unicode_charmap = NULL;
    for ( FT_UInt i = 0; i < face->num_charmaps; i++ )
    {
        if ( face->charmaps[i]->encoding == FT_ENCODING_UNICODE )
        {
            unicode_charmap = FT_CMAP( face->charmaps[i] );
        }
    }

    if ( unicode_charmap == NULL )
    {
        *map = NULL;
        return FT_Err_Ok;
    }

    FT_Error error;

    if ( FT_NEW( *map ) )
    {
        goto Exit;
    }

    FT_Int capacity = 10;
    FT_Int size = 0;

    if ( FT_NEW_ARRAY((*map)->entries, capacity) )
    {
        goto Exit;
    }
#ifdef FT_DEBUG_LEVEL_TRACE
    int failed_lookups = 0;
#endif
    for ( FT_Int i = 0; i < AF_ADJUSTMENT_DATABASE_LENGTH; i++ )
    {
        FT_UInt32 codepoint = adjustment_database[i].codepoint;
        FT_Int glyph = unicode_charmap->clazz->char_index(unicode_charmap, codepoint);
        if ( glyph == 0 )
        {
#ifdef FT_DEBUG_LEVEL_TRACE
            failed_lookups++;
#endif
            continue;
        }
        if ( size == capacity )
        {
            capacity += capacity / 2;
            if ( FT_RENEW_ARRAY((*map)->entries, size, capacity) )
            {
                goto Exit;
            }
        }
        size++;
        ( *map )->entries[i].glyph_index = glyph;
        ( *map )->entries[i].codepoint = codepoint;
    }
    ( *map )->length = size;

Exit:
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
#ifdef FT_DEBUG_LEVEL_TRACE
    FT_TRACE4(( "    reverse character map built successfully"\
                " with %d entries and %d failed lookups.\n", size, failed_lookups ));
#endif
    return FT_Err_Ok;
}

FT_LOCAL_DEF( FT_Error )
af_reverse_character_map_done( AF_ReverseCharacterMap map, FT_Memory memory ) {
    FT_FREE( map->entries );
    return FT_Err_Ok;
}
