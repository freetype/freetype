/***************************************************************************/
/*                                                                         */
/*  gxltypes.h                                                             */
/*                                                                         */
/*    Data types of AAT/TrueTypeGX based layout engine                     */
/*   (specification)                                                       */
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

#ifndef __GXLTYPES_H__
#define __GXLTYPES_H__ 

#include <ft2build.h>
#include FT_INTERNAL_TRUETYPE_TYPES_H
#include FT_INTERNAL_FTL_TYPES_H
#include FT_GXLAYOUT_H

FT_BEGIN_HEADER
  
  typedef enum
  {
    GXL_ASCENDING  = 0,
    GXL_DESCENDING = 1
  } GXL_Order;

  typedef struct GXL_Feature_ExclusiveRec_
  {
    FT_Bool     exclusive;
    GXL_Setting setting;
  } GXL_Feature_ExclusiveRec, *GXL_Feature_Exclusive;

  typedef struct GXL_NameRec_
  {
    const FT_String * string;
    FT_Short index;
  } GXL_NameRec, *GXL_Name;
    
  typedef struct GXL_SettingRec_
  {
    FT_UShort    value;
    GXL_NameRec  name;
    GXL_Feature  feature;
  } GXL_SettingRec; /* *GXL_Setting; */

  typedef struct GXL_FeatureRec_
  {
    GXL_FeaturesRequest request;
    FT_UShort   value;
    GXL_Feature_ExclusiveRec exclusive;
    GXL_NameRec  name;
    FT_UShort   nSettings;
    GXL_Setting setting;
  } GXL_FeatureRec; /* , * GXL_Feature; */

  typedef struct GXL_FeaturesRequestRec_
  {
    FTL_FeaturesRequestRec root;
    GXL_Initial_State initial_state;
    FT_ULong      nFeatures;
    GXL_Feature   feature;
  } GXL_FeaturesRequestRec; /* , *GXL_FeaturesRequest; */


  FT_LOCAL ( void )
  gxl_features_request_free( GXL_FeaturesRequest request, FT_Memory memory );

  FT_LOCAL ( GXL_Setting )
  gxl_feature_get_setting_by_value (GXL_Feature feature, FT_UShort value);

  FT_LOCAL ( GXL_Feature )
  gxl_features_request_get_feature_by_type ( GXL_FeaturesRequest request,
					     FT_UShort featureType );

  FT_LOCAL ( FT_Error )
  gxl_get_font ( FT_Face face, FTL_Font * font);

  FT_LOCAL ( FTL_EngineType )
  gxl_get_engine_type ( FT_Face face );

  FT_LOCAL ( FT_Error ) 
  gxl_new_features_request( FT_Face face, FTL_FeaturesRequest * request);
  FT_LOCAL ( FT_Error ) 
  gxl_done_features_request( FTL_FeaturesRequest request);
  FT_LOCAL ( FT_Error ) 
  gxl_copy_features_request( FTL_FeaturesRequest from,
			     FTL_FeaturesRequest to);
  FT_LOCAL ( FT_UShort )
  gxl_get_ligature_caret_count ( FT_Face face,
				 FT_UShort glyphID );
  FT_LOCAL ( FT_UShort )
  gxl_get_ligature_caret_division( FT_Face face, 
				   FT_UShort glyphID, 
				   FT_UShort nth );
  FT_LOCAL ( FT_Error )
  gxl_substitute_glyphs ( FT_Face face,
			  FTL_FeaturesRequest request,
			  FTL_GlyphArray in,
			  FTL_GlyphArray out );

FT_END_HEADER

#endif /* Not def: __GXLTYPES_H__ */


/* END */
