/***************************************************************************/
/*                                                                         */
/*  gxfeatreg.c                                                            */
/*                                                                         */
/*    Database of font features pre-defined by Apple Computer, Inc.        */
/*    http://developer.apple.com/fonts/Registry/                           */
/*    (body).                                                              */
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/

#include "gxfeatreg.h"

#define EMPTYFEAT {0, 0, {NULL}}

/***************************************************************************/
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/***************************************************************************/
/* 
 * If you add a new setting to a feature, check the number of setting
 * in the feature. If the number is greater than value defined as 
 * FEATREG_MAX_SETTING, update the value. Nor you cannot initialize 
 * featreg_table.
 * This program is written in C.                                           
 */
/***************************************************************************/
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/***************************************************************************/

#define FEATREG_MAX_SETTING 12

typedef struct GX_Feature_RegistryRec_
{
  const FT_String * feat_name;
  FT_Bool exclusive;
  FT_String *setting_name[FEATREG_MAX_SETTING];
  FT_UShort feature_type;		/* Initialized on the fly. Thread unsafe? */
} GX_Feature_RegistryRec;

static GX_Feature_RegistryRec featreg_table [] = {
  {				/* 0 */
    "All Typographic Features",
    0,
    { 
      "All Type Features",
      NULL
    }
  }, {				/* 1 */
    "Ligatures",
    0,
    {
      "Required Ligatures",
      "Common Ligatures",
      "Rare Ligatures",
      "Logos",
      "Rebus Pictures",
      "Diphthong Ligatures",
      "Squared Ligatures",
      "Squared Ligatures, Abbreviated",
      NULL
    }
  }, {				/* 2 */
    "Cursive Connection",
    1,
    {
      "Unconnected",
      "Partially Connected",
      "Cursive",
      NULL
    }
  }, {				/* 3 */
    "Letter Case",
    1,
    {
      "Upper & Lower Case",
      "All Caps",
      "All Lower Case",
      "Small Caps",
      "Initial Caps",
      "Initial Caps & Small Caps",
      NULL
    }
  }, {				/* 4 */
    "Vertical Substitution",
    0,
    {
      /* "Substitute Vertical Forms", */
      "Turns on the feature",
      NULL
    }
  }, {				/* 5 */
    "Linguistic Rearrangement",
    0,
    {
      /* "Linguistic Rearrangement", */
      "Turns on the feature",
      NULL
    }
  }, {				/* 6 */
    "Number Spacing",
    1,
    {
      "Monospaced Numbers",
      "Proportional Numbers",
      NULL
    }
  }, {				/* 7 */
    "Apple Reserved 1",
    0,
    {NULL}
  }, {				/* 8 */
    "Smart Swashes",
    0,
    {
      "Word Initial Swashes",
      "Word Final Swashes",
      "Line Initial Swashes",
      "Line Final Swashes",
      "Non-Final Swashes",
      NULL
    }
  }, {				/* 9 */
    "Diacritics",
    1,
    {
      "Show Diacritics",
      "Hide Diacritics",
      "Decompose Diacritics",
      NULL
    }
  }, {				/* 10 */
    "Vertical Position",
    1,
    {
      /* "Normal Position", */
      "No Vertical Position",
      "Superiors",
      "Inferiors",
      "Ordinals",
      NULL
    }
  }, {				/* 11 */
    "Fractions",
    1,
    {
      "No Fractions",
      "Vertical Fractions",
      "Diagonal Fractions",
      NULL
    }
  }, {				/* 12 */
    "Apple Reserved 2",
    0,
    {NULL}
  }, {				/* 13 */
    "Overlapping Characters",
    0,
    {
      /* "Prevent Overlap", */
      "Turns on the feature",
      NULL
    }
  }, {				/* 14 */
    "Typographic Extras",
    0,
    {
      "Hyphens to Em Dash",
      "Hyphens to En Dash",
      "Unslashed Zero",
      "Form Interrobang",
      "Smart Quotes",
      "Periods to Ellipsis",
      NULL
    }
  }, {				/* 15 */
    "Mathematical Extras",
    0,
    {
      "Hyphens to Minus",
      "Asterisk to Multiply",
      "Slash to Divide",
      "Inequality Ligatures",
      "Exponents",
      NULL
    }
  }, {				/* 16 */
    "Ornament Sets",
    1,
    {
      "No Ornaments",
      "Dingbats",
      "Pi Characters",
      "Fleurons",
      "Decorative Borders",
      "International Symbols",
      "Math Symbols",
      NULL
    }
  }, {				/* 17 */
    "Character Alternatives",
    1,
    {
      "No Alternates",
      /* TODO */
      NULL
    }
  }, {				/* 18 */
    "Design Complexity",
    1,
    {
      "Design Level 1",
      "Design Level 2",
      "Design Level 3",
      "Design Level 4",
      "Design Level 5",
      /* TODO */
      NULL
    }
  }, {				/* 19 */
    "Style Options",
    1,
    {
      "No Style Options",
      "Display Text",
      "Engraved Text",
      "Illuminated Caps",
      "Tilling Caps",
      "Tall Caps",
      NULL
    }
  }, {				/* 20 */
    "Character Shape",
    1,
    {
      "Traditional Characters",
      "Simplified Characters",
      "JIS 1978 Characters",
      "JIS 1983 Characters",
      "JIS 1990 Characters",
      "Traditional Characters, Alternative Set 1",
      "Traditional Characters, Alternative Set 2",
      "Traditional Characters, Alternative Set 3",
      "Traditional Characters, Alternative Set 4",
      "Traditional Characters, Alternative Set 5",
      "Expert Characters",
      NULL			/* count=>12 */
    }
  }, {				/* 21 */
    "Number Case",
    1,
    {
      "Lower Case Numbers",
      "Upper Case Numbers",
      NULL
    }
  }, {				/* 22 */
    "Text Spacing",
    1,
    {
      "Proportional",
      "Monospaced",
      "Half-width",
      "Normal",
      NULL
    }
  }, /* Here after Newer */  { /* 23 */
    "Transliteration",
    1,
    {
      "No Transliteration",
      "Hanja To Hangul",
      "Hiragana to Katakana",
      "Katakana to Hiragana",
      "Kana to Romanization",
      "Romanization to Hiragana",
      "Romanization to Katakana",
      "Hanja to Hangul, Alternative Set 1",
      "Hanja to Hangul, Alternative Set 2",
      "Hanja to Hangul, Alternative Set 3",
      NULL
    }
  }, {				/* 24 */
    "Annotation",
    1,
    {
      "No Annotation",
      "Box Annotation",
      "Rounded Box Annotation",
      "Circle Annotation",
      "Inverted Circle Annotation",
      "Parenthesis Annotation",
      "Period Annotation",
      "Roman Numeral Annotation",
      "Diamond Annotation",
      NULL
    }
  }, {				/* 25 */
    "Kana Spacing",
    1,
    {
      "Full Width",
      "Proportional",
      NULL
    }
  }, {				/* 26 */
    "Ideographic Spacing",
    1,
    {
      "Full Width",
      "Proportional",
      NULL
    }
  }, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 27-30 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 31-35 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 36-40 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 40-45 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 46-50 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 51-55 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 56-60 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 61-65 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 66-70 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 71-75 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 76-80 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 81-85 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 86-90 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 91-95 */
  EMPTYFEAT, EMPTYFEAT, EMPTYFEAT, /* 96-98 */
  EMPTYFEAT, /* 99 */ { /* 100 => 22*/
    "Text Spacing",
    1,
    {
      "Proportional",
      "Monospaced",
      "Half-width",
      "Normal",
      NULL
    }
  }, {				/* 101 => 25 */
    "Kana Spacing",
    1,
    {
      "Full Width",
      "Proportional",
      NULL
    }
  }, {				/* 102 => 26 */
    "Ideographic Spacing",
    1,
    {
      "Full Width",
      "Proportional",
      NULL
    }
  }, {				/* 103 */
    "CJK Roman Spacing",
    1,
    {
      "Half-width",
      "Proportional",
      "Default Roman",
      "Full-width Roman",
      NULL
    }
  }, {				/* 104 => 1 */
    "All Typographic Features",
    0,
    { 
      "All Type Features",
      NULL
    }
  }
#define FEATREG_MAX_INDEX 104
};



