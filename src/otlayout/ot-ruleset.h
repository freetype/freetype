/*
 * ot-ruleset.h: Shaping using OpenType features
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

#ifndef __OT_RULESET_H__
#define __OT_RULESET_H__

#include "ftxopen.h"
#include "ot-types.h"
#include "ot-array.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _OTRuleset
{
	OTArray *rules;
	OTInfo *info;
};

/* Dynamic lifecycle */

OTRuleset *ot_ruleset_new (OTInfo *info);
OTRuleset *ot_ruleset_delete (OTRuleset *ruleset);

/* Automatic lifecycle */

OTRuleset *ot_ruleset_setup (OTRuleset *ruleset, OTInfo *info);
OTRuleset *ot_ruleset_release (OTRuleset *ruleset);

void ot_ruleset_copy ( OTRuleset * from, OTRuleset * to );

void ot_ruleset_add_feature (OTRuleset   *ruleset,
			     OTTableType  table_type,
			     FT_UInt             feature_index,
			     FT_ULong            property_bit);
OTGlyphString *ot_ruleset_shape ( OTRuleset     *ruleset,
				  OTGlyphString *glyphs );


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __OT_RULESET_H__ */
