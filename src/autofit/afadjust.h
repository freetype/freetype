#ifndef AFADJUST_H_
#define AFADJUST_H_

#include <freetype/fttypes.h>

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
} AF_VerticalSeparationAdjustmentType;

typedef struct AF_AdjustmentDatabaseEntry_ {
    FT_UInt32 codepoint;
    AF_VerticalSeparationAdjustmentType vertical_separation_adjustment_type;
  } AF_AdjustmentDatabaseEntry;

struct AF_ReverseCharacterMap_;

typedef struct AF_ReverseCharacterMap_ *AF_ReverseCharacterMap;

FT_END_HEADER

#endif
