/*******************************************************************
 *
 *  t1tokens.h
 *
 *  Type 1 tokens definition
 *
 *  Copyright 2000 David Turner, Robert Wilhelm and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT. By continuing to use, modify or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *  This file only contains macros that are expanded when compiling
 *  the "t1load.c" source file.
 *
 ******************************************************************/

 /* define the font info dictionary parsing callbacks */
#undef  FACE
#define FACE  (face->type1.font_info)

  PARSE_STRING("version",version)
  PARSE_STRING("Notice",notice)
  PARSE_STRING("FullName",full_name)
  PARSE_STRING("FamilyName",family_name)
  PARSE_STRING("Weight",weight)
  
  PARSE_INT("ItalicAngle",italic_angle)
  PARSE_BOOL("isFixedPitch",is_fixed_pitch)
  PARSE_NUM("UnderlinePosition",underline_position,T1_Short)
  PARSE_NUM("UnderlineThickness",underline_thickness,T1_UShort)

 /* define the private dict parsing callbacks */ 
 
#undef  FACE
#define FACE  (face->type1.private_dict)

   PARSE_INT("UniqueID",unique_id)
   PARSE_INT("lenIV",lenIV)
   
   PARSE_COORDS( "BlueValues", num_blues,       14, blue_values)
   PARSE_COORDS( "OtherBlues", num_other_blues, 10, other_blues)
   
   PARSE_COORDS( "FamilyBlues", num_family_blues,       14, family_blues)
   PARSE_COORDS( "FamilyOtherBlues", num_family_other_blues, 10, family_other_blues)
 
   PARSE_FIXED( "BlueScale", blue_scale)
   PARSE_INT( "BlueShift", blue_shift)
   
   PARSE_INT( "BlueFuzz", blue_fuzz)

   PARSE_COORDS2( "StdHW", 1, standard_width  )
   PARSE_COORDS2( "StdVW", 1, standard_height )
   
   PARSE_COORDS( "StemSnapH", num_snap_widths,  12, stem_snap_widths )
   PARSE_COORDS( "StemSnapV", num_snap_heights, 12, stem_snap_heights )
   
   PARSE_INT( "LanguageGroup", language_group )
   PARSE_INT( "password", password )
   PARSE_COORDS2( "MinFeature", 2, min_feature )

 /* define the top-level dictionary parsing callbacks */

#undef  FACE
#define FACE  (face->type1)

 
/* PARSE_STRING( "FontName", font_name ) -- handled by special routine */
   PARSE_NUM( "PaintType", paint_type, T1_Byte )
   PARSE_NUM( "FontType", font_type, T1_Byte )
   PARSE_FIXEDS2( "FontMatrix", 4, font_matrix )
/*  PARSE_COORDS2( "FontBBox", 4, font_bbox ) -- handled by special func */
   PARSE_INT( "StrokeWidth", stroke_width )

#undef FACE


