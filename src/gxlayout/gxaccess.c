/***************************************************************************/
/*                                                                         */
/*  gxaccess.c                                                             */
/*                                                                         */
/*    AAT/TrueTypeGX private data accessor implementation(body).           */
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
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_CALC_H
#include "gxlookuptbl.h"
#include "gxstatetbl.h"
#include "gxutils.h"
#include "gxaccess.h"
#include "gxobjs.h"
#include "gxerrors.h"
#include "gxvm.h"
#include "gxltypes.h"

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                           Features                              ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


FT_LOCAL_DEF ( FT_Bool )
gx_feat_has_feature_type ( GX_Feat feat, FT_UShort feature_type )
{
  FT_Int i;
  for ( i = 0; i < feat->featureNameCount; i++ )
    {
      if ( feat->names[i].feature == feature_type )
	return 1;
    }
  return 0;
}

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                        Glyph Properties                         ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

FT_LOCAL_DEF ( FT_UShort )
gx_prop_get( GX_Prop   prop, FT_Long   glyph )
{
  GX_LookupTable lookup_table = &prop->lookup_data;
  FT_UShort default_properties = prop->default_properties;
  FT_UShort properties = 0;
  GX_LookupResultRec result;
  FT_UShort * segment_array;
  FT_Long index_in_segment;


  result = gx_LookupTable_lookup( lookup_table, glyph );

  if ( result.value == NULL )
    properties = default_properties;
  else if ( result.firstGlyph == GX_LOOKUP_RESULT_NO_FIRST_GLYPH )
    properties = result.value->raw.s;
  else
    {
      index_in_segment = glyph - result.firstGlyph;
      segment_array    = result.value->extra.word;
      properties       = segment_array[index_in_segment];
    }
  return properties;
}

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                         Ligature Carret                         ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

FT_LOCAL_DEF ( GX_LigCaretClassEntry )
gx_lcar_get ( GX_Lcar lcar, FT_UShort glyphID )
{ /* TODO: We could put cache mechanism here. */
  GX_LookupResultRec result;

  result = gx_LookupTable_lookup ( &lcar->lookup, glyphID );
  if ( result.value == NULL )
    return NULL;
  else if ( result.firstGlyph == GX_LOOKUP_RESULT_NO_FIRST_GLYPH )
    return result.value->extra.lcar_class_entry;
  else
    return &result.value->extra.lcar_segment->class_entry[glyphID - result.firstGlyph];
}


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                        Glyph Metamorphosis                      ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxchain

FT_LOCAL_DEF ( FT_Error )
gx_mort_foreach_feature ( GX_Mort mort, GX_Mort_Feature_Func func, FT_Pointer user )
{
  FT_Error error = GX_Err_Ok;
  FT_Int i, j;
  GX_MetamorphosisChain chain;
  GX_MetamorphosisFeatureTable feat_Subtbl;
  for ( i = 0; i < mort->nChains; i++ )
    {
      chain = &mort->chain[i];
      for ( j = 0; j < chain->header.nFeatureEntries; j++ )
	{
	  feat_Subtbl = &chain->feat_Subtbl[j];
	  if (( error = func ( feat_Subtbl, user ) ))
	    return error;
	}
    }
  return error;
}

#define GX_MORT_COUNT_FEAT_DATA_ZERO {0, NULL}
typedef struct gx_mort_count_feat_data_rec_
{
  FT_UShort count;
  GX_Feat feat;
} gx_mort_count_feat_data_rec, *gx_mort_count_feat_data;

static FT_Error
gx_mort_count_feat_not_in_feat_cb ( GX_MetamorphosisFeatureTable feat_Subtbl, FT_Pointer user )
{
  gx_mort_count_feat_data data = user;
  FT_UShort featureType = feat_Subtbl->featureType;
  if ( !gx_feat_has_feature_type ( data->feat, featureType ) )
    data->count++;
  return GX_Err_Ok;
}

FT_LOCAL_DEF ( FT_UShort )
gx_mort_count_feat_not_in_feat ( GX_Mort mort, GX_Feat feat )
{
  gx_mort_count_feat_data_rec data;
  data.count = 0;
  data.feat  = feat;
  gx_mort_foreach_feature ( mort, gx_mort_count_feat_not_in_feat_cb, &data );
  return data.count;
}

