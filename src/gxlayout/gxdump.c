/***************************************************************************/
/*                                                                         */
/*  gxdump.c                                                               */
/*                                                                         */
/*    Debug functions for AAT/TrueTypeGX driver (body).                    */
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

#define GX_DEBUG_MORT_LIGATURE_TABLE_LAYOUT 0
#define GX_DEBUG_MORX_LIGATURE_TABLE_LAYOUT 0

#define FEATREG_MAX 200
#define SETTING_MAX 30

#include <ft2build.h>
#include <stdio.h>

#include FT_TYPES_H
#include FT_FREETYPE_H
#include FT_TRUETYPE_TAGS_H
#include FT_INTERNAL_DEBUG_H
#include FT_SFNT_NAMES_H
#include FT_INTERNAL_TRUETYPE_TYPES_H

#include "gxtypes.h"
#include "gxobjs.h"
#include "gxutils.h"
#include "gxdump.h"
#include "gxlookuptbl.h"
#include "gxstatetbl.h"
#include "gxaccess.h"
#include "gxltypes.h"

FT_UShort mort_tmp_firstGlyph 	  = 0;
FT_UShort mort_tmp_nGlyphs 	  = 0;
FT_UShort mort_tmp_ligActionTable = 0;
FT_UShort mort_tmp_componentTable = 0;
FT_UShort mort_tmp_ligatureTable  = 0;

FT_UShort morx_tmp_firstGlyph 	  = 0;
FT_UShort morx_tmp_nGlyphs 	  = 0;
FT_UShort morx_tmp_nLigature      = 0;
FT_ULong morx_tmp_ligatureTable   = 0;
FT_UShort morx_tmp_format   = 0;

#define GX_XML_FORMAT 
#ifdef GX_XML_FORMAT
#define INDENT(t) do {				\
    int ttt;					\
    for ( ttt = 2*t; ttt > 0; ttt-- )		\
      putc(' ', stdout);			\
  } while (0)
#define INDENTPP(t) do{ INDENT(t); (t)++; } while (0)
#define INDENTMM(t) do{ (t)--; INDENT(t); } while (0)
#define NEWLINE() fprintf(stdout, "\n")
#define NEWLINE10(t, i) do {			\
    if ( (i%10) == 0 )				\
      {						\
	if ( i != 0 )				\
	  NEWLINE();				\
	INDENT(t);				\
      }						\
  } while(0);

#define NEWLINEX(t, i, x) do {			\
    if ( (i%x) == 0 )				\
      {						\
	if ( i != 0 )				\
	  NEWLINE();				\
	INDENT(t);				\
      }						\
  } while(0);

#define POPEN(t,tag)	do {			\
    INDENTPP(t);				\
    fprintf(stdout, "<" #tag ">");		\
    NEWLINE();					\
  } while (0)

#define POPEN1(t,tag,prop,val,format)  do {	\
    INDENTPP(t);				\
    fprintf(stdout, "<" #tag " "		\
    #prop "=\"" #format "\"" ">\n", val);	\
  } while (0)

#define POPEN2(t,tag,prop1,val1,format1,prop2,val2,format2)  do {	\
    INDENTPP(t);							\
    fprintf(stdout, "<" #tag " "					\
    #prop1 "=\"" #format1 "\"" " "					\
    #prop2 "=\"" #format2 "\"" " "					\
    ">\n", val1, val2);							\
  } while (0)

#define POPEN3(t,tag,prop1,val1,format1,prop2,val2,format2,prop3,val3,format3)  do { \
    INDENTPP(t);							\
    fprintf(stdout, "<" #tag " "					\
    #prop1 "=\"" #format1 "\"" " "					\
    #prop2 "=\"" #format2 "\"" " "					\
    #prop3 "=\"" #format3 "\"" " "					\
    ">\n", val1, val2, val3);						\
  } while (0)

#define PCLOSE(t,tag)	do {			\
    INDENTMM(t);				\
    fprintf(stdout, "</" #tag ">\n");		\
  } while (0)

#define PTAG(t,xmltag,valuetag) do {					\
    INDENT(t);								\
    fprintf(stdout, "<" #xmltag ">" "%c%c%c%c" "</" #xmltag ">\n",      \
	    (char)(0xff & (valuetag >> 24)),				\
	    (char)(0xff & (valuetag >> 16)),				\
	    (char)(0xff & (valuetag >> 8)), 				\
	    (char)(0xff & (valuetag >> 0)));				\
  } while (0)

#define PFIELD(t,base,field,format)					\
PVALUE(t,field,base->field,format)

#define PVALUE(t,tag,value,format) do {					\
    INDENT(t);								\
    fprintf(stdout, "<" #tag ">" #format "</" #tag ">\n",  value);	\
  } while (0)

#define PFIELD1(t,base,field,format,prop,pval,pform)	\
  PVALUE1(t,field,base->field,format,prop,pval,pform)

#define PFIELD2(t,base,field,format,prop1,pval1,pform1,prop2,pval2,pform2)	\
  PVALUE2(t,field,base->field,format,prop1,pval1,pform1,prop2,pval2,pform2)

#define PFIELD3(t,base,field,format,prop1,pval1,pform1,prop2,pval2,pform2,prop3,pval3,pform3)	\
  PVALUE3(t,field,base->field,format,prop1,pval1,pform1,prop2,pval2,pform2,prop3,pval3,pform3)

#define PVALUE1(t,tag,value,format,prop1,pval1,pform1) do {	\
    INDENT(t);							\
    fprintf(stdout, "<" #tag " "				\
	    #prop1 "=\"" #pform1 "\" "				\
	    ">" #format "</" #tag ">\n",			\
	    pval1, value);					\
  } while (0)

#define PVALUE2(t,tag,value,format,prop1,pval1,pform1,prop2,pval2,pform2) do { \
    INDENT(t);								\
    fprintf(stdout, "<" #tag " "					\
	    #prop1 "=\"" #pform1 "\" "					\
	    #prop2 "=\"" #pform2 "\" "					\
	    ">" #format "</" #tag ">\n",				\
	    pval1, pval2, value);					\
  } while (0)

#define PVALUE3(t,tag,value,format,prop1,pval1,pform1,prop2,pval2,pform2,prop3,pval3,pform3) do { \
    INDENT(t);								\
    fprintf(stdout, "<" #tag " "					\
	    #prop1 "=\"" #pform1 "\" "					\
	    #prop2 "=\"" #pform2 "\" "					\
	    #prop3 "=\"" #pform3 "\" "					\
	    ">" #format "</" #tag ">\n",				\
	    pval1, pval2, pval3, value);				\
  } while (0)

#define PVALUE4(t,tag,value,format,prop1,pval1,pform1,prop2,pval2,pform2,prop3,pval3,pform3,prop4,pval4,pform4) do { \
    INDENT(t);								\
    fprintf(stdout, "<" #tag " "					\
	    #prop1 "=\"" #pform1 "\" "					\
	    #prop2 "=\"" #pform2 "\" "					\
	    #prop3 "=\"" #pform3 "\" "					\
	    #prop4 "=\"" #pform4 "\" "					\
	    ">" #format "</" #tag ">\n",				\
	    pval1, pval2, pval3, pval4, value);				\
  } while (0)

#define PVALUE5(t,tag,value,format,prop1,pval1,pform1,prop2,pval2,pform2,prop3,pval3,pform3,prop4,pval4,pform4,prop5,pval5,pform5) do { \
    INDENT(t);								\
    fprintf(stdout, "<" #tag " "					\
	    #prop1 "=\"" #pform1 "\" "					\
	    #prop2 "=\"" #pform2 "\" "					\
	    #prop3 "=\"" #pform3 "\" "					\
	    #prop4 "=\"" #pform4 "\" "					\
	    #prop5 "=\"" #pform5 "\" "					\
	    ">" #format "</" #tag ">\n",				\
	    pval1, pval2, pval3, pval4, pval5, value);			\
  } while (0)

#define COMMDENT(t, comment) do {		\
    INDENT(t);					\
    fprintf(stdout, " <!-- %s --> ", comment);	\
  } while (0)
#define COMMDENTNL(t, comment) do {		\
    COMMDENT(t, commdent); \
    fprintf(stdout, "\n"); \
  } while (0)


#endif /* Def: GX_XML_FORMAT */

static void  dump_table_info(GX_Table table, int n);

void gx_face_dump_trak(GX_Face face, GX_Trak trak);
void gx_face_dump_feat(GX_Face face, GX_Feat feat);
void gx_face_dump_prop(GX_Face face, GX_Prop prop);
void gx_face_dump_opbd(GX_Face face, GX_Opbd opbd);
void gx_face_dump_lcar(GX_Face face, GX_Lcar lcar);
void gx_face_dump_bsln(GX_Face face, GX_Bsln bsln);
void gx_face_dump_mort(GX_Face face, GX_Mort mort);
void gx_face_dump_fmtx(GX_Face face, GX_Fmtx fmtx);
void gx_face_dump_fdsc(GX_Face face, GX_Fdsc fdsc);
void gx_face_dump_morx(GX_Face face, GX_Morx morx);
void gx_face_dump_just(GX_Face face, GX_Just just);
void gx_face_dump_kern(GX_Face face, GX_Kern kern);

static FT_Error generic_dump_lookup_table_generic ( GX_LookupTable_Format format,
							  GX_LookupValue value,
							  FT_Pointer user );
static FT_Error
generic_dump_lookup_table_segment( GX_LookupTable_Format format,
				   FT_UShort lastGlyph,
				   FT_UShort firstGlyph,
				   GX_LookupValue value,
				   FT_Pointer user );

GX_LookupTable_FuncsRec generic_lookup_table_funcs = {
  generic_dump_lookup_table_generic, /* generic_func */
  NULL,				     /* simple_array_func */
  NULL,				     /* segment_single_func */
  generic_dump_lookup_table_segment, /* segment_array_func */
  NULL,				     /* single_table_func */
  NULL				     /* trimmed_array_func */
};

/* ACTION must dump GX_EntrySubtable::flags and its following 
   items(if they exist.) A pointer to an int which means indent value 
   is passed to ACTION as the 2nd argument USER. */
static void  gx_face_dump_state_table ( GX_Face face, 
					   GX_StateTable state_table,
					   GX_StateTable_Entry_Action action,
					   int t );
static void  gx_face_dump_xstate_table ( GX_Face face, 
					 GX_XStateTable state_table,
					 GX_XStateTable_Entry_Action action,
					 int t );

/* If funcs is NULL or funcs->generic_func is NULL,
   dump_lookup_table_generic is used. */
static void gx_face_dump_LookupTable_low( GX_Face face,
					  GX_LookupTable lookup_table,
					  GX_LookupTable_Funcs funcs,
					  int t);

static void gx_face_dump_LookupTable_high( GX_Face face,
					   GX_LookupTable lookup_table,
					   GX_LookupTable_Glyph_Func func,
					   int t );

