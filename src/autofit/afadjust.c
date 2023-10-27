#include "afadjust.h"
#include <freetype/freetype.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftmemory.h>
#include <freetype/internal/ftdebug.h>

#define AF_ADJUSTMENT_DATABASE_LENGTH ( sizeof(adjustment_database)/sizeof(adjustment_database[0]) )
#undef  FT_COMPONENT
#define FT_COMPONENT  afadjust

#ifdef FT_CONFIG_OPTION_USE_HARFBUZZ
    #include <hb.h>
    #include <hb-ot.h>
#endif

/*
  All entries in this list must be sorted by unicode codepoint ascending
  The table entries are 3 numbers consisting of:
  - unicode codepoint.  THESE MUST BE LISTED IN ASCENDING ORDER
  - the vertical adjustment type.  One of the entries in AF_VerticalSeparationAdjustmentType
  - 0 if the topmost contour is a tilde and should be prevented from flattening.
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

/*FT_LOCAL_DEF( FT_Bool )
af_adjustment_database_entry_equals( const AF_AdjustmentDatabaseEntry* a, const AF_AdjustmentDatabaseEntry* b )
{
    return a->codepoint == b->codepoint && a->vertical_separation_adjustment_type == b->vertical_separation_adjustment_type;
}*/

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

/* qsort compare function for reverse character map */
FT_LOCAL_DEF( FT_Int )
af_reverse_character_map_entry_compare( const void *a, const void *b ) {
    const AF_ReverseMapEntry entry_a = *((const AF_ReverseMapEntry *)a);
    const AF_ReverseMapEntry entry_b = *((const AF_ReverseMapEntry *)b);
    return entry_a.glyph_index < entry_b.glyph_index ? -1 : entry_a.glyph_index > entry_b.glyph_index ? 1 : 0;
}

#ifdef FT_CONFIG_OPTION_USE_HARFBUZZ
/*
Recursive algorithm to find all glyphs that a codepoint could turn into from the GSUB table.

buffer: a buffer containing only the input codepoint
feature_tag_pool: the current list of features under consideration
current_features: the current list of features being applied
num_features: length of current_features
result: the set of glyphs that the input codepoint can map to.

The algorithm works by running the hb_ot_shape_glyphs_closure function on different lists of features
to see which features will map to glyph onto something different.  This functions returns
the result of transforming a glyph using a list of features as well as all intermediate
forms if the glyph was transformed multiple times.
With no features enabled, hb_ot_shape_glyphs_closure will only return the glyph given by cmap.
This character will be the first to be placed into the results set.
Next, the algorithm will test the same lookup enabline one feature at a time
and see if any of those features change the result.
If any new glyph variants are found this way, they are added to the results set
and the algorithm will recurce, trying that feature in combination will every other feature
to look for further glyph variants.

example:
suppose we have the following features in the GSUB table:
f1:
a -> b

f2:
b -> c

f3:
d -> e

The algorithm will take the following steps to find all variants of "a":
- a is added to the results
- lookup with feature list {f1}, yielding {a, b}. b is added to the results list and the algorithm recurses
- - lookup with feature list {f1, f2}, yielding {a, b, c}.  c is added to the results list and the algorithm recurses
- - - lookup with feature list {f1, f2, f3} yielding {a, b, c}.  No new glyphs
- - lookup with feature list {f1, f3}, yielding {a, b}.  No new glyphs
- lookup with feature list {f2}, yielding {a}. No new glyphs
- lookup with feature list {f3}, yielding {a}. No new glyphs

*/
FT_LOCAL_DEF( FT_Error )
af_all_glyph_variants_helper( hb_font_t *font,
                              hb_buffer_t *buffer,
                              hb_set_t *feature_tag_pool,
                              hb_feature_t *current_features,
                              FT_UInt32 num_features,
                              hb_set_t* result )
{
    FT_Error error;
    /*get the list of glyphs that are created by only transforming based on the
    features in current_features*/
    hb_set_t *baseline_glyphs = NULL, *new_glyphs = NULL;
    baseline_glyphs = hb_set_create();
    if ( !hb_set_allocation_successful( baseline_glyphs ) )
    {
        error = FT_Err_Out_Of_Memory;
        goto Exit;
    }



    hb_ot_shape_glyphs_closure ( font,
                                 buffer,
                                 current_features,
                                 num_features,
                                 baseline_glyphs );
    if ( !hb_set_allocation_successful( baseline_glyphs ) )
    {
        error = FT_Err_Out_Of_Memory;
        goto Exit;
    }

    /*Add these baseline glyphs to the results.  The baseline glyphs
    will contain at minimum the glyph specified by CMAP*/
    hb_set_union( result, baseline_glyphs );
    if ( !hb_set_allocation_successful( result ) )
    {
        error = FT_Err_Out_Of_Memory;
        goto Exit;
    }
    if ( hb_set_get_population( feature_tag_pool ) == 0 )
    {
        error = FT_Err_Out_Of_Memory;
        goto Exit;
    }

    /*setup to try adding different features to current_features
    to see if any of them have an effect of the glyphs we get from
    hb_ot_shape_glyphs_closure*/
    current_features[num_features].start = HB_FEATURE_GLOBAL_START;
    current_features[num_features].end = HB_FEATURE_GLOBAL_END;
    current_features[num_features].value = 1; /*set the feature to enabled*/
    /*quote from docs about value attribute:
    0 disables the feature, non-zero (usually 1) enables the feature. For features implemented as lookup type 3 (like 'salt') the value is a one based index into the alternates.
    this algorithm does not handle these lookup type 3 cases fully*/

    new_glyphs = hb_set_create();
    if ( !hb_set_allocation_successful( new_glyphs ) )
    {
        error = FT_Err_Out_Of_Memory;
        goto Exit;
    }

    hb_tag_t feature_tag = HB_SET_VALUE_INVALID;
    while ( hb_set_next( feature_tag_pool, &feature_tag ) )
    {
        hb_set_clear( new_glyphs );
        current_features[num_features].tag = feature_tag;
        hb_ot_shape_glyphs_closure ( font,
                                     buffer,
                                     current_features,
                                     num_features + 1,
                                     new_glyphs );
        if ( !hb_set_allocation_successful( new_glyphs ) )
        {
            error = FT_Err_Out_Of_Memory;
            goto Exit;
        }

        hb_set_subtract( new_glyphs, result );
        /*new_glyphs now contains all glyphs that appeared in the result
        of hb_ot_shape_glyphs_closure that haven't already been accounted for in the result.
        If this contains any glyphs, we also need to try this feature
        in combination will other features by recursing
        */
        if ( hb_set_get_population( new_glyphs ) != 0 )
        {
            /*remove this feature from the feature pool so that
            the later recursion won't try it*/
            hb_set_del( feature_tag_pool, feature_tag );
            error = af_all_glyph_variants_helper( font,
                                                  buffer,
                                                  feature_tag_pool,
                                                  current_features,
                                                  num_features + 1,
                                                  result );
            if ( error )
            {
                goto Exit;
            }

            /*add back the feature we removed*/
            hb_set_add( feature_tag_pool, feature_tag );
            if ( !hb_set_allocation_successful( feature_tag_pool ) ) {
                return FT_Err_Out_Of_Memory;
            }
        } /* if( !hb_set_is_subset( glyphs, result ) ) */

    } /*while ( hb_set_next( feature_tag_pool, &feature_tag ) )*/
Exit:
    hb_set_destroy( baseline_glyphs );
    hb_set_destroy( new_glyphs );
    return FT_Err_Ok;
}

