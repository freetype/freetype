/***************************************************************************/
/*                                                                         */
/*  gxlayout.c                                                             */
/*                                                                         */
/*    AAT/TrueTypeGX based layout engine(body).                            */
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

#include FT_LIST_H
#include FT_GXLAYOUT_H
#include FT_INTERNAL_FTL_TYPES_H

#include "gxltypes.h"
#include "gxtypes.h"
#include "gxaccess.h"
#include "gxutils.h"
#include "gxlookuptbl.h"
#include "gxfeatreg.h"
#include "gxlfeatreg.h"
#include "gxerrors.h"

static int gxl_feature_compare (const void * a, const void * b);
static FT_Error gxl_feature_initialize_for_mort ( GXL_Feature feature,
						  FT_Memory memory,
						  GX_Mort mort, 
						  GX_Feat feat,
						  FT_UShort * index );
static FT_Error gxl_feature_initialize_for_morx ( GXL_Feature feature,
						  FT_Memory memory,
						  GX_Morx morx, 
						  GX_Feat feat,
						  FT_UShort * index );
/*
 * FEAT
 */
FT_LOCAL_DEF ( GXL_Setting )
gxl_feature_get_setting_by_value (GXL_Feature feature, FT_UShort value)
{
  FT_Int i;
  if ( feature->exclusive.exclusive )
    {
      if ( value == feature->exclusive.setting->value )
	return feature->exclusive.setting;
    }
  else
    {
      for ( i = 0; i < feature->nSettings; i++ )
	{
	  if ( value == feature->setting[i].value )
	    return &feature->setting[i];
	}
    }
  return NULL;
}

FT_LOCAL_DEF ( GXL_Feature )
gxl_features_request_get_feature_by_type ( GXL_FeaturesRequest request,
					   FT_UShort featureType )
{
  GXL_Feature feature = NULL;
  int i;
  for ( i = 0; i < request->nFeatures; i++ )
    {
      if ( request->feature[i].value == featureType )
	{
	  feature = &request->feature[i];
	  break;
	}
    }
  return feature;
}

FT_LOCAL_DEF ( void )
gxl_features_request_free( GXL_FeaturesRequest request, FT_Memory memory )
{
  FT_Int i;
  GXL_Feature feature;

  for ( i = request->nFeatures; i > 0; i-- )
    {
      feature = &(request->feature[i - 1]);
      FT_FREE( feature->setting );
    }
  FT_FREE ( request->feature );
  FT_FREE ( request );
}

static GXL_Setting
gxl_setting_get_exclusive_setting( GXL_Setting setting )
{
  return setting->feature->exclusive.setting;
}

static void
gxl_setting_set_exclusive_setting( GXL_Setting setting )
{
  setting->feature->exclusive.setting = setting; 
}

static void
gxl_setting_set_exclusive_state( GXL_Setting setting , FT_Bool state )
{
  if ( state )
    gxl_setting_set_exclusive_setting ( setting );
}

static FT_Bool
gxl_setting_get_exclusive_state ( GXL_Setting setting )
{
  if ( gxl_setting_get_exclusive_setting (setting) == setting )
    return 1;
  else
    return 0;
}

FT_EXPORT ( void )
GXL_FeaturesRequest_Set_Initial_State ( GXL_FeaturesRequest request,
					GXL_Initial_State initial_state )
{
  FT_ASSERT( request );
  request->initial_state = initial_state;
}

FT_EXPORT ( GXL_Initial_State )
GXL_FeaturesRequest_Get_Initial_State ( GXL_FeaturesRequest request )
{
  FT_ASSERT( request );
  return request->initial_state;
}

FT_EXPORT_DEF ( FT_ULong )
GXL_FeaturesRequest_Get_Feature_Count ( GXL_FeaturesRequest request )
{
  if ( request )
    return request->nFeatures;
  else
    return 0;
}

