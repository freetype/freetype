/* 
 * ot-info.c: Store tables for OpenType
 *
 * Copyright (C) 2003 Red Hat K.K.
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

#include "ot-info.h"
#include "ot-ruleset.h"
#include "ot-unicode.h"
#include "fterrcompat.h"
#include FT_INTERNAL_OBJECTS_H
#include FT_MODULE_H


#define noINFO_DEBUG_MEMORY

enum
{
	INFO_LOADED_GDEF = 1 << 0,
	INFO_LOADED_GSUB = 1 << 1,
	INFO_LOADED_GPOS = 1 << 2
};


static FT_Error ot_table_check( FT_Face face );
OTInfo *
ot_info_new ( FT_Face face )
{
        FT_Error error;
	OTInfo *info;
	FT_Memory memory;

	if ( ot_table_check( face ) ) 
		return NULL;

	memory = FT_FACE_MEMORY( face );
	if ( FT_NEW ( info ) )
		return NULL;

	ot_info_setup (info);

	return info;
}

static FT_Error
ot_table_check( FT_Face face )
{
  FT_Stream        stream = face->stream;
  TT_Face          tt_face = (TT_Face)face;
  
  FT_Error         error_gdef, error_gpos, error_gsub;
  
  error_gdef = tt_face->goto_table( tt_face, TTAG_GDEF, stream, 0 );
  error_gsub = tt_face->goto_table( tt_face, TTAG_GSUB, stream, 0 );
  error_gpos = tt_face->goto_table( tt_face, TTAG_GPOS, stream, 0 );
  
  if ( (!error_gdef) || (!error_gsub) || (!error_gpos) )
    return FT_Err_Ok;
  else
    return OT_Err_Unknown_File_Format;
}

OTInfo *
ot_info_delete (OTInfo *info)
{
        FT_Memory memory = FT_FACE_MEMORY( info->face );
	if (info) {
		ot_info_release (info);
		FT_FREE( info );
	}

	return NULL;
}

OTInfo *
ot_info_ref (OTInfo *info)
{
        FT_ASSERT (info != NULL );
        FT_ASSERT (info->refcount > 0);

	info->refcount += 1;

	return info;
}

OTInfo *
ot_info_unref (OTInfo *info)
{
        FT_ASSERT (info != NULL);
        FT_ASSERT (info->refcount > 0);

	info->refcount -= 1;

	if (info->refcount == 0) {
		ot_info_delete (info);
		return NULL;
	}

	return info;
}

#ifdef INFO_DEBUG_MEMORY
static int ot_info_count = 0;
#endif

OTInfo *
ot_info_setup (OTInfo *info)
{
  
#ifdef INFO_DEBUG_MEMORY
	ot_info_count++;
	FT_Message ("ot_info_setup: %d\n", ot_info_count);
#endif	
	info->refcount = 1;

	info->gsub = NULL;
	info->gdef = NULL;
	info->gpos = NULL;
	info->ruleset = NULL;

	return info;
}

OTInfo *
ot_info_release (OTInfo *info)
{
	FT_ASSERT (info != NULL);
	
#ifdef INFO_DEBUG_MEMORY
	ot_info_count--;
	FT_Message ("ot_info_release: %d\n", ot_info_count);
#endif
	if (info->gdef)
	{
		TT_Done_GDEF_Table (info->gdef);
		info->gdef = NULL;
	}
	if (info->gsub)
	{
		TT_Done_GSUB_Table (info->gsub);
		info->gsub = NULL;
	}
	if (info->gpos)
	{
		TT_Done_GPOS_Table (info->gpos);
		info->gpos = NULL;
	}

	if (info->ruleset)
	{
		ot_ruleset_delete (info->ruleset);
		info->ruleset = NULL;
	}
	return info;
}

/* static int info_count = 0; */
void
ot_info_finalizer (void *object)
{
	FT_Face face = object;	
	OTInfo *info = face->generic.data;

	ot_info_unref (info);
	info->face = NULL;
}

