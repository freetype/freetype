/***************************************************************************/
/*                                                                         */
/*  gxvm.h                                                                 */
/*                                                                         */
/*    AAT/TrueTypeGX glyph substitution automaton (specification).         */
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

#ifndef __GX_VM_H__
#define __GX_VM_H__ 

#include <ft2build.h>
#include FT_FREETYPE_H
#include "gxtypes.h"

FT_BEGIN_HEADER

/*
 * Mort
 */
FT_LOCAL( FT_Error )
gx_rearrangement_subst ( GX_MetamorphosisRearrangementBody body,
			 GXL_Initial_State initial_state,
			 FTL_Glyphs_Array garray );

FT_LOCAL( FT_Error )
gx_contextual_subst( GX_MetamorphosisContextualBody body,
		     GXL_Initial_State initial_state,
		     FTL_Glyphs_Array garray );

FT_LOCAL( FT_Error )
gx_ligature_subst( GX_MetamorphosisLigatureBody body,
		   GXL_Initial_State initial_state,
		   FTL_Glyphs_Array garray );

FT_LOCAL( FT_Error )
gx_noncontextual_subst( GX_MetamorphosisNoncontextualBody body,
			FTL_Glyphs_Array garray );

FT_LOCAL( FT_Error )
gx_insertion_subst( GX_MetamorphosisInsertionBody body,
		    GXL_Initial_State initial_state,
		    FTL_Glyphs_Array garray );

/*
 * Morx
 */
#define gx_xnoncontextual_subst gx_noncontextual_subst
FT_LOCAL( FT_Error )
gx_xcontextual_subst( GX_XMetamorphosisContextualBody body,
		      GXL_Initial_State initial_state,
		      FTL_Glyphs_Array garray );

FT_LOCAL( FT_Error )
gx_xligature_subst( GX_XMetamorphosisLigatureBody body,
		    GXL_Initial_State initial_state,
		    FTL_Glyphs_Array garray );

FT_LOCAL( FT_Error )
gx_xinsertion_subst( GX_XMetamorphosisInsertionBody body,
		     GXL_Initial_State initial_state,
		     FTL_Glyphs_Array garray );

FT_LOCAL( FT_Error )
gx_xrearrangement_subst ( GX_XMetamorphosisRearrangementBody body,
			  GXL_Initial_State initial_state,
			  FTL_Glyphs_Array garray );

/*
 * Kern
 */
FT_LOCAL( FT_Error )
gx_contextual_kerning_calc ( GX_KerningSubtableFormat1Body kern_fmt1,
			     FTL_Glyphs_Array garray,
			     FTL_Direction dir,
			     FT_Bool cross_stream,
			     GXL_Initial_State initial_state,
			     FT_Vector * kerning );

FT_END_HEADER

#endif /* Not def: __GX_VM_H__ */


/* END */
