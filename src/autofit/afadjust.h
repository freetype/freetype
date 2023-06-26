#ifndef AFADJUST_H_
#define AFADJUST_H_

#include <freetype/fttypes.h>

FT_BEGIN_HEADER

/*The type of adjustment that should be done to prevent cases where 2 parts of a character*/
/*stacked vertically merge, even though they should be separate*/
typedef enum AF_VerticalSeparationAdjustmentType_
{
    AF_VERTICAL_ADJUSTMENT_TOP_CONTOUR_UP,
    /*This means that the hinter should find the topmost contour and push it up until its lowest point is 1 pixel*/
    /*above the highest point not part of that contour.*/
    AF_VERTICAL_ADJUSTMENT_NONE

    /*others will be needed, such as the case where the lower contour should be moved in the adjustment instead of the upper one*/
    /*or umlats, where there are 2 contours which should be moved together*/
    /*and a way of handling A and O, where the letter consists of 2 contours*/
} AF_VerticalSeparationAdjustmentType;

typedef struct AF_AdjustmentDatabaseEntry_
{
  FT_UInt32 codepoint;
  AF_VerticalSeparationAdjustmentType vertical_separation_adjustment_type;
} AF_AdjustmentDatabaseEntry;

struct AF_ReverseCharacterMap_;

typedef struct AF_ReverseCharacterMap_ *AF_ReverseCharacterMap;

FT_LOCAL(AF_VerticalSeparationAdjustmentType)
af_lookup_vertical_seperation_type( AF_ReverseCharacterMap map, FT_Int glyph_index );

FT_LOCAL( FT_UInt32 )
af_reverse_character_map_lookup( AF_ReverseCharacterMap map, FT_Int glyph_index );

/*allocate and populate the reverse character map, using the character map within the face*/
FT_LOCAL( FT_Error )
af_reverse_character_map_new( FT_Face face, AF_ReverseCharacterMap *map, FT_Memory memory );

/*free the reverse character map*/
FT_LOCAL( FT_Error )
af_reverse_character_map_done( AF_ReverseCharacterMap map, FT_Memory memory );

FT_END_HEADER

#endif