static FT_ULong
gx_chain_calc_selector ( GX_MetamorphosisChain chain, GXL_FeaturesRequest request)
{
  FT_ULong j_features;
  FT_ULong result;
  GX_MetamorphosisFeatureTable feat_Subtbl;
  GXL_Feature feature;

  result = chain->header.defaultFlags;
  for ( j_features = 0; j_features < chain->header.nFeatureEntries; j_features++ )
    {
      feat_Subtbl = &chain->feat_Subtbl[j_features];
      feature = gxl_features_request_get_feature_by_type(request,
							 feat_Subtbl->featureType);
      if ( !feature )
	continue ;

      if ( gxl_feature_get_setting_by_value(feature,
					    feat_Subtbl->featureSetting ) )
	{
	  result &= feat_Subtbl->disableFlags;
	  result |= feat_Subtbl->enableFlags ;
	}
    }
  return result;
}

FT_LOCAL_DEF( FT_Error )
gx_mort_substitute_glyph ( GX_Mort mort,
			   GXL_FeaturesRequest request,
			   FTL_Glyphs_Array in,
			   FTL_Glyphs_Array out )
{
  FT_Error error = GX_Err_Ok;
  GX_MetamorphosisChain chain;
  FT_ULong i_chain, k_subtbl;
  FT_ULong selector;
  GX_MetamorphosisSubtable chain_Subtbl;
  FT_UShort coverage;
  FT_UShort subtable_type;
  GXL_Order order;

  if (( error = FTL_Copy_Glyphs_Array ( in, out ) ))
    return error;


  for ( i_chain = 0; i_chain < mort->nChains; i_chain++ )
    {
      chain  	     = &mort->chain[i_chain];
      selector = gx_chain_calc_selector( chain, request );

      FT_TRACE2(( "Mort Chain No.%d, Address 0x%p\n", i_chain, chain ));
      
      for ( k_subtbl = 0; k_subtbl < chain->header.nSubtables; k_subtbl++ )
	{
	  chain_Subtbl = &chain->chain_Subtbl[k_subtbl];
	  if ( !(chain_Subtbl->header.subFeatureFlags & selector) )
	    continue;

	  coverage = chain_Subtbl->header.coverage;
	  if ( ( !(coverage & GX_MORT_COVERAGE_ORIENTATION_INDEPENDENT) )
	       /* Orientation dependent */
	       && ((( coverage & GX_MORT_COVERAGE_HORIZONTAL_OR_VERTICAL_TEXT )
		    /* Vertical only */
		    && (FTL_Get_FeaturesRequest_Direction(&request->root) != FTL_VERTICAL)
		    /* But the request is NOT vertical */)
		   || (!( coverage & GX_MORT_COVERAGE_HORIZONTAL_OR_VERTICAL_TEXT )
		       /* Horizontal only */
		       && (FTL_Get_FeaturesRequest_Direction(&request->root) != FTL_HORIZONTAL)
		       /* But the request is NOT horizontal */)))
	    continue;

	  order = (coverage & GX_MORT_COVERAGE_ORDER_OF_PROCESSING_GLYPH_ARRAY)
	    ? GXL_DESCENDING
	    : GXL_ASCENDING;
	  if ( order == GXL_DESCENDING )
	    gx_glyphs_array_reverse( out->glyphs, out->length );

	  subtable_type = coverage & GX_MORT_COVERAGE_SUBTABLE_TYPE;
	  FT_TRACE2(( "\tSubtable No.%d, Address 0x%p, Type %d\n", 
		      k_subtbl, chain_Subtbl, subtable_type ));
	  
	  switch ( subtable_type )
	    {
	    case GX_MORT_REARRANGEMENT_SUBTABLE:
	      gx_rearrangement_subst( chain_Subtbl->body.rearrangement,
				      request->initial_state,
				      out );
	      break;
	    case GX_MORT_CONTEXTUAL_SUBTABLE:
	      error = gx_contextual_subst( chain_Subtbl->body.contextual,
					   request->initial_state,
					   out );
	      break;
	    case GX_MORT_LIGATURE_SUBTABLE:
	      error = gx_ligature_subst( chain_Subtbl->body.ligature,
					 request->initial_state,
					 out );
	      break;
	    case GX_MORT_RESERVED_SUBTABLE:
	      FT_ERROR(("Reserved\n"));
	      break;
	    case GX_MORT_NONCONTEXTUAL_SUBTABLE:
	      error = gx_xnoncontextual_subst( chain_Subtbl->body.noncontextual, out );
	      break;
	    case GX_MORT_INSERTION_SUBTABLE:
	      error = gx_insertion_subst( chain_Subtbl->body.insertion,
					  request->initial_state,
					  out );
	      break;
	    }

	  if ( order == GXL_DESCENDING )
	    gx_glyphs_array_reverse( out->glyphs, out->length );
	}
    }
  return error;
}

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                   Extended Glyph Metamorphosis                  ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