FT_EXPORT_DEF ( GXL_Feature )
GXL_FeaturesRequest_Get_Feature       ( GXL_FeaturesRequest request, 
					FT_ULong index)
{
  GXL_Feature feature = NULL;
  if ( index < GXL_FeaturesRequest_Get_Feature_Count ( request ) )
    feature = &(request->feature[index]);
  return feature;
}

FT_EXPORT_DEF( FT_Error )
GXL_Feature_Get_Name                  ( GXL_Feature feature,
					FT_UShort   platform_id,
					FT_UShort   encoding_id,
					FT_UShort   language_id,
					FT_SfntName  *aname )
{
  GXL_Font font;
  FT_Face face;
  font = (GXL_Font)feature->request->root.font;
  face = font->root.face;
  
  if ( ! face )
    return GX_Err_Invalid_Face_Handle;
  
  if ( ! feature->name.string )
    return gx_get_name_from_id ( face,
				 feature->name.index,
				 platform_id, encoding_id, language_id,
				 aname );
  else
    {
      aname->platform_id = 0;
      aname->encoding_id = 0;
      aname->language_id = 0;
      aname->name_id 	 = 0;
      aname->string  	 = (FT_Byte*)feature->name.string;
      aname->string_len  = ft_strlen ( feature->name.string );
      return GX_Err_Ok;
    }
}

FT_EXPORT_DEF( FT_UShort )
GXL_Feature_Get_Setting_Count         ( GXL_Feature feature )
{
  return feature->nSettings;
}


FT_EXPORT_DEF( GXL_Setting )
GXL_Feature_Get_Setting               ( GXL_Feature feature,
					FT_ULong index )
{
  GXL_Setting setting = NULL;
  if ( index < feature->nSettings )
    setting = &feature->setting[index];
  return setting;
}

FT_EXPORT_DEF ( FT_Bool )
GXL_Feature_Is_Setting_Exclusive      ( GXL_Feature feature )
{
  return feature->exclusive.exclusive;
}


FT_EXPORT_DEF( FT_Bool )
GXL_Setting_Get_State                 ( GXL_Setting setting )
{
  GXL_Feature feature = setting->feature;
  if ( GXL_Feature_Is_Setting_Exclusive ( feature ) )
    return gxl_setting_get_exclusive_state ( setting );
  else
    return gx_feat_setting_state(setting->value);
}

FT_EXPORT_DEF( FT_Error )
GXL_Setting_Get_Name                  ( GXL_Setting setting, 
					FT_UShort  platform_id,
					FT_UShort  encoding_id,
					FT_UShort  language_id,
					FT_SfntName  *aname )
{
  GXL_Font font = (GXL_Font)setting->feature->request->root.font;
  FT_Face face = font->root.face;
  
  if ( ! setting->name.string )
    return gx_get_name_from_id ( face,
				 setting->name.index,
				 platform_id, encoding_id, language_id,
				 aname );
  else
    {
      aname->platform_id = 0;
      aname->encoding_id = 0;
      aname->language_id = 0;
      aname->name_id 	= 0;
      aname->string  	= (FT_Byte*)setting->name.string;
      aname->string_len = ft_strlen ( setting->name.string );
      return GX_Err_Ok;
    }
}

FT_EXPORT_DEF( void )
GXL_Setting_Set_State                 ( GXL_Setting setting, 
					FT_Bool state )
{
  if ( setting->feature->exclusive.exclusive )
    gxl_setting_set_exclusive_state( setting, state );
  else if ( state )
    setting->value = gx_feat_setting_on( setting->value );
  else
    setting->value = gx_feat_setting_off( setting->value );
}

/*
 * Substitution
 */
static int
gxl_feature_compare (const void * a, const void * b)
{
  return (FT_Long)((GXL_Feature)a)->value - (FT_Long)((GXL_Feature)b)->value;
}

typedef struct gxl_feature_initialize_for_mort_data_rec_
{
  FT_Int count;
  GXL_Feature feature;
  GX_Feat feat;
  FT_Memory memory;
} gxl_feature_initialize_for_mort_data_rec, *gxl_feature_initialize_for_mort_data;


