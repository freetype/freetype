/* 
 * ot-ruleset.c: Shaping using OpenType features
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

#include "ot-ruleset.h"
#include "ot-info.h"

#define noRULESET_DEBUG_MEMORY

#include FT_INTERNAL_MEMORY_H	/* For FT_Free() */

#define OT_SCALE_26_6 (OT_SCALE / (1<<6))
#define OT_UNITS_26_6(d) (OT_SCALE_26_6 * (d))

typedef struct _OTRule OTRule;

struct _OTRule {
	FT_ULong     property_bit;
	FT_UShort  feature_index;
	FT_UInt      table_type : 1;
};

OTRuleset *
ot_ruleset_new (OTInfo *info)
{
	OTRuleset *ruleset;

	ruleset = malloc (sizeof (OTRuleset));

	ot_ruleset_setup (ruleset, info);

	return ruleset;
}

OTRuleset *
ot_ruleset_delete (OTRuleset *ruleset)
{
	if (ruleset) {
		ot_ruleset_release (ruleset);
		free (ruleset);
	}

	return NULL;
}

#ifdef RULESET_DEBUG_MEMORY
static ot_ruleset_count = 0;
#endif

OTRuleset *
ot_ruleset_setup (OTRuleset *ruleset, OTInfo *info)
{
        FT_Memory memory = FT_FACE_MEMORY( info->face );
	FT_ASSERT (ruleset != NULL);

#ifdef RULESET_DEBUG_MEMORY
	ot_ruleset_count++;
	FT_Message("ot_ruleset_setup: %d\n", ot_ruleset_count);
#endif
	ruleset->info = info;

	ruleset->rules = OT_Array_New ( sizeof (OTRule), memory );

	return ruleset;
}

OTRuleset *
ot_ruleset_release (OTRuleset *ruleset)
{
#ifdef RULESET_DEBUG_MEMORY
	ot_ruleset_count--;
	FT_Message("ot_ruleset_release: %d\n", ot_ruleset_count);
#endif
	if (ruleset->rules) {
		OT_Array_Free ( ruleset->rules );
		ruleset->rules = NULL;
	}

	if (ruleset->info) {
		ruleset->info = NULL;
	}

	return NULL;
}

void
ot_ruleset_copy ( OTRuleset * from, OTRuleset * to )
{
  OTArray *from_array;
  OTArray *to_array;
  FT_UInt from_length;
  FT_UInt i;
  
  to->info    = from->info;
  from_array  = from->rules;
  from_length = from_array->length;
  to_array    = to->rules;
  OT_Array_Set_Size( to_array, from_length );
  for ( i = 0; i < from_length; i++ )
    OT_Array_Index(to_array, OTRule, i) = OT_Array_Index(from_array, OTRule, i);
}

/**
 * ot_ruleset_add_feature:
 * @ruleset: a #OTRuleset.
 * @table_type: the table type to add a feature to.
 * @feature_index: the index of the feature to add.
 * @property_bit: the property bit to use for this feature. 
 *
 * Adds a feature to the ruleset. See ot_ruleset_shape()
 * for an explanation of @property_bit.
 **/
void
ot_ruleset_add_feature (OTRuleset *ruleset, OTTableType  table_type, FT_UInt feature_index, FT_ULong property_bit)
{
	OTRule tmp_rule;
	
	FT_ASSERT (ruleset != NULL);
	
	tmp_rule.table_type = table_type;
	tmp_rule.feature_index = feature_index;
	tmp_rule.property_bit = property_bit;
	
	OT_Array_Append_Val (ruleset->rules, &tmp_rule);
}

/**
 * ot_ruleset_shape:
 * @ruleset: a #OTRuleset.
 * @glyphs: a pointer to a #OTGlyphString.
 *
 * Shapes a string of glyphs with the given properties according to @ruleset.
 **/