FT_LOCAL_DEF ( FT_Error )
gx_morx_foreach_feature ( GX_Morx morx, GX_Morx_Feature_Func func, FT_Pointer user )
{
  FT_Error error = GX_Err_Ok;
  FT_Int i, j;
  GX_XMetamorphosisChain chain;
  GX_XMetamorphosisFeatureTable feat_Subtbl;
  for ( i = 0; i < morx->nChains; i++ )
    {
      chain = &morx->chain[i];
      for ( j = 0; j < chain->header.nFeatureEntries; j++ )
	{
	  feat_Subtbl = &chain->feat_Subtbl[j];
	  if (( error = func ( feat_Subtbl, user ) ))
	    return error;

	}
    }
  return error;
}

#define GX_MORT_COUNT_FEAT_DATA_ZERO {0, NULL}
typedef struct gx_morx_count_feat_data_rec_
{
  FT_UShort count;
  GX_Feat feat;
} gx_morx_count_feat_data_rec, *gx_morx_count_feat_data;

static FT_Error
gx_morx_count_feat_not_in_feat_cb ( GX_MetamorphosisFeatureTable feat_Subtbl, FT_Pointer user )
{
  gx_morx_count_feat_data data = user;
  FT_UShort featureType = feat_Subtbl->featureType;
  if ( !gx_feat_has_feature_type ( data->feat, featureType ) )
    data->count++;
  return GX_Err_Ok;
}

FT_LOCAL_DEF ( FT_UShort )
gx_morx_count_feat_not_in_feat ( GX_Morx morx, GX_Feat feat )
{
  gx_morx_count_feat_data_rec data;
  data.count = 0;
  data.feat  = feat;
  gx_morx_foreach_feature ( morx, gx_morx_count_feat_not_in_feat_cb, &data );
  return data.count;
}

static FT_ULong
gx_xchain_calc_selector ( GX_XMetamorphosisChain chain, GXL_FeaturesRequest request)
{
  FT_ULong j_features;
  FT_ULong result;
  GX_MetamorphosisFeatureTable feat_Subtbl;
  GXL_Feature feature;

  result = chain->header.defaultFlags;
  for ( j_features = 0; j_features < chain->header.nFeatureEntries; j_features++ )
    {
      feat_Subtbl = &chain->feat_Subtbl[j_features];
      feature = gxl_features_request_get_feature_by_type(request,
							 feat_Subtbl->featureType);
      if ( !feature )
	continue ;

      if ( gxl_feature_get_setting_by_value(feature,
					    feat_Subtbl->featureSetting ) )
	{
	  result &= feat_Subtbl->disableFlags;
	  result |= feat_Subtbl->enableFlags;
	}
    }
  return result;
}

