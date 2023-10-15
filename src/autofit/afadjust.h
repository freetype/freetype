#ifndef AFADJUST_H_
#define AFADJUST_H_
#include <freetype/fttypes.h>
#include "aftypes.h"
#include "afglobal.h"

FT_BEGIN_HEADER

/*The type of adjustment that should be done to prevent cases where 2 parts of a character*/
/*stacked vertically merge, even though they should be separate*/
typedef enum AF_VerticalSeparationAdjustmentType_
{
    /*This means that the hinter should find the topmost contour and push it up until its lowest point is 1 pixel*/
    /*above the highest point not part of that contour.*/
    AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP,

    /*This is the opposite direction.  The hinter should find the bottommost contour*/
    /*And push it down until there is a 1 pixel gap*/
    AF_VERTICAL_ADJUSTMENT_BOTTOM_CONTOUR_DOWN,

    AF_VERTICAL_ADJUSTMENT_NONE

    /*others will be needed, such as umlats, where there are 2 contours which should be moved together*/
} AF_VerticalSeparationAdjustmentType;

typedef struct AF_AdjustmentDatabaseEntry_
{
  FT_UInt32 codepoint;
  AF_VerticalSeparationAdjustmentType vertical_separation_adjustment_type;
  FT_Bool apply_tilde;
} AF_AdjustmentDatabaseEntry;

FT_LOCAL(AF_VerticalSeparationAdjustmentType)
af_lookup_vertical_seperation_type( AF_ReverseCharacterMap map, FT_Int glyph_index );

FT_LOCAL( FT_Bool )
af_lookup_tilde_correction_type( AF_ReverseCharacterMap map, FT_Int glyph_index );

FT_LOCAL( FT_UInt32 )
af_reverse_character_map_lookup( AF_ReverseCharacterMap map, FT_Int glyph_index );

/*allocate and populate the reverse character map, using the character map within the face*/
FT_LOCAL( FT_Error )
af_reverse_character_map_new( AF_ReverseCharacterMap *map, AF_FaceGlobals globals );

/*free the reverse character map*/
FT_LOCAL( FT_Error )
af_reverse_character_map_done( AF_ReverseCharacterMap map, FT_Memory memory );

FT_END_HEADER

#endif