OTGlyphString *
ot_ruleset_shape ( OTRuleset *ruleset, OTGlyphString *glyphs )
{    
        FT_Error error;
	int i;
	int last_cluster;
	int result;
	
	TTO_GSUB gsub = NULL;
	TTO_GPOS gpos = NULL;
	
	TTO_GSUB_String *in_string = NULL;
	TTO_GSUB_String *out_string = NULL;
	TTO_GSUB_String *result_string = NULL;

	FT_Bool need_gsub = 0;
	FT_Bool need_gpos = 0;
	
	FT_ASSERT (ruleset != NULL);
	
	for (i = 0; i < ruleset->rules->length; i++)
	{
		OTRule *rule = &OT_Array_Index (ruleset->rules, OTRule, i);
		
		if (rule->table_type == OT_TABLE_GSUB)
			need_gsub = 1;
		else 
			need_gpos = 1;
	}
	
	if (need_gsub)
	{
		gsub = ot_info_get_gsub (ruleset->info);
		
		if (gsub)
			TT_GSUB_Clear_Features (gsub);
	}
	
	if (need_gpos)
	{
		gpos = ot_info_get_gpos (ruleset->info);
		
		if (gpos)
			TT_GPOS_Clear_Features (gpos);
	}
	
	for (i = 0; i < ruleset->rules->length; i++)
	{
		OTRule *rule = &OT_Array_Index (ruleset->rules, OTRule, i);
		
		if (rule->table_type == OT_TABLE_GSUB)
		{
			if (gsub)
				TT_GSUB_Add_Feature (gsub, rule->feature_index, rule->property_bit);
		}
		else
		{
			if (gpos)
				TT_GPOS_Add_Feature (gpos, rule->feature_index, rule->property_bit);
		}
	}
	
	if (!gsub && !gpos)
		return glyphs;
	
	result = TT_GSUB_String_New (ruleset->info->face->memory, &in_string);
	FT_ASSERT (result == FT_Err_Ok);
	
	result = TT_GSUB_String_Set_Length (in_string, glyphs->length);
	FT_ASSERT (result == FT_Err_Ok);
	
	for (i = 0; i < glyphs->length; i++)
	{
		in_string->string[i] = glyphs->glyphs[i].gid;
		in_string->properties[i] = glyphs->glyphs[i].ot_prop;
#if 0
		in_string->logClusters[i] = glyphs->log_clusters[i];
#endif
	}
	in_string->max_ligID = i;
	
	if (gsub)
	{
		result = TT_GSUB_String_New (ruleset->info->face->memory,
					     &out_string);
		FT_ASSERT (result == FT_Err_Ok);
		result_string = out_string;
		
		TT_GSUB_Apply_String (gsub, in_string, out_string);
	}
	else
		result_string = in_string;

#if 0 /* TODO: implement this using nr-glyphs */
	if (gpos)
	{
		TTO_GPOS_Data *outgpos = NULL;
		
		if (!TT_GPOS_Apply_String (ruleset->info->face, gpos, 0, result_string, &outgpos,
					   FALSE /* enable device-dependant values */,
					   FALSE /* Even though this might be r2l text, RTL is handled elsewhere */))
		{
			for (i = 0; i < result_string->length; i++)
			{
				FT_Pos x_pos = outgpos[i].x_pos;
				FT_Pos y_pos = outgpos[i].y_pos;
				int back = i;
				int j;
				
				while (outgpos[back].back != 0)
				{
					back  -= outgpos[back].back;
					x_pos += outgpos[back].x_pos;
					y_pos += outgpos[back].y_pos;
				}
				
				for (j = back; j < i; j++)
					glyphs->glyphs[i].geometry.x_offset -= glyphs->glyphs[j].geometry.width;
				
				glyphs->glyphs[i].geometry.x_offset += OT_UNITS_26_6(x_pos);
				glyphs->glyphs[i].geometry.y_offset += OT_UNITS_26_6(y_pos);
				
				if (outgpos[i].new_advance)
					glyphs->glyphs[i].geometry.width  = OT_UNITS_26_6(outgpos[i].x_advance);
				else
					glyphs->glyphs[i].geometry.width += OT_UNITS_26_6(outgpos[i].x_advance);
			}
			
			FT_Free(gpos->memory, (void *)outgpos);
		}
	}
#endif

	if (glyphs->length != result_string->length)
	  {
	    if (( error = FTL_Set_Glyphs_Array_Length( glyphs, result_string->length ) ))
	      return glyphs;
	  }
	    
	last_cluster = -1;
	for (i = 0; i < result_string->length; i++)
	{
		glyphs->glyphs[i].gid = result_string->string[i];
		/* TODO: Other fields */
#if 0 /* TODO: */
		glyphs->log_clusters[i] = result_string->logClusters[i];
		if (glyphs->log_clusters[i] != last_cluster)
			glyphs->glyphs[i].attr.is_cluster_start = 1;
		else
			glyphs->glyphs[i].attr.is_cluster_start = 0;
		
		last_cluster = glyphs->log_clusters[i];
#endif
	}
	
	if (in_string)
		TT_GSUB_String_Done (in_string);
	if (out_string)
		TT_GSUB_String_Done (out_string);

	return glyphs;
}