FT_LOCAL_DEF( FT_Error )
gx_morx_substitute_glyph ( GX_Morx morx,
			   GXL_FeaturesRequest request,
			   FTL_Glyphs_Array in,
			   FTL_Glyphs_Array out )
{
  FT_Error error = GX_Err_Ok;
  GX_XMetamorphosisChain xchain;
  FT_ULong i_chain, k_subtbl;
  FT_ULong selector;
  GX_XMetamorphosisSubtable xchain_Subtbl;
  FT_UShort coverage;
  FT_UShort subtable_type;
  GXL_Order order;

  if (( error = FTL_Copy_Glyphs_Array ( in, out ) ))
    return error;

  for ( i_chain = 0; i_chain < morx->nChains; i_chain++ )
    {
      xchain    = &morx->chain[i_chain];
      selector = gx_xchain_calc_selector( xchain, request );
      
      FT_TRACE2(( "Morx Chain No.%d, Address 0x%p\n", i_chain, xchain ));
      
      for ( k_subtbl = 0; k_subtbl < xchain->header.nSubtables; k_subtbl++ )
	{
	  xchain_Subtbl = &xchain->chain_Subtbl[k_subtbl];
	  if ( !(xchain_Subtbl->header.subFeatureFlags & selector) )
	    continue;

	  coverage = xchain_Subtbl->header.coverage;
	  if ( ( !(coverage & GX_MORX_COVERAGE_ORIENTATION_INDEPENDENT) )
	       /* Orientation dependent */
	       && ((( coverage & GX_MORX_COVERAGE_HORIZONTAL_OR_VERTICAL_TEXT )
		    /* Vertical only */
		    && (FTL_Get_FeaturesRequest_Direction(&request->root) != FTL_VERTICAL)
		    /* But the request is NOT vertical */)
		   || (!( coverage & GX_MORX_COVERAGE_HORIZONTAL_OR_VERTICAL_TEXT )
		       /* Horizontal only */
		       && (FTL_Get_FeaturesRequest_Direction(&request->root) != FTL_HORIZONTAL)
		       /* But the request is NOT horizontal */)))
	    continue;
	  order = (coverage & GX_MORX_COVERAGE_ORDER_OF_PROCESSING_GLYPH_ARRAY)
	    ? GXL_DESCENDING
	    : GXL_ASCENDING;
	  if ( order == GXL_DESCENDING )
	    gx_glyphs_array_reverse( out->glyphs, out->length );

	  subtable_type = coverage & GX_MORX_COVERAGE_SUBTABLE_TYPE;
	  FT_TRACE2(( "\tSubtable No.%d, Address 0x%p, Type %d\n", 
		      k_subtbl, xchain_Subtbl, subtable_type ));
	  switch ( subtable_type )
	    {
	    case GX_MORX_REARRANGEMENT_SUBTABLE:
	      error = gx_xrearrangement_subst( xchain_Subtbl->body.rearrangement,
					       request->initial_state,
					       out );
	      break;
	    case GX_MORX_CONTEXTUAL_SUBTABLE:
	      error = gx_xcontextual_subst( xchain_Subtbl->body.contextual,
					    request->initial_state,
					    out );
	      break;
	    case GX_MORX_LIGATURE_SUBTABLE:
	      error = gx_xligature_subst( xchain_Subtbl->body.ligature,
					  request->initial_state,
					  out );
	      break;
	    case GX_MORX_RESERVED_SUBTABLE:
	      FT_ERROR(("Reserved format\n"));
	      break;
	    case GX_MORX_NONCONTEXTUAL_SUBTABLE:
	      error = gx_xnoncontextual_subst( xchain_Subtbl->body.noncontextual, out );
	      break;
	    case GX_MORX_INSERTION_SUBTABLE:
	      error = gx_xinsertion_subst ( xchain_Subtbl->body.insertion,
					    request->initial_state,
					    out );
	      break;
	    }
	  if ( order == GXL_DESCENDING )
	    gx_glyphs_array_reverse( out->glyphs, out->length );
	}
    }
  return FT_Err_Ok;
}


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                           Kerning                               ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#undef  PAIR_TAG
#define PAIR_TAG( left, right )  ( ( (FT_ULong)left << 16 ) | \
                                     (FT_ULong)right        )

static FT_Pos gx_kern_get_fmt0( GX_KerningSubtableFormat0Body fmt0,
				FT_UShort left_glyph,
				FT_UShort right_glyph );
static int gx_kern_fmt0_compar ( const void * a, const void * b);

static FT_Pos gx_kern_get_fmt1( GX_KerningSubtableFormat1Body fmt1,
				FT_UShort left_glyph,
				FT_UShort right_glyph );
static FT_Pos gx_kern_get_fmt2( GX_KerningSubtableFormat2Body fmt2,
				FT_UShort left_glyph,
				FT_UShort right_glyph );