/**
 * ot_info_get_ruleset
 *
 * @returns: the #OTRulset for @info. This object will
 *   have the same lifetime as OTInfo.
 * 
 * Returns the #OTRuleset structure for the info.
 */
OTRuleset *
ot_info_get_ruleset (OTInfo *info)
{
	if (! info->ruleset) {
		info->ruleset = ot_ruleset_new (info);
	}

	return info->ruleset;
}

/**
 * ot_info_get:
 * @face: a #FT_Face.
 * @returns: the #OTInfo for @face. This object will
 *   have the same lifetime as FT_Face.
 * 
 * Returns the #OTInfo structure for the given FreeType font.
 *
 * Since: 1.2
 **/
OTInfo *
ot_info_get (FT_Face face)
{
	OTInfo *info;
	
	if (face->generic.data)
		return face->generic.data;
	else
	{
		info = ot_info_new ( face );
		if ( info ) {
			face->generic.data = info;
			face->generic.finalizer = ot_info_finalizer;
			info->face = face;
		}
	}
	
	return info;
}

/* There must be be a better way to do this
 */
static FT_Bool
is_truetype (FT_Face face)
{
	return ( 
		(strcmp (FT_MODULE_CLASS (face->driver)->module_name, "truetype") == 0)
		|| (strcmp (FT_MODULE_CLASS (face->driver)->module_name, "ot") == 0)
		);
}

typedef struct _GlyphInfo GlyphInfo;

struct _GlyphInfo {
	FT_UShort glyph;
	FT_UShort class;
};

static int
compare_glyph_info (const void * a,
		    const void * b)
{
	const GlyphInfo *info_a = a;
	const GlyphInfo *info_b = b;
	
	return (info_a->glyph < info_b->glyph) ? -1 :
		(info_a->glyph == info_b->glyph) ? 0 : 1;
}

/* Make a guess at the appropriate class for a glyph given 
 * a character code that maps to the glyph
 */

static FT_Bool
set_unicode_charmap (FT_Face face)
{
	int charmap;
	
	for (charmap = 0; charmap < face->num_charmaps; charmap++)
		if (face->charmaps[charmap]->encoding == ft_encoding_unicode)
		{
			FT_Error error = FT_Set_Charmap(face, face->charmaps[charmap]);
			return error == FT_Err_Ok;
		}
	
	return FALSE;
}

/* Synthesize a GDEF table using the font's charmap and the
 * unicode property database. We'll fill in class definitions
 * for glyphs not in the charmap as we walk through the tables.
 */
static void
synthesize_class_def (OTInfo *info)
{
	OTArray *glyph_infos;
	FT_UShort *glyph_indices;
	FT_UShort *classes;
	FT_ULong charcode;
	FT_UInt glyph;
	int i, j;
	FT_CharMap old_charmap;
	FT_Error error;
	FT_Memory memory = FT_FACE_MEMORY( info->face );
	
	old_charmap = info->face->charmap;
	
	if (!old_charmap || !old_charmap->encoding != ft_encoding_unicode)
		if (!set_unicode_charmap (info->face))
			return;
	
	glyph_infos = OT_Array_New ( sizeof (GlyphInfo), memory );
	
	/* Collect all the glyphs in the charmap, and guess
	 * the appropriate classes for them
	 */
	charcode = FT_Get_First_Char (info->face, &glyph);
	while (glyph != 0)
	{
		GlyphInfo glyph_info;
		
		if (glyph <= 65535)
		{
			glyph_info.glyph = glyph;
			glyph_info.class = ot_get_glyph_class (charcode);
			
			OT_Array_Append_Val ( glyph_infos, &glyph_info );
		}
		
		charcode = FT_Get_Next_Char (info->face, charcode, &glyph);
	}
	
	/* Sort and remove duplicates
	 */
	OT_Array_Sort ( glyph_infos, compare_glyph_info );
	FT_ALLOC_ARRAY( glyph_indices, glyph_infos->length, FT_UShort );
	FT_ALLOC_ARRAY( classes, glyph_infos->length, FT_UShort );

	for (i = 0, j = 0; i < glyph_infos->length; i++)
	{
		GlyphInfo *info = &OT_Array_Index (glyph_infos, GlyphInfo, i);
		
		if (j == 0 || info->glyph != glyph_indices[j - 1])
		{
			glyph_indices[j] = info->glyph;
			classes[j] = info->class;
			
			j++;
		}
	}
	
	OT_Array_Free ( glyph_infos );
	
	TT_GDEF_Build_ClassDefinition (info->gdef, info->face->num_glyphs, j,
				       glyph_indices, classes);
	FT_FREE( glyph_indices );
	FT_FREE( classes );
	
	if (old_charmap && info->face->charmap != old_charmap)
		FT_Set_Charmap (info->face, old_charmap);
}

