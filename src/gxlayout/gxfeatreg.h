/***************************************************************************/
/*                                                                         */
/*  gxfeatreg.h                                                            */
/*                                                                         */
/*    Database of font features pre-defined by Apple Computer, Inc.        */
/*    http://developer.apple.com/fonts/Registry/                           */
/*    (specification only).                                                */
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

#ifndef __GXFEATREG_H__
#define __GXFEATREG_H__ 

#include <ft2build.h>
#include FT_FREETYPE_H

FT_BEGIN_HEADER

typedef struct GX_Feature_RegistryRec_ *GX_Feature_Registry;

FT_LOCAL( GX_Feature_Registry ) gx_get_feature_registry( FT_UShort feature_type );
FT_LOCAL( FT_UShort )           gx_feature_registry_get_value(GX_Feature_Registry featreg);
FT_LOCAL( const FT_String * )   gx_feature_registry_get_name( GX_Feature_Registry featreg );
FT_LOCAL( FT_Bool )             gx_feature_registry_is_setting_exclusive( GX_Feature_Registry featreg );
FT_LOCAL( const FT_String * )   gx_feature_registry_get_setting_name( GX_Feature_Registry featreg, FT_UShort nth );
FT_LOCAL( FT_UShort )           gx_feature_registry_count_setting ( GX_Feature_Registry featreg );

FT_END_HEADER
#endif /* Not def: __GXFEATREG_H__ */

/* END */
