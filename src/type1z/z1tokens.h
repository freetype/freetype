/***************************************************************************/
/*                                                                         */
/*  z1tokens.h                                                             */
/*                                                                         */
/*    Experimental Type 1 tokenizer (specification).                       */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#undef  FT_STRUCTURE
#define FT_STRUCTURE  T1_FontInfo

  Z1_FONTINFO_STRING( "version", version )
  Z1_FONTINFO_STRING( "Notice", notice )
  Z1_FONTINFO_STRING( "FullName", full_name )
  Z1_FONTINFO_STRING( "FamilyName", family_name )
  Z1_FONTINFO_STRING( "Weight", weight )

  Z1_FONTINFO_NUM   ( "ItalicAngle", italic_angle )
  Z1_FONTINFO_BOOL  ( "isFixedPitch", is_fixed_pitch )
  Z1_FONTINFO_NUM   ( "UnderlinePosition", underline_position )
  Z1_FONTINFO_NUM   ( "UnderlineThickness", underline_thickness )


#undef  FT_STRUCTURE
#define FT_STRUCTURE  T1_Private

  Z1_PRIVATE_NUM       ( "UniqueID", unique_id )
  Z1_PRIVATE_NUM       ( "lenIV", lenIV )
  Z1_PRIVATE_NUM       ( "LanguageGroup", language_group )
  Z1_PRIVATE_NUM       ( "password", password )

  Z1_PRIVATE_FIXED     ( "BlueScale", blue_scale )
  Z1_PRIVATE_NUM       ( "BlueShift", blue_shift )
  Z1_PRIVATE_NUM       ( "BlueFuzz",  blue_fuzz )

  Z1_PRIVATE_NUM_TABLE ( "BlueValues", blue_values, 14, num_blue_values )
  Z1_PRIVATE_NUM_TABLE ( "OtherBlues", other_blues, 10, num_other_blues )
  Z1_PRIVATE_NUM_TABLE ( "FamilyBlues", family_blues, 14, num_family_blues )
  Z1_PRIVATE_NUM_TABLE ( "FamilyOtherBlues", family_other_blues, 10, \
                                             num_family_other_blues )

  Z1_PRIVATE_NUM_TABLE2( "StdHW", standard_width,  1 )
  Z1_PRIVATE_NUM_TABLE2( "StdVW", standard_height, 1 )
  Z1_PRIVATE_NUM_TABLE2( "MinFeature", min_feature, 2 )

  Z1_PRIVATE_NUM_TABLE ( "StemSnapH", snap_widths, 12, num_snap_widths )
  Z1_PRIVATE_NUM_TABLE ( "StemSnapV", snap_heights, 12, num_snap_heights )


#undef  FT_STRUCTURE
#define FT_STRUCTURE  T1_Font

  Z1_TOPDICT_NUM( "PaintType", paint_type )
  Z1_TOPDICT_NUM( "FontType", font_type )
  Z1_TOPDICT_NUM( "StrokeWidth", stroke_width )

/* END */
