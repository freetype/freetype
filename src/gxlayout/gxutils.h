/***************************************************************************/
/*                                                                         */
/*  gxutils.h                                                              */
/*                                                                         */
/*    AAT/TrueTypeGX private utility functions (specification).            */
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

#ifndef __GXUTILS_H__
#define __GXUTILS_H__ 

#include <ft2build.h>
#include FT_TYPES_H
#include FT_SFNT_NAMES_H
#include FT_LAYOUT_H

FT_BEGIN_HEADER

FT_LOCAL( FT_Int )    gx_zero_shift_bits      ( FT_ULong mask );
FT_LOCAL( FT_Long )   gx_mask_zero_shift      ( FT_ULong value, 
						FT_ULong mask );
FT_LOCAL( FT_Long )   gx_sign_extend          ( FT_ULong value, 
						FT_ULong mask );

FT_LOCAL( FT_Error )  gx_get_name_from_id     ( FT_Face  face,
						FT_UShort name_id,
						FT_UShort platform_id,
						FT_UShort encoding_id,
						FT_UShort language_id,
						FT_SfntName  *aname );

FT_LOCAL( FT_UShort ) gx_feat_setting_on      ( FT_UShort setting );
FT_LOCAL( FT_UShort ) gx_feat_setting_off     ( FT_UShort setting );
FT_LOCAL( FT_Bool )   gx_feat_setting_state   ( FT_UShort setting );

FT_LOCAL ( void )     gx_glyphs_array_reverse ( FTL_Glyph glyphs, 
						FT_ULong length );

#ifdef FT_DEBUG_LEVEL_ERROR

#define GX_ASSERT_NOT_REACHED()                                     \
          do                                                        \
          {                                                         \
              FT_Panic( "should not be reached to line %d of file %s\n", \
                        __LINE__, __FILE__ );                       \
          } while ( 0 )

#else /* !FT_DEBUG_LEVEL_ERROR */

#define GX_ASSERT_NOT_REACHED()  do ; while ( 0 )

#endif /* !FT_DEBUG_LEVEL_ERROR */

FT_END_HEADER

#endif /* Not def: __GXUTILS_H__ */


/* END */
