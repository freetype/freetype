/***************************************************************************/
/*                                                                         */
/*  gxaccess.h                                                             */
/*                                                                         */
/*    AAT/TrueTypeGX private data accessor (specification only).           */
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

#ifndef __GXACCESS_H__
#define __GXACCESS_H__ 

#include <ft2build.h>
#include FT_TYPES_H

#include "gxltypes.h"
#include "gxtypes.h"

FT_BEGIN_HEADER

/* feat */
  FT_LOCAL ( FT_Bool )  
  gx_feat_has_feature_type ( GX_Feat feat, 
			     FT_UShort feature_type );

/* prop */
  FT_LOCAL ( FT_UShort ) 
  gx_prop_get( GX_Prop   prop, 
	       FT_Long   glyph );

/* lcar */
  FT_LOCAL( GX_LigCaretClassEntry ) 
  gx_lcar_get ( GX_Lcar lcar, 
		FT_UShort glyphID );

/* mort */
  typedef FT_Error (* GX_Mort_Feature_Func)( GX_MetamorphosisFeatureTable feat_Subtbl, 
					     FT_Pointer user );
  FT_LOCAL( FT_Error )  gx_mort_foreach_feature ( GX_Mort mort, 
						  GX_Mort_Feature_Func func, 
						  FT_Pointer user );
  FT_LOCAL( FT_UShort ) 
  gx_mort_count_feat_not_in_feat ( GX_Mort mort, 
				   GX_Feat feat );
  FT_LOCAL( FT_Error ) 
  gx_mort_substitute_glyph ( GX_Mort mort,
			     GXL_FeaturesRequest request,
			     FTL_GlyphArray in,
			     FTL_GlyphArray out );

/* morx */
  typedef FT_Error (* GX_Morx_Feature_Func)( GX_XMetamorphosisFeatureTable feat_Subtbl, 
					     FT_Pointer user );
  FT_LOCAL( FT_Error )
  gx_morx_foreach_feature ( GX_Morx morx, 
			    GX_Morx_Feature_Func func, 
			    FT_Pointer user );

  FT_LOCAL( FT_UShort )
  gx_morx_count_feat_not_in_feat ( GX_Morx morx, 
				   GX_Feat feat );
  FT_LOCAL( FT_Error )
  gx_morx_substitute_glyph ( GX_Morx morx,
			     GXL_FeaturesRequest request,
			     FTL_GlyphArray in,
			     FTL_GlyphArray out );

/* kern */
  FT_LOCAL( FT_Error ) gx_kern_get_pair_kerning ( GX_Kern kern, 
						  FT_UShort     left_glyph, 
						  FT_UShort     right_glyph,
						  FTL_Direction dir,
						  FT_Vector*  kerning );

  FT_LOCAL( FT_Error ) gx_kern_get_contextual_kerning( GX_Kern kern,
						       FTL_GlyphArray garray,
						       FTL_Direction dir,
						       GXL_Initial_State initial_state,
						       FT_Vector * kerning );

/* trak */
  FT_LOCAL( FT_Error ) gx_trak_get( GX_Trak   trak,
				    FT_Fixed  track,
				    FT_Fixed  size,
				    FTL_Direction dir,
				    FT_FWord* value );

  FT_LOCAL( FT_UShort ) gx_trak_count_name_index( GX_Trak trak );
  FT_LOCAL( FT_Error )  gx_trak_get_name ( GX_Trak trak,
					   FT_UShort index,
					   FT_UShort     * name_index,
					   FTL_Direction * dir,
					   FT_Fixed      * track );
FT_END_HEADER

#endif /* Not def: __GXACCESS_H__ */


/* END */