static FT_Error
gxl_feature_initialize_for_mort_cb ( GX_MetamorphosisFeatureTable feat_Subtbl, FT_Pointer user )
{
  FT_Error error;
  GX_Feature_Registry featreg;
  gxl_feature_initialize_for_mort_data data = user;
  FT_Int i;
  
  if ( gx_feat_has_feature_type ( data->feat, feat_Subtbl->featureType ) )
    {				/* In feat table? */
      return GX_Err_Ok;
    }

  for ( i = 0; i < data->count; i ++ )
    {				/* Duplication check */
      if ( data->feature[i].value == feat_Subtbl->featureType )
	return GX_Err_Ok;
    }

  if (!( featreg = gx_get_feature_registry ( feat_Subtbl->featureType ) ))
    {				/* Not in feature_registry */
      return GX_Err_Ok;
    }
  
  error = gxl_feature_registry_fill_feature( featreg, data->memory, &(data->feature[data->count]) );
  if ( error )
    goto Exit;
  data->count++;
 Exit:
  return error;
}

static FT_Error
gxl_feature_initialize_for_mort ( GXL_Feature feature,
				  FT_Memory memory,
				  GX_Mort mort, 
				  GX_Feat feat,
				  FT_UShort * index )
{
  FT_Error error;
  FT_Int i;
  gxl_feature_initialize_for_mort_data_rec data;
  data.count   = 0;
  data.feature = feature;
  data.feat    = feat;
  data.memory  = memory;
  
  error = gx_mort_foreach_feature ( mort, gxl_feature_initialize_for_mort_cb, &data );
  if ( error )
    {
      for ( i = data.count; i > 0; i-- )
	FT_FREE (feature[i - 1].setting);
      return error;
    }
  else
    {
      *index = data.count;
      return GX_Err_Ok ;
    }
}

typedef struct gxl_feature_initialize_for_morx_data_rec_
{
  FT_Int count;
  GXL_Feature feature;
  GX_Feat feat;
  FT_Memory memory;
} gxl_feature_initialize_for_morx_data_rec, *gxl_feature_initialize_for_morx_data;


static FT_Error
gxl_feature_initialize_for_morx_cb ( GX_MetamorphosisFeatureTable feat_Subtbl, FT_Pointer user )
{
  FT_Error error;
  GX_Feature_Registry featreg;
  gxl_feature_initialize_for_morx_data data = user;
  FT_Int i;
  
  if ( gx_feat_has_feature_type ( data->feat, feat_Subtbl->featureType ) )
    return GX_Err_Ok;

  for ( i = 0; i < data->count; i ++ )
    {				/* Duplication check */
      if ( data->feature[i].value == feat_Subtbl->featureType )
	return GX_Err_Ok;
    }

  featreg = gx_get_feature_registry ( feat_Subtbl->featureType );
  if ( ! featreg  )
    return GX_Err_Ok;

  error = gxl_feature_registry_fill_feature( featreg, data->memory, &(data->feature[data->count]) );
  if ( error )
    return error;
  data->count++;
  return GX_Err_Ok;
}