FT_LOCAL_DEF( FT_Error )
af_all_glyph_variants( FT_Face face,
                       hb_font_t *hb_font,
                       FT_UInt32 codepoint,
                       hb_set_t* result )
{
    FT_Memory memory = face->memory;
    FT_Error error;
    hb_face_t *hb_face = hb_font_get_face( hb_font );

    /*The set of all feature tags in the font*/
    hb_set_t *feature_tags = hb_set_create();
    hb_set_t *type_3_lookup_indicies = hb_set_create();
    hb_buffer_t *codepoint_buffer = hb_buffer_create();
    hb_codepoint_t *type_3_alternate_glyphs_buffer;
    if ( !hb_set_allocation_successful( feature_tags ) )
    {
        error = FT_Err_Out_Of_Memory;
        goto Exit;
    }
    if ( !hb_buffer_allocation_successful( codepoint_buffer ) )
    {
        error = FT_Err_Out_Of_Memory;
        goto Exit;
    }
    if ( !hb_set_allocation_successful ( type_3_lookup_indicies ) )
    {
        error = FT_Err_Out_Of_Memory;
        goto Exit;
    }

    /*populate feature_tags using the output of hb_ot_layout_table_get_feature_tags*/
    FT_Bool feature_list_done = 0;
    unsigned int start_offset = 0;
    while ( !feature_list_done )
    {
        unsigned int feature_count = 20;
        hb_tag_t tags[20];
        hb_ot_layout_table_get_feature_tags ( hb_face,
                                              HB_OT_TAG_GSUB,
                                              start_offset,
                                              &feature_count,
                                              tags );
        start_offset += 20;
        if ( feature_count < 20 )
        {
            feature_list_done = 1;
        }
        for ( int i = 0; i < feature_count; i++ )
        {
            hb_set_add( feature_tags, tags[i] );
        }
        if ( !hb_set_allocation_successful( feature_tags ) )
        {
            error = FT_Err_Out_Of_Memory;
            goto Exit;
        }
    }

    /*make a buffer only consisting of the given codepoint*/
    if ( !hb_buffer_pre_allocate( codepoint_buffer, 1 ) )
    {
        error = FT_Err_Out_Of_Memory;
        goto Exit;
    }
    hb_buffer_set_direction ( codepoint_buffer,
                              HB_DIRECTION_LTR );
    hb_buffer_add( codepoint_buffer, codepoint, 0 );

    /*The array of features that will be used by the recursive part
    it will have at most as many entries as there are features, so
    make the length = length of feature_tags*/
    hb_feature_t *feature_buffer;
    if (( error = FT_NEW_ARRAY( feature_buffer, hb_set_get_population( feature_tags ) ) ))
    {
        goto Exit;
    }

    error = af_all_glyph_variants_helper( hb_font,
                                          codepoint_buffer,
                                          feature_tags,
                                          feature_buffer,
                                          0,
                                          result );
    if ( error )
    {
        goto Exit;
    }

    /*
    Add the alternative glyph forms that come from features using
    type 3 lookups.
    This file from gtk was very useful in figuring out my approach:
    https://github.com/val-verde/gtk/blob/212e85e92628eda9f9650e5cc8d88c44062642ae/gtk/gtkfontchooserwidget.c#L2085
    */
    /*list of features containing type 3 lookups:*/
    hb_tag_t feature_list[] = {
        HB_TAG('s','a','l','t'),
        HB_TAG('s','w','s','h'),
        HB_TAG('n','a','l','t'),
        HB_TAG_NONE
    };


    hb_ot_layout_collect_lookups ( hb_face,
                                   HB_OT_TAG_GSUB,
                                   NULL,
                                   NULL,
                                   feature_list,
                                   type_3_lookup_indicies);

    if ( !hb_set_allocation_successful( type_3_lookup_indicies ) )
    {
        error = FT_Err_Out_Of_Memory;
        goto Exit;
    }

    if (( error = FT_NEW_ARRAY( type_3_alternate_glyphs_buffer, 100 ) ))
    {
        goto Exit;
    }
    hb_codepoint_t lookup_index = HB_SET_VALUE_INVALID;
    FT_UInt base_glyph_index = FT_Get_Char_Index( face, codepoint );
    if ( base_glyph_index != 0 ) {
        while ( hb_set_next( type_3_lookup_indicies, &lookup_index ) )
        {
            unsigned alternate_count = 100;

            hb_ot_layout_lookup_get_glyph_alternates
                                        ( hb_face,
                                          lookup_index,
                                          base_glyph_index,
                                          0,
                                          &alternate_count,
                                          type_3_alternate_glyphs_buffer );

            for ( unsigned i = 0; i < alternate_count; i++ )
            {
                hb_set_add( result, type_3_alternate_glyphs_buffer[i] );
            }
        }
    }



Exit:
    hb_set_destroy( feature_tags );
    hb_buffer_destroy( codepoint_buffer );
    FT_FREE( feature_buffer );
    FT_FREE( type_3_alternate_glyphs_buffer );
    return error;
}
#endif /*FT_CONFIG_OPTION_USE_HARFBUZZ*/

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