TTO_GDEF 
ot_info_get_gdef (OTInfo *info)
{
	FT_ASSERT (info != NULL);
	
	if (!(info->loaded & INFO_LOADED_GDEF))
	{
		FT_Error error;
		
		info->loaded |= INFO_LOADED_GDEF;
		
		if (is_truetype (info->face))
		{
			error = TT_Load_GDEF_Table (info->face, &info->gdef);
			
			if (error && error != TT_Err_Table_Missing)
			        FT_ERROR (("Error loading GDEF table %d", error));
			
			if (!info->gdef)
				error = TT_New_GDEF_Table (info->face, &info->gdef);
			
			if (info->gdef && !info->gdef->GlyphClassDef.loaded)
				synthesize_class_def (info);
		}
	}
	
	return info->gdef;
}

TTO_GSUB
ot_info_get_gsub (OTInfo *info)
{
	FT_ASSERT (info != NULL);
	
	if (!(info->loaded & INFO_LOADED_GSUB))
	{
		FT_Error error;
		TTO_GDEF gdef = ot_info_get_gdef (info);
		
		info->loaded |= INFO_LOADED_GSUB;
		
		if (is_truetype (info->face))
		{
			error = TT_Load_GSUB_Table (info->face, &info->gsub, gdef);
			
			if (error && error != TT_Err_Table_Missing)
				FT_ERROR (("Error loading GSUB table %d", error));
		}
	}
	
	return info->gsub;
}

TTO_GPOS
ot_info_get_gpos (OTInfo *info)
{
	FT_ASSERT (info != NULL);
	
	if (!(info->loaded & INFO_LOADED_GPOS))
	{
		FT_Error error;
		TTO_GDEF gdef = ot_info_get_gdef (info);
		
		info->loaded |= INFO_LOADED_GPOS;
		
		if (is_truetype (info->face))
		{
			error = TT_Load_GPOS_Table (info->face, &info->gpos, gdef);
			
			if (error && error != TT_Err_Table_Missing)
				FT_ERROR (("Error loading GPOS table %d", error));
		}
	}
	
	return info->gpos;
}

static FT_Bool
get_tables (OTInfo      *info,
	    OTTableType  table_type,
	    TTO_ScriptList  **script_list,
	    TTO_FeatureList **feature_list)
{
	if (table_type == OT_TABLE_GSUB)
	{
		TTO_GSUB gsub = ot_info_get_gsub (info);
		
		if (!gsub)
			return FALSE;
		else
		{
			if (script_list)
				*script_list = &gsub->ScriptList;
			if (feature_list)
				*feature_list = &gsub->FeatureList;
			return TRUE;
		}
	}
	else
	{
		TTO_GPOS gpos = ot_info_get_gpos (info);
		
		if (!gpos)
			return FALSE;
		else
		{
			if (script_list)
				*script_list = &gpos->ScriptList;
			if (feature_list)
				*feature_list = &gpos->FeatureList;
			return TRUE;
		}
	}
}

/**
 * ot_info_find_script:
 * @info: a #OTInfo.
 * @table_type: the table type to obtain information about.
 * @script_tag: the tag of the script to find.
 * @script_index: location to store the index of the script, or %NULL.
 * @returns: %TRUE if the script was found.
 * 
 * Finds the index of a script.
 **/