static FT_Pos gx_kern_get_fmt3( GX_KerningSubtableFormat3Body fmt3,
				FT_UShort left_glyph,
				FT_UShort right_glyph );

FT_LOCAL_DEF( FT_Error )
gx_kern_get_pair_kerning ( GX_Kern kern,
			   FT_UShort left_glyph,
			   FT_UShort right_glyph,
			   FTL_Direction dir,
			   FT_Vector*  kerning )
{
  FT_Error error = GX_Err_Ok;
  FT_ULong i;
  GX_KerningSubtable subtable;
  GX_KerningSubtableHeader header;
  FT_UShort coverage;
  GX_KerningFormat fmt;
  GX_KerningSubtableBody body;
  FT_Pos * value;

  kerning->x = 0;
  kerning->y = 0;

  for ( i = 0; i < kern->nTables; i++ )
    {
      subtable = &(kern->subtables[i]);
      header   = &subtable->header;
      coverage = header->coverage;
      if ( coverage & GX_KERN_COVERAGE_VERTICAL )
	{
	  if ( dir != FTL_VERTICAL )
	    continue;
	  if ( coverage & GX_KERN_COVERAGE_CROSS_STREAM )
	    value = &kerning->x;
	  else
	    value = &kerning->y;
	}
      else
	{
	  if ( dir != FTL_HORIZONTAL )
	    continue;
	  if ( coverage & GX_KERN_COVERAGE_CROSS_STREAM )
	    value = &kerning->y;
	  else
	    value = &kerning->x;
	}

      fmt = coverage & GX_KERN_COVERAGE_FORMAT_MASK;
      body = &subtable->body;
      switch ( fmt )
	{
	case GX_KERN_FMT_ORDERED_LIST_OF_KERNING_PAIRS:
	  *value += gx_kern_get_fmt0 ( body->fmt0, left_glyph, right_glyph );
	  break;
	case GX_KERN_FMT_STATE_TABLE_FOR_CONTEXTUAL_KERNING:
	  *value += gx_kern_get_fmt1 ( body->fmt1, left_glyph, right_glyph );
	  break;
	case GX_KERN_FMT_SIMPLE_NXM_ARRAY_OF_KERNING_VALUES:
	  *value += gx_kern_get_fmt2 ( body->fmt2, left_glyph, right_glyph );
	  break;
	case GX_KERN_FMT_SIMPLE_NXM_ARRAY_OF_KERNING_INDICES:
	  *value += gx_kern_get_fmt3 ( body->fmt3, left_glyph, right_glyph );
	  break;
	default:
	  break;
	}
    }
  return error;
}

static FT_Pos
gx_kern_get_fmt0( GX_KerningSubtableFormat0Body fmt0,
		  FT_UShort left_glyph,
		  FT_UShort right_glyph )
{
  GX_KerningSubtableFormat0Entry entry;
  FT_ULong  search_tag = PAIR_TAG( left_glyph, right_glyph );
  if ( !fmt0->nPairs )
    return 0;

  entry = ft_bsearch(&search_tag,
		     fmt0->entries,
		     fmt0->nPairs,
		     sizeof(*fmt0->entries),
		     gx_kern_fmt0_compar);

  if ( entry )
    return entry->value;
  else
    return 0;
}

static int
gx_kern_fmt0_compar ( const void * search_tag, const void * entry)
{
  FT_ULong tag;
  tag = PAIR_TAG( ((GX_KerningSubtableFormat0Entry)entry)->left,
		  ((GX_KerningSubtableFormat0Entry)entry)->right );
  if ( *(FT_ULong*)search_tag < tag )
    return -1;
  else if ( *(FT_ULong*)search_tag > tag )
    return 1;
  else
    return 0;
}

static FT_Pos
gx_kern_get_fmt1( GX_KerningSubtableFormat1Body fmt1,
 		  FT_UShort left_glyph, FT_UShort right_glyph )
{
  return 0;			/* Do nothing */
}