static FT_Error
gxl_feature_initialize_for_morx ( GXL_Feature feature,
				  FT_Memory memory,
				  GX_Morx morx, 
				  GX_Feat feat,
				  FT_UShort * index )
{
  FT_Error error;
  FT_Int i;
  gxl_feature_initialize_for_morx_data_rec data;
  data.count 	= 0;
  data.feature  = feature;
  data.feat 	= feat;
  data.memory 	= memory;
  error = gx_morx_foreach_feature ( morx, gxl_feature_initialize_for_morx_cb, &data );
  if ( error )
    {
      for ( i = data.count; i > 0; i-- )
	FT_FREE (feature[i - 1].setting);
      return error;
    }
  else
    {
      *index = data.count;
      return GX_Err_Ok ;
    }
}


  FT_LOCAL_DEF ( FT_Error )
  gxl_get_font ( FT_Face face, FTL_Font * font)
  {
    *font = ((GX_Face)face)->extra.data;
    
    if ( *font )
      return FT_Err_Ok;
    else
      return GX_Err_Invalid_Face_Handle;
  }

  FT_LOCAL_DEF ( FTL_EngineType )
  gxl_get_engine_type ( FT_Face face )
  {
    return FTL_TRUETYPEGX_ENGINE;
  }
  
  FT_LOCAL_DEF ( FT_Error )
  gxl_new_features_request( FT_Face face, FTL_FeaturesRequest * ftl_request)
  {
    FT_Memory  memory = face->driver->root.memory;
    FT_Error error;
    GXL_Font font;
    GXL_FeaturesRequest request;

    GX_Feat feat;
    GX_Mort mort;
    GX_Morx morx;
  
    GXL_Feature feature;
    GXL_Setting setting;
    FT_UShort default_setting_index;
    GXL_Setting default_setting;
    FT_UShort infeat_feat_count, inmort_feat_count = 0, inmorx_feat_count = 0;
  
    FT_Int i_feat, i_setting, j_feat, i_mort, i_morx;

    if (( error = gxl_get_font ( face, (FTL_Font*)&font ) ))
      return error;

    if ( FT_NEW ( request ) )
      goto Exit;
    if (( error =  FTL_FeaturesRequest_Init ( face, (FTL_FeaturesRequest)request ) ))
      {
	FT_FREE( request );
	goto Exit;
      }

    request->initial_state = GXL_START_OF_TEXT_STATE;

    feat = font->feat;
    mort = font->mort;
    morx = font->morx;
  
    if ( !feat )
      {
	error = FT_Err_Ok;
	(request)->nFeatures = 0;
	(request)->feature   = NULL;
	*ftl_request = (FTL_FeaturesRequest)request;
	goto Exit;
      }

    if ( mort )
      inmort_feat_count = gx_mort_count_feat_not_in_feat( mort, feat );

    if ( morx )
      inmorx_feat_count = gx_morx_count_feat_not_in_feat( morx, feat );

    if ( ( mort == NULL ) && ( morx == NULL ) )
      {
	error = GX_Err_Missing_Glyph_Substitution_Data;
	goto Failure_request;
      }

    infeat_feat_count 	= feat->featureNameCount;
    request->nFeatures = infeat_feat_count + inmort_feat_count + inmorx_feat_count;
  
    if ( FT_NEW_ARRAY ( request->feature, request->nFeatures ) )
      goto Failure_request;
    for ( i_feat = 0; i_feat < infeat_feat_count; i_feat++ )
      {
	feature 		    = (&request->feature[i_feat]);
	feature->value 	    = feat->names[i_feat].feature;
	feature->exclusive.exclusive = ((feat->names[i_feat].featureFlags)
					& GX_FEAT_MASK_EXCLUSIVE_SETTINGS)
	  ? 1: 0;
	feature->exclusive.setting     = NULL; /* dummy */
	feature->name.index    = feat->names[i_feat].nameIndex;
	feature->name.string   = NULL;
	feature->request = request;
	feature->nSettings    = feat->names[i_feat].nSettings;
	if ( FT_NEW_ARRAY ( feature->setting, feature->nSettings ) )
	  goto Failure_loop_feat;
	for ( i_setting = 0; i_setting < feature->nSettings; i_setting++ )
	  {
	    setting 	     	= &feature->setting[i_setting];
	    setting->value     	= feat->names[i_feat].settingName[i_setting].setting;
	    setting->name.index 	= feat->names[i_feat].settingName[i_setting].nameIndex;
	    setting->name.string  = NULL;
	    setting->feature = feature;
	  }
	if ( feature->exclusive.exclusive )
	  {
	    default_setting_index = 0;
	    if ( (feat->names[i_feat].featureFlags)&GX_FEAT_MASK_DYNAMIC_DEFAULT )
	      default_setting_index = (feat->names[i_feat].featureFlags)&GX_FEAT_MASK_DEFAULT_SETTING;
	    default_setting = &feature->setting[default_setting_index];
	    if ( default_setting_index >= feature->nSettings )
	      {
		error = GX_Err_Invalid_File_Format;
		goto Failure_loop_feat;
	      }
	    GXL_Setting_Set_State ( default_setting, 1 );	  
	  }
	else if ( feat->names[i_feat].featureFlags&GX_FEAT_MASK_DYNAMIC_DEFAULT )
	  {
	    FT_Bool state;
	    default_setting_index   = feat->names[i_feat].featureFlags&GX_FEAT_MASK_DEFAULT_SETTING;
	    
	    if ( default_setting_index < feature->nSettings )
	      {
		default_setting = &feature->setting[default_setting_index];
		state = 1;
	      }
	    else if ( default_setting_index == 1 )
	      {
		/* If default_setting_index is 1 but nSettings is also 1,
		   there is not setting for default_setting_index in the
		   font file. In this case setting[0] should be off. */
		default_setting = &feature->setting[0];
		state = 0;
	      }
	    else 
	      {
		error = GX_Err_Invalid_File_Format;
		goto Failure_loop_feat;
	      }
	    GXL_Setting_Set_State ( default_setting, state );
	  }
	else
	  {			/* TODO: getting from mort's and morx's default? */
	    default_setting_index = 0;
	    default_setting       = &feature->setting[default_setting_index];
	    GXL_Setting_Set_State ( default_setting, 1 );
	  }
      }
  
    if ( inmort_feat_count )
      {
	inmort_feat_count = 0;
	feature 		    = (&request->feature[i_feat]);
	if (( error = gxl_feature_initialize_for_mort (feature, memory, mort, feat, &inmort_feat_count) ))
	  goto Failure_loop_feat;
      
	for ( i_mort = 0; i_mort < inmort_feat_count; i_mort++ )
	  feature[i_mort].request = request;
      
	i_feat += inmort_feat_count;
	request->nFeatures = i_feat;
	feature 		    = (&request->feature[inmort_feat_count]);
      }

    if ( inmorx_feat_count )
      {
	inmorx_feat_count = 0;
	feature = (&(request)->feature[i_feat]);
	if (( error = gxl_feature_initialize_for_morx (feature, memory, morx, feat, &inmorx_feat_count) ))
	  goto Failure_loop_feat;
      
	for ( i_morx = 0; i_morx < inmorx_feat_count; i_morx++ )
	  feature[i_morx].request = request;
	i_feat += inmorx_feat_count;
	request->nFeatures = i_feat;
      }

    if ( inmort_feat_count || inmorx_feat_count )
      {
	ft_qsort(request->feature, 
		 request->nFeatures, 
		 sizeof (*(request->feature)), 
		 gxl_feature_compare );
	for ( i_feat = 0; i_feat < request->nFeatures; i_feat++ )
	  {
	    feature = &request->feature[i_feat];
	    for ( i_setting = 0; i_setting < feature->nSettings; i_setting++ )
	      feature->setting[i_setting].feature = feature;
	  }
      }
    *ftl_request = (FTL_FeaturesRequest)request;
  Exit:
    return error;
  Failure_loop_feat:
    for ( j_feat = i_feat; j_feat > 0; j_feat-- )
      FT_FREE( request->feature[j_feat - 1].setting );
  Failure_request:
    FT_FREE(request);
    return error;
  }

  FT_LOCAL_DEF ( FT_Error )
  gxl_done_features_request( FTL_FeaturesRequest request)
  {
    FTL_Font font;
    FT_Face face;
    
    FT_Memory  memory;

    FT_ASSERT( request );
    font = request->font;
    FT_ASSERT( font );
    face = font->face;
    if ( !face )
      return GX_Err_Invalid_Argument;
    memory =  face->driver->root.memory;
    gxl_features_request_free( (GXL_FeaturesRequest)request, memory );
    return FTL_FeaturesRequest_Finalize ( request );
  }

  FT_LOCAL_DEF ( FT_Error ) 
  gxl_copy_features_request( FTL_FeaturesRequest from,
			     FTL_FeaturesRequest to)
  {
    FT_Error error;
    FT_Int i_feat, i_setting;
    GXL_Feature from_feature, to_feature;
    GXL_Setting from_setting, to_setting;
  
    if (( error = FTL_FeaturesRequest_Copy( from, to ) ))
      return error;

    for ( i_feat = 0; i_feat < ((GXL_FeaturesRequest)from)->nFeatures; i_feat++ )
      {
	from_feature 	      = GXL_FeaturesRequest_Get_Feature((GXL_FeaturesRequest)from, 
								i_feat);
	to_feature 	      = GXL_FeaturesRequest_Get_Feature((GXL_FeaturesRequest)to, 
								i_feat);
	
	for ( i_setting = 0; 
	      i_setting < GXL_Feature_Get_Setting_Count(from_feature); 
	      i_setting++ )
	  {
	    from_setting = GXL_Feature_Get_Setting ( from_feature, i_setting );
	    to_setting   = GXL_Feature_Get_Setting ( to_feature, i_setting );
	    GXL_Setting_Set_State(to_setting,
				  GXL_Setting_Get_State(from_setting));
	  }
      }
    return GX_Err_Ok;
  }

  FT_LOCAL_DEF ( FT_UShort )
  gxl_get_ligature_caret_count ( FT_Face face,
				 FT_UShort glyphID )
  {
    FTL_Font font;
    GX_Lcar  lcar;
    GX_LigCaretClassEntry class_entry;
    
    if ( FTL_Get_Font( face, &font ) )
      return 0;
    lcar = ((GXL_Font)font)->lcar;
    if ( !lcar )
      return 0;
    class_entry = gx_lcar_get( lcar, glyphID );
    if ( class_entry )
      return class_entry->count;
    else
      return 0;
  }