FT_Bool 
ot_info_find_script (OTInfo      *info,
		     OTTableType  table_type,
		     OTTag        script_tag,
		     FT_UInt            *script_index)
{
	TTO_ScriptList *script_list;
	int i;
	
	FT_ASSERT (info != NULL);
	
	if (!get_tables (info, table_type, &script_list, NULL))
		return FALSE;
	
	for (i=0; i < script_list->ScriptCount; i++)
	{
		if (script_list->ScriptRecord[i].ScriptTag == script_tag)
		{
			if (script_index)
				*script_index = i;
			
			return TRUE;
		}
	}
	
	return FALSE;
}

/**
 * ot_info_find_language:
 * @info: a #OTInfo.
 * @table_type: the table type to obtain information about.
 * @script_index: the index of the script whose languages are searched.
 * @language_tag: the tag of the language to find.
 * @language_index: location to store the index of the language, or %NULL.
 * @required_feature_index: location to store the required feature index of 
 *    the language, or %NULL.
 * @returns: %TRUE if the language was found.
 * 
 * Finds the index of a language and its required feature index.  
 **/
FT_Bool
ot_info_find_language (OTInfo      *info,
		       OTTableType  table_type,
		       FT_UInt             script_index,
		       OTTag        language_tag,
		       FT_UInt            *language_index,
		       FT_UInt            *required_feature_index)
{
	TTO_ScriptList *script_list;
	TTO_Script *script;
	int i;
	
	FT_ASSERT (info != NULL);
	
	if (!get_tables (info, table_type, &script_list, NULL))
		return FALSE;
	
	FT_ASSERT (script_index < script_list->ScriptCount);
	
	script = &script_list->ScriptRecord[script_index].Script;
	
	for (i = 0; i < script->LangSysCount; i++)
	{
		if (script->LangSysRecord[i].LangSysTag == language_tag)
		{
			if (language_index)
				*language_index = i;
			if (required_feature_index)
				*required_feature_index = script->LangSysRecord[i].LangSys.ReqFeatureIndex;
			return TRUE;
		}
	}
	
	return FALSE;
}

/**
 * ot_info_find_feature:
 * @info: a #OTInfo.
 * @table_type: the table type to obtain information about.
 * @feature_tag: the tag of the feature to find.
 * @script_index: the index of the script.
 * @language_index: the index of the language whose features are searched,
 *     or 0xffff to use the default language of the script.
 * @feature_index: location to store the index of the feature, or %NULL. 
 * @returns: %TRUE if the feature was found.
 * 
 * Finds the index of a feature.
 **/
FT_Bool
ot_info_find_feature  (OTInfo      *info,
		       OTTableType  table_type,
		       OTTag        feature_tag,
		       FT_UInt             script_index,
		       FT_UInt             language_index,
		       FT_UInt            *feature_index)
{
	TTO_ScriptList *script_list;
	TTO_FeatureList *feature_list;
	TTO_Script *script;
	TTO_LangSys *lang_sys;
	
	int i;
	
	FT_ASSERT (info != NULL);
	
	if (!get_tables (info, table_type, &script_list, &feature_list))
		return FALSE;
	
	FT_ASSERT (script_index < script_list->ScriptCount);
	
	script = &script_list->ScriptRecord[script_index].Script;
	
	if (language_index == 0xffff)
		lang_sys = &script->DefaultLangSys;
	else
	{
		FT_ASSERT (language_index < script->LangSysCount);
		lang_sys = &script->LangSysRecord[language_index].LangSys;
	}
	
	for (i = 0; i < lang_sys->FeatureCount; i++)
	{
		FT_UShort index = lang_sys->FeatureIndex[i];
		
		if (feature_list->FeatureRecord[index].FeatureTag == feature_tag)
		{
			if (feature_index)
				*feature_index = index;
			
			return TRUE;
		}
	}
	
	return FALSE;
}

/**
 * ot_info_list_scripts:
 * @info: a #OTInfo.
 * @table_type: the table type to obtain information about.
 * @returns: a newly-allocated array containing the tags of the
 *   available scripts.
 *
 * Obtains the list of available scripts. 
 **/