static FT_Pos
gx_kern_get_fmt2( GX_KerningSubtableFormat2Body fmt2,
		  FT_UShort left_glyph, FT_UShort right_glyph )
{
  GX_KerningSubtableFormat2ClassTable left_class, right_class;
  FT_UShort left_index, right_index;
  FT_Byte kern_index;

  left_class = &fmt2->leftClass;
  right_class = &fmt2->rightClass;

  if (( left_class->firstGlyph > left_glyph )
      || ( !( left_class->firstGlyph + left_class->nGlyphs > left_glyph ) ))
    return 0;
  if (( right_class->firstGlyph > right_glyph )
      || ( !( right_class->firstGlyph + right_class->nGlyphs > right_glyph ) ))
    return 0;

  left_index  = left_glyph - left_class->firstGlyph;
  right_index = right_glyph - right_class->firstGlyph;

  FT_ASSERT( left_index < left_class->max_class );
  FT_ASSERT( right_index < right_class->max_class );

  kern_index = left_class->classes[left_index]
    + right_class->classes[right_index];
  return fmt2->values[kern_index];
}

static FT_Pos
gx_kern_get_fmt3( GX_KerningSubtableFormat3Body fmt3,
		  FT_UShort left_glyph, FT_UShort right_glyph )
{
  FT_Byte left_class, right_class;
  FT_Byte kern_index;

  if ( !( fmt3->glyphCount > left_glyph ) &&
       ( fmt3->glyphCount > right_glyph ) )
    return 0;

  left_class  = fmt3->leftClass[left_glyph];
  right_class = fmt3->rightClass[right_glyph];
  kern_index  = fmt3->kernIndex[left_class * fmt3->rightClassCount + right_class];
  FT_ASSERT ( kern_index < fmt3->kernValueCount );
  return fmt3->kernValue[kern_index];
}

FT_LOCAL_DEF( FT_Error )
gx_kern_get_contextual_kerning( GX_Kern kern,
				FTL_Glyphs_Array garray,
				FTL_Direction dir,
				GXL_Initial_State initial_state,
				FT_Vector * kerning )
{
  GX_KerningSubtable subtable;
  GX_KerningSubtableHeader header;
  FT_UShort coverage;
  GX_KerningFormat fmt;
  GX_KerningSubtableFormat1Body fmt1;
  FT_Bool cross_stream = 0;
  
  FT_ULong i;
  
  for ( i = 0; i < kern->nTables; i++ )
    {
      subtable = &(kern->subtables[i]);
      header   = &subtable->header;
      coverage =  header->coverage;
      if ( coverage & GX_KERN_COVERAGE_VERTICAL )
	{
	  if ( dir != FTL_VERTICAL )
	    continue;
	  if ( coverage & GX_KERN_COVERAGE_CROSS_STREAM )
	    cross_stream = 1;
	}
      else
	{
	  if ( dir != FTL_HORIZONTAL )
	    continue;
	  if ( coverage & GX_KERN_COVERAGE_CROSS_STREAM )
	    cross_stream = 1;
	}
      fmt = coverage & GX_KERN_COVERAGE_FORMAT_MASK;
      if ( fmt != GX_KERN_FMT_STATE_TABLE_FOR_CONTEXTUAL_KERNING )
	continue;
      fmt1 = subtable->body.fmt1;
      gx_contextual_kerning_calc( fmt1, 
				  garray, dir, cross_stream, initial_state, 
				  kerning );
    }
  return FT_Err_Ok;
}


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                          Tracking                               ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

typedef enum track_interpolation_range_ {
  TRACK_NO_INTERPOLATION = 0,
  TRACK_INTERPOLATION,
  TRACK_EXTRAPOLATION_SMALLER,
  TRACK_EXTRAPOLATION_LARGER
} track_interpolation_range;


static track_interpolation_range track_find_entry ( GX_TrackData track_data,
						    FT_Fixed track,
						    GX_TrackTableEntry * smaller_entry,
						    GX_TrackTableEntry * larger_entry );
static track_interpolation_range track_find_size (  GX_TrackData track_data,
						    FT_Fixed size,
						    FT_Fixed *smaller_size, FT_UShort *smaller_size_index,
						    FT_Fixed *larger_size,  FT_UShort *larger_size_index );

static FT_FWord track_interpolate( FT_Fixed x_t, 
				   FT_Fixed x_p, FT_Fixed x_q,
				   FT_FWord y_p, FT_FWord y_q );



