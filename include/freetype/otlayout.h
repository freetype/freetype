/***************************************************************************/
/*                                                                         */
/*  otlayout.h                                                             */
/*                                                                         */
/*    OpenType based layout engine                                         */
/*    (For application developers, specification only).                    */
/*                                                                         */
/***************************************************************************/


#ifndef __OTLAYOUT_H__
#define __OTLAYOUT_H__ 

#include <ft2build.h>
#include FT_FREETYPE_H
FT_BEGIN_HEADER

  typedef FT_ULong OTTag;
  typedef enum {
    OT_TABLE_GSUB,
    OT_TABLE_GPOS
  } OTTableType;

  typedef struct OTL_FeaturesRequestRec_ *OTL_FeaturesRequest;
  typedef struct OTL_Tag_ListRec_
  {
    OTL_FeaturesRequest request;
    OTTag * tags;			/* 0 is terminator. */
  } OTL_Tag_ListRec, *OTL_Tag_List;

  FT_EXPORT( FT_Bool )
  OTL_FeaturesRequest_Find_Script    ( OTL_FeaturesRequest request,
				       OTTableType table_type,
				       OTTag       script_tag,
				       FT_UInt    *script_index);

  FT_EXPORT( FT_Bool )
  OTL_FeaturesRequest_Find_Language  ( OTL_FeaturesRequest request,
				       OTTableType table_type,
				       FT_UInt     script_index,
				       OTTag       language_tag,
				       FT_UInt    *language_index,
				       FT_UInt    *required_feature_index );

  FT_EXPORT ( FT_Bool )
  OTL_FeaturesRequest_Find_Feature   ( OTL_FeaturesRequest request,
				       OTTableType         table_type,
				       OTTag               feature_tag,
				       FT_UInt             script_index,
				       FT_UInt             language_index,
				       FT_UInt            *feature_index );

  FT_EXPORT ( OTL_Tag_List )
  OTL_FeaturesRequest_List_Scripts   ( OTL_FeaturesRequest request,
				       OTTableType         table_type );

  FT_EXPORT ( OTL_Tag_List ) 
  OTL_FeaturesRequest_List_Languages ( OTL_FeaturesRequest request,
				       OTTableType         table_type,
				       FT_UInt             script_index );

  FT_EXPORT ( OTL_Tag_List )
  OTL_FeaturesRequest_List_Features  ( OTL_FeaturesRequest request,
				       OTTableType         table_type,
				       FT_UInt             script_index,
				       FT_UInt             language_index );

  FT_EXPORT ( FT_Error )
  OTL_Tag_List_Done                  ( OTL_Tag_List taglist );

  FT_EXPORT ( FT_Error )
  OTL_FeaturesRequest_Add_Feature  ( OTL_FeaturesRequest request,
				     OTTableType         table_type,
				     FT_UInt             feature_index,
				     FT_ULong            property_bit);

FT_END_HEADER

#endif /* Not def: __OTLAYOUT_H__ */