FT_EXPORT_DEF ( FT_Error )
gx_face_dump( FT_Face face, FT_ULong tables, const char * fname )
{
  GXL_Font gx_font;
  int count = 0;
  int t = 0;
  
  if ( FTL_Get_Font(face, (FTL_Font*)&gx_font) )
    return FT_Err_Invalid_Argument;

#define COUNT(x)				\
  if ((tables & GX_DUMP_##x) && gx_font->x) count++

#define DUMP(x)								\
  if (tables & GX_DUMP_##x)						\
  {									\
    int t = 1;								\
    GX_Table table;                                                     \
    if (gx_font->x)                                                     \
      {									\
	POPEN(t, x);							\
	table = (GX_Table)gx_font->x;              			\
	dump_table_info(table, t);			                \
	gx_face_dump_##x((GX_Face)face, (void*)table);			\
	PCLOSE(t,x);							\
      }									\
  }
  COUNT(mort);
  COUNT(morx);
  COUNT(trak);
  COUNT(kern);
  COUNT(feat);
  COUNT(just);
  COUNT(prop);
  COUNT(lcar);
  COUNT(opbd);
  COUNT(bsln);
  COUNT(fmtx);
  COUNT(fdsc);
  POPEN2(t,gxdump,name,fname,%s,count,count,%d);
  DUMP(mort);
  DUMP(morx);
  DUMP(trak);
  DUMP(kern);
  DUMP(feat);
  DUMP(just);
  DUMP(prop);
  DUMP(lcar);
  DUMP(opbd);
  DUMP(bsln);
  DUMP(fmtx);
  DUMP(fdsc);
  PCLOSE(t,gxdump);
  return FT_Err_Ok;
}


/******************************TRAK************************************/
void gx_face_dump_trak_data(GX_Face face, GX_TrackData data, int t);
void gx_face_dump_trak_data_table_entry(GX_Face face, GX_TrackTableEntry table_entry, FT_UShort nSizes, int t);

void
gx_face_dump_trak(GX_Face face, GX_Trak trak)
{
  int t = 2;
  PFIELD(t, trak, version, 0x%08lx);
  PFIELD(t, trak, format, %u);
  PFIELD(t, trak, horizOffset, %u);
  PFIELD(t, trak, vertOffset, %u);
  PFIELD(t, trak, reserved, %u);
  POPEN(t, horizData);
  if (trak->horizOffset)
    gx_face_dump_trak_data(face, &trak->horizData, t);
  PCLOSE(t, horizData);
  POPEN(t, vertData);
  if (trak->vertOffset)
    gx_face_dump_trak_data(face, &trak->vertData, t);
  PCLOSE(t, vertData);
}

void
gx_face_dump_trak_data(GX_Face face, GX_TrackData data, int t)
{
  int i;
  
  PFIELD(t,data,nTracks,%u);
  PFIELD(t,data,nSizes,%u);
  PFIELD(t,data,sizeTableOffset,%lu);
  POPEN(t,tableEntries);
  for ( i = 0; i < data->nTracks; i++ )
    {
      POPEN1(t, tableEntry, index,i,%d);
      gx_face_dump_trak_data_table_entry(face, &(data->trackTable[i]), data->nSizes, t);
      PCLOSE(t, tableEntry);
    } 
  PCLOSE(t,tableEntries);
  POPEN(t,sizes);
  for ( i = 0; i < data->nSizes; i++ )
    {
      NEWLINE10(t, i);
      fprintf(stdout, "0x%08lx[%d] ", data->sizeTable[i], i);
    }
  NEWLINE();
  PCLOSE(t,sizes);

}

void
gx_face_dump_trak_data_table_entry(GX_Face face, GX_TrackTableEntry table_entry, FT_UShort nSizes, int t)
{
  FT_Error error;
  FT_Memory memory = face->root.driver->root.memory;
  FT_SfntName sfnt_name;
  FT_String * string;
  int i;
  PFIELD(t, table_entry,track,0x%08lx);
  
  if (( error = gx_get_name_from_id((FT_Face)face, 
				    table_entry->nameIndex, 
				    0, 0, 0,
				    &sfnt_name) ))
    PFIELD(t,table_entry,nameIndex,%u);
  else
    {
      if ( FT_NEW_ARRAY(string, sfnt_name.string_len + 1) )
	goto NameFailure;
      
      string[sfnt_name.string_len] = '\0';
      for ( i = 0; i < sfnt_name.string_len; i++)
	string[i] = sfnt_name.string[i];
      PFIELD1(t, table_entry,nameIndex,%u,name,string,%s);
      FT_FREE(string);
    }

  PFIELD(t, table_entry, offset,%d);
  POPEN(t,trackingValue);
  for (i = 0; i < nSizes; i++)
    {
      NEWLINE10(t, i);
      fprintf(stdout, "%d[%d] ", table_entry->tracking_value[i], i);
    }
  NEWLINE();
  PCLOSE(t,trackingValue);
  return;

 NameFailure:
  fprintf(stderr, "[%s:trak]Error in name index\n", face->root.family_name);
  exit(error);
}


/******************************FEAT************************************/
void gx_face_dump_feat_names(GX_Face face, GX_FeatureName names, int t);
void gx_face_dump_feat_settingName(GX_Face face, GX_FeatureSettingName settingName, int i, int t);

void
gx_face_dump_feat(GX_Face face, GX_Feat feat)
{
  int i, t = 2;
  PFIELD(t, feat, version, 0x%08lx);
  PFIELD(t, feat, featureNameCount, %u);
  PFIELD(t, feat, reserved1, %u);
  PFIELD(t, feat, reserved2, %lu);
  POPEN(t, names);
  for ( i = 0; i < feat->featureNameCount; i++)
    gx_face_dump_feat_names(face, &(feat->names[i]), t);
  PCLOSE(t, names);
}

void
gx_face_dump_feat_names(GX_Face face, GX_FeatureName names, int t)
{
  FT_Error error;
  FT_Memory memory = face->root.driver->root.memory;
  FT_SfntName sfnt_name;
  FT_String * string;
  int i;
  POPEN(t,name);
  PFIELD(t,names,feature,%u);
  PFIELD(t,names,nSettings,%u);
  PFIELD(t,names,settingTable,%lu);
  POPEN(t,featureFlags);
  PVALUE(t,value,names->featureFlags,%08x);
  PVALUE(t,exclusive,(names->featureFlags & GX_FEAT_MASK_EXCLUSIVE_SETTINGS),%04x);
  PVALUE(t,dynamicDefault,(names->featureFlags & GX_FEAT_MASK_DYNAMIC_DEFAULT),%04x);
  PVALUE(t,unused,(names->featureFlags & GX_FEAT_MASK_UNUSED),%04x);
  PVALUE(t,defaultSetting,(names->featureFlags & GX_FEAT_MASK_DEFAULT_SETTING),%04x);
  PCLOSE(t,featureFlags);
  if (( error = gx_get_name_from_id((FT_Face)face, 
				    names->nameIndex, 
				    0, 0, 0,
				    &sfnt_name) ))
    PFIELD(t,names,nameIndex,%u);
  else
    {
      if ( FT_NEW_ARRAY(string, sfnt_name.string_len + 1) )
	goto NameFailure;
      
      string[sfnt_name.string_len] = '\0';
      for ( i = 0; i < sfnt_name.string_len; i++)
	string[i] = sfnt_name.string[i];
      PFIELD1(t, names,nameIndex,%u,name,string,%s);
      FT_FREE(string);
    }
  POPEN(t,settingNames);
  for (i = 0; i < names->nSettings; i++)
    gx_face_dump_feat_settingName(face, &(names->settingName[i]), i, t);
  PCLOSE(t,settingNames);
  PCLOSE(t,name);
  return ;
 NameFailure:
  exit(error);
}

void
gx_face_dump_feat_settingName(GX_Face face, GX_FeatureSettingName settingName, int i, int t)
{
  FT_Error error;
  FT_Memory memory = face->root.driver->root.memory;
  FT_SfntName sfnt_name;
  FT_String * string;
  int j;  

  if (( error = gx_get_name_from_id((FT_Face)face, 
				    settingName->nameIndex, 
				    0, 0, 0,
				    &sfnt_name) ))
    {
      PFIELD1(t,settingName,setting,%u,index,i,%d);
      PFIELD1(t,settingName,nameIndex,%u,index,i,%d);
    }
  else
    {
      if ( FT_NEW_ARRAY(string, sfnt_name.string_len + 1) )
	goto NameFailure;
      
      string[sfnt_name.string_len] = '\0';
      for ( j = 0; j < sfnt_name.string_len; j++)
	{
	  /* Don't use '&' in pseudo XML file.
	     Here I replace '&' with '|'. */
	  if ( sfnt_name.string[j] == '&' )
	    string[j] = '|' ;
	  else
	    string[j] = sfnt_name.string[j];
	}
      PFIELD1(t,settingName,setting,%u,index,i,%d);
      PFIELD2(t,settingName,nameIndex,%u,index,i,%d,name,string,%s);
      FT_FREE(string);
    }
  return;
 NameFailure:
  exit(error);  
}



/******************************PROP************************************/
static void
prop_dump_prop( FT_UShort value, int index, int t )
{
  if ( index < 0 )
    {
      POPEN(t, prop);
      INDENT(t);
      fprintf(stdout, 
	      "floater: %u hang-off-left-top: %u hang-off-right-bottom: %u complementary-bracket: %u complementary-bracket-offset: %u attaching-to-right: %u reserved: %u directionality-class: %u\n",
	      value & GX_PROP_MASK_FLOATER,
	      value & GX_PROP_MASK_HANG_OFF_LEFT_TOP,
	      value & GX_PROP_MASK_HANG_OFF_RIGHT_BOTTOM,
	      value & GX_PROP_MASK_USE_COMPLEMENTARY_BRACKET,
	      value & GX_PROP_MASK_COMPLEMENTARY_BRACKET_OFFSET,
	      value & GX_PROP_MASK_ATTACHING_TO_RIGHT,
	      value & GX_PROP_MASK_RESERVED,
	      value & GX_PROP_MASK_DIRECTIONALITY_CLASS);
      PCLOSE(t, prop);
    }
  else
    {
      POPEN1(t,prop,index,index,%d);
      INDENT(t);
      fprintf(stdout, 
	      "floater: %u hang-off-left-top: %u hang-off-right-bottom: %u complementary-bracket: %u complementary-bracket-offset: %u attaching-to-right: %u reserved: %u directionality-class: %u\n",
	      value & GX_PROP_MASK_FLOATER,
	      value & GX_PROP_MASK_HANG_OFF_LEFT_TOP,
	      value & GX_PROP_MASK_HANG_OFF_RIGHT_BOTTOM,
	      value & GX_PROP_MASK_USE_COMPLEMENTARY_BRACKET,
	      value & GX_PROP_MASK_COMPLEMENTARY_BRACKET_OFFSET,
	      value & GX_PROP_MASK_ATTACHING_TO_RIGHT,
	      value & GX_PROP_MASK_RESERVED,
	      value & GX_PROP_MASK_DIRECTIONALITY_CLASS);
      PCLOSE(t, prop);
  }
}

static FT_Error
prop_dump_lookup_table_segment ( GX_LookupTable_Format format,
				  FT_UShort lastGlyph,
				  FT_UShort firstGlyph,
				  GX_LookupValue value,
				  FT_Pointer user )
{
  FT_UShort segment_count = lastGlyph - firstGlyph;
  FT_UShort * extra 	  = value->extra.word;
  FT_Int * t = (FT_Int*) user;
  int i;
  POPEN(*t, segmentArrayElement);
  PFIELD(*t,value,raw.s,%d);
  PVALUE(*t,lastGlyph,lastGlyph,%u);
  PVALUE(*t,firstGlyph,firstGlyph,%u);
  POPEN(*t, extra);
  for ( i = 0; i < segment_count; i++ )
    prop_dump_prop(extra[i], i, *t);
  PCLOSE(*t, extra);
  PCLOSE(*t, segmentArrayElement);
  return FT_Err_Ok;
}

static FT_Error
prop_dump_lookup_table_generic ( GX_LookupTable_Format format,
				 GX_LookupValue value,
				 FT_Pointer user )
{		    /* TODO? more things here? learn freetype more. */
  FT_Int *t = (FT_Int*) user;
  prop_dump_prop(value->raw.s, -1, *t);
  return FT_Err_Ok;
}

GX_LookupTable_FuncsRec prop_dump_lookup_table_funcs = {
  prop_dump_lookup_table_generic,
  NULL,
  NULL,
  prop_dump_lookup_table_segment,
  NULL,
  NULL
};


void
gx_face_dump_prop(GX_Face face, GX_Prop prop)
{
  int t = 2;
  PFIELD(t,prop,version,0x%08lx);
  PFIELD(t,prop,format,%u);
  PFIELD(t,prop,default_properties,%u);
  gx_face_dump_LookupTable_low( face, &(prop->lookup_data), &prop_dump_lookup_table_funcs, t);
}

/******************************OPBD************************************/  
static FT_Error
opbd_dump_data(FT_UShort raw, GX_OpticalBoundsData data, GX_OpticalBoundsFormat fmt, int t)
{
  if (fmt == GX_OPBD_DISTANCE)
    {
#define DUMP_DISTANCE(side)						\
      if (data->distance.side##_side == GX_OPBD_NO_OPTICAL_EDGE)	\
	fprintf(stdout, "%s: empty ",#side);				\
      else								\
	fprintf(stdout, "%s: %d ", #side, data->distance.side##_side); /* %d??? */
      POPEN1(t,distance,raw,raw,%u);
      INDENT(t);
      DUMP_DISTANCE(left);
      DUMP_DISTANCE(top);
      DUMP_DISTANCE(right);
      DUMP_DISTANCE(bottom);
      NEWLINE();
      PCLOSE(t,distance);
    }
  else
    {
      POPEN1(t,control-points,raw,raw,%u);
      INDENT(t);
      fprintf(stdout, "left: %u top: %u right: %u bottom: %u ",
	      data->control_points.left_side,
	      data->control_points.top_side,
	      data->control_points.right_side,
	      data->control_points.bottom_side);
      NEWLINE();
      PCLOSE(t,control-points);
    }
  return FT_Err_Ok;
}

static FT_Error
opbd_dump_lookup_table_distance_generic ( GX_LookupTable_Format format,
					GX_LookupValue value,
					FT_Pointer user )
{
  FT_Int *t = (FT_Int*) user;
  opbd_dump_data(value->raw.s, value->extra.opbd_data, GX_OPBD_DISTANCE, *t);
  return FT_Err_Ok;
}

static FT_Error
opbd_dump_lookup_table_ctlpoint_generic ( GX_LookupTable_Format format,
					   GX_LookupValue value,
					   FT_Pointer user )
{
  FT_Int *t = (FT_Int*) user;
  opbd_dump_data(value->raw.s, value->extra.opbd_data, GX_OPBD_DISTANCE, *t);
  return FT_Err_Ok;
}

static FT_Error
opbd_dump_lookup_table_distance_segment ( GX_LookupTable_Format format,
					  FT_UShort lastGlyph,
					  FT_UShort firstGlyph,
					  GX_LookupValue value,
					  FT_Pointer user )
{
  FT_UShort segment_count = lastGlyph - firstGlyph;
  GX_OpticalBoundsData extra 	  = value->extra.opbd_data;
  FT_Int * t = (FT_Int*) user;
  int i;
  POPEN(*t, segmentArrayElement);
  PFIELD(*t,value,raw.s,%d);
  PVALUE(*t,lastGlyph,lastGlyph,%u);
  PVALUE(*t,firstGlyph,firstGlyph,%u);
  POPEN(*t, extra);
  for ( i = 0; i < segment_count; i++ )
    opbd_dump_data(value->raw.s,&extra[i], GX_OPBD_DISTANCE, *t);
  PCLOSE(*t, extra);
  PCLOSE(*t, segmentArrayElement);
  return FT_Err_Ok;
}

static FT_Error
opbd_dump_lookup_table_ctlpoint_segment ( GX_LookupTable_Format format,
					   FT_UShort lastGlyph,
					   FT_UShort firstGlyph,
					   GX_LookupValue value,
					   FT_Pointer user )
{
  FT_UShort segment_count = lastGlyph - firstGlyph;
  GX_OpticalBoundsData extra 	  = value->extra.opbd_data;
  FT_Int * t = (FT_Int*) user;
  int i;
  POPEN(*t, segmentArrayElement);
  PFIELD(*t,value,raw.s,%d);
  PVALUE(*t,lastGlyph,lastGlyph,%u);
  PVALUE(*t,firstGlyph,firstGlyph,%u);
  POPEN(*t, extra);
  for ( i = 0; i < segment_count; i++ )
    opbd_dump_data(value->raw.s,&extra[i], GX_OPBD_CONTROL_POINTS, *t);
  PCLOSE(*t, extra);
  PCLOSE(*t, segmentArrayElement);
  return FT_Err_Ok;
}

GX_LookupTable_FuncsRec opbd_dump_lookup_table_distance_funcs = {
  opbd_dump_lookup_table_distance_generic,    /* generic_func */
  NULL,					      /* simple_array_func */
  NULL,					      /* segment_single_func */
  opbd_dump_lookup_table_distance_segment,    /* segment_array_func */
  NULL,
  NULL
};

GX_LookupTable_FuncsRec opbd_dump_lookup_table_ctlpoint_funcs = {
  opbd_dump_lookup_table_ctlpoint_generic,
  NULL,
  NULL,
  opbd_dump_lookup_table_ctlpoint_segment,
  NULL,
  NULL
};

void
gx_face_dump_opbd(GX_Face face, GX_Opbd opbd)
{
  int t = 2;
  PFIELD(t,opbd,version,0x%08lx);
  PFIELD(t,opbd,format,%u);
  if (opbd->format == GX_OPBD_DISTANCE)
    gx_face_dump_LookupTable_low( face, &(opbd->lookup_data), 
				  &opbd_dump_lookup_table_distance_funcs, 
				  t);
  else
    gx_face_dump_LookupTable_low( face, &(opbd->lookup_data), 
				  &opbd_dump_lookup_table_ctlpoint_funcs, 
				  t);
}


/******************************LCAR************************************/  
#if 0

static FT_Error
lcar_dump_data(FT_UShort raw, GX_LigCaretClassEntry entry, GX_LigCaretFormat fmt, int t)
{
  int i;
  if (fmt == GX_LCAR_DISTANCE)
    {
      POPEN2(t,distance,raw,raw,%u,count,entry->count,%u);
      for ( i = 0; i < entry->count; i++ )
	{
	  NEWLINE10(t,i);
	  fprintf(stdout, "%d[%d] ", entry->partials[i], i);
	}
      NEWLINE();
      PCLOSE(t,distance);
    }
  else
    {
      POPEN2(t,control-points,raw,raw,%u,count,entry->count,%u);
      for ( i = 0; i < entry->count; i++ )
	{
	  NEWLINE10(t,i);
	  fprintf(stdout, "%d[%d] ", entry->partials[i], i);
	}
      NEWLINE();
      PCLOSE(t,control-points);
    }
  return FT_Err_Ok;
}

static FT_Error
lcar_dump_lookup_table_distance_generic ( GX_LookupTable_Format format,
					  GX_LookupValue value,
					  FT_Pointer user )
{
  FT_Int *t = (FT_Int*) user;
  lcar_dump_data(value->raw.s, value->extra.lcar_class_entry, GX_LCAR_DISTANCE, *t);
  return FT_Err_Ok;
}

static FT_Error
lcar_dump_lookup_table_ctlpoint_generic ( GX_LookupTable_Format format,
					  GX_LookupValue value,
					  FT_Pointer user )
{
  FT_Int *t = (FT_Int*) user;
  lcar_dump_data(value->raw.s, value->extra.lcar_class_entry, GX_LCAR_DISTANCE, *t);
  return FT_Err_Ok;
}

static FT_Error
lcar_dump_lookup_table_generic_segment ( FT_UShort lastGlyph,
					 FT_UShort firstGlyph,
					 GX_LookupValue value,
					 GX_LigCaretFormat lcar_format,
					 FT_Pointer user )
{
  FT_UShort segment_count = lastGlyph - firstGlyph;
  GX_LigCaretSegment extra 	  = value->extra.lcar_segment;
  FT_Int * t = (FT_Int*) user;
  int i;
  POPEN(*t, segmentArrayElement);
  PFIELD(*t,value,raw.s,%d);
  PVALUE(*t,lastGlyph,lastGlyph,%u);
  PVALUE(*t,firstGlyph,firstGlyph,%u);
  POPEN(*t, extra);
  for ( i = 0; i < segment_count; i++ )
    {
      POPEN1(*t,LigCaretSegment,offset,extra->offset,%u);
      lcar_dump_data(value->raw.s,extra[i].class_entry, lcar_format, *t);
      PCLOSE(*t,LigCaretSegment);
    }
  PCLOSE(*t, extra);
  PCLOSE(*t, segmentArrayElement);
  return FT_Err_Ok;
}

static FT_Error
lcar_dump_lookup_table_distance_segment ( GX_LookupTable_Format format,
					  FT_UShort lastGlyph,
					  FT_UShort firstGlyph,
					  GX_LookupValue value,
					  FT_Pointer user )
{
  return lcar_dump_lookup_table_generic_segment(lastGlyph, 
						firstGlyph, 
						value, 
						GX_LCAR_DISTANCE,
						user);
}

static FT_Error
lcar_dump_lookup_table_ctlpoint_segment ( GX_LookupTable_Format format,
					   FT_UShort lastGlyph,
					   FT_UShort firstGlyph,
					   GX_LookupValue value,
					   FT_Pointer user )
{
  return lcar_dump_lookup_table_generic_segment(lastGlyph, 
						firstGlyph, 
						value, 
						GX_LCAR_CONTROL_POINTS,
						user);
}


GX_LookupTable_FuncsRec lcar_dump_lookup_table_distance_funcs = {
  lcar_dump_lookup_table_distance_generic,
  NULL,
  NULL,
  lcar_dump_lookup_table_distance_segment,
  NULL,
  NULL
};

GX_LookupTable_FuncsRec lcar_dump_lookup_table_ctlpoint_funcs = {
  lcar_dump_lookup_table_ctlpoint_generic,
  NULL,
  NULL,
  lcar_dump_lookup_table_ctlpoint_segment,
  NULL,
  NULL
};

void
gx_face_dump_lcar(GX_Face face, GX_Lcar lcar)
{
  int t = 2;
  PFIELD(t,lcar,version,0x%08lx);
  PFIELD(t,lcar,format,%u);  
  if (lcar->format == GX_LCAR_DISTANCE)
    gx_face_dump_LookupTable_low( face, &(lcar->lookup), 
				  &lcar_dump_lookup_table_distance_funcs, 
				  t);
  else
    gx_face_dump_LookupTable_low( face, &(lcar->lookup), 
				  &lcar_dump_lookup_table_ctlpoint_funcs, 
				  t);
}

#endif /* Not def: 0 */


static FT_Error
lcar_dump_data(FT_UShort glyph, FT_UShort raw, GX_LigCaretClassEntry entry, GX_LigCaretFormat fmt, int t)
{
  int i;
  if (fmt == GX_LCAR_DISTANCE)
    {
      POPEN2(t,distance,glyph,glyph,%u,count,entry->count,%u);
      for ( i = 0; i < entry->count; i++ )
	{
	  NEWLINE10(t,i);
	  fprintf(stdout, "%d[%d] ", entry->partials[i], i);
	}
      NEWLINE();
      PCLOSE(t,distance);
    }
  else
    {
      POPEN2(t,control-points,glyph,glyph,%u,count,entry->count,%u);
      for ( i = 0; i < entry->count; i++ )
	{
	  NEWLINE10(t,i);
	  fprintf(stdout, "%d[%d] ", entry->partials[i], i);
	}
      NEWLINE();
      PCLOSE(t,control-points);
    }
  return FT_Err_Ok;
}

static FT_Error
lcar_dump_lookup_table_distance_funcs ( FT_UShort glyph,
					GX_LookupValue value,
					FT_Long firstGlyph,
					FT_Pointer user )
{
  FT_Int *t = (FT_Int*)user;
  GX_LigCaretSegment extra = value->extra.lcar_segment;
  if ( firstGlyph == GX_LOOKUP_RESULT_NO_FIRST_GLYPH )
    lcar_dump_data(glyph, value->raw.s, value->extra.lcar_class_entry, GX_LCAR_DISTANCE, *t);
  else
    lcar_dump_data(glyph, value->raw.s,extra[glyph - firstGlyph].class_entry, GX_LCAR_DISTANCE, *t);
  return FT_Err_Ok;
}

static FT_Error
lcar_dump_lookup_table_ctlpoint_funcs ( FT_UShort glyph,
					GX_LookupValue value,
					FT_Long firstGlyph,
					FT_Pointer user )
{
  FT_Int *t = (FT_Int*)user;
  GX_LigCaretSegment extra = value->extra.lcar_segment;
  if ( firstGlyph == GX_LOOKUP_RESULT_NO_FIRST_GLYPH )
    lcar_dump_data(glyph, value->raw.s, value->extra.lcar_class_entry, GX_LCAR_CONTROL_POINTS, *t);
  else
    lcar_dump_data(glyph, value->raw.s,extra[glyph - firstGlyph].class_entry, GX_LCAR_CONTROL_POINTS, *t);
  return FT_Err_Ok;
}

void
gx_face_dump_lcar(GX_Face face, GX_Lcar lcar)
{
  int t = 2;
  PFIELD(t,lcar,version,0x%08lx);
  PFIELD(t,lcar,format,%u);  
  if (lcar->format == GX_LCAR_DISTANCE)
    gx_face_dump_LookupTable_high( face, &(lcar->lookup), 
				  &lcar_dump_lookup_table_distance_funcs, 
				  t);
  else
    gx_face_dump_LookupTable_high( face, &(lcar->lookup), 
				   &lcar_dump_lookup_table_ctlpoint_funcs, 
				   t);
}


/******************************BSLN************************************/  
static void
gx_bsln_dump_deltas(FT_UShort deltas[], int t)
{
  int i;
  POPEN(t,deltas);
  for ( i = 0; i < GX_BSLN_VALUE_COUNT; i++ )
    {
      NEWLINE10(t,i);
      fprintf(stdout, "%u[%d] ", deltas[i], i);
    }
  NEWLINE();
  PCLOSE(t,deltas);
}

static void
gx_bsln_dump_ctlPoints(FT_UShort ctlPoints[], int t)
{
  int i;
  POPEN(t,ctlPoints);
  INDENT(t);
  for (i = 0; i < GX_BSLN_VALUE_COUNT; i++ )
    {
      if ( ctlPoints[i] == GX_BSLN_VALUE_EMPTY )
	fprintf(stdout, "%s[%d] ", "empty", i);
      else
	fprintf(stdout, "%u[%d] ", ctlPoints[i], i);
    }
  NEWLINE();
  PCLOSE(t,deltas);
}

void
gx_face_dump_bsln(GX_Face face, GX_Bsln bsln)
{
  int t = 2;
  GX_BaselineFormat0Part f0part;
  GX_BaselineFormat1Part f1part;
  GX_BaselineFormat2Part f2part;
  GX_BaselineFormat3Part f3part;

  PFIELD(t,bsln,version,0x%08lx);
  PFIELD(t,bsln,format,%u);
  PFIELD(t,bsln,defaultBaseline,%u);
  switch ( bsln->format )
  {
  case GX_BSLN_FMT_DISTANCE_NO_MAPPING:
    f0part = bsln->parts.fmt0;
    gx_bsln_dump_deltas(f0part->deltas, t);
    break;
  case GX_BSLN_FMT_DISTANCE_WITH_MAPPING:
    f1part = bsln->parts.fmt1;
    gx_bsln_dump_deltas(f1part->deltas, t);
    gx_face_dump_LookupTable_low(face, &f1part->mappingData, 
				 &generic_lookup_table_funcs, t);
    break;
  case GX_BSLN_FMT_CONTROL_POINT_NO_MAPPING:
    f2part = bsln->parts.fmt2;
    PFIELD(t,f2part,stdGlyph,%u);
    gx_bsln_dump_ctlPoints(f2part->ctlPoints, t);
    break;
  case GX_BSLN_FMT_CONTROL_POINT_WITH_MAPPING:
    f3part = bsln->parts.fmt3;
    PFIELD(t,f3part,stdGlyph,%u);
    gx_bsln_dump_ctlPoints(f3part->ctlPoints, t);
    gx_face_dump_LookupTable_low(face, &f3part->mappingData, 
				 &generic_lookup_table_funcs, t);
  }
}


/******************************MORT************************************/
#if 1
static FT_Error
mort_dump_lookup_table_segment ( GX_LookupTable_Format format,
				 FT_UShort lastGlyph,
				 FT_UShort firstGlyph,
				 GX_LookupValue value,
				 FT_Pointer user )
{
  FT_Int * t = (FT_Int*) user;
  POPEN(*t, segmentArrayElement);
  PFIELD(*t,value,raw.s,%d);
  PVALUE(*t,lastGlyph,lastGlyph,%u);
  PVALUE(*t,firstGlyph,firstGlyph,%u);
  PCLOSE(*t, segmentArrayElement);
  return FT_Err_Ok;
}

static FT_Error
mort_dump_lookup_table_single ( GX_LookupTable_Format format,
				FT_UShort glyph,
				GX_LookupValue value,
				FT_Pointer user )
{
  FT_Int * t = (FT_Int*) user;
  PVALUE1(*t,value,value->raw.s,%d,glyph,glyph,%u);
  return FT_Err_Ok;
}


GX_LookupTable_FuncsRec mort_dump_lookup_table_funcs = {
  NULL,
  NULL,
  NULL,
  mort_dump_lookup_table_segment,
  mort_dump_lookup_table_single,
  NULL
};
#endif /* 1 */


static FT_Error 
mort_dump_lookup_table_func( FT_UShort glyph,
			     GX_LookupValue value,
			     FT_Long firstGlyph,
			     FT_Pointer user )
{
  FT_Int * t = (FT_Int*) user;
  if ( firstGlyph == GX_LOOKUP_RESULT_NO_FIRST_GLYPH )
    PVALUE1(*t,value,value->raw.u,%u,glyph,glyph,%u);
  else
    PVALUE1(*t,value,value->extra.word[glyph - firstGlyph],%u,glyph,glyph,%u);
  return FT_Err_Ok;
}
			     
static void
gx_face_dump_mort_subtable_type(GX_Face face, FT_UShort subtype, int t)
{
  POPEN(t, subtableType);
  PVALUE(t,value,subtype,%u);
  switch(subtype)
    {
    case GX_MORT_REARRANGEMENT_SUBTABLE:
      PVALUE(t,symbol,"rearrangement", %s);
      break;
    case GX_MORT_CONTEXTUAL_SUBTABLE:
      PVALUE(t,symbol,"contextual", %s);
      break;
    case GX_MORT_LIGATURE_SUBTABLE:
      PVALUE(t,symbol,"ligature", %s);
      break;
    case GX_MORT_RESERVED_SUBTABLE:
      PVALUE(t,symbol,"reserved", %s);
      break;
    case GX_MORT_NONCONTEXTUAL_SUBTABLE:
      PVALUE(t,symbol,"noncontextual", %s);
      break;
    case GX_MORT_INSERTION_SUBTABLE:
      PVALUE(t,symbol,"insertion", %s);
      break;
    default:
      PVALUE(t,symbol,"**UNKNOWN**", %s);
      break;
    }
  PCLOSE(t, subtableType);
}

static void
gx_face_dump_mort_subtable_header_coverage(GX_Face face, 
					   FT_UShort coverage, int t)
{
  POPEN(t, coverage);
  PVALUE(t,value,coverage,0x%04x);
  PVALUE(t,horiz-or-vertical,
	 coverage&GX_MORT_COVERAGE_HORIZONTAL_OR_VERTICAL_TEXT, %u);
  PVALUE(t,oreder-of-processing-glyph-array,
	 coverage&GX_MORT_COVERAGE_ORDER_OF_PROCESSING_GLYPH_ARRAY, %u);
  PVALUE(t,orientation-indepedent,
	 coverage&GX_MORT_COVERAGE_ORIENTATION_INDEPENDENT, %u);
  PVALUE(t,reserved,
	 coverage&GX_MORT_COVERAGE_RESERVED, %u);
  gx_face_dump_mort_subtable_type(face, coverage&GX_MORT_COVERAGE_SUBTABLE_TYPE, t);
  PCLOSE(t, coverage);
}

static void
gx_face_dump_mort_subtable_header(GX_Face face, GX_MetamorphosisSubtableHeader header, int t)
{
  POPEN1(t, header,
	 position,header->position,%lu);
  PFIELD(t,header,length,%u);
  gx_face_dump_mort_subtable_header_coverage(face, header->coverage, t);
  PFIELD(t,header,subFeatureFlags,%lu);
  PCLOSE(t, header);
}

static FT_Error 
gx_face_dump_mort_rearrangement_entry( GX_EntrySubtable entry_subtable,
				       FT_Pointer user )
{
  int t 	    = *(int *)user;
  char * verbString = "";
  switch (entry_subtable->flags&GX_MORT_REARRANGEMENT_FLAGS_VERB)
    {
    case GX_MORT_REARRANGEMENT_VERB_NO_CHANGE:  verbString = "NO CHANGE"; break;
#define GENCASE(x) case GX_MORT_REARRANGEMENT_VERB_##x: verbString = #x; break
      GENCASE(Ax2xA);
      GENCASE(xD2Dx);
      GENCASE(AxD2DxA);
      GENCASE(ABx2xAB);
      GENCASE(ABx2xBA);
      GENCASE(xCD2CDx);
      GENCASE(xCD2DCx);
      GENCASE(AxCD2CDxA);
      GENCASE(AxCD2DCxA);
      GENCASE(ABxD2DxAB);
      GENCASE(ABxD2DxBA);
      GENCASE(ABxCD2CDxAB);
      GENCASE(ABxCD2CDxBA);
      GENCASE(ABxCD2DCxAB);
      GENCASE(ABxCD2DCxBA);
    default: 
      fprintf(stderr, "Error in gx_face_dump_mort_rearrangement_entry\n");
      exit(1);
    };
  PVALUE5(t,flags,entry_subtable->flags,%u,
	  markFirst,entry_subtable->flags&GX_MORT_REARRANGEMENT_FLAGS_MARK_FIRST,0x%x,
	  dontAdvance,entry_subtable->flags&GX_MORT_REARRANGEMENT_FLAGS_DONT_ADVANCE,0x%x,
	  markLast,entry_subtable->flags&GX_MORT_REARRANGEMENT_FLAGS_MARK_FIRST,0x%x,
	  verb,entry_subtable->flags&GX_MORT_REARRANGEMENT_FLAGS_VERB,0x%x,
	  verbString,verbString,%s);
  return FT_Err_Ok;
}

static void
gx_face_dump_mort_rearrangement_subtable(GX_Face face,
					 GX_MetamorphosisRearrangementBody body,
					 int t)
{
  GX_StateTable state_table;
  state_table = &body->state_table;
  POPEN(t, rearrangementSubtable);
  gx_face_dump_state_table(face, state_table, 
			   gx_face_dump_mort_rearrangement_entry,
			   t);
  PCLOSE(t, rearrangementSubtable);
  
}

static FT_Error 
gx_face_dump_mort_contextual_entry( GX_EntrySubtable entry_subtable,
				    FT_Pointer user )
{
  int t = *(int *)user;
  GX_MetamorphosisContextualPerGlyph per_glyph = entry_subtable->glyphOffsets.contextual;
  
  PVALUE2(t,flags,entry_subtable->flags,%u,
	  setMark,entry_subtable->flags&GX_MORT_CONTEXTUAL_FLAGS_SET_MARK,0x%x,
	  dontAdvance,entry_subtable->flags&GX_MORT_CONTEXTUAL_FLAGS_DONT_ADVANCE,0x%x);
  
  POPEN2(t,glyphOffsets,
	 markOffset,per_glyph->markOffset,%d,      /* Was:FT_UShort, see gxtype.h. */
	 currentOffset,per_glyph->currentOffset,%d /* Was:FT_UShort, see gxtype.h. */
	 );
  PCLOSE(t,glyphOffsets);
  return FT_Err_Ok;
}

static void
gx_face_dump_mort_contextual_substitution_table( GX_Face face,
						 GX_MetamorphosisContextualSubstitutionTable substitutionTable,
						 int t)
{
  int i;
  POPEN2(t,substitutionTable,
	 offset,substitutionTable->offset,%u,
	 nGlyphIndexes,substitutionTable->nGlyphIndexes,%u);
  for( i = 0; i < substitutionTable->nGlyphIndexes; i++ )
    {
      NEWLINE10(t,i);
      fprintf(stdout, "%u[%d] ", substitutionTable->glyph_indexes[i], i);
    }
  NEWLINE();
  PCLOSE(t,substitutionTable);
}

static void
gx_face_dump_mort_contextual_subtable(GX_Face face,
				      GX_MetamorphosisContextualBody body,
				      int t)
{
  GX_StateTable state_table;
  state_table = &body->state_table;
  POPEN(t, contextualSubtable);
  gx_face_dump_state_table(face, state_table, 
			   gx_face_dump_mort_contextual_entry,
			   t);
  gx_face_dump_mort_contextual_substitution_table(face,
						  &body->substitutionTable,
						  t);
  PCLOSE(t, contextualSubtable);
}

static FT_Error 
gx_face_dump_mort_ligature_entry( GX_EntrySubtable entry_subtable,
				  FT_Pointer user )
{
  int t 	   = *(int *)user;
  FT_UShort offset = entry_subtable->flags&GX_MORT_LIGATURE_FLAGS_OFFSET;
  PVALUE3(t,flags,entry_subtable->flags,%u,
	  setComponent,entry_subtable->flags&GX_MORT_LIGATURE_FLAGS_SET_COMPONENT,0x%x,
	  dontAdvance,entry_subtable->flags&GX_MORT_LIGATURE_FLAGS_DONT_ADVANCE,0x%x,
	  offset,offset,%u);
  return FT_Err_Ok;
}

static void
gx_face_dump_mort_ligature_action_table(GX_Face face, 
					GX_MetamorphosisLigatureActionTable ligActionTable, 
					int t)
{
  int i;
  FT_ULong offset;
  FT_Long  soffset;
  FT_Long  bsoffset;
  FT_Long  nbsoffset;
  mort_tmp_ligActionTable = ligActionTable->offset;
  
  POPEN2(t,ligActionTable,
	 offset,ligActionTable->offset,%u,
	 nActions,ligActionTable->nActions,%u);
  for ( i = 0; i < ligActionTable->nActions; i++ )
    {
      offset  = ligActionTable->body[i]&GX_MORT_LIGATURE_ACTION_OFFSET;
      soffset = gx_sign_extend(offset,GX_MORT_LIGATURE_ACTION_OFFSET);
      
      PVALUE5(t,action,ligActionTable->body[i],%lu,
	      actionIndexX4,4*i,%d,
	      last,ligActionTable->body[i]&GX_MORT_LIGATURE_ACTION_LAST,0x%lx,
	      store,ligActionTable->body[i]&GX_MORT_LIGATURE_ACTION_STORE,0x%lx,
	      offset,offset,%lu,
	      offsetSE,(2*soffset),%ld);
      
      bsoffset 	= 2*(mort_tmp_firstGlyph + soffset);
      nbsoffset = 2*(mort_tmp_firstGlyph + mort_tmp_nGlyphs + soffset);

#if GX_DEBUG_MORT_LIGATURE_TABLE_LAYOUT
      /* TODO: tables order are not considered. */
      if (!(((mort_tmp_componentTable <= bsoffset) &&
	     (bsoffset < mort_tmp_ligatureTable )) ||
	    ((mort_tmp_componentTable <= nbsoffset) &&
	     (nbsoffset < mort_tmp_ligatureTable )) ||
	    ((bsoffset < mort_tmp_componentTable) &&
	     mort_tmp_ligatureTable <= nbsoffset)
	    ))
	fprintf(stderr, "range out: componentTable: %u, offset: %ld[%d+%d=%ld], ligatureTable: %u\n",
		mort_tmp_componentTable,
		bsoffset, 
		mort_tmp_firstGlyph, mort_tmp_nGlyphs, nbsoffset,
		mort_tmp_ligatureTable);
      else
	fprintf(stderr, "ok: offset: %ld[%d+%d=%ld], \n",
		bsoffset, 
		mort_tmp_firstGlyph, mort_tmp_nGlyphs, nbsoffset);
#endif /* GX_DEBUG_MORT_LIGATURE_TABLE_LAYOUT */
    }
  
  PCLOSE(t,ligActionTable);
    
}

static void
gx_face_dump_mort_component_table( GX_Face face, 
				   GX_MetamorphosisComponentTable componentTable, 
				   int t)
{
  int i;
  POPEN2(t, componentTable,
	 offset,componentTable->offset,%u,
	 nComponent,componentTable->nComponent,%u);
  for ( i = 0; i < componentTable->nComponent; i++ )
    {
      NEWLINE10(t,i);
      fprintf(stdout, "%u[%d] ", componentTable->body[i], i);
    }
  NEWLINE();
  PCLOSE(t, componentTable);
}

static void
gx_face_dump_mort_ligature_table( GX_Face face, 
				  GX_MetamorphosisLigatureTable ligatureTable, 
				  int t)
{
  int i;
  POPEN2(t, ligatureTable,
	 offset,ligatureTable->offset,%u,
	 nLigature,ligatureTable->nLigature,%u);
  for ( i = 0; i < ligatureTable->nLigature; i++ )
    {
      NEWLINE10(t,i);
      fprintf(stdout, "%u[%d] ", ligatureTable->body[i], i);
    }
  NEWLINE();
  PCLOSE(t, ligatureTable);
}

static void
gx_face_dump_mort_ligature_subtable (GX_Face face,
				     GX_MetamorphosisLigatureBody body,
				     int t)
{
  GX_StateTable state_table;
  state_table = &body->state_table;
  mort_tmp_componentTable = body->componentTable.offset;
  mort_tmp_ligatureTable = body->ligatureTable.offset;

  POPEN(t, ligatureSubtable);
  gx_face_dump_state_table(face, state_table, 
			   gx_face_dump_mort_ligature_entry, t);
  gx_face_dump_mort_ligature_action_table(face, &body->ligActionTable, t);
  gx_face_dump_mort_component_table(face, &body->componentTable, t);
  gx_face_dump_mort_ligature_table(face, &body->ligatureTable, t);
  PCLOSE(t,ligatureSubtable);
}

static void
gx_face_dump_mort_noncontextual_subtable (GX_Face face,
					  GX_MetamorphosisNoncontextualBody body,
					  int t)
{
#if 1
  gx_face_dump_LookupTable_low(face, 
			       &body->lookup_table,
			       &mort_dump_lookup_table_funcs,
			       t);
#else 
  gx_face_dump_LookupTable_high( face,
				 &body->lookup_table,
				 &mort_dump_lookup_table_func,
				 t );
#endif /* 0 */
}

static FT_Error 
gx_face_dump_mort_insertion_entry ( GX_EntrySubtable entry_subtable,
				    FT_Pointer user )
{
  int t = *(int *)user;
  int i;
  GX_MetamorphosisInsertionPerGlyph per_glyph = entry_subtable->glyphOffsets.insertion;
  GX_MetamorphosisInsertionList currentInsertList = &per_glyph->currentInsertList;
  GX_MetamorphosisInsertionList markedInsertList  = &per_glyph->markedInsertList;

  FT_UShort current_count = gx_mask_zero_shift(entry_subtable->flags, 
					       GX_MORT_INSERTION_FLAGS_CURRENT_INSERT_COUNT);
  FT_UShort marked_count = gx_mask_zero_shift(entry_subtable->flags, 
					      GX_MORT_INSERTION_FLAGS_MARKED_INSERT_COUNT);

  POPEN1(t,flags,value,entry_subtable->flags,%u);
  PVALUE(t,setMark,entry_subtable->flags&GX_MORT_INSERTION_FLAGS_SET_MARK,%u);
  PVALUE(t,dontAdvance,entry_subtable->flags&GX_MORT_INSERTION_FLAGS_DONT_ADVANCE,%u);
  PVALUE(t,currentIsKashidaLike,entry_subtable->flags&GX_MORT_INSERTION_FLAGS_CURRENT_IS_KASHIDA_LIKE,%u);
  PVALUE(t,markedIsKashidaLike,entry_subtable->flags&GX_MORT_INSERTION_FLAGS_MARKED_IS_KASHIDA_LIKE,%u);
  PVALUE(t,currentInsertBefore,entry_subtable->flags&GX_MORT_INSERTION_FLAGS_CURRENT_INSERT_BEFORE,%u);
  PVALUE(t,markedInsertBefore,entry_subtable->flags&GX_MORT_INSERTION_FLAGS_MARKED_INSERT_BEFORE,%u);
  PVALUE(t,currentInsertCount,entry_subtable->flags&GX_MORT_INSERTION_FLAGS_CURRENT_INSERT_COUNT,%u);
  PVALUE(t,markedInsertCount,entry_subtable->flags&GX_MORT_INSERTION_FLAGS_MARKED_INSERT_COUNT,%u);
  PCLOSE(t,flags);
  
  POPEN1(t,currentInsertList,offset,currentInsertList->offset,%u);
  for ( i = 0; i < current_count; i++ )
    {
      NEWLINE10(t,i);
      fprintf(stdout, "%u[%d] ", currentInsertList->glyphcodes[i], i);
    }
  NEWLINE();
  PCLOSE(t,currentInsertList);
  
  POPEN1(t,markedInsertList,offset,markedInsertList->offset,%u);
  for ( i = 0; i < marked_count; i++ )
    {
      NEWLINE10(t,i);
      fprintf(stdout, "%u[%d] ", markedInsertList->glyphcodes[i], i);
    }
  NEWLINE();
  PCLOSE(t,markedInsertList);
  
  return FT_Err_Ok;
}

static void
gx_face_dump_mort_insertion_subtable (GX_Face face,
				      GX_MetamorphosisInsertionBody body,
				      int t)
{
  GX_StateTable state_table;
  state_table = &body->state_table;
  POPEN(t, insertionSubtable);
  gx_face_dump_state_table(face, state_table, 
			   gx_face_dump_mort_insertion_entry, t);
  PCLOSE(t, insertionSubtable);
}

static void
gx_face_dump_mort_subtable(GX_Face face, GX_MetamorphosisSubtable subtable, 
			   FT_UShort nSubtables, int t)
{
  GX_MetamorphosisSubtable psubtable;
  int n;
  POPEN(t, SubTables);
  for ( n = 0; n < nSubtables; n++ )
    {
      POPEN(t, chainSubtable);
      psubtable = &subtable[n];
      gx_face_dump_mort_subtable_header(face, &psubtable->header, t);
      switch(psubtable->header.coverage&GX_MORT_COVERAGE_SUBTABLE_TYPE)
	{
	case GX_MORT_REARRANGEMENT_SUBTABLE:
	  gx_face_dump_mort_rearrangement_subtable(face, 
						   psubtable->body.rearrangement,
						   t);
	  break;
	case GX_MORT_CONTEXTUAL_SUBTABLE:
	  gx_face_dump_mort_contextual_subtable(face, 
						psubtable->body.contextual,
						t);
	  break;
	case GX_MORT_LIGATURE_SUBTABLE:
	  gx_face_dump_mort_ligature_subtable(face, 
					      psubtable->body.ligature,
					      t);
	  break;
	case GX_MORT_RESERVED_SUBTABLE:
	  PVALUE(t,error,"**RESERVED**", %s);
	  break;
	case GX_MORT_NONCONTEXTUAL_SUBTABLE:
	  gx_face_dump_mort_noncontextual_subtable(face,
						   psubtable->body.noncontextual,
						   t);
	  break;
	case GX_MORT_INSERTION_SUBTABLE:
	  gx_face_dump_mort_insertion_subtable(face, 
					       psubtable->body.insertion,
					       t);
	  break;
	default:
	  PVALUE(t,error,"**UNKNOWN**", %s);
	  break;
	}
      PCLOSE(t, chainSubtable);
    }
  PCLOSE(t, SubTables);
}

static void
gx_face_dump_mort_chain_header(GX_Face face, GX_MetamorphosisChainHeader header, int t)
 {
   POPEN(t, header);
   PFIELD(t,header,defaultFlags,%lu);
   PFIELD(t,header,chainLength,%lu);
   PFIELD(t,header,nFeatureEntries,%d);
   PFIELD(t,header,nSubtables,%d);
   PCLOSE(t,header);
 }

static void
gx_face_dump_mort_feature_table(GX_Face face,
				GX_MetamorphosisFeatureTable tbl,
				FT_UShort nFeatureEntries,
				int t)
{
  GX_MetamorphosisFeatureTable ptbl;
  int n;
  
  POPEN(t, FeatureTables);
  for ( n = 0; n < nFeatureEntries; n++ )
    {
      ptbl = &tbl[n];
      POPEN(t, FeatureTable);
      PFIELD(t, ptbl, featureType, %u);
      
      PFIELD(t, ptbl, featureSetting, %u);
      PFIELD(t, ptbl, enableFlags, 0x%08lx);
      PFIELD(t, ptbl, disableFlags, 0x%08lx);
      PCLOSE(t, FeatureTable);
    }
  PCLOSE(t, FeatureTables);
}

static void
gx_face_dump_mort_chain(GX_Face face, GX_MetamorphosisChain chain, int t)
{
  POPEN(t,chain);
  gx_face_dump_mort_chain_header(face, &chain->header, t);
  gx_face_dump_mort_feature_table(face, chain->feat_Subtbl, chain->header.nFeatureEntries, t);
  gx_face_dump_mort_subtable(face, chain->chain_Subtbl, chain->header.nSubtables, t);
  PCLOSE(t,chain);
}

void
gx_face_dump_mort(GX_Face face, GX_Mort mort)
{
  int i, t = 2;
  PFIELD(t,mort,version,0x%08lx);
  PFIELD(t,mort,nChains,%lu);
  POPEN(t, chains);
  for ( i = 0; i < mort->nChains; i++ )
    gx_face_dump_mort_chain(face, &mort->chain[i], t);
  PCLOSE(t, chains);
}

/******************************FMTX************************************/
void
gx_face_dump_fmtx(GX_Face face, GX_Fmtx fmtx)
{
  int t = 2;
  PFIELD(t,fmtx,version,0x%08lx);
  PFIELD(t,fmtx,glyphIndex,%lu);
  PFIELD(t,fmtx,horizontalBefore,%u);
  PFIELD(t,fmtx,horizontalAfter,%u);
  PFIELD(t,fmtx,horizontalCaretHead,%u);
  PFIELD(t,fmtx,horizontalCaretBase,%u);
  PFIELD(t,fmtx,verticalBefore,%u);
  PFIELD(t,fmtx,verticalAfter,%u);
  PFIELD(t,fmtx,verticalCaretHead,%u);
  PFIELD(t,fmtx,verticalCaretBase,%u);
}

/******************************FDSC************************************/
void
gx_face_dump_fdsc(GX_Face face, GX_Fdsc fdsc)
{
  int i, t = 2;
  GX_FontDescriptor desc;
  char * string_tag = NULL;
  
  PFIELD(t,fdsc,version,0x%08lx);
  PFIELD(t,fdsc,descriptorCount,%lu);
  POPEN(t, descriptors);
  for ( i = 0; i < fdsc->descriptorCount; i++ )
    {
      desc = &(fdsc->descriptor[i]);
      switch ( desc->tag )
	{
	case TTAG_wght:
	  string_tag = "wght";
	  break;
	case TTAG_wdth:
	  string_tag = "wdth";
	  break;
	case TTAG_slnt:
	  string_tag = "slnt";
	  break;
	case TTAG_opsz:
	  string_tag = "opsz";
	  break;
	case TTAG_nalf:
	  string_tag = "nalf";
	  break;
	}
      POPEN(t, descriptor);
      if ( string_tag )
	PVALUE(t,tag,string_tag,%s);
      else
	PVALUE(t,tag,desc->tag,%lx);
      PFIELD(t,desc,value,%lu);
      PCLOSE(t, descriptor);
    }
  PCLOSE(t, descriptors);
}



/******************************MORX************************************/
#define gx_face_dump_morx_subtable_type          gx_face_dump_mort_subtable_type
#define gx_face_dump_morx_feature_table          gx_face_dump_mort_feature_table
#define gx_face_dump_morx_noncontextual_subtable gx_face_dump_mort_noncontextual_subtable
#define gx_face_dump_morx_rearrangement_entry    gx_face_dump_mort_rearrangement_entry
#define gx_face_dump_morx_insertion_entry        gx_face_dump_mort_insertion_entry
#define morx_dump_lookup_table_func              mort_dump_lookup_table_func 
static void
gx_face_dump_morx_chain_header(GX_Face face, GX_XMetamorphosisChainHeader header, int t)
{
   POPEN(t,header);
   PFIELD(t,header,defaultFlags,%lu);
   PFIELD(t,header,chainLength,%lu);
   PFIELD(t,header,nFeatureEntries,%lu);
   PFIELD(t,header,nSubtables,%lu);
   PCLOSE(t,header);
}

static void
gx_face_dump_morx_subtable_header_coverage( GX_Face face,
					    FT_ULong coverage,
					    int t )
{
  POPEN(t, coverage);
  PVALUE(t,value,coverage,0x%08lx);
  PVALUE(t,horiz-or-vertical,
	 coverage&GX_MORX_COVERAGE_HORIZONTAL_OR_VERTICAL_TEXT, %lu);
  PVALUE(t,oreder-of-processing-glyph-array,
	 coverage&GX_MORX_COVERAGE_ORDER_OF_PROCESSING_GLYPH_ARRAY, %lu);
  PVALUE(t,orientation-indepedent,
	 coverage&GX_MORX_COVERAGE_ORIENTATION_INDEPENDENT, %lu);
  PVALUE(t,reserved,
	 coverage&GX_MORX_COVERAGE_RESERVED, %lu);
  gx_face_dump_morx_subtable_type(face, coverage&GX_MORX_COVERAGE_SUBTABLE_TYPE, t);
  PCLOSE(t, coverage);
}

static void
gx_face_dump_morx_subtable_header(GX_Face face, 
				  GX_XMetamorphosisSubtableHeader header, int t)
{
  POPEN1(t, header,
	 position,header->position,%lu);
  PFIELD(t,header,length,%lu);
  gx_face_dump_morx_subtable_header_coverage(face, header->coverage, t);
  PFIELD(t,header,subFeatureFlags,%lu);
  PCLOSE(t, header);
}

static void
gx_face_dump_morx_rearrangement_subtable( GX_Face face,
					  GX_XMetamorphosisRearrangementBody body,
					  int t )
{
  GX_XStateTable state_table;
  state_table = &body->state_table;
  POPEN(t, rearrangementSubtable);
  gx_face_dump_xstate_table(face, state_table,
			    gx_face_dump_morx_rearrangement_entry,
			    t);
  PCLOSE(t, rearrangementSubtable);
}

static FT_Error 
gx_face_dump_morx_contextual_entry( GX_EntrySubtable entry_subtable,
				    FT_Pointer user )
{
  int t = *(int *)user;
  GX_XMetamorphosisContextualPerGlyph per_glyph = entry_subtable->glyphOffsets.xcontextual;
  
  PVALUE2(t,flags,entry_subtable->flags,%u,
	  setMark,entry_subtable->flags&GX_MORX_CONTEXTUAL_FLAGS_SET_MARK,0x%x,
	  dontAdvance,entry_subtable->flags&GX_MORX_CONTEXTUAL_FLAGS_DONT_ADVANCE,0x%x);
  if ( !per_glyph )
    POPEN(t,glyphIndexes);
  else if ( per_glyph->markIndex == GX_MORX_NO_SUBSTITUTION )
    {
      if ( per_glyph->currentIndex == GX_MORX_NO_SUBSTITUTION )
	POPEN2(t,glyphIndexes,
	       markIndex,"no substitution",%s,
	       currentInsert,"no substitution",%s);
      else
	POPEN2(t,glyphIndexes,
	       markIndex,"no substitution",%s,
	       currentIndex,per_glyph->currentIndex,%u);
    }
  else if ( per_glyph->currentIndex == GX_MORX_NO_SUBSTITUTION )
    POPEN2(t,glyphIndexes,
	   markIndex,per_glyph->markIndex,%u,
	   currentInsert,"no substitution",%s);
  else
    POPEN2(t,glyphIndexes,
	   markIndex,per_glyph->markIndex,%u,
	   currentIndex,per_glyph->currentIndex,%u);
  PCLOSE(t,glyphIndexes);
  return FT_Err_Ok;
}


static void
gx_face_dump_morx_contextual_substitution_table( GX_Face face,
						 GX_XMetamorphosisContextualSubstitutionTable substitutionTable,
						 int t)
{
  FT_ULong i;
  POPEN2(t,substitutionTable,
	 offset,substitutionTable->offset,%lu,
	 nTables,substitutionTable->nTables,%u);
  for( i = 0; i < substitutionTable->nTables; i++ )
    {
      POPEN1(t,lookupTables,index,i,%lu);
      if ( substitutionTable->lookupTables[i] )
	{
	  gx_face_dump_LookupTable_low ( face,
					 substitutionTable->lookupTables[i],
					 NULL, /* use default */
					 t );
	}
      PCLOSE(t,lookupTables);
    }
  PCLOSE(t,substitutionTable);
}

static void
gx_face_dump_morx_contextual_subtable(GX_Face face, 
				      GX_XMetamorphosisContextualBody body,
				      int t)
{
  GX_XStateTable state_table;
  state_table = &body->state_table;
  POPEN(t, contextualSubtable);
  gx_face_dump_xstate_table(face, state_table, 
			    gx_face_dump_morx_contextual_entry,
			    t); 
  gx_face_dump_morx_contextual_substitution_table(face,
						  &body->substitutionTable,
						  t);
  PCLOSE(t, contextualSubtable);
}

static FT_Error 
gx_face_dump_morx_ligature_entry( GX_EntrySubtable entry_subtable,
				  FT_Pointer user )
{
  int t 	   		     = *(int *)user;
  GX_EntrySubtablePerGlyph per_glyph = &entry_subtable->glyphOffsets;
  PVALUE4(t,flags,entry_subtable->flags,%u,
	  setComponent,entry_subtable->flags&GX_MORX_LIGATURE_FLAGS_SET_COMPONENT,0x%x,
	  dontAdvance,entry_subtable->flags&GX_MORX_LIGATURE_FLAGS_DONT_ADVANCE,0x%x,
	  performAction,entry_subtable->flags&GX_MORX_LIGATURE_FLAGS_PERFORM_ACTION,0x%x,
	  ligActionIndex,per_glyph->ligActionIndex,%u);
  return FT_Err_Ok;
}

static void
gx_face_dump_morx_ligature_action_table(GX_Face face, 
					GX_XMetamorphosisLigatureActionTable ligActionTable, 
					int t)
{
  int i;
  FT_Long  index;
  FT_Long  sindex;

  FT_ULong  bsindex;
  FT_ULong  nbsindex;
  FT_ULong  area;
  
  POPEN2(t,ligActionTable,
	 offset,ligActionTable->offset,%lu,
	 nActions,ligActionTable->nActions,%u);
  for ( i = 0; i < ligActionTable->nActions; i++ )
    {
      index  =  ligActionTable->body[i]&GX_MORX_LIGATURE_ACTION_OFFSET;
      sindex =  gx_sign_extend(index,GX_MORX_LIGATURE_ACTION_OFFSET);
      
      PVALUE5(t,action,ligActionTable->body[i],%lu,
	      actionIndex,i,%d,
	      last,ligActionTable->body[i]&GX_MORX_LIGATURE_ACTION_LAST,0x%lx,
	      store,ligActionTable->body[i]&GX_MORX_LIGATURE_ACTION_STORE,0x%lx,
	      index,index,%lu,
	      indexSE,sindex,%ld);
      
      bsindex 	= (morx_tmp_firstGlyph + sindex);
      nbsindex = (morx_tmp_firstGlyph + morx_tmp_nGlyphs + sindex);
      area = morx_tmp_nLigature;
#if GX_DEBUG_MORX_LIGATURE_TABLE_LAYOUT
      if ( area < bsindex )
	1 && fprintf ( stderr, "[MAX: %d I: %d]range out[bsindex is too large: %lu > area: %lu]\n",
		       ligActionTable->nActions, i,
		       bsindex, area);
      else
	1 && fprintf ( stderr, "[MAX: %d I: %d]ok[bsindex: %lu nbsindex: %lu area: %lu]\n", 
		  ligActionTable->nActions, i,
		  bsindex, nbsindex, area);
#endif /* GX_DEBUG_MORX_LIGATURE_TABLE_LAYOUT */
    }
  
  PCLOSE(t,ligActionTable);
}

static void
gx_face_dump_morx_component_table( GX_Face face, 
				   GX_XMetamorphosisComponentTable componentTable, 
				   int t)
{
  int i;
  POPEN2(t, componentTable,
	 offset,componentTable->offset,%lu,
	 nComponent,componentTable->nComponent,%u);
  for ( i = 0; i < componentTable->nComponent; i++ )
    {
      NEWLINE10(t,i);
      fprintf(stdout, "%u[%d] ", componentTable->body[i], i);
    }
  NEWLINE();
  PCLOSE(t, componentTable);
}

static void
gx_face_dump_morx_ligature_table( GX_Face face, 
				  GX_XMetamorphosisLigatureTable ligatureTable, 
				  int t)
{
  int i;
  POPEN2(t, ligatureTable,
	 offset,ligatureTable->offset,%lu,
	 nLigature,ligatureTable->nLigature,%u);
  for ( i = 0; i < ligatureTable->nLigature; i++ )
    {
      NEWLINE10(t,i);
      fprintf(stdout, "%u[%d] ", ligatureTable->body[i], i);
    }
  NEWLINE();
  PCLOSE(t, ligatureTable);
}

static void
gx_face_dump_morx_ligature_subtable( GX_Face face,
				     GX_XMetamorphosisLigatureBody body,
				     int t )
{
  GX_XStateTable state_table;
  state_table = &body->state_table;
  
  morx_tmp_nLigature 	 = body->ligatureTable.nLigature;
  morx_tmp_ligatureTable = body->ligatureTable.offset;
  
  POPEN(t, ligatureSubtable);
  gx_face_dump_xstate_table(face, state_table, 
			    gx_face_dump_morx_ligature_entry,
			    t);
  gx_face_dump_morx_ligature_action_table(face, &body->ligActionTable, t);
  gx_face_dump_morx_component_table      (face, &body->componentTable, t);
  gx_face_dump_morx_ligature_table       (face, &body->ligatureTable,  t);  
  PCLOSE(t, ligatureSubtable);
}  

static void
gx_face_dump_morx_insertion_subtable ( GX_Face face,
				       GX_XMetamorphosisInsertionBody body,
				       int t )
{
  GX_XStateTable state_table;
  state_table = &body->state_table;
  POPEN(t, insertionSubtable);
  gx_face_dump_xstate_table(face, state_table, 
			    gx_face_dump_morx_insertion_entry, 
			    t);
  PCLOSE(t, insertionSubtable);
}

static void
gx_face_dump_morx_subtable(GX_Face face, GX_XMetamorphosisSubtable subtable,
			   FT_ULong nSubtables, int t)
{
  GX_XMetamorphosisSubtable psubtable;
  int n;
  POPEN(t, SubTables);
  for ( n = 0; n < nSubtables; n++ )
    {
      POPEN(t, chainSubtable);
      psubtable = &subtable[n];
      gx_face_dump_morx_subtable_header(face, &psubtable->header, t);
      switch(psubtable->header.coverage&GX_MORX_COVERAGE_SUBTABLE_TYPE)
	{
	case GX_MORX_REARRANGEMENT_SUBTABLE:
	  gx_face_dump_morx_rearrangement_subtable(face, 
						   psubtable->body.rearrangement,
						   t);
	  break;
	case GX_MORX_CONTEXTUAL_SUBTABLE:
	  gx_face_dump_morx_contextual_subtable(face, 
						psubtable->body.contextual,
						t);
	  break;
	case GX_MORX_LIGATURE_SUBTABLE:
	  gx_face_dump_morx_ligature_subtable(face, 
					      psubtable->body.ligature,
					      t);
	  break;
	case GX_MORX_RESERVED_SUBTABLE:
	  PVALUE(t,error,"**RESERVED**", %s);
	  break;
	case GX_MORX_NONCONTEXTUAL_SUBTABLE:
	  gx_face_dump_morx_noncontextual_subtable ( face,
						     psubtable->body.noncontextual,
						     t );
	  break;
	case GX_MORX_INSERTION_SUBTABLE:
	  gx_face_dump_morx_insertion_subtable(face, 
					       psubtable->body.insertion,
					       t);
	  break;
	default:
	  PVALUE(t,error,"**UNKNOWN**", %s);
	  break;
	}
      PCLOSE(t, chainSubtable);
    }
  PCLOSE(t, SubTables);
}

static void
gx_face_dump_morx_chain(GX_Face face, GX_XMetamorphosisChain chain, int t)
{
  POPEN(t,chain);
  gx_face_dump_morx_chain_header(face, &chain->header, t);
  gx_face_dump_morx_feature_table(face, chain->feat_Subtbl, chain->header.nFeatureEntries, t);
  gx_face_dump_morx_subtable(face, chain->chain_Subtbl, chain->header.nSubtables, t);
  PCLOSE(t,chain);
}

void
gx_face_dump_morx(GX_Face face, GX_Morx morx)
{
  int i, t = 2;
  PFIELD(t,morx,version,0x%08lx);
  PFIELD(t,morx,nChains,%lu);
  POPEN(t, chains);
  for ( i = 0; i < morx->nChains; i++ )
    gx_face_dump_morx_chain(face, &morx->chain[i], t);
  PCLOSE(t, chains);
}



/******************************JUST************************************/
void
gx_face_dump_just(GX_Face face, GX_Just just)
{
  int t = 2;
  PFIELD(t, just, version, 0x%08lx);
  PFIELD(t, just, format, %u);
  PFIELD(t, just, horizOffset, %u);
  PFIELD(t, just, vertOffset, %u);
}


/******************************KERN************************************/
void
gx_face_dump_kern_subtable_header_coverage(GX_Face face,
					   FT_UShort coverage, int t)
{
  POPEN(t, coverage);
  PVALUE(t, value, coverage, 0x%04x);
  PVALUE(t, vertical, coverage&GX_KERN_COVERAGE_VERTICAL, %u);
  PVALUE(t, corss-stream, coverage&GX_KERN_COVERAGE_CROSS_STREAM, %u);
  PVALUE(t, variation, coverage&GX_KERN_COVERAGE_VARIATION, %u);
  PVALUE(t, format, coverage&GX_KERN_COVERAGE_FORMAT_MASK, %u);
  PCLOSE(t, coverage);
}
void
gx_face_dump_kern_sutable_header(GX_Face face, GX_KerningSubtableHeader header, int t)
{
  POPEN(t, header);
  PFIELD(t, header, length, %lu);
  gx_face_dump_kern_subtable_header_coverage(face, header->coverage, t);
  PFIELD(t, header, tupleIndex, %u);
  PCLOSE(t, header);
}

void
gx_face_dump_kern_fmt0_subtable(GX_Face face, GX_KerningSubtableFormat0Body fmt0, int t)
{
  int i;
  POPEN(t, fmt0);
  PFIELD(t, fmt0, nPairs, %u);
  PFIELD(t, fmt0, searchRange, %u);
  PFIELD(t, fmt0, entrySelector, %u);
  PFIELD(t, fmt0, rangeShift, %u);
  POPEN (t, pairs);
  for ( i = 0; i < fmt0->nPairs; i++ )
    PVALUE3(t,
	    value,fmt0->entries[i].value,%d,
	    index,i,%d,
	    left,fmt0->entries[i].left,%u,
	    right,fmt0->entries[i].right,%u);
  PCLOSE(t, pairs);
  PCLOSE(t, fmt0);
}


static FT_Error 
gx_face_dump_kern_fmt1_entry( GX_EntrySubtable entry_subtable,
			      FT_Pointer user )
{
  int t = *(int *)user;
  POPEN1(t,flags,value,entry_subtable->flags,%u);
  PVALUE(t,push,entry_subtable->flags&GX_KERN_ACTION_PUSH,%u);
  PVALUE(t,dontAdvance,entry_subtable->flags&GX_KERN_ACTION_DONT_ADVANCE,%u);
  PVALUE(t,valueOffset,entry_subtable->flags&GX_KERN_ACTION_VALUE_OFFSET,%u);
  PCLOSE(t,flags);
  return FT_Err_Ok;
}

void
gx_face_dump_kern_fmt1_subtable( GX_Face face, GX_KerningSubtableFormat1Body fmt1, int t)
{
  FT_ULong i;
  POPEN (t, fmt1);
  gx_face_dump_state_table ( face, &fmt1->state_table, 
			     gx_face_dump_kern_fmt1_entry, t);
  PFIELD(t,fmt1,valueTable,%u);
  PFIELD(t,fmt1,value_absolute_pos,%lu);
  PFIELD(t,fmt1,nValues,%lu);
  POPEN2(t, values,
	 absolutePosition, fmt1->value_absolute_pos, %lu,
	 nValues, fmt1->nValues, %lu );
  for ( i = 0; i < fmt1->nValues; i++ )
    {
      NEWLINE10(t,i);
      fprintf(stdout, "%u[%lu] ", fmt1->values[i], i);
    }
  NEWLINE();
  PCLOSE(t,values);
  PCLOSE (t, fmt1);
}

void
gx_face_dump_kern_fmt2_class_table ( GX_Face face, 
				     GX_KerningSubtableFormat2ClassTable class_table, int t)
{
  int i;
  PFIELD(t, class_table, firstGlyph, %u );
  PFIELD(t, class_table, nGlyphs, %u );
  POPEN1(t, classes, max, class_table->max_class,%d);
  for ( i = 0; i < class_table->nGlyphs; i++ )
    {
      NEWLINE10(t,i);
      fprintf(stdout, "%u[%d] ", class_table->classes[i], i);
    }
  NEWLINE();
  PCLOSE(t, classes);
}

void
gx_face_dump_kern_fmt2_subtable( GX_Face face, GX_KerningSubtableFormat2Body fmt2, int t)
{
  int i;
  POPEN(t, fmt2);
  PFIELD(t, fmt2, rowWidth, %u);
  PFIELD(t, fmt2, leftClassTable, %u);
  PFIELD(t, fmt2, rightClassTable, %u);
  PFIELD(t, fmt2, array, %u);
  POPEN(t, leftClass);
  gx_face_dump_kern_fmt2_class_table( face, &fmt2->leftClass, t );
  PCLOSE(t, leftClass);
  POPEN(t, rightClass);
  gx_face_dump_kern_fmt2_class_table( face, &fmt2->rightClass, t );
  PCLOSE(t, rightClass);
  
  POPEN(t, values);
  for ( i = 0; i < fmt2->leftClass.max_class + fmt2->rightClass.max_class; i++ )
    {
      NEWLINE10(t, i);
      fprintf(stdout, "%d[%d] ", fmt2->values[i], i);
    }
  NEWLINE();
  PCLOSE(t, values);  
  PCLOSE(t, fmt3);
}

void
gx_face_dump_kern_fmt3_subtable( GX_Face face, GX_KerningSubtableFormat3Body fmt3, int t)
{
  int i;
  POPEN(t, fmt3);
  PFIELD(t, fmt3, glyphCount, %u);
  PFIELD(t, fmt3, kernValueCount, %u);
  PFIELD(t, fmt3, leftClassCount, %u);
  PFIELD(t, fmt3, rightClassCount, %u);
  PFIELD(t, fmt3, flags, %u);
  
  POPEN(t, kernValue);
  for ( i = 0; i < fmt3->kernValueCount; i++ )
    {
      NEWLINE10(t, i);
      fprintf(stdout, "%d[%d] ", fmt3->kernValue[i], i);
    }
  NEWLINE();
  PCLOSE(t,kernValue);

  POPEN(t, leftClass);
  for ( i = 0; i < fmt3->glyphCount; i++ )
    {
      NEWLINE10(t, i);
      fprintf(stdout, "%u[%d] ", fmt3->leftClass[i], i);
    }
  NEWLINE();
  PCLOSE(t,leftClass);
  
  POPEN(t, rightClass);
  for ( i = 0; i < fmt3->glyphCount; i++ )
    {
      NEWLINE10(t, i);
      fprintf(stdout, "%u[%d] ", fmt3->rightClass[i], i);
    }
  NEWLINE();
  PCLOSE(t,rightClass);
  
  POPEN(t, kernIndex);
  for ( i = 0; i < fmt3->leftClassCount * fmt3->rightClassCount ; i++ )
    {
      NEWLINE10(t, i);
      fprintf(stdout, "%u[%d] ", fmt3->kernIndex[i], i);
    }
  NEWLINE();
  PCLOSE(t,kernIndex);
  PCLOSE(t, fmt3);
}

void
gx_face_dump_kern_sutable(GX_Face face, GX_KerningSubtable subtable, int t)
{
  GX_KerningFormat format = subtable->header.coverage&GX_KERN_COVERAGE_FORMAT_MASK;
  POPEN(t, subtable);
  gx_face_dump_kern_sutable_header(face, &subtable->header, t);
  switch ( format )
    {
    case GX_KERN_FMT_ORDERED_LIST_OF_KERNING_PAIRS:
      gx_face_dump_kern_fmt0_subtable(face, subtable->body.fmt0, t);
      break;
    case GX_KERN_FMT_STATE_TABLE_FOR_CONTEXTUAL_KERNING:
      gx_face_dump_kern_fmt1_subtable(face, subtable->body.fmt1, t);
      break;
    case GX_KERN_FMT_SIMPLE_NXM_ARRAY_OF_KERNING_VALUES:
      gx_face_dump_kern_fmt2_subtable(face, subtable->body.fmt2, t);
      break;
    case GX_KERN_FMT_SIMPLE_NXM_ARRAY_OF_KERNING_INDICES:
      gx_face_dump_kern_fmt3_subtable(face, subtable->body.fmt3, t);
      break;
    }
  PCLOSE(t, subtable);
}
void
gx_face_dump_kern(GX_Face face, GX_Kern kern)
{
  int i, t = 2;
  PFIELD(t, kern, version, 0x%08lx);
  PFIELD(t, kern, nTables, %lu);
  POPEN(t, subtables);
  for ( i = 0; i < kern->nTables; i++ )
    gx_face_dump_kern_sutable(face, &kern->subtables[i], t);
  PCLOSE(t, subtables);
}


/****************************State***********************************/
static void
gx_face_dump_state_header(GX_Face face, GX_StateHeader header, int t)
{
  POPEN(t,header);
  PFIELD(t,header,position,%lu);
  PFIELD(t,header,stateSize,%u);
  PFIELD(t,header,classTable,%u);
  PFIELD(t,header,stateArray,%u);
  PFIELD(t,header,entryTable,%u);
  PCLOSE(t,header);
}

static void
gx_face_dump_class_subtable(GX_Face face, GX_ClassSubtable subtbl, FT_UShort stateSize, int t)
{
  int i;
  POPEN(t,classSubtable);
  PFIELD(t,subtbl,firstGlyph,%u);
  mort_tmp_firstGlyph = subtbl->firstGlyph;
  PFIELD(t,subtbl,nGlyphs,%u);
  mort_tmp_nGlyphs = subtbl->nGlyphs;
  POPEN(t,classArray);
  for ( i = 0; i < subtbl->nGlyphs; i++ )
    {
      NEWLINEX(t,i,stateSize);
      fprintf(stdout, "%u[%d] ", subtbl->classArray[i], i);
    }
  NEWLINE();
  PCLOSE(t,classArray);
  PCLOSE(t,classSubtable);
}

static void
gx_face_dump_entry_table( GX_Face face, FT_Byte nEntries, GX_EntrySubtable subtbl, 
			  GX_StateTable_Entry_Action action, int t)
{
  int i;
  POPEN1(t,EntrySubtables, nEntries,nEntries,%u);
  for ( i = 0; i < nEntries; i++ )
    {
      POPEN(t,EntrySubtable);
      PFIELD(t,(&(subtbl[i])),newState,%u);
      if (action)
	action((&(subtbl[i])), &t);
      else
	PFIELD(t,(&(subtbl[i])),flags,%u);
      PCLOSE(t,EntrySubtable);
    }
  PCLOSE(t,EntrySubtables);
}

static void
gx_face_dump_state_array(GX_Face face,
			 FT_ULong nStates, FT_UShort state_size, 
			 FT_Byte * state_array, FT_UShort start, int t)
{
  int i, j;
  FT_Byte * row;
  POPEN(t, stateArray);
  for ( i = 0; i < nStates; i++ )
    {
      row = state_array + (i * state_size);
      POPEN2(t,row,
	     state,i,%d,
	     offset,start+(i * state_size),%d);
      for (j = 0; j < state_size; j++)
	{
	  NEWLINE10(t, j);
	  fprintf(stdout, "%u[%d] ", row[j], j);
	}
      NEWLINE();
      PCLOSE(t,row);
    }
  PCLOSE(t, stateArray);
}
static void
gx_face_dump_state_table ( GX_Face face, 
			   GX_StateTable state_table,
			   GX_StateTable_Entry_Action action,
			   int t )
{
  POPEN(t, stateTable);
  gx_face_dump_state_header(face, &state_table->header, t);
  gx_face_dump_class_subtable(face,&state_table->class_subtable,
			      state_table->header.stateSize, t);
  gx_face_dump_state_array(face,
			   state_table->nStates, state_table->header.stateSize,
			   state_table->state_array,
			   state_table->header.stateArray,
			   t);
  gx_face_dump_entry_table(face,state_table->nEntries,state_table->entry_subtable, 
			   action, t);
  PCLOSE(t, stateTable);
}


/****************************XState***********************************/
static void
gx_face_dump_xstate_header(GX_Face face, GX_XStateHeader header, int t)
{
  POPEN(t,header);
  PFIELD(t,header,position,%lu);
  PFIELD(t,header,nClasses,%lu);
  PFIELD(t,header,classTableOffset,%lu);
  PFIELD(t,header,stateArrayOffset,%lu);
  PFIELD(t,header,entryTableOffset,%lu);
  PCLOSE(t,header);
}

static void
gx_face_dump_xstate_array( GX_Face face,
			   FT_ULong nStates, FT_ULong nClasses,
			   FT_UShort * state_array, int t )
{
  unsigned long i, j;
  FT_UShort * row;
  POPEN(t, stateArray);
  for ( i = 0; i < nStates; i++ )
    {
      row = state_array + (i * nClasses);
      POPEN1(t,row,
	     state,i,%lu);
      INDENT(t);
      for (j = 0; j < nClasses; j++)
	fprintf(stdout, "%u[%lu] ", row[j], j);
      NEWLINE();
      PCLOSE(t,row);
    }
  PCLOSE(t, stateArray);
}

static FT_Error
tmp_morx_simple_array_count_glyph( GX_LookupTable_Format format,
				   FT_UShort index,
				   GX_LookupValue value,
				   FT_Pointer user )
{
  if ( morx_tmp_firstGlyph == 0 )
    morx_tmp_firstGlyph = index;
  if ( morx_tmp_firstGlyph > index )
    morx_tmp_firstGlyph = index;
  if ( ( index - morx_tmp_firstGlyph ) > morx_tmp_nGlyphs )
    morx_tmp_nGlyphs = ( index - morx_tmp_firstGlyph );
  return FT_Err_Ok;
}

static FT_Error
tmp_morx_segment_single_count_glyph( GX_LookupTable_Format format,
				    FT_UShort lastGlyph,
				    FT_UShort firstGlyph,
				    GX_LookupValue value,
				    FT_Pointer user )
{
  morx_tmp_firstGlyph = firstGlyph;
  morx_tmp_nGlyphs = lastGlyph - firstGlyph;
  return FT_Err_Ok;
}

static FT_Error
tmp_morx_segment_array_count_glyph( GX_LookupTable_Format format,
				    FT_UShort lastGlyph,
				    FT_UShort firstGlyph,
				    GX_LookupValue value,
				    FT_Pointer user )
{
  morx_tmp_firstGlyph = firstGlyph;
  morx_tmp_nGlyphs = lastGlyph - firstGlyph;
  return FT_Err_Ok;
}

static FT_Error
tmp_morx_single_table_count_glyph( GX_LookupTable_Format format,
			     FT_UShort glyph,
			     GX_LookupValue value,
			     FT_Pointer user )
{
  /* fprintf(stderr, "format 6: glyph: %u\n", glyph ); */
  if ( morx_tmp_firstGlyph == 0 )
    morx_tmp_firstGlyph = glyph;
  if ( morx_tmp_firstGlyph > glyph )
    morx_tmp_firstGlyph = glyph;
  if ( ( glyph - morx_tmp_firstGlyph ) > morx_tmp_nGlyphs )
    morx_tmp_nGlyphs = ( glyph - morx_tmp_firstGlyph );
  return FT_Err_Ok;
}

static FT_Error
tmp_morx_trimmed_array_count_glyph( GX_LookupTable_Format format,
			      FT_UShort index,
			      FT_UShort firstGlyph,
			      FT_UShort lastGlyph,
			      GX_LookupValue value,
			      FT_Pointer user )
{
  morx_tmp_firstGlyph = firstGlyph;
  morx_tmp_nGlyphs = lastGlyph - firstGlyph;
  return FT_Err_Ok;
}

static void
gx_face_dump_xstate_table ( GX_Face face, 
			    GX_XStateTable state_table,
			    GX_XStateTable_Entry_Action action,
			    int t )
{
  GX_LookupTable_FuncsRec tmp_funcs = {
    NULL,
    tmp_morx_simple_array_count_glyph,
    tmp_morx_segment_single_count_glyph,
    tmp_morx_segment_array_count_glyph,
    tmp_morx_single_table_count_glyph,
    tmp_morx_trimmed_array_count_glyph
  };

  POPEN(t, XstateTable);
  gx_face_dump_xstate_header(face, &state_table->header, t);

  morx_tmp_firstGlyph = 0;
  morx_tmp_nGlyphs = 0;
  morx_tmp_format = state_table->class_subtable.format;
  gx_LookupTable_traverse_low( &state_table->class_subtable, &tmp_funcs, NULL );
#if 1
  gx_face_dump_LookupTable_low(face, &state_table->class_subtable, 
			       &generic_lookup_table_funcs, t);
#else 
  gx_face_dump_LookupTable_high(face, &state_table->class_subtable, 
				&morx_dump_lookup_table_func, t);
#endif /* 0 */

  gx_face_dump_xstate_array(face,
			    state_table->nStates, state_table->header.nClasses,
			    state_table->state_array, t);
  gx_face_dump_entry_table(face, state_table->nEntries, state_table->entry_subtable,
			   action, t);
  PCLOSE(t, XstateTable);
}

/****************************GENERIC***********************************/  
static void
dump_table_info(GX_Table table_info, int n)
{
  POPEN(n,tableInfo);
  PFIELD(n,table_info,position,%lu);
  PFIELD(n,table_info,length,%lu);
  PCLOSE(n,tableInfo);
}

static FT_Error
generic_dump_lookup_table_generic ( GX_LookupTable_Format format,
				    GX_LookupValue value,
				    FT_Pointer user )
{
  FT_Int * t = (FT_Int*) user;
  PFIELD(*t,value,raw.s,%d);
  return FT_Err_Ok;
}

static FT_Error
generic_dump_lookup_table_segment( GX_LookupTable_Format format,
				   FT_UShort lastGlyph,
				   FT_UShort firstGlyph,
				   GX_LookupValue value,
				   FT_Pointer user )
{
  FT_UShort segment_count = lastGlyph - firstGlyph;
  FT_UShort * extra 	  = value->extra.word;
  FT_Int * t = (FT_Int*) user;
  int i;
  POPEN(*t, segmentArrayElement);
  PFIELD(*t,value,raw.s,%d);
  PVALUE(*t,lastGlyph,lastGlyph,%u);
  PVALUE(*t,firstGlyph,firstGlyph,%u);
  POPEN(*t, extra);
  for ( i = 0; i < segment_count; i++ )
    {
      NEWLINE10(*t, i);
      fprintf(stdout, "%u[%d] ", extra[i], i);
    }
  NEWLINE();
  PCLOSE(*t, extra);
  PCLOSE((*t), segmentArrayElement);
  return FT_Err_Ok;
}

static void
gx_face_dump_binSrchHeader( GX_Face face,
			    GX_BinSrchHeader binSrchHeader,
			    int t)
{
  POPEN(t, binSrchHeader);
  PFIELD(t,binSrchHeader,unitSize,%u);
  PFIELD(t,binSrchHeader,nUnits,%u);
  PFIELD(t,binSrchHeader,searchRange,%u);
  PFIELD(t,binSrchHeader,entrySelector,%u);
  PFIELD(t,binSrchHeader,rangeShift,%u);
  PCLOSE(t, binSrchHeader);
}
static void
gx_face_dump_LookupTable_low( GX_Face face,
			      GX_LookupTable lookup_table,
			      GX_LookupTable_Funcs funcs,
			      int t)
{
  GX_LookupTable_Trimmed_Array trimmed_array ;
  GX_LookupTable_BinSrch  binsrch;
  GX_LookupTable_FuncsRec default_funcs = {
    generic_dump_lookup_table_generic,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
  };

  if (!funcs)
    funcs = &default_funcs;
  
  if (!funcs->generic_func)
    funcs->generic_func = generic_dump_lookup_table_generic;
  
  POPEN2(t,lookupTable,
	 position,lookup_table->position,%lu,
	 format,lookup_table->format,%u);
  
  switch ( lookup_table->format )
    {
    case GX_LOOKUPTABLE_SIMPLE_ARRAY:
      /* DO NOTHING */
      break;
    case GX_LOOKUPTABLE_SEGMENT_SINGLE:
    case GX_LOOKUPTABLE_SEGMENT_ARRAY:
    case GX_LOOKUPTABLE_SINGLE_TABLE:
      binsrch = lookup_table->fsHeader.bin_srch;
      gx_face_dump_binSrchHeader( face, &binsrch->binSrchHeader, t );
      break;
    case GX_LOOKUPTABLE_TRIMMED_ARRAY:
      trimmed_array = lookup_table->fsHeader.trimmed_array;
      PFIELD(t,trimmed_array, firstGlyph, %u);
      PFIELD(t,trimmed_array, glyphCount, %u);
      break;
    }
  POPEN(t,values);
  gx_LookupTable_traverse_low( lookup_table, funcs, &t );
  PCLOSE(t,values);
  PCLOSE(t,lookupTable);
}

static void
gx_face_dump_LookupTable_high( GX_Face face,
			       GX_LookupTable lookup_table,
			       GX_LookupTable_Glyph_Func func,
			       int t )
{
  GX_LookupTable_Trimmed_Array trimmed_array ;
  GX_LookupTable_BinSrch  binsrch;

  FT_ASSERT(func);
  
  POPEN2(t,lookupTable,
	 position,lookup_table->position,%lu,
	 format,lookup_table->format,%u);
  
  switch ( lookup_table->format )
    {
    case GX_LOOKUPTABLE_SIMPLE_ARRAY:
      /* DO NOTHING */
      break;
    case GX_LOOKUPTABLE_SEGMENT_SINGLE:
    case GX_LOOKUPTABLE_SEGMENT_ARRAY:
    case GX_LOOKUPTABLE_SINGLE_TABLE:
      binsrch = lookup_table->fsHeader.bin_srch;
      gx_face_dump_binSrchHeader( face, &binsrch->binSrchHeader, t );
      break;
    case GX_LOOKUPTABLE_TRIMMED_ARRAY:
      trimmed_array = lookup_table->fsHeader.trimmed_array;
      PFIELD(t,trimmed_array, firstGlyph, %u);
      PFIELD(t,trimmed_array, glyphCount, %u);
      break;
    }
  POPEN(t,values);
  gx_LookupTable_traverse_high( lookup_table, func, &t );
  PCLOSE(t,values);
  PCLOSE(t,lookupTable);
}


FT_EXPORT_DEF ( void )
gxl_features_request_dump ( GXL_FeaturesRequest request, FILE * stream )
{
  FTL_Direction dir;
  unsigned long i;
  if ( !stream )
    stream = stderr;
  fprintf(stream, "Features Request: \n");
  dir = FTL_Get_FeaturesRequest_Direction((FTL_FeaturesRequest)request);
  fprintf(stream, "\tDirection: %s\n", (dir == FTL_HORIZONTAL)? "horizontal": "vertical");
  for ( i = 0; i < request->nFeatures; i++ )
    gxl_feature_dump ( &request->feature[i], stream );
}

FT_EXPORT ( void )
gx_feature_registory_dump ( FILE * stream )
{
  int i, j;
  GX_Feature_Registry featreg;
  const FT_String * setting_name;

  if ( !stream )
    stream = stderr;
  
  for ( i = 0; i < FEATREG_MAX ; i++ )
    {
      featreg = gx_get_feature_registry( i );
      if ( !featreg )
	continue ;
      fprintf(stdout, "[%d]: %s, %s\n", 
	      i, 
	      gx_feature_registry_get_name(featreg),
	      gx_feature_registry_is_setting_exclusive( featreg )? "exclusive": "non-exclusive");
      for ( j = 0; j < SETTING_MAX; j++ )
	{
	  setting_name = gx_feature_registry_get_setting_name( featreg, j );
	  if ( !setting_name )
	    break;
	  fprintf(stdout, "\t%s\n", setting_name );
	}
    }
}

FT_EXPORT_DEF ( void )
gxl_feature_dump ( GXL_Feature feature, FILE * stream )
{
  unsigned i;
  FT_SfntName feature_name;
  
  if ( !stream )
    stream = stderr;
  
  fprintf(stream, "\tFeatures: name=\"");
  GXL_Feature_Get_Name ( feature, 0, 0, 0, &feature_name );
  for ( i = 0; i < feature_name.string_len; i++ )
    fputc(feature_name.string[i], stream);
  fprintf(stream, "\" ");
  fprintf(stream, "value=%u ", feature->value);
  if ( feature->exclusive.exclusive )
    fprintf(stream, "exclusive=%u", feature->exclusive.setting->value);
  fprintf(stream, "\n");
	  
  for ( i = 0; i < feature->nSettings; i++ )
    gxl_setting_dump(&feature->setting[i], stream);

}

FT_EXPORT_DEF ( void )
gxl_setting_dump ( GXL_Setting setting, FILE * stream )
{
    unsigned i;
  FT_SfntName setting_name;
  
  if ( !stream )
    stream = stderr;

  GXL_Setting_Get_Name ( setting, 0, 0, 0, &setting_name );  
  
  fprintf(stream, "\t\tSetting: name=\"");
  for ( i = 0; i < setting_name.string_len; i++ )
    fputc(setting_name.string[i], stream);
  fprintf(stream, "\" ");
  fprintf(stream, "value=%u(%s)\n", setting->value, 
	  GXL_Setting_Get_State(setting)? "on": "off");
}

/* END */
