/***************************************************************************/
/*                                                                         */
/*  svlayout.h                                                             */
/*                                                                         */
/*    The FreeType Layout services (specification).                        */
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

#ifndef __SVLAYOUT_H__
#define __SVLAYOUT_H__

#include FT_INTERNAL_SERVICE_H
#include FT_LAYOUT_H
#include FT_INTERNAL_FTL_TYPES_H

FT_BEGIN_HEADER

#define FT_SERVICE_ID_LAYOUT  "layout"

  typedef FT_Error
  (*FTL_Get_Font_Func)( FT_Face face,
			FTL_Font* font );

  typedef FTL_EngineType
  (*FTL_Get_EngineType_Func) ( FT_Face face );
			       
  typedef FT_Error
  (*FTL_New_FeaturesRequest_Func)( FT_Face face,
				   FTL_FeaturesRequest* request );
  typedef FT_Error
  (*FTL_Done_FeaturesRequest_Func)( FTL_FeaturesRequest request );

  typedef FT_Error
  (*FTL_Copy_FeaturesRequest_Func)( FTL_FeaturesRequest from,
				    FTL_FeaturesRequest to );

  typedef FT_UShort
  (*FTL_Get_LigatureCaret_Count_Func)  ( FT_Face face, FT_UShort glyphID );
  
  typedef FT_UShort
  (*FTL_Get_LigatureCaret_Division_Func) ( FT_Face face, 
					   FT_UShort glyphID, 
					   FT_UShort nth );
  typedef FT_Error
  (*FTL_Substitute_Glyphs_Func) ( FT_Face face,
				  FTL_FeaturesRequest request,
				  FTL_GlyphArray in,
				  FTL_GlyphArray out );

  FT_DEFINE_SERVICE( Layout )
  {
    FTL_Get_Font_Func                   get_font;
    FTL_Get_EngineType_Func             get_engine_type;
    FTL_New_FeaturesRequest_Func        new_features_request;
    FTL_Done_FeaturesRequest_Func       done_features_request;
    FTL_Copy_FeaturesRequest_Func       copy_features_request;
    FTL_Get_LigatureCaret_Count_Func    get_ligature_caret_count;
    FTL_Get_LigatureCaret_Division_Func get_ligature_caret_division;
    FTL_Substitute_Glyphs_Func          substitute_glyphs;
  };

  /* */


FT_END_HEADER

#endif /* __SVLAYOUT_H__ */


/* END */