#ifdef FT_CONFIG_OPTION_USE_HARFBUZZ
    hb_font_t *hb_font = globals->hb_font;
    /*hb_face_t *hb_face = hb_font_get_face( hb_font );*/
    hb_set_t *result_set = hb_set_create();
    if ( !hb_set_allocation_successful( result_set ) )
    {
        error = FT_Err_Out_Of_Memory;
        goto harfbuzz_path_Exit;
    }

    /*find alll glyph variants of the codepoints, then make an entry from
    the glyph to the codepoint for each one*/
    for ( FT_Long i = 0; i < AF_ADJUSTMENT_DATABASE_LENGTH; i++ )
    {
        FT_UInt32 codepoint = adjustment_database[i].codepoint;

        error = af_all_glyph_variants( face,
                                       hb_font,
                                       codepoint,
                                       result_set );
        if ( error )
        {
            goto harfbuzz_path_Exit;
        }

        hb_codepoint_t glyph = HB_SET_VALUE_INVALID;
        while ( hb_set_next( result_set, &glyph ) )
        {
            error = af_reverse_character_map_expand( *map, &capacity, memory );
            if ( error ) {
                goto harfbuzz_path_Exit;
            }

            FT_Long insert_point = ( *map )->length;
            ( *map )->length++;
            ( *map )->entries[insert_point].glyph_index = glyph;
            ( *map )->entries[insert_point].codepoint = codepoint;
        }

        hb_set_clear( result_set );
    }
harfbuzz_path_Exit:
    hb_set_destroy( result_set );
    if ( error )
    {
        goto Exit;
    }
#else /* !FT_CONFIG_OPTION_USE_HARFBUZZ */
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

#endif /*FT_CONFIG_OPTION_USE_HARFBUZZ*/

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