OTTag *
ot_info_list_scripts (OTInfo      *info,
		      OTTableType  table_type)
{
        FT_Error error;
  	FT_Memory memory = FT_FACE_MEMORY( info->face );
	OTTag *result;
	TTO_ScriptList *script_list;
	int i;
	
	FT_ASSERT (info != NULL);
	
	if (!get_tables (info, table_type, &script_list, NULL))
		return NULL;
	if ( FT_ALLOC_ARRAY ( result, 
			      script_list->ScriptCount + 1, 
			      OTTag ) )
	        return NULL;
	
	for (i=0; i < script_list->ScriptCount; i++)
		result[i] = script_list->ScriptRecord[i].ScriptTag;
	
	result[i] = 0;
	
	return result;
}

/**
 * ot_info_list_languages:
 * @info: a #OTInfo.
 * @table_type: the table type to obtain information about.
 * @script_index: the index of the script to list languages for.
 * @language_tag: unused parameter.
 * @returns: a newly-allocated array containing the tags of the
 *   available languages.
 * 
 * Obtains the list of available languages for a given script.
 **/ 
OTTag *
ot_info_list_languages (OTInfo      *info,
			OTTableType  table_type,
			FT_UInt             script_index,
			OTTag        language_tag)
{
	FT_Error error;
  	FT_Memory memory = FT_FACE_MEMORY( info->face );
	OTTag *result;
	TTO_ScriptList *script_list;
	TTO_Script *script;
	int i;
	
	FT_ASSERT (info != NULL);
	
	if (!get_tables (info, table_type, &script_list, NULL))
		return NULL;
	
	FT_ASSERT (script_index < script_list->ScriptCount);
	
	script = &script_list->ScriptRecord[script_index].Script;

	if ( FT_ALLOC_ARRAY ( result, 
			      script->LangSysCount + 1, 
			      OTTag ) )
	        return NULL;
	
	for (i = 0; i < script->LangSysCount; i++)
		result[i] = script->LangSysRecord[i].LangSysTag;
	
	result[i] = 0;
	
	return result;
}

/** 
 * ot_info_list_features:
 * @info: a #OTInfo.
 * @table_type: the table type to obtain information about.
 * @tag: unused parameter.
 * @script_index: the index of the script to obtain information about. 
 * @language_index: the indes of the language to list features for, or
 *     0xffff, to list features for the default language of the script.
 * @returns: a newly-allocated array containing the tags of the available
 *    features. 
 *
 * Obtains the list of features for the given language of the given script.
 **/
OTTag *
ot_info_list_features  (OTInfo      *info,
			OTTableType  table_type,
			OTTag        tag,
			FT_UInt             script_index,
			FT_UInt             language_index)
{
	FT_Error error;
  	FT_Memory memory = FT_FACE_MEMORY( info->face );
	OTTag *result;
	
	TTO_ScriptList *script_list;
	TTO_FeatureList *feature_list;
	TTO_Script *script;
	TTO_LangSys *lang_sys;
	
	int i;
	
	FT_ASSERT (info != NULL);
	
	if (!get_tables (info, table_type, &script_list, &feature_list))
		return NULL;
	
	FT_ASSERT (script_index < script_list->ScriptCount);
	
	script = &script_list->ScriptRecord[script_index].Script;
	
	if (language_index == 0xffff)
		lang_sys = &script->DefaultLangSys;
	else
	{
		FT_ASSERT (language_index < script->LangSysCount);
		lang_sys = &script->LangSysRecord[language_index].LangSys;
	}

	if ( FT_ALLOC_ARRAY ( result, 
			      lang_sys->FeatureCount + 1,
			      OTTag ) )
	        return NULL;

	for (i = 0; i < lang_sys->FeatureCount; i++)
	{
		FT_UShort index = lang_sys->FeatureIndex[i];
		
		result[i] = feature_list->FeatureRecord[index].FeatureTag;
	}
	
	result[i] = 0;
	
	return result;
}