FT_LOCAL_DEF( GX_Feature_Registry )
gx_get_feature_registry( FT_UShort feature_type )
{
  GX_Feature_Registry featreg;

  if ( feature_type > FEATREG_MAX_INDEX )
    return NULL;

  featreg = &featreg_table[feature_type];
  if ( featreg->feat_name == 0 )
    return NULL;
  
  featreg->feature_type = feature_type;
  return featreg;
}

FT_LOCAL_DEF( const FT_String * )
gx_feature_registry_get_name( GX_Feature_Registry featreg )
{
  if ( featreg )
    return featreg->feat_name;
  else
    return NULL;
}
      
FT_LOCAL_DEF ( FT_Bool )
gx_feature_registry_is_setting_exclusive( GX_Feature_Registry featreg )
{
  if ( featreg )
    return featreg->exclusive;
  else
    return 0;
}

FT_LOCAL_DEF ( const FT_String * )
gx_feature_registry_get_setting_name( GX_Feature_Registry featreg, FT_UShort nth )
{
  FT_UShort max;
  if ( !featreg )
    return NULL;

  max = gx_feature_registry_count_setting ( featreg );
  if ( nth < max )
    return featreg->setting_name[nth];
  else
    return NULL;
}

FT_LOCAL_DEF ( FT_UShort )
gx_feature_registry_count_setting ( GX_Feature_Registry featreg )
{
  FT_Int i;
  if ( !featreg )
    return 0;
  
  for ( i = 0; featreg->setting_name[i]; i++ )
    ;				/* Do nothing */
  return (FT_UShort)i;
}

FT_LOCAL_DEF( FT_UShort )
gx_feature_registry_get_value(GX_Feature_Registry featreg)
{
  return featreg->feature_type;
}

/* END */