FT_LOCAL_DEF( FT_Error )
gx_trak_get( GX_Trak trak, FT_Fixed track, FT_Fixed size, FTL_Direction dir, FT_FWord* value )
{
  GX_TrackData track_data = ( dir == FTL_HORIZONTAL )
    ? (&trak->horizData)
    : (&trak->vertData);

  GX_TrackTableEntry larger_entry = NULL, smaller_entry;
  FT_Fixed larger_track, smaller_track;
  track_interpolation_range entry_range;

  FT_UShort larger_size_index, smaller_size_index;
  FT_Fixed larger_size = 0, smaller_size;
  track_interpolation_range size_range;

  FT_FWord a, b, c, d;
  FT_FWord p, q;
  
  if ( !track_data )
    {
      *value = 0;
      return FT_Err_Ok;
    }


  entry_range  	= track_find_entry ( track_data, track, &smaller_entry, &larger_entry );
  larger_track 	= larger_entry->track;
  smaller_track = smaller_entry->track;

  size_range  	= track_find_size  ( track_data, size,
				   &smaller_size, &smaller_size_index,
				   &larger_size,  &larger_size_index );

  a = smaller_entry->tracking_value[smaller_size_index];
  b = smaller_entry->tracking_value[larger_size_index];
  c = larger_entry->tracking_value[smaller_size_index];
  d = larger_entry->tracking_value[larger_size_index];

  /* -------------------------------------------------------------------
     
                ^ size
                |
            B   |   D
                |
            A   |   C
  --------------+---------------> track 
               O|

    A = ( smaller_track, smaller_size, a )
    B = ( smaller_track, larger_size,  b )
    C = ( larger_track,  smaller_size, c )
    D = ( larger_track,  larger_size,  d )
    -----------V---------------------------
    P = ( track,         smaller_size, p )
    Q = ( track          larger_size,  q )
    --------------------------V------------
    T = ( track,         size,        *value)

  ------------------------------------------------------------------- */
  
  if ( entry_range == TRACK_NO_INTERPOLATION )
    {
      p = a;
      q = b;
    }
  else
    {
      p = track_interpolate ( track, 
			      smaller_track, larger_track,
			      a, c );
      q = track_interpolate ( track,
			      smaller_track, larger_track,
			      b, d );
    }

  if ( size_range == TRACK_NO_INTERPOLATION )
    *value = p;
  else
    *value = track_interpolate( size, 
				smaller_size, larger_size, 
				p, q );
  return FT_Err_Ok;
}

static track_interpolation_range
track_find_entry ( GX_TrackData track_data,
		   FT_Fixed track,
		   GX_TrackTableEntry * smaller_entry,
		   GX_TrackTableEntry * larger_entry )
{
  FT_UShort i;
  track_interpolation_range range;
  FT_Bool extrapolation = FALSE;

  if ( track < track_data->trackTable[0].track )
    {
      i     	    = 0;
      range 	    =  TRACK_EXTRAPOLATION_SMALLER;
      extrapolation = TRUE;
    }
  else if ( track_data->trackTable[track_data->nTracks - 1].track < track )
    {
      i     	    = track_data->nTracks - 2;
      range 	    = TRACK_EXTRAPOLATION_LARGER;
      extrapolation = TRUE;
    }

  if ( extrapolation )
    {
      *smaller_entry = &track_data->trackTable[i];
      *larger_entry  = &track_data->trackTable[i + 1];
      return range;
    }


  for ( i = 0; i < track_data->nTracks; i++ )
    {
      *smaller_entry = *larger_entry;
      *larger_entry = &track_data->trackTable[i];
      if ( track == (*larger_entry)->track )
	{
	  *smaller_entry = *larger_entry;
	  return TRACK_NO_INTERPOLATION;
	}
      else if ( track < (*larger_entry)->track )
	return TRACK_INTERPOLATION;
    }
  GX_ASSERT_NOT_REACHED();
  return TRACK_INTERPOLATION;
}

