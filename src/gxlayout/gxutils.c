/***************************************************************************/
/*                                                                         */
/*  gxutils.c                                                              */
/*                                                                         */
/*    AAT/TrueTypeGX private utility functions (body).                     */
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

#include <ft2build.h>

#include "gxerrors.h"
#include "gxutils.h"
#include "gxobjs.h"


FT_LOCAL_DEF( FT_Int )
gx_zero_shift_bits ( FT_ULong mask )
{
  FT_Int i      = 0;
  for ( i = 0; !(mask & (0x1L << i)); i++)
    ;				/* Do nothing */
  return i;
}

FT_LOCAL_DEF( FT_Long )
gx_mask_zero_shift ( FT_ULong value, FT_ULong mask )
{
  FT_Int shift_bits = gx_zero_shift_bits( mask );
  return ((value & mask) >> shift_bits);
}

FT_LOCAL_DEF( FT_Long )
gx_sign_extend     ( FT_ULong value, FT_ULong mask )
{
  /* Copied from icu/source/layout/LigatureSubstProc.cpp */
#define ExtendedComplement(m) ((FT_Long) (~((FT_ULong) (m))))
#define SignBit(m) ((ExtendedComplement(m) >> 1) & (m))
#define SignExtend(v,m) (((v) & SignBit(m))? ((v) | ExtendedComplement(m)): (v))
  return SignExtend(value,mask);
}


/* Generic */
FT_LOCAL_DEF( FT_Error )
gx_get_name_from_id ( FT_Face  face,
		      FT_UShort name_id,
		      FT_UShort platform_id,
		      FT_UShort encoding_id,
		      FT_UShort language_id,
		      FT_SfntName  *aname )
{
  FT_Int i;
  FT_UInt nnames = FT_Get_Sfnt_Name_Count ((FT_Face)face);
  FT_Bool any_id = ( (platform_id == 0) &&
		     (encoding_id == 0) &&
		     (language_id == 0)   )? 1: 0;
  for ( i = 0; i < nnames; i++ )
    {
      if ( FT_Get_Sfnt_Name((FT_Face)face, i, aname) )
	continue ;
      if ( ( aname->name_id == name_id )         &&
	   ( any_id || 
	     ( ( aname->platform_id == platform_id ) &&
	       ( aname->encoding_id == encoding_id ) && 
	       ( aname->language_id == language_id )    )))
	return GX_Err_Ok;
    }
  return GX_Err_Invalid_Argument;
}


FT_LOCAL_DEF ( FT_UShort )
gx_feat_setting_on  ( FT_UShort setting )
{
  if ( !gx_feat_setting_state ( setting ) )
    return setting -1;
  else
    return setting;

}

FT_LOCAL_DEF ( FT_UShort )
gx_feat_setting_off ( FT_UShort setting )
{
  if ( gx_feat_setting_state ( setting ) )
    return setting + 1;
  else
    return setting;
}

FT_LOCAL_DEF ( FT_Bool )
gx_feat_setting_state  ( FT_UShort setting )
{				
  /* odd => off */
  if ( setting % 2 )
    return 0;			/* odd => off */
  else
    return 1;			/* even => on  */
}

FT_LOCAL_DEF ( void )
gx_glyphs_array_reverse( FTL_Glyph glyphs, FT_ULong length )
{
  FT_ULong i, j;
  FTL_GlyphRec tmp;
  for ( i = 0, j = length -1; i < j; i++, j-- )
    {
      tmp 	= glyphs[i];
      glyphs[i] = glyphs[j];
      glyphs[j] = tmp;
    }
}


/* END */