/* TODO: return an error */
  FT_LOCAL_DEF ( FT_UShort )
  gxl_get_ligature_caret_division( FT_Face face, 
				   FT_UShort glyph_id, 
				   FT_UShort nth )
  {
    FTL_Font              font;
    GX_Lcar               lcar;
    GX_LigCaretClassEntry class_entry;
    FT_UShort             partials;
    FTL_Direction         direction;
    FT_Outline            *outline;
    FT_Vector             point;
    if ( FTL_Get_Font( face, &font ) )
      return 0;

    lcar = ((GXL_Font)font)->lcar;
    if ( !lcar )
      return 0;
    
    class_entry = gx_lcar_get( lcar, glyph_id );
    if ( !class_entry )
      return 0;
    if ( nth >= class_entry->count )
      return 0;

    partials = class_entry->partials[nth];
    if ( lcar->format == GX_LCAR_DISTANCE )
      return partials;
    
    /* font -> feature_requst -> direction */
    direction = FTL_Get_FeaturesRequest_Direction ( font->features_request );

    /* glyph_id -> glyph -> outline -> point   -> x or y */
    if ( FT_Load_Glyph ( face, glyph_id, 
			 FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP ) )
      return 0;

    outline = &face->glyph->outline;
    if ( ! ( partials < outline->n_points ) )
      return 0;
    point = outline->points[partials];
    
    /* TODO: Are the unit for the outline and that 
       for ligature caret same? */
    if ( direction == FTL_HORIZONTAL )
      return (FT_UShort)point.x;
    else
      return (FT_UShort)point.y;
  }

  FT_LOCAL_DEF ( FT_Error )
  gxl_substitute_glyphs ( FT_Face face,
			  FTL_FeaturesRequest request,
			  FTL_Glyphs_Array in,
			  FTL_Glyphs_Array out )
  {
    FT_Error error;
    GXL_Font font;
    GX_Mort mort;
    GX_Morx morx;
    
    if (( error = FTL_Get_Font ( face, (FTL_Font*)&font )))
      return error;
    
    mort = font->mort;
    morx = font->morx;
    if ( mort )
      return gx_mort_substitute_glyph(mort, (GXL_FeaturesRequest)request, in, out );
    else if ( morx )
      return gx_morx_substitute_glyph(morx, (GXL_FeaturesRequest)request, in, out );
    else
      return FT_Err_Invalid_Argument;
  }

/* END */