static track_interpolation_range
track_find_size (  GX_TrackData track_data,
		   FT_Fixed size,
		   FT_Fixed *smaller_size, FT_UShort *smaller_size_index,
		   FT_Fixed *larger_size, FT_UShort *larger_size_index)
{
  FT_UShort i;
  track_interpolation_range range;
  FT_Bool extrapolation = FALSE;

  if ( size < track_data->sizeTable[0] )
    {
      i 	    = 0;
      range 	    = TRACK_EXTRAPOLATION_SMALLER;
      extrapolation = TRUE;
    }
  else if ( track_data->sizeTable[track_data->nSizes - 1] < size )
    {
      i 	    = track_data->nSizes - 2;
      range 	    = TRACK_EXTRAPOLATION_LARGER;
      extrapolation = TRUE;
    }
  if ( extrapolation )
    {
      *smaller_size 	 = track_data->sizeTable[i];
      *smaller_size_index = i;
      *larger_size 	 = track_data->sizeTable[i + 1];
      *larger_size_index  = i + 1;
      return range;
    }

  for ( i = 0; i < track_data->nSizes; i++ )
    {
      *smaller_size 	  = *larger_size;
      *smaller_size_index = *larger_size_index;
      *larger_size 	 = track_data->sizeTable[i];
      *larger_size_index = i;
      if ( size == *larger_size )
	{
	  *smaller_size = *larger_size;
	  *smaller_size_index = *larger_size_index;
	  return TRACK_NO_INTERPOLATION;
	}
      else if ( size < *larger_size )
	return TRACK_INTERPOLATION;
    }
  GX_ASSERT_NOT_REACHED();
  return TRACK_INTERPOLATION;
}

static FT_FWord
track_interpolate( FT_Fixed x_t, 
		   FT_Fixed x_p, FT_Fixed x_q,
		   FT_FWord y_p, FT_FWord y_q )
{
  FT_FWord y_t;

  /* TODO: better to use FT_MulDiv_No_Round. */
  y_t = ( y_q * ( x_t - x_p ) + y_p * ( x_q - x_t )) / (x_q - x_p);
  return y_t;
}

FT_LOCAL_DEF( FT_UShort )
gx_trak_count_name_index( GX_Trak trak )
{
  GX_TrackData track_data;
  GX_TrackTableEntry track_table;
  FT_UShort nTracks;
  FT_UShort i;
  
  FT_UShort h = 0;
  FT_UShort v = 0;

  /*
   * Horiz
   */
  track_data = &trak->horizData;
  nTracks    = track_data->nTracks;
  for ( i = 0; i < nTracks; i++ )
    {
      track_table = &track_data->trackTable[i];
      if ( track_table->nameIndex )
	h++;
    }

  /*
   * Vert
   */
  track_data = &trak->vertData;
  nTracks    = track_data->nTracks;
  for ( i = 0; i < nTracks; i++ )
    {
      track_table = &track_data->trackTable[i];
      if ( track_table->nameIndex )
	h++;
    }
  
  return h + v;
}

FT_LOCAL( FT_Error )
gx_trak_get_name ( GX_Trak trak,
		   FT_UShort index,
		   FT_UShort     * name_index,
		   FTL_Direction * dir,
		   FT_Fixed      * track )
{
  GX_TrackData track_data;
  GX_TrackTableEntry track_table;
  FT_UShort i;
  
  /*
   * Horiz
   */
  track_data = &trak->horizData;
  for ( i = 0; i < track_data->nTracks; i++ )
    {
      track_table = &track_data->trackTable[i];
      if ( track_table->nameIndex == 0 )
	continue ;
     
      if ( index == 0 )
	goto Set_Values;
      else
	index--;
    }

  /*
   * Vert
   */
  track_data = &trak->vertData;
  for ( i = 0; i < track_data->nTracks; i++ )
    {
      track_table = &track_data->trackTable[i];
      if ( track_table->nameIndex  == 0 )
	continue ;

      if ( index == 0 )
	goto Set_Values;
      else
	index--;
    }
  
  return GX_Err_Invalid_Argument;
  
 Set_Values:
  *name_index = track_table->nameIndex;
  *dir = ( track_data = &trak->horizData )
    ? FTL_HORIZONTAL
    : FTL_VERTICAL;
  *track = track_table->track;
  return FT_Err_Ok;
}
