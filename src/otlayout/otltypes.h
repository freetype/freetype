/***************************************************************************/
/*                                                                         */
/*  otltypes.h                                                             */
/*                                                                         */
/*    Implicit data types of OpenType based layout engine                  */
/*    (For application developers but using implicitly,                    */
/*     specification only).                                                */
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

#ifndef __OTLTYPES_H__
#define __OTLTYPES_H__ 

#include <ft2build.h>
#include FT_INTERNAL_TRUETYPE_TYPES_H
#include FT_INTERNAL_FTL_TYPES_H
#include FT_OTLAYOUT_H
#include "ot-ruleset.h"
#include "ot-info.h"

FT_BEGIN_HEADER

  typedef TT_Face OT_Face;

  typedef struct OTL_FontRec_
  {
    FTL_FontRec root;
    OTInfo *    info;
  } OTL_FontRec, *OTL_Font;

  typedef struct OTL_FeaturesRequestRec_
  {
    FTL_FeaturesRequestRec root;
    OTRuleset             *ruleset;
  } OTL_FeaturesRequestRec;

  FT_LOCAL ( FT_Error )
  otl_get_font ( FT_Face face, FTL_Font * font);

  FT_LOCAL_DEF ( FTL_EngineType )
  otl_get_engine_type ( FT_Face face );

  FT_LOCAL ( FT_Error ) 
  otl_new_features_request( FT_Face face, FTL_FeaturesRequest * request);

  FT_LOCAL ( FT_Error ) 
  otl_done_features_request( FTL_FeaturesRequest request);

  FT_LOCAL ( FT_Error ) 
  otl_copy_features_request( FTL_FeaturesRequest from,
			     FTL_FeaturesRequest to);

  FT_LOCAL ( FT_Error )
  otl_substitute_glyphs ( FT_Face face,
			  FTL_FeaturesRequest request,
			  FTL_Glyphs_Array in,
			  FTL_Glyphs_Array out );
FT_END_HEADER

#endif /* Not def: __OTLTYPES_H__ */


/* END */
