#include "afadjust.h"
#include <freetype/freetype.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftmemory.h>

#define AF_ADJUSTMENT_DATABASE_LENGTH 12

/*TODO: find out whether capital u/U with accent entries are needed*/
/*the accent won't merge with the rest of the glyph because the accent mark is sitting above empty space*/
FT_LOCAL_ARRAY_DEF( AF_AdjustmentDatabaseEntry )
adjustment_database[AF_ADJUSTMENT_DATABASE_LENGTH] = {
    {'i',   AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE},
    {'j',   AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE},
    {0xC8,  AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE}, /*E with grave*/
    {0xCC,  AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE}, /*I with grave*/
    {0xD9,  AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE}, /*U with grave*/
    {0xE0,  AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE}, /*a with grave*/
    {0xEC,  AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE}, /*i with grave*/
    {0x114, AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE}, /*E with macron*/
    {0x12A, AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE}, /*I with macron*/
    {0x12B, AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE}, /*i with macron*/
    {0x16A, AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE}, /*U with macron*/
    {0x16B, AF_VERTICAL_ADJUSTMENT_ONE_ON_ONE}  /*u with macron*/
    /*TODO: find out why E won't work, even though it appears to be one-on-one*/
};

/*Helper function: get the adjustment database entry for a codepoint*/
FT_LOCAL_DEF( const AF_AdjustmentDatabaseEntry* )
af_adjustment_database_lookup( FT_UInt32 codepoint ) {
    for ( FT_Int entry = 0; entry < AF_ADJUSTMENT_DATABASE_LENGTH; entry++ ) {
        if ( adjustment_database[entry].codepoint == codepoint ) {
            return &adjustment_database[entry];
        }
    }

    return NULL;
}

FT_LOCAL_DEF( AF_VerticalSeparationAdjustmentType )
af_lookup_vertical_seperation_type( AF_ReverseCharacterMap map, FT_Int glyph_index ) {
    FT_UInt32 codepoint = af_reverse_character_map_lookup( map, glyph_index );
    const AF_AdjustmentDatabaseEntry *entry = af_adjustment_database_lookup( codepoint );
    if ( entry == NULL ) {
        return AF_VERTICAL_ADJUSTMENT_NONE;
    }
    return entry->vertical_separation_adjustment_type;
}

typedef struct AF_ReverseMapEntry_ {
    FT_Int glyph_index;
    FT_UInt32 codepoint;
} AF_ReverseMapEntry;

typedef struct AF_ReverseCharacterMap_ {
    FT_UInt length;
    AF_ReverseMapEntry *entries;
} AF_ReverseCharacterMap_Rec;

FT_LOCAL_DEF(FT_UInt32)
af_reverse_character_map_lookup( AF_ReverseCharacterMap map, FT_Int glyph_index ) {
    if ( map == NULL ) {
        return 0;
    }

    for ( FT_UInt entry = 0; entry < map->length; entry++ ) {
        if ( map->entries[entry].glyph_index == glyph_index ) {
            return map->entries[entry].codepoint;
        }
    }

    return 0;
}

FT_LOCAL_DEF( FT_Error )
af_reverse_character_map_new( FT_Face face, AF_ReverseCharacterMap *map, FT_Memory memory ) {
    /* Search for a unicode charmap */
    /* If there isn't one, create a blank map */
    
    /*TODO: change this to logic that searches for a "preferred" unicode charmap that maps the most codepoints*/
    /*see find_unicode_charmap*/
    /*TODO: use GSUB lookups    */
    FT_CMap unicode_charmap = NULL;
    for ( FT_UInt i = 0; i < face->num_charmaps; i++ ) {
        if ( face->charmaps[i]->encoding == FT_ENCODING_UNICODE ) {
            unicode_charmap = FT_CMAP( face->charmaps[i] );
        }
    }
    
    if ( unicode_charmap == NULL ) {
        *map = NULL;
        return FT_Err_Ok;
    }

    FT_Error error;

    if ( FT_NEW( *map ) ) {
        goto Exit;
    }

    FT_Int capacity = 10;
    FT_Int size = 0;
    
    if ( FT_NEW_ARRAY((*map)->entries, capacity) ) {
        goto Exit;
    }
    for ( FT_Int i = 0; i < AF_ADJUSTMENT_DATABASE_LENGTH; i++ ) {
        FT_UInt32 codepoint = adjustment_database[i].codepoint;
        FT_Int glyph = unicode_charmap->clazz->char_index(unicode_charmap, codepoint);
        if ( glyph == 0 ) {
            continue;
        }
        if (size == capacity) {
            capacity += capacity / 2;
            if ( FT_RENEW_ARRAY((*map)->entries, size, capacity) ) {
                goto Exit;
            }
        }
        size++;
        (*map)->entries[i].glyph_index = glyph;
        (*map)->entries[i].codepoint = codepoint;
    }
    (*map)->length = size;

Exit:
    if ( error ) {
        if ( *map ) {
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