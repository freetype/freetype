/* OTInfo
 *
 * Copyright (C) 2000 Red Hat Software
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __OT_INFO_H__
#define __OT_INFO_H__

#include "ftxopen.h"
#include "ot-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct _OTInfo
{
	FT_Int refcount;

	FT_UInt loaded;
	
	FT_Face face;

	TTO_GSUB gsub;
	TTO_GDEF gdef;
	TTO_GPOS gpos;

	OTRuleset *ruleset;
};

/* Don't use these directly */

OTInfo *ot_info_new ( FT_Face face );
OTInfo *ot_info_delete (OTInfo *info);

OTInfo *ot_info_ref (OTInfo *info);
OTInfo *ot_info_unref (OTInfo *info);

OTInfo *ot_info_setup (OTInfo *info);
OTInfo *ot_info_release (OTInfo *info);

/* Member access */

TTO_GDEF ot_info_get_gdef (OTInfo *info);
TTO_GSUB ot_info_get_gsub (OTInfo *info);
TTO_GPOS ot_info_get_gpos (OTInfo *info);

OTRuleset *ot_info_get_ruleset (OTInfo *info);

OTInfo *ot_info_get (FT_Face face);

FT_Bool ot_info_find_script   (OTInfo      *info,
				OTTableType  table_type,
				OTTag        script_tag,
				FT_UInt            *script_index);
FT_Bool ot_info_find_language (OTInfo      *info,
				OTTableType  table_type,
				FT_UInt             script_index,
				OTTag        language_tag,
				FT_UInt            *language_index,
				FT_UInt            *required_feature_index);
FT_Bool ot_info_find_feature  (OTInfo      *info,
				OTTableType  table_type,
				OTTag        feature_tag,
				FT_UInt             script_index,
				FT_UInt             language_index,
				FT_UInt            *feature_index);

OTTag *ot_info_list_scripts   (OTInfo      *info,
			       OTTableType  table_type);
OTTag *ot_info_list_languages (OTInfo      *info,
			       OTTableType  table_type,
			       FT_UInt             script_index,
			       OTTag        language_tag);
OTTag *ot_info_list_features  (OTInfo      *info,
			       OTTableType  table_type,
			       OTTag        tag,
			       FT_UInt             script_index,
			       FT_UInt             language_index);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __OT_INFO_H__ */
