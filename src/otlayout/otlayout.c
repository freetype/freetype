/***************************************************************************/
/*                                                                         */
/*  otlayout.c                                                             */
/*                                                                         */
/*    OpenType based layout engine(body).                                  */
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

/*
 * Experimental: No support for gpos now.
 */

#include <ft2build.h>
#include FT_LIST_H
#include FT_OTLAYOUT_H

#include "otltypes.h"
#include "ot-types.h"
#include "ot-ruleset.h"
#include "ot-info.h"
#include "oterrors.h"

  FT_LOCAL_DEF ( FT_Error )
  otl_get_font ( FT_Face face, FTL_Font * font)
  {
    *font = ((OT_Face)face)->extra.data;
    
    if ( *font )
      return FT_Err_Ok;
    else
      return OT_Err_Invalid_Face_Handle;
  }

  FT_LOCAL_DEF ( FTL_EngineType )
  otl_get_engine_type ( FT_Face face )
  {
    return FTL_OPENTYPE_ENGINE;
  }

  FT_LOCAL ( FT_Error ) 
  otl_new_features_request( FT_Face face, FTL_FeaturesRequest * ftl_request)
  {
    FT_Error error;
    FT_Memory memory = FT_FACE_MEMORY(face);
    OTL_Font font;
    OTL_FeaturesRequest request;
    
    if (( error = otl_get_font ( face, (FTL_Font*)&font ) ))
      goto Exit;
    
    if ( FT_NEW(request) )
      goto Exit;
    
    request->ruleset = ot_ruleset_new ( font->info );
    if ( !request->ruleset )
      {
	error = OT_Err_Invalid_Argument; /* ??? */
	goto Failure;
      }
    
    if (( error =  FTL_FeaturesRequest_Init ( face, (FTL_FeaturesRequest)request ) ))
	goto Failure;
    *ftl_request = (FTL_FeaturesRequest)request;
  Exit:
    return error;    
  Failure:
    if ( request->ruleset )
      ot_ruleset_delete( request->ruleset );
    FT_FREE( request );
    return error;
  }

  FT_LOCAL ( FT_Error ) 
  otl_done_features_request( FTL_FeaturesRequest request)
  {
    FT_Error error;
    FTL_Font font;
    FT_Face face;
    FT_Memory memory;
    
    FT_ASSERT( request );
    font = FTL_FEATURES_REQUEST_FONT ( request );
    
    FT_ASSERT( font );
    face = FTL_FONT_FACE(font);

    if ( !face )
      return OT_Err_Invalid_Argument;
    memory = FT_FACE_MEMORY( face );
    ot_ruleset_delete ( ((OTL_FeaturesRequest)request)->ruleset );
    error = FTL_FeaturesRequest_Finalize ( request );
    FT_FREE( request );		/* broken */
    return error;
  }

  FT_LOCAL ( FT_Error ) 
  otl_copy_features_request( FTL_FeaturesRequest from,
			     FTL_FeaturesRequest to)
  {
    FT_Error error;
    ot_ruleset_copy(((OTL_FeaturesRequest)from)->ruleset, ((OTL_FeaturesRequest)to)->ruleset);
    error = FTL_FeaturesRequest_Copy( from, to );
    return error;
  }

  FT_LOCAL ( FT_Error )
  otl_substitute_glyphs ( FT_Face face,
			  FTL_FeaturesRequest request,
			  FTL_GlyphArray in,
			  FTL_GlyphArray out )
  {
    FT_Error error = FT_Err_Ok;
    if (( error = FTL_Copy_Glyphs_Array ( in, out ) ))
      return error;
    ot_ruleset_shape ( ((OTL_FeaturesRequest)request)->ruleset, out );
    return FT_Err_Ok;
  }

  FT_EXPORT( FT_Bool )
  OTL_FeaturesRequest_Find_Script    ( OTL_FeaturesRequest request,
				       OTTableType table_type,
				       OTTag       script_tag,
				       FT_UInt    *script_index)
  {
    OTL_Font font = (OTL_Font)FTL_FEATURES_REQUEST_FONT( request );
    return ot_info_find_script ( font->info, 
				 table_type, 
				 script_tag, 
				 script_index )
      ? 1: 0;
      
  }

  FT_EXPORT( FT_Bool )
  OTL_FeaturesRequest_Find_Language  ( OTL_FeaturesRequest request,
				       OTTableType table_type,
				       FT_UInt     script_index,
				       OTTag       language_tag,
				       FT_UInt    *language_index,
				       FT_UInt    *required_feature_index )
  {
    OTL_Font font = (OTL_Font)FTL_FEATURES_REQUEST_FONT( request );
    return ot_info_find_language ( font->info, 
				   table_type, 
				   script_index, 
				   language_tag,
				   language_index,
				   required_feature_index )
      ? 1: 0;
  }

  FT_EXPORT ( FT_Bool )
  OTL_FeaturesRequest_Find_Feature   ( OTL_FeaturesRequest request,
				       OTTableType         table_type,
				       OTTag               feature_tag,
				       FT_UInt             script_index,
				       FT_UInt             language_index,
				       FT_UInt            *feature_index )
  {
    OTL_Font font = (OTL_Font)FTL_FEATURES_REQUEST_FONT( request );
    return ot_info_find_feature ( font->info, 
				  table_type,
				  feature_tag,
				  script_index, 
				  language_index,
				  feature_index )
      ? 1: 0;
  }

  FT_EXPORT ( OTL_Tag_List )
  OTL_FeaturesRequest_List_Scripts   ( OTL_FeaturesRequest request,
				       OTTableType         table_type )
  {
    FT_Error error;
    OTL_Font font = (OTL_Font)FTL_FEATURES_REQUEST_FONT( request );
    FT_Face face = FTL_FONT_FACE(font);
    FT_Memory memory = FT_FACE_MEMORY(face);
    OTL_Tag_List tag_list;
    if ( FT_NEW(tag_list) )
      return NULL;

    tag_list->request = request;
    tag_list->tags 	       = ot_info_list_scripts( font->info, table_type );
    return tag_list;
  }

  FT_EXPORT ( OTL_Tag_List ) 
  OTL_FeaturesRequest_List_Languages ( OTL_FeaturesRequest request,
				       OTTableType         table_type,
				       FT_UInt             script_index )
  {
    FT_Error error;
    OTL_Font font = (OTL_Font)FTL_FEATURES_REQUEST_FONT( request );
    FT_Face face = FTL_FONT_FACE( font );
    FT_Memory memory = FT_FACE_MEMORY( face );
    OTL_Tag_List tag_list;
    if ( FT_NEW(tag_list) )
      return NULL;

    tag_list->request = request;
    tag_list->tags 	       = ot_info_list_languages( font->info, 
							 table_type,
							 script_index,
							 0 );
    return tag_list;
  }

  FT_EXPORT ( OTL_Tag_List )
  OTL_FeaturesRequest_List_Features  ( OTL_FeaturesRequest request,
				       OTTableType         table_type,
				       FT_UInt             script_index,
				       FT_UInt             language_index )
  {
    FT_Error error;
    OTL_Font font = (OTL_Font)FTL_FEATURES_REQUEST_FONT( request );
    FT_Face face = FTL_FONT_FACE(font);
    FT_Memory memory = FT_FACE_MEMORY( face );
    OTL_Tag_List tag_list;
    if ( FT_NEW(tag_list) )
      return NULL;

    tag_list->request = request;
    tag_list->tags 	       = ot_info_list_features( font->info,
							table_type,
							0,
							script_index,
							language_index );
    return tag_list;
  }

  FT_EXPORT ( FT_Error )
  OTL_Tag_List_Done                  ( OTL_Tag_List taglist )
  {
    FTL_FeaturesRequest request = (FTL_FeaturesRequest)taglist->request;
    FTL_Font font = FTL_FEATURES_REQUEST_FONT( request );
    FT_Face face  = FTL_FONT_FACE(font);
    FT_Memory memory = FT_FACE_MEMORY( face );
    FT_FREE ( taglist );
    return FT_Err_Ok;
  }

  FT_EXPORT ( FT_Error )
  OTL_FeaturesRequest_Add_Feature  ( OTL_FeaturesRequest request,
				     OTTableType         table_type,
				     FT_UInt             feature_index,
				     FT_ULong            property_bit)
  {
    ot_ruleset_add_feature( request->ruleset,
			    table_type,
			    feature_index,
			    property_bit );
    return FT_Err_Ok;
  }

/* END */
