/***************************************************************************/
/*                                                                         */
/*  gxload.c                                                               */
/*                                                                         */
/*    Functions load AAT/TrueTypeGX tables(body)                           */
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

/* TODO: cleanup: use variable to get the size of type instead of type */

#include <ft2build.h>

#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_MEMORY_H
#include FT_TRUETYPE_TAGS_H

#include "gxtypes.h"
#include "gxload.h"
#include "gxlookuptbl.h"
#include "gxstatetbl.h"
#include "gxutils.h"
#include "gxerrors.h"
#include "gxaccess.h"

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxload

  static void
  gx_feat_done( GX_Feat   feat,
		FT_Memory memory );
  static void
  gx_trak_done( GX_Trak   trak,
		FT_Memory memory );

  static void
  gx_kern_done( GX_Kern   kern,
		FT_Memory memory );

  static void
  gx_prop_done( GX_Prop   prop,
		FT_Memory memory );

  static void
  gx_opbd_done( GX_Opbd   opbd,
		FT_Memory memory );

  static void
  gx_lcar_done( GX_Lcar   lcar,
		FT_Memory memory );

  static void
  gx_bsln_done( GX_Bsln   bsln,
		FT_Memory memory );

  static void
  gx_mort_done( GX_Mort   mort,
		FT_Memory memory );

  static void
  gx_morx_done( GX_Morx   morx,
		FT_Memory memory );

  static void
  gx_fmtx_done( GX_Fmtx   fmtx,
		FT_Memory memory );

  static void
  gx_fdsc_done( GX_Fdsc   fdsc,
		FT_Memory memory );

  static void
  gx_just_done( GX_Just   just,
		FT_Memory memory );

  static void
  gx_fvar_done( GX_Fvar   fvar,
		FT_Memory memory );

#define GENERIC_LOOKUP_TABLE_CB_DATA_ZERO {NULL, NULL, NULL, 0, NULL}
typedef struct generic_lookup_table_cb_data_rec_
{
  GX_Face face;
  FT_Stream stream;
  GX_LookupTable lookup_table;
  /* From the spec file:
     If TABLE_TAG == 0, "The value of each lookupSegment is a 16-bit
     offset from the start of the lookup table to an array of 16-bit
     property values, one for each glyph in the segment. "
     If not "the value of each lookupSingle is a 16-bit offset from
     the start of the table " specified by TABLE_TAG. */
  FT_Int table_tag;		
  FT_Pointer extra;
} generic_lookup_table_cb_data_rec, *generic_lookup_table_cb_data;

static FT_Error
generic_lookup_table_segment_array_loader ( GX_LookupTable_Format format,
					    FT_UShort lastGlyph,
					    FT_UShort firstGlyph,
					    GX_LookupValue value,
					    FT_Pointer user );
static FT_Error
generic_lookup_table_segment_array_finalizer ( GX_LookupTable_Format format,
					       FT_UShort lastGlyph,
					       FT_UShort firstGlyph,
					       GX_LookupValue value,
					       FT_Pointer user );

#define GENERIC_STATE_TABLE_CB_DATA_ZERO {0, 0, NULL, NULL, NULL}
#define GENERIC_XSTATE_TABLE_CB_DATA_ZERO {0, 0, NULL, NULL, NULL}
typedef struct generic_state_table_cb_data_rec_
{
  FT_ULong base;
  FT_ULong table_offset;
  FT_Stream stream;
  GX_Face   face;
  FT_Pointer extra;
} generic_state_table_cb_data_rec, 
  generic_xstate_table_cb_data_rec,
  *generic_state_table_cb_data, 
  *generic_xstate_table_cb_data;

static FT_Error
generic_load_noncontextual_subtable ( GX_Face face,
				      FT_Stream stream,
				      FT_UShort length,
				      GX_MetamorphosisSubtableBody body,
				      FT_Int tag );

static void
generic_triple_offset_diff ( FT_ULong ligActionTable_offset,
			     FT_ULong componentTable_offset,
			     FT_ULong ligatureTable_offset,
			     FT_ULong length,
			     FT_UShort *ligActionTable_nAction,
			     FT_UShort *componentTable_nComponent,
			     FT_UShort *ligatureTable_nLigature );

static FT_Error
gx_table_init(GX_Table table_info, 
		   GX_Face face, 
		   FT_ULong tag,
		   FT_Stream stream,
		   GX_Table_Done_Func done_table);


/******************************TRAK************************************/
  static FT_Error
  gx_face_load_trak_data( GX_Face   face,
			  FT_Stream stream,
			  FT_UShort offset,
			  GX_TrackData data );
  static FT_Error
  gx_face_load_trak_values ( GX_Face face, FT_Stream stream,
			     FT_UShort nTracks, FT_UShort nSizes,
			     GX_TrackTableEntry trackTable );
  static FT_Error
  gx_face_load_trak_table_entry ( GX_Face   face,
				  FT_Stream stream,
				  FT_UShort nTracks,
				  GX_TrackTableEntry * table_entry );
  static FT_Error
  gx_face_load_trak_size_table ( GX_Face   face,
				 FT_Stream stream,
				 FT_UShort nSizes,
				 FT_ULong  sizeTableOffset,
				 FT_Long **sizeTable );

  static void
  gx_trak_done_values ( FT_Memory memory,
			     FT_UShort nTracks, 
			     GX_TrackTableEntry trackTable );

  static void
  gx_trak_done_data(FT_Memory memory, GX_TrackData data);

  static FT_Error
  gx_face_load_trak_values ( GX_Face face, FT_Stream stream,
			     FT_UShort nTracks, FT_UShort nSizes,
			     GX_TrackTableEntry trackTable )
  {
    FT_Int i, j;
    FT_Error error;
    FT_Memory memory = stream->memory;
    FT_FWord * base_value;
    FT_ULong table_offset;
    
    if ( FT_NEW_ARRAY ( base_value, nTracks * nSizes ) )
      return error;

    /* assert -> nTracks != 0, nSizes != 0 */
    
    if ( ( error = face->goto_table( face, TTAG_trak, stream, 0 ) ) )
      return error;
    table_offset = FT_STREAM_POS();
	
    for ( i = 0; i < nTracks; i++ )
      {
	trackTable[i].tracking_value = base_value + (i * nSizes);
	if ( FT_STREAM_SEEK( table_offset + trackTable[i].offset ) ||
	     FT_FRAME_ENTER ( sizeof(FT_UShort) * nSizes )           )
	  goto Failure;

	for ( j = 0; j < nSizes; j++ )
	  trackTable[i].tracking_value[j] = FT_GET_SHORT();

	FT_FRAME_EXIT();
      }
    return GX_Err_Ok;
  Failure:
    gx_trak_done_values ( memory, nTracks, trackTable );
    return error;
  }

  static void
  gx_trak_done_values ( FT_Memory memory,
			     FT_UShort nTracks, 
			     GX_TrackTableEntry trackTable )
  {
    FT_Int i;
    FT_FREE (trackTable[0].tracking_value);
    for ( i = 0; i < nTracks; i++ )
      trackTable[i].tracking_value = NULL;
  }

  static FT_Error
  gx_face_load_trak_table_entry ( GX_Face   face,
				  FT_Stream stream,
				  FT_UShort nTracks,
				  GX_TrackTableEntry * table_entry )
  {
    FT_Int i;
    FT_Error error;
    FT_Memory memory = stream->memory;
    
    const FT_Frame_Field track_table_entry_fields [] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_TrackTableEntryRec
	FT_FRAME_START ( 8 ),
	    FT_FRAME_LONG  (track),
	    FT_FRAME_USHORT (nameIndex),
	    FT_FRAME_USHORT (offset),
	FT_FRAME_END
      };
    
    if ( FT_NEW_ARRAY( *table_entry, nTracks ) )
      return error;
    
    for ( i = 0; i < nTracks ; i++ )
      {
	(*table_entry)[i].tracking_value = NULL;
	if ( FT_STREAM_READ_FIELDS( track_table_entry_fields, 
				    &(*table_entry)[i] ) )
	  goto Failure;
      }
    return GX_Err_Ok;
  Failure:
    FT_FREE(*table_entry);
    *table_entry = NULL;
    return error;
  }

  static FT_Error
  gx_face_load_trak_size_table ( GX_Face   face,
				 FT_Stream stream,
				 FT_UShort nSizes,
				 FT_ULong  sizeTableOffset,
				 FT_Long **sizeTable )
  {
    FT_Int i;
    FT_Error error;
    FT_Memory memory = stream->memory;    
    
    if ( (error = face->goto_table( face, TTAG_trak, stream, 0 )) || 
	 FT_STREAM_SKIP( sizeTableOffset ) )
      goto Exit;

    *sizeTable = NULL;

    if ( FT_NEW_ARRAY( *sizeTable, nSizes ) )
      goto Exit;
    
    if ( FT_FRAME_ENTER ( sizeof( **sizeTable ) * nSizes ) )
      goto Failure;
    for ( i = 0; i < nSizes; i++)
      (*sizeTable)[i] = FT_GET_ULONG();
    FT_FRAME_EXIT();

  Exit:
    return error;
  Failure:
    FT_FREE(*sizeTable);
    return error;
  }

  static FT_Error
  gx_face_load_trak_data( GX_Face   face,
			  FT_Stream stream,
			  FT_UShort offset,
			  GX_TrackData data )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;

    const FT_Frame_Field track_data_fields [] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_TrackDataRec
	FT_FRAME_START( 8 ),
	    FT_FRAME_USHORT ( nTracks ),
	    FT_FRAME_USHORT ( nSizes ),
	    FT_FRAME_ULONG  ( sizeTableOffset ),
	FT_FRAME_END
      };

    if ( ( error = face->goto_table( face, TTAG_trak, stream, 0 ) ) ||
	 FT_STREAM_SKIP( offset )                                   ||
	 FT_STREAM_READ_FIELDS( track_data_fields, data ) )
      goto Exit;
    
    if ( data->nTracks < 3 )
      {
	FT_TRACE2(( " Too few nTracks in kern\n" ));
	error = GX_Err_Invalid_File_Format;
	goto Exit;
      }
    if ( data->nSizes < 2 )
      {
	FT_TRACE2(( " Too few nSizes in kern\n" ));
	error = GX_Err_Invalid_File_Format;
	goto Exit;
      }
    
    if ( ( error = gx_face_load_trak_table_entry ( face, stream, 
						   data->nTracks,
						   &data->trackTable ) )) 
      goto Exit;
    
    if ( ( error = gx_face_load_trak_values ( face, stream, 
					      data->nTracks, data->nSizes,
					      data->trackTable ) ) )
      goto Failure;
    

    if ( ( error = gx_face_load_trak_size_table ( face, stream,
						  data->nSizes,
						  data->sizeTableOffset,
						  &data->sizeTable ) ) )

      {
	gx_trak_done_values( memory, data->nTracks, data->trackTable);
	goto Failure;
      }
    
  Exit:
    return error;
  Failure:
    FT_FREE(data->trackTable);
    data->trackTable = NULL;
    return error;
  }

  static void
  gx_trak_done_data(FT_Memory memory, GX_TrackData data)
  {
    gx_trak_done_values(memory, data->nTracks, data->trackTable);
    FT_FREE(data->sizeTable);
    data->sizeTable = NULL;
    FT_FREE(data->trackTable);
    data->trackTable = NULL;
  }


  FT_LOCAL_DEF ( FT_Error )
  gx_face_load_trak( GX_Face   face,
		     FT_Stream stream,
		     GX_Trak trak )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    
    const FT_Frame_Field trak_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_TrakRec
	FT_FRAME_START( 12 ),
	    FT_FRAME_ULONG ( version ),
	    FT_FRAME_USHORT ( format ),
	    FT_FRAME_USHORT ( horizOffset ),
	    FT_FRAME_USHORT ( vertOffset ),
	    FT_FRAME_USHORT ( reserved ),
	FT_FRAME_END
      };
    
    if (( error = gx_table_init( &(trak->root), face, TTAG_trak, stream,
				 (GX_Table_Done_Func)gx_trak_done) ))
      goto Exit;
    
    if ( FT_STREAM_READ_FIELDS( trak_fields, trak ) )
      goto Exit;

    if ( trak->horizOffset &&
	 ( error = gx_face_load_trak_data (face, stream, 
					   trak->horizOffset, 
					   &trak->horizData) ))
      goto Exit;

  
    if ( trak->vertOffset && 
	 ( error = gx_face_load_trak_data (face, stream, 
					   trak->vertOffset, 
					   &trak->vertData) ))
      goto Failure;
    /* FT_TRACE2(( "loaded\n" )); */
  Exit:
    return error;
  Failure:
    gx_trak_done_data(memory, &trak->horizData);
    return error;
  }


  FT_LOCAL_DEF ( void )
  gx_trak_done( GX_Trak trak,
		FT_Memory memory )
  {
    if ( trak->horizOffset )
      gx_trak_done_data(memory, &trak->horizData);
    if ( trak->vertOffset )
      gx_trak_done_data(memory, &trak->vertData);
  }


/******************************FEAT************************************/
  FT_Error
  gx_face_load_feat_settingName ( GX_Face face,
				  FT_Stream stream,
				  FT_ULong  settingTable,
				  FT_UShort nSettings,
				  GX_FeatureSettingName settingName )
  {
    FT_Error error;
    FT_Int i;

    const FT_Frame_Field feature_settingName_fields [] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_FeatureSettingNameRec
	    FT_FRAME_START  ( 4 ),
	    FT_FRAME_USHORT (setting),
	    FT_FRAME_SHORT  (nameIndex),
	FT_FRAME_END
      };
    
    error = face->goto_table( face, TTAG_feat, stream, 0 );
    if ( error ||
	 FT_STREAM_SKIP( settingTable ) )
      goto Exit;
    
    for ( i = 0; i < nSettings; i++ )
      if ( FT_STREAM_READ_FIELDS( feature_settingName_fields,
				  &(settingName[i]) ))
	goto Exit;
  Exit:
    return error;
  }

  FT_Error
  gx_face_load_feat_names( GX_Face   face,
			   FT_Stream stream,
			   FT_UShort featureNameCount,
			   GX_FeatureName names)
  {
    FT_Error error;
    FT_Memory memory 	   	       = stream->memory;
    FT_Int total_nSettings, offset;
    GX_FeatureSettingName settingName;
    FT_Int i;

    const FT_Frame_Field feature_name_fields [] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_FeatureNameRec		
	FT_FRAME_START  ( 12 ),
	    FT_FRAME_USHORT ( feature ),
	    FT_FRAME_USHORT ( nSettings ),
	    FT_FRAME_ULONG  ( settingTable ),
	    FT_FRAME_USHORT ( featureFlags ),
	    FT_FRAME_SHORT  ( nameIndex ),
	FT_FRAME_END
      };
    
    for ( i = 0, total_nSettings = 0; i < featureNameCount; i++ )
      {
	if ( FT_STREAM_READ_FIELDS( feature_name_fields, &(names[i]) ) )
	  return error;
	total_nSettings += names[i].nSettings;
      }

      
    if ( FT_NEW_ARRAY ( settingName, total_nSettings ) )
      return error;
    
    for ( i = 0, offset = 0; i < featureNameCount; i++ )
      {
	names[i].settingName = settingName + offset;
	offset += names[i].nSettings;
      }

    for ( i = 0; i < featureNameCount; i++ )
      {
	error = gx_face_load_feat_settingName(face, 
					      stream,
					      names[i].settingTable,
					      names[i].nSettings, 
					      names[i].settingName);
	if ( error )
	  {
	    FT_FREE( settingName );
	    for ( i = 0; i < featureNameCount; i++ )
	      names[i].settingName = NULL;
	    return error;
	  }
      }
    return error;
  }
  
  static void
  gx_feat_done_names( FT_Memory memory,
		      FT_UShort featureNameCount,
		      GX_FeatureName names)
  {
    GX_FeatureSettingName settingName;
    FT_Int i;
    
    settingName = names[0].settingName;
    for ( i = 0; i < featureNameCount; i++ )
      names[i].settingName = NULL;
    FT_FREE(settingName);
  }

  FT_LOCAL_DEF ( FT_Error )
  gx_face_load_feat( GX_Face   face,
		     FT_Stream stream,
		     GX_Feat feat )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    
    const FT_Frame_Field feat_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_FeatRec
	FT_FRAME_START ( 12 ),
	    FT_FRAME_LONG   ( version ),
	    FT_FRAME_USHORT ( featureNameCount ),
	    FT_FRAME_USHORT ( reserved1 ),
	    FT_FRAME_ULONG  ( reserved2 ),
	FT_FRAME_END
      };
    
    /* FT_TRACE2(( "Feature " )); */
    if (( error = gx_table_init( &(feat->root), face, TTAG_feat, stream,
				 (GX_Table_Done_Func) gx_feat_done) ))
      goto Exit;
    
    if ( FT_STREAM_READ_FIELDS( feat_fields, feat ) )
      goto Exit;		/* err? */

    /* FT_TRACE3(( "\n version: 0x%8x" "(0x00010000 is expected)\n", */
    /*              feat->version )); */

    if ( feat->featureNameCount == 0 )
      {
	feat->names = NULL;
	goto Exit;
      }
    
    if ( FT_NEW_ARRAY( feat->names, feat->featureNameCount ) )
      goto Exit;

    error = gx_face_load_feat_names ( face, stream, 
				      feat->featureNameCount,
				      feat->names );
    if ( error )
      {
	FT_FREE( feat->names );
	feat->names = NULL;
	goto Exit;
      }

  Exit:
    return error;
  }

  FT_LOCAL_DEF ( void )
  gx_feat_done( GX_Feat   feat,
		FT_Memory memory )
		     
  {
    GX_FeatureName names;
    
    names = feat->names;
    if ( feat->names )
      {
	gx_feat_done_names (memory, feat->featureNameCount, feat->names);
	feat->names = NULL;
	FT_FREE(names);
      }
  }

/******************************PROP************************************/
  FT_LOCAL_DEF ( FT_Error )
  gx_face_load_prop( GX_Face   face,
		     FT_Stream stream,
		     GX_Prop   prop )
  {
    FT_Error error;
    GX_LookupTable lookup_table = &prop->lookup_data;

    const FT_Frame_Field prop_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_PropRec
	FT_FRAME_START ( 8 ),
	    FT_FRAME_ULONG  ( version ), /* LONG? */
	    FT_FRAME_USHORT ( format ),
	    FT_FRAME_USHORT ( default_properties ),
	FT_FRAME_END
      };

    GX_LookupTable_FuncsRec funcs 	       = GX_LOOKUP_TABLE_FUNC_ZERO;
    generic_lookup_table_cb_data_rec user_data = GENERIC_LOOKUP_TABLE_CB_DATA_ZERO;

    funcs.segment_array_func = generic_lookup_table_segment_array_loader;
    user_data.face 	    = face;
    user_data.stream 	    = stream;
    user_data.lookup_table  = lookup_table;
    user_data.table_tag     = 0;
    /* FT_TRACE2(( "Glyph Properties " )); */
    
    if (( error = gx_table_init( &(prop->root), face, TTAG_prop, stream,
				 (GX_Table_Done_Func)gx_prop_done) ))
      goto Exit;

    if ( FT_STREAM_READ_FIELDS( prop_fields, prop ) )
      goto Exit;		/* err? */
    /* FT_TRACE3(( "\n version: 0x%8x\n", prop->version )); */

    if ( prop->format == 0)
      {
	/* NO LOOKUP DATA */
	lookup_table->format  = 0;
	lookup_table->fsHeader.any = 0;
	goto Exit;
      }
    
    if (( error = gx_face_load_LookupTable ( face, stream, lookup_table )))
      goto Exit;
    if ( lookup_table->format == GX_LOOKUPTABLE_SEGMENT_ARRAY )
      {
	if (( error = gx_LookupTable_traverse_low( lookup_table, 
						   &funcs, 
						   &user_data ) ))
	  goto Failure;
      }
  Exit:
    return error;
  Failure:
    gx_LookupTable_free( lookup_table, face->root.driver->root.memory );
    return error;
  }

  FT_LOCAL_DEF( void )
  gx_prop_done( GX_Prop   prop,
		FT_Memory memory )
		     
  {
    GX_LookupTable lookup_table   = &prop->lookup_data;
    GX_LookupTable_FuncsRec funcs = GX_LOOKUP_TABLE_FUNC_ZERO;

    funcs.segment_array_func = generic_lookup_table_segment_array_finalizer;

    if ( prop->format != 0)
      {
	if ( lookup_table->format == GX_LOOKUPTABLE_SEGMENT_ARRAY )
	  gx_LookupTable_traverse_low( lookup_table,
				       &funcs,
				       memory );
	gx_LookupTable_free ( lookup_table, memory );
      }
  }  


/******************************OPBD************************************/
  static FT_Error
  gx_opbd_load_data ( GX_Face face, 
		      FT_Stream stream,
		      FT_ULong offset,
		      FT_UShort format,
		      GX_OpticalBoundsData data )
  {
    FT_Error error;
    const FT_Frame_Field opbd_distance_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_OpticalBoundsDataRec
	FT_FRAME_START ( 8 ),
	FT_FRAME_SHORT (distance.left_side),
	FT_FRAME_SHORT (distance.top_side),
	FT_FRAME_SHORT (distance.right_side),
	FT_FRAME_SHORT (distance.bottom_side),
	FT_FRAME_END
      };
    const FT_Frame_Field opbd_control_points_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_OpticalBoundsDataRec
	FT_FRAME_START ( 8 ),
	FT_FRAME_USHORT (control_points.left_side),
	FT_FRAME_USHORT (control_points.top_side),
	FT_FRAME_USHORT (control_points.right_side),
	FT_FRAME_USHORT (control_points.bottom_side),
	FT_FRAME_END
      };

    if ( FT_STREAM_SEEK(offset) )
      return error;
    
    if ( format == GX_OPBD_DISTANCE )
      {
	if ( FT_STREAM_READ_FIELDS( opbd_distance_fields, data ) )
	  return error;
      }
    else
      {
	if ( FT_STREAM_READ_FIELDS( opbd_control_points_fields, data ) )
	  return error;
      }
    return error;
  }

  static FT_Error 
  opbd_lookup_table_generic_loader( GX_LookupTable_Format format,
				    GX_LookupValue value,
				    FT_Pointer user )
  {
    FT_Error error;
    GX_Face face       	    = ((generic_lookup_table_cb_data)user)->face;
    FT_Pointer extra        = ((generic_lookup_table_cb_data)user)->extra;
    FT_UShort data_format   = *(FT_UShort*)extra;
    FT_Stream stream   	    = ((generic_lookup_table_cb_data)user)->stream;
    FT_Memory memory = stream->memory;
    FT_Short offset = value->raw.s;
    GX_OpticalBoundsData opbd_data;
   
    if ( ( error = face->goto_table(face, TTAG_opbd, stream, 0 ) ) )
      return error;
   
    if ( FT_MEM_NEW(opbd_data) )
      return error;
    
    if ( ( error = gx_opbd_load_data( face,
				      stream,
				      FT_STREAM_POS() + offset, 
				      data_format,
				      opbd_data ) ) )
      goto Failure;
    value->extra.opbd_data = opbd_data;
    return error;
  Failure:
    FT_FREE(opbd_data);
    return error;
  }
 
  static FT_Error
  opbd_lookup_table_segment_array_loader ( GX_LookupTable_Format format,
					   FT_UShort lastGlyph,
					   FT_UShort firstGlyph,
					   GX_LookupValue value,
					   FT_Pointer user )
  {
    FT_Error error;
    FT_UShort segment_count = lastGlyph - firstGlyph + 1;
    
    GX_Face face          = ((generic_lookup_table_cb_data)user)->face;
    FT_Pointer extra        = ((generic_lookup_table_cb_data)user)->extra;
    FT_UShort data_format   = *(FT_UShort*)extra;
    FT_Stream stream 	  = ((generic_lookup_table_cb_data)user)->stream;
    FT_Memory memory      = stream->memory;
    FT_Short offset  	  = value->raw.s;
    GX_OpticalBoundsData opbd_data;
    
    FT_ULong base_offset;
    FT_Int   opbd_data_advance = 4 * sizeof(FT_Short);
    FT_Int i;
    
    /* Offset from the start of opbd table (written in spec) */
    if ( ( error = face->goto_table(face, TTAG_opbd, stream, 0 ) ) )
      goto Exit;
    
    if ( ( FT_NEW_ARRAY ( opbd_data, segment_count ) ) )
      goto Exit;
    
    base_offset = FT_STREAM_POS() + offset;
    
    for ( i = 0; i < segment_count; i++ )
      {
	if ( ( error = gx_opbd_load_data( face,
					  stream,
					  base_offset + i * opbd_data_advance,
					  data_format,
					  &(opbd_data[i]) ) ) )
	  goto Failure;
      }
    value->extra.opbd_data = opbd_data;
  Exit:
    return error;
  Failure:
    FT_FREE(opbd_data);
    return error;
  }

  FT_LOCAL_DEF( FT_Error )
  gx_face_load_opbd( GX_Face   face,
		     FT_Stream stream,
		     GX_Opbd   opbd )
  {
    FT_Error error;
    const FT_Frame_Field opbd_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_OpbdRec
	FT_FRAME_START ( 6 ),
	  FT_FRAME_ULONG ( version ),
	  FT_FRAME_USHORT( format ),
	FT_FRAME_END
      };
    GX_LookupTable_FuncsRec funcs 		   = GX_LOOKUP_TABLE_FUNC_ZERO;
    generic_lookup_table_cb_data_rec callback_data = GENERIC_LOOKUP_TABLE_CB_DATA_ZERO;

    funcs.generic_func 	     = opbd_lookup_table_generic_loader;
    funcs.segment_array_func = opbd_lookup_table_segment_array_loader;
    callback_data.face 	     = face;
    callback_data.stream     = stream;

    /* FT_TRACE2(( "Optical Bounds" )); */
    if (( error = gx_table_init( &(opbd->root), face, TTAG_opbd, stream,
				 (GX_Table_Done_Func)gx_opbd_done) ))
      goto Exit;
    
    if ( FT_STREAM_READ_FIELDS( opbd_fields, opbd ) )
      goto Exit;
    error = gx_face_load_LookupTable( face, stream, &(opbd->lookup_data) );
    if ( error )
      goto Exit;
    
    callback_data.extra = &opbd->format;
    error = gx_LookupTable_traverse_low( &(opbd->lookup_data),
					 &funcs,
					 &callback_data );
  Exit:
    return error;
  }

  static FT_Error 
  opbd_lookup_table_generic_finalizer( GX_LookupTable_Format format,
				       GX_LookupValue value,
				       FT_Pointer user )
  {
    FT_Memory memory = user;
    GX_OpticalBoundsData opbd_data;
    
    if ( !value->extra.opbd_data )
      return GX_Err_Ok;

    opbd_data = value->extra.opbd_data;
    FT_FREE(opbd_data);
    value->extra.opbd_data = NULL;
    return GX_Err_Ok;
  }

  FT_LOCAL_DEF ( void )
  gx_opbd_done( GX_Opbd   opbd,
		FT_Memory memory )
  {
    GX_LookupTable_FuncsRec funcs = GX_LOOKUP_TABLE_FUNC_ZERO;

    funcs.generic_func 	     = opbd_lookup_table_generic_finalizer;
    funcs.segment_array_func = generic_lookup_table_segment_array_finalizer;

    gx_LookupTable_traverse_low( &(opbd->lookup_data),
				 &funcs,
				 memory );
    gx_LookupTable_free ( &(opbd->lookup_data), memory );
  }


/******************************LCAR************************************/
  static FT_Error
  gx_lcar_load_partials ( FT_Stream stream, 
			  FT_UShort count,
			  FT_Short *partials)
  {
    FT_Error error;
    FT_Int   i;
    
    if ( FT_FRAME_ENTER ( sizeof( *partials ) * count ) )
      goto Exit;
    
    for ( i = 0; i < count; i++ )
      partials[i] = FT_GET_USHORT();
    
    FT_FRAME_EXIT();
  Exit:
    return error;
  }
	
  static FT_Error
  gx_lcar_load_class_entry ( GX_Face face,
			     FT_Stream stream,
			     FT_ULong offset,
			     GX_LigCaretClassEntry * class_entry )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    FT_UShort count;
    FT_Int base_size = sizeof (**class_entry);
    
    if ( FT_STREAM_SEEK(offset) )
      goto Exit;
    
    if ( FT_READ_USHORT(count) )
      goto Exit;

    if (count == 0)
      {
	*class_entry = NULL;
	return GX_Err_Ok;
      }

    if ( FT_ALLOC ( *class_entry, 
		    base_size + count * sizeof(*(*class_entry)->partials) ) )
      goto Exit;
     
    (*class_entry)->count = count;
    (*class_entry)->partials = (FT_Short *)(((char *)*class_entry) + base_size);
    
    error = gx_lcar_load_partials(stream, 
				  (*class_entry)->count, 
				  (*class_entry)->partials);
    if ( error )
      goto Failure;
  Exit:
    return error;
  Failure:
    FT_FREE(*class_entry);
    *class_entry = NULL;
    return error;
    
  }
  
  static void
  gx_lcar_free_class_entry ( FT_Memory memory,
			     GX_LigCaretClassEntry class_entry )
  {
    class_entry->count 	  = 0;	/* Overkill? */
    class_entry->partials = NULL;
    FT_FREE( class_entry );
  }
			      
  static FT_Error 
  lcar_lookup_table_generic_loader( GX_LookupTable_Format format,
				    GX_LookupValue value,
				    FT_Pointer user )
  {
    FT_Error error;
    GX_Face face       	    = ((generic_lookup_table_cb_data)user)->face;
    FT_Stream stream   	    = ((generic_lookup_table_cb_data)user)->stream;    
    FT_Short offset = value->raw.s;
    GX_LigCaretClassEntry class_entry;


    if ( ( error = face->goto_table( face, TTAG_lcar, stream, 0 ) ) )
      return error;

    if ( ( error = gx_lcar_load_class_entry ( face, 
					      stream, 
					      FT_STREAM_POS() + offset,
					      &class_entry ) ) )
      return error;
    value->extra.lcar_class_entry = class_entry;
    return error;
  }

  static FT_Error
  lcar_lookup_table_segment_array_loader ( GX_LookupTable_Format format,
					   FT_UShort lastGlyph,
					   FT_UShort firstGlyph,
					   GX_LookupValue value,
					   FT_Pointer user )
					 
  {
    FT_Error error;
    GX_Face   face          = ((generic_lookup_table_cb_data)user)->face;
    FT_Stream stream   	    = ((generic_lookup_table_cb_data)user)->stream;
    FT_Memory memory   	    = stream->memory;

    FT_Short  value_offset  = value->raw.s;
    FT_ULong  table_offset;

    FT_UShort segment_count = lastGlyph - firstGlyph + 1;
    GX_LigCaretSegment segment;
    
    FT_Int i, j;
    
    /* disk seek chain: 
       1. table_offset -> 2. value_offset -> 3. class_entry_offset */
    
    /* 1. table_offset */
    if ( (error = face->goto_table( face, TTAG_lcar, stream, 0 )) )
      goto Exit;
    table_offset = FT_STREAM_POS();

    /* 2. value_offset 
     * -----------------------------------------------------------------
     * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 
     * Pfaedit will not work?
     * In spec: "Sometimes they are 16-bit offsets from the start of 
     * the table to the data. " The table? Is the table the lookup table
     * or a table that uses the lookup table?
     * Here I assume the table is the table that uses the lookup table.
     * However, I have no conviction. 
     * As far as reading spec(fonts.apple.com/TTRefMan/RM06/Chap6lcar.html),
     * table_offset + value_offset is correct. It seems that  pfaedit uses
     * lookup_table_offset + value_offset.
     * -----------------------------------------------------------------*/
    if ( FT_STREAM_SEEK( table_offset + value_offset ) )
      goto Exit;
	
    if ( FT_NEW_ARRAY ( segment, segment_count ) )
      goto Exit;

    if ( FT_FRAME_ENTER ( sizeof( segment[0].offset ) * segment_count ) )
      goto Failure;
    for ( i = 0; i < segment_count; i++ )
      segment[i].offset = FT_GET_USHORT();
    FT_FRAME_EXIT();
    
    /* 3. class_entry_offset */
    for ( i = 0; i < segment_count; i++ )
      {
	segment[i].class_entry = NULL;
	if (( error = gx_lcar_load_class_entry(face, stream, 
					       table_offset + segment[i].offset,
					       &(segment[i].class_entry)) ))
	  {	
	    /* Rewind to free the resources */
	    for ( j = i - 1; j >= 0; j-- )
	      {
		gx_lcar_free_class_entry (memory, segment[j].class_entry);
		segment[j].class_entry = NULL;
	      }
	    goto Failure;
	  }
      }
    value->extra.lcar_segment = segment;
    error 	 = GX_Err_Ok;
  Exit:
    return error;
  Failure:
    FT_FREE( segment );
    return error;
  }

  
  FT_LOCAL_DEF( FT_Error )
  gx_face_load_lcar( GX_Face   face,
		     FT_Stream stream,
		     GX_Lcar   lcar )
  {
    FT_Error error;
    const FT_Frame_Field lcar_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_LcarRec
	FT_FRAME_START ( 6 ),
  	  FT_FRAME_ULONG  ( version ),
	  FT_FRAME_USHORT ( format ),
	FT_FRAME_END
      };
    GX_LookupTable_FuncsRec funcs 		   = GX_LOOKUP_TABLE_FUNC_ZERO;
    generic_lookup_table_cb_data_rec callback_data = GENERIC_LOOKUP_TABLE_CB_DATA_ZERO;

    funcs.generic_func = lcar_lookup_table_generic_loader;
    funcs.segment_array_func = lcar_lookup_table_segment_array_loader;
    callback_data.face 	     = face;
    callback_data.stream     = stream;
    
    /* FT_TRACE2(( "Ligature Caret" )); */
    if (( error = gx_table_init( &(lcar->root), face, TTAG_lcar, stream,
				 (GX_Table_Done_Func)gx_lcar_done) ))
      goto Exit;

    if ( FT_STREAM_READ_FIELDS( lcar_fields, lcar ) )
      goto Exit;
    error = gx_face_load_LookupTable ( face, stream, &(lcar->lookup) );
    if ( error )
      goto Exit;
    error = gx_LookupTable_traverse_low( &(lcar->lookup),
					 &funcs,
					 &callback_data );
  Exit:
    return error;
  }

  static FT_Error 
  lcar_lookup_table_generic_finalizer( GX_LookupTable_Format format,
				       GX_LookupValue value,
				       FT_Pointer user )
  {
    FT_Memory memory = user;
    GX_LigCaretClassEntry class_entry;

    if ( !value->extra.lcar_class_entry )
      return GX_Err_Ok;

    class_entry = value->extra.lcar_class_entry;
    gx_lcar_free_class_entry(memory, class_entry);
    value->extra.lcar_class_entry = NULL;
    return GX_Err_Ok;
  }

  static FT_Error
  lcar_lookup_table_segment_array_finalizer( GX_LookupTable_Format format,
					     FT_UShort lastGlyph,
					     FT_UShort firstGlyph,
					     GX_LookupValue value,
					     FT_Pointer user )
  {
    FT_Memory memory 	       = user;
    GX_LigCaretSegment segment = value->extra.lcar_segment;
    FT_UShort segment_count = lastGlyph - firstGlyph;
    FT_Long i;
    
    if ( !segment )
      return GX_Err_Ok;

    value->extra.lcar_segment = NULL;
    
    for ( i = 0; i < segment_count; i++ )
      {
	gx_lcar_free_class_entry(memory, segment[i].class_entry);
	segment[i].class_entry = NULL;
      }
    FT_FREE(segment);
    return GX_Err_Ok;
  }

  FT_LOCAL_DEF ( void )
  gx_lcar_done( GX_Lcar   lcar,
		FT_Memory memory )
		     
  {
    GX_LookupTable_FuncsRec funcs = GX_LOOKUP_TABLE_FUNC_ZERO;

    funcs.generic_func 	     = lcar_lookup_table_generic_finalizer;
    funcs.segment_array_func = lcar_lookup_table_segment_array_finalizer;

    gx_LookupTable_traverse_low( &(lcar->lookup), &funcs, memory );
    gx_LookupTable_free ( &(lcar->lookup), memory );
  }


/******************************BSLN************************************/
  typedef FT_Error (* gx_bsln_perfmt_loader)( GX_Face face,
					      FT_Stream stream,
					      GX_BaselineParts parts );

  static FT_Error
  gx_bsln_load_32_ushort( GX_Face face,
			  FT_Stream stream,
			  FT_UShort slots[GX_BSLN_VALUE_COUNT] )
  {
    FT_Error error;
    FT_Int i;
    
    if ( FT_FRAME_ENTER ( sizeof( *slots ) *  GX_BSLN_VALUE_COUNT) )
      return error;
    for ( i = 0; i < GX_BSLN_VALUE_COUNT; i++ )
      slots[i] = FT_GET_USHORT();
    FT_FRAME_EXIT();
    return error;
  }

  static FT_Error
  gx_bsln_load_fmt_distance_no_mapping ( GX_Face face,
					 FT_Stream stream,
					 GX_BaselineParts parts )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_BaselineFormat0Part fmt0part;
    
    if ( FT_NEW( fmt0part ) )
      goto Exit;
    
    if (( error = gx_bsln_load_32_ushort( face, stream, fmt0part->deltas ) ))
      goto Failure;
    parts->fmt0 = fmt0part;
  Exit:
    return error;
  Failure:
    FT_FREE(fmt0part);
    return error;
  }

  static FT_Error
  gx_bsln_load_fmt_distance_with_mapping( GX_Face face,
					  FT_Stream stream,
					  GX_BaselineParts parts )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_BaselineFormat1Part fmt1part;
    GX_LookupTable lookup_table;
    GX_LookupTable_FuncsRec funcs 	       = GX_LOOKUP_TABLE_FUNC_ZERO;
    generic_lookup_table_cb_data_rec user_data = GENERIC_LOOKUP_TABLE_CB_DATA_ZERO;

    funcs.segment_array_func 	  = generic_lookup_table_segment_array_loader;
    user_data.face 		  = face;
    user_data.stream 		  = stream;
    user_data.lookup_table 	  = NULL;
    user_data.table_tag 	  = 0;
    
    if ( FT_NEW ( fmt1part ) )
      goto Exit;
    
    if (( error = gx_bsln_load_32_ushort( face, stream, 
					  fmt1part->deltas ) ))
      goto Failure;
  
    lookup_table = &(fmt1part->mappingData);
    if (( error = gx_face_load_LookupTable ( face, stream, 
					     lookup_table ) ))
      goto Failure;
    if ( lookup_table->format == GX_LOOKUPTABLE_SEGMENT_ARRAY )
      {
	user_data.lookup_table = lookup_table;
	if (( error = gx_LookupTable_traverse_low( lookup_table, 
						   &funcs, &user_data ) ))
	  goto Failure_Lookup_Table;
      }
    parts->fmt1 = fmt1part;
  Exit:
    return error;
  Failure_Lookup_Table:
    gx_LookupTable_free( lookup_table, memory );
  Failure:
    FT_FREE ( fmt1part );
    return error;
  }

  static FT_Error
  gx_bsln_load_fmt_control_point_no_mapping( GX_Face face,
					     FT_Stream stream,
					     GX_BaselineParts parts )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_BaselineFormat2Part fmt2part;    
    FT_UShort stdGlyph;

    if ( FT_READ_USHORT(stdGlyph) )
      goto Exit;

    if ( FT_NEW( fmt2part ) )
      goto Exit;
    
    fmt2part->stdGlyph = stdGlyph;
    if (( error = gx_bsln_load_32_ushort( face, stream, 
					  fmt2part->ctlPoints ) ))
      goto Failure; 
    parts->fmt2 = fmt2part;
  Exit:
    return error;
  Failure:
    FT_FREE(fmt2part);
    return error;
  }

  static FT_Error
  gx_bsln_load_fmt_control_point_with_mapping( GX_Face face,
					       FT_Stream stream,
					       GX_BaselineParts parts )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_BaselineFormat3Part fmt3part;    
    FT_UShort stdGlyph;
  
    GX_LookupTable lookup_table;
    
    GX_LookupTable_FuncsRec funcs = GX_LOOKUP_TABLE_FUNC_ZERO;
    generic_lookup_table_cb_data_rec user_data = GENERIC_LOOKUP_TABLE_CB_DATA_ZERO;

    funcs.segment_array_func = generic_lookup_table_segment_array_loader;
    user_data.face           = face;
    user_data.stream 	     = stream;
    
    if ( FT_READ_USHORT(stdGlyph) ) 
      goto Exit;

    if ( FT_NEW( fmt3part ) )
      goto Exit;
    
    fmt3part->stdGlyph = stdGlyph;
    if (( error = gx_bsln_load_32_ushort( face, stream, 
					  fmt3part->ctlPoints ) ))
      goto Failure; 

    lookup_table = &(fmt3part->mappingData);
    if (( error = gx_face_load_LookupTable ( face, stream, 
					     lookup_table ) ))
      goto Failure;
    if ( lookup_table->format == GX_LOOKUPTABLE_SEGMENT_ARRAY )
      {
	user_data.lookup_table = lookup_table;
	if (( error = gx_LookupTable_traverse_low( lookup_table, 
						   &funcs, &user_data ) ))
	  goto Failure_Lookup_Table;
      }
    parts->fmt3 = fmt3part;
  Exit:
    return error;
  Failure_Lookup_Table:
    gx_LookupTable_free( lookup_table, memory );
  Failure:
    FT_FREE(fmt3part);
    return error;
  }
  
  
  FT_LOCAL_DEF( FT_Error )
  gx_face_load_bsln( GX_Face   face,
		     FT_Stream stream,
		     GX_Bsln   bsln )
  {
    FT_Error error;
    gx_bsln_perfmt_loader fmt_loader;

    const FT_Frame_Field bsln_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_BslnRec
	FT_FRAME_START ( 8 ),
	    FT_FRAME_ULONG ( version ),
	    FT_FRAME_USHORT ( format ),
	    FT_FRAME_USHORT ( defaultBaseline ),
	FT_FRAME_END
      };
    /* FT_TRACE2(( "Baseline" ));*/
    
    if (( error = gx_table_init( &(bsln->root), face, TTAG_bsln, stream,
				 (GX_Table_Done_Func)gx_bsln_done) ))
      goto Exit;
    
    if ( FT_STREAM_READ_FIELDS( bsln_fields, bsln ) )
      goto Exit;
    
    switch ( bsln->format )
      {
      case GX_BSLN_FMT_DISTANCE_NO_MAPPING:
	fmt_loader = gx_bsln_load_fmt_distance_no_mapping;
	break;
      case GX_BSLN_FMT_DISTANCE_WITH_MAPPING:
	fmt_loader = gx_bsln_load_fmt_distance_with_mapping;
	break;
      case GX_BSLN_FMT_CONTROL_POINT_NO_MAPPING:
	fmt_loader = gx_bsln_load_fmt_control_point_no_mapping;
	break;
      case GX_BSLN_FMT_CONTROL_POINT_WITH_MAPPING:
	fmt_loader = gx_bsln_load_fmt_control_point_with_mapping;
	break;
      default:
	fmt_loader = NULL;
      }
    FT_ASSERT(fmt_loader);
    error = fmt_loader(face, stream, &(bsln->parts));
  Exit:
    return error;
  }

  FT_LOCAL_DEF ( void )
  gx_bsln_done( GX_Bsln   bsln,
		FT_Memory memory )
		     
  {
    GX_BaselineParts parts     = &bsln->parts;
    GX_LookupTable mappingData = NULL;
    GX_LookupTable_FuncsRec funcs = GX_LOOKUP_TABLE_FUNC_ZERO;

    funcs.segment_array_func = generic_lookup_table_segment_array_finalizer;

    if ( bsln->format == GX_BSLN_FMT_DISTANCE_WITH_MAPPING )
      mappingData = &(parts->fmt1->mappingData);
    else if ( bsln->format == GX_BSLN_FMT_CONTROL_POINT_WITH_MAPPING )
      mappingData = &(parts->fmt3->mappingData);

    if (mappingData)
      {
	if ( mappingData->format == GX_LOOKUPTABLE_SEGMENT_ARRAY )
	  gx_LookupTable_traverse_low ( mappingData, &funcs, memory );
	gx_LookupTable_free ( mappingData, memory );
      }
    FT_FREE(parts->any);
    bsln->parts.any = NULL;
  }


/******************************MORT************************************/
  typedef FT_Error (* gx_mort_subtable_loader) ( GX_Face face,
						 FT_Stream stream,
						 FT_UShort length,
						 GX_MetamorphosisSubtableBody body );

  static FT_Error
  gx_mort_load_rearrangement_subtable( GX_Face face,
				       FT_Stream stream,
				       FT_UShort length,
				       GX_MetamorphosisSubtableBody body )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_MetamorphosisRearrangementBody rearrangement_body;

    if ( FT_NEW( rearrangement_body ) )
      goto Exit;
    
    if (( error = gx_face_load_StateTable ( face, stream, 
					    &rearrangement_body->state_table, 
					    NULL, NULL )))
      goto Failure;
    body->rearrangement = rearrangement_body;
  Exit:
    return error;
  Failure:
    FT_FREE( rearrangement_body );
    body->rearrangement = NULL;
    return error;
      
  }

  static void
  gx_mort_free_rearrangement_subtable( FT_Memory memory,
				       GX_MetamorphosisRearrangementBody rearrangement_body )
  {
    gx_StateTable_free ( &rearrangement_body->state_table, memory, NULL, NULL );
    FT_FREE( rearrangement_body );
  }

  static FT_Error 
  gx_mort_load_contextual_subtable_entry( GX_Face face,
					  FT_Stream stream,
					  GX_EntrySubtable entry_subtable,
					  FT_Pointer user )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_MetamorphosisContextualPerGlyph per_glyph;
    
    const FT_Frame_Field contextual_subtable_entry_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_MetamorphosisContextualPerGlyphRec
	FT_FRAME_START ( 4 ),
	FT_FRAME_SHORT ( markOffset ),    /* Was:FT_UShort, see gxtype.h */
	FT_FRAME_SHORT ( currentOffset ), /* Was:FT_UShort */
	FT_FRAME_END
      };

    if ( FT_NEW ( per_glyph ) )
      goto Exit;

    if ( FT_STREAM_READ_FIELDS ( contextual_subtable_entry_fields,
				 per_glyph ) )
      goto Failure;

    entry_subtable->glyphOffsets.contextual = per_glyph;
  Exit:
    return error;
  Failure:
    FT_FREE( per_glyph );
    entry_subtable->glyphOffsets.contextual = NULL;
    return error;
  }

  static void 
  gx_mort_free_contextual_subtable_entry( FT_Memory memory,
					  GX_EntrySubtable entry_subtable,
					  FT_Pointer user )
  {
    FT_FREE( entry_subtable->glyphOffsets );
    entry_subtable->glyphOffsets.contextual = NULL;
  }

  static FT_Error
  gx_mort_load_contextual_subtable( GX_Face face,
				    FT_Stream stream,
				    FT_UShort length,
				    GX_MetamorphosisSubtableBody body )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_MetamorphosisContextualBody contextual_body;
    GX_StateHeader state_header;
    GX_MetamorphosisContextualSubstitutionTable substitutionTable;
    GX_StateTable_Entry_Load_FuncsRec funcs = GX_STATE_TABLE_ENTRY_LOAD_FUNCS_ZERO;
    FT_Int i;

    funcs.loader 	 = gx_mort_load_contextual_subtable_entry;
    funcs.finalizer      = gx_mort_free_contextual_subtable_entry;

    if ( FT_NEW( contextual_body ) )
      goto Exit;
    
    if (( error = gx_face_load_StateTable ( face, stream, 
					    &contextual_body->state_table, 
					    &funcs, NULL) ))
      goto Failure;
    
    state_header      = &contextual_body->state_table.header;
    substitutionTable = &contextual_body->substitutionTable;
    if ( FT_STREAM_SEEK( state_header->position + GX_STATE_HEADER_ADVANCE ) ||
	 FT_READ_USHORT(substitutionTable->offset))
      goto Failure_stateTable;

    substitutionTable->nGlyphIndexes = (length - substitutionTable->offset)/2;
    
    substitutionTable->glyph_indexes  = NULL;
    if ( FT_NEW_ARRAY( substitutionTable->glyph_indexes, 
		       substitutionTable->nGlyphIndexes ) )
      goto Failure_stateTable;
    if ( FT_STREAM_SEEK( state_header->position 
			 + substitutionTable->offset ) )
      goto Failure_substitutionTable;
    if ( FT_FRAME_ENTER( substitutionTable->nGlyphIndexes * sizeof( substitutionTable->glyph_indexes[0] ) ))
      goto Failure_substitutionTable;
    
    for ( i = 0; i < substitutionTable->nGlyphIndexes; i++ )
      substitutionTable->glyph_indexes[i] = FT_GET_USHORT();
    FT_FRAME_EXIT();
    body->contextual = contextual_body;
  Exit:
    return error;
  Failure_substitutionTable:
    fprintf(stderr,"hell: 0x%x\n", (substitutionTable->nGlyphIndexes - 1));
    FT_FREE( substitutionTable->glyph_indexes );
    substitutionTable->glyph_indexes = NULL;
  Failure_stateTable:
    gx_StateTable_free( &contextual_body->state_table, memory, funcs.finalizer, NULL );
  Failure:
    FT_FREE( contextual_body );
    body->contextual = NULL;
    return error;
      
  }

  static void
  gx_mort_free_contextual_subtable ( FT_Memory memory,
				     GX_MetamorphosisContextualBody contextual_body )
  {
    GX_MetamorphosisContextualSubstitutionTable  substitutionTable;
    
    substitutionTable = &contextual_body->substitutionTable;;
    FT_FREE( substitutionTable->glyph_indexes );
    substitutionTable->glyph_indexes = NULL;
    gx_StateTable_free( &contextual_body->state_table, 
			memory,
			gx_mort_free_contextual_subtable_entry,
			NULL );
    FT_FREE( contextual_body );
  }

#if 0
  static FT_Error
  mort_max_ligAction_offset( GX_EntrySubtable entry_subtable,
			     FT_Pointer user )
  {
    FT_UShort * max_offset = (FT_UShort *)user;
    FT_UShort offset = entry_subtable->flags & GX_MORT_LIGATURE_FLAGS_OFFSET;
    if ( *max_offset < offset )
      *max_offset = offset;
    return GX_Err_Ok;
  }

  static FT_UShort
  gx_mort_count_ligAction_entry(GX_Face face, 
				GX_MetamorphosisLigatureBody ligature_body)
  {
    FT_UShort max_offset = 0;
    gx_StateTable_traverse_entries( &ligature_body->state_table,
				    mort_max_ligAction_offset,
				    &max_offset );
    /* Because ligActionTable is an array of FT_ULong, we devide the offset by
       sizeof(FT_ULong) here.  */
    return max_offset
      ?(((max_offset - ligature_body->ligActionTable.offset)/sizeof(FT_ULong)) + 1)
      : max_offset;
  }
#endif /* 0 */  
  static FT_Error
  gx_mort_load_ligature_subtable( GX_Face face,
				  FT_Stream stream,
				  FT_UShort length,
				  GX_MetamorphosisSubtableBody body )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_MetamorphosisLigatureBody ligature_body;
    GX_StateHeader state_header;
    GX_MetamorphosisLigatureActionTable ligActionTable;
    GX_MetamorphosisComponentTable componentTable;
    GX_MetamorphosisLigatureTable ligatureTable;

    FT_Int i;

    if ( FT_NEW( ligature_body ) )
      goto Exit;
    
    if (( error = gx_face_load_StateTable( face, stream,
					   &ligature_body->state_table,
					   NULL, NULL )))
      goto Failure;
    
    state_header   = &ligature_body->state_table.header;
    ligActionTable = &ligature_body->ligActionTable;
    componentTable  = &ligature_body->componentTable;
    ligatureTable  = &ligature_body->ligatureTable;
    if ( FT_STREAM_SEEK( state_header->position + GX_STATE_HEADER_ADVANCE ) ||
	 FT_FRAME_ENTER( sizeof( ligActionTable->offset ) 
			 + sizeof( componentTable->offset )
			 + sizeof( ligatureTable->offset )))
      goto Failure_stateTable;
    ligActionTable->offset = FT_GET_USHORT();
    componentTable->offset = FT_GET_USHORT();
    ligatureTable->offset  = FT_GET_USHORT();
    FT_FRAME_EXIT();

    /* -----------------------------------------------------------------
     * Calculate the order of tables: ligActionTable, componentTable and 
     * ligatureTable
     * ----------------------------------------------------------------- */
    generic_triple_offset_diff(ligActionTable->offset, componentTable->offset, ligatureTable->offset,
			       length, 
			       &ligActionTable->nActions, &componentTable->nComponent, &ligatureTable->nLigature);
    
    /* Load ligActionTable */
#if 0
    ligActionTable->nActions = gx_mort_count_ligAction_entry( face,
							      ligature_body );
#endif /* 0 */

    ligActionTable->body = NULL;
    if ( FT_NEW_ARRAY( ligActionTable->body, ligActionTable->nActions ) )
      goto Failure_stateTable;
    if ( FT_STREAM_SEEK( state_header->position + ligActionTable->offset ) ||
	 FT_FRAME_ENTER( ligActionTable->nActions * sizeof( ligActionTable->body[0] ) ) )
      goto Failure_ligActionTable;
    for ( i = 0; i < ligActionTable->nActions; i++ )
      ligActionTable->body[i] = FT_GET_ULONG();
    FT_FRAME_EXIT();
    
    
    /* Load componentTable */
    componentTable->body       = NULL;
    if ( FT_NEW_ARRAY ( componentTable->body, componentTable->nComponent ) )
      goto Failure_ligActionTable;
    if ( FT_STREAM_SEEK ( state_header->position + componentTable->offset ) ||
	 FT_FRAME_ENTER ( componentTable->nComponent * sizeof ( componentTable->body[0] ) ) )
      goto Failure_componentTable;
    for ( i = 0; i < componentTable->nComponent; i++ )
      componentTable->body[i] = FT_GET_USHORT();
    FT_FRAME_EXIT();

    /* Load ligatureTable */
    ligatureTable->body       = NULL;
    if ( FT_NEW_ARRAY ( ligatureTable->body, ligatureTable->nLigature ) )
      goto Failure_componentTable;
    if ( FT_STREAM_SEEK ( state_header->position + ligatureTable->offset ) ||
	 FT_FRAME_ENTER ( ligatureTable->nLigature * sizeof ( ligatureTable->body[0] ) ) )
      goto Failure_ligatureTable;
    for ( i = 0; i < ligatureTable->nLigature; i++ )
      ligatureTable->body[i] = FT_GET_USHORT();
    FT_FRAME_EXIT();
    body->ligature = ligature_body;
  Exit:
    return error;
  Failure_ligatureTable:
    FT_FREE (ligatureTable->body);
    ligatureTable->body = NULL;
  Failure_componentTable:
    FT_FREE (componentTable->body);
    componentTable->body = NULL;
  Failure_ligActionTable:
    FT_FREE (ligActionTable->body);
    ligActionTable->body = NULL;
  Failure_stateTable:
    gx_StateTable_free( &ligature_body->state_table, 
			memory,
			NULL, /* CHECK, Thu Oct 23 14:14:43 2003 */
			NULL);
  Failure:
    FT_FREE( ligature_body );
    return error;
  }

  static void
  gx_mort_free_ligature_subtable ( FT_Memory memory,
				   GX_MetamorphosisLigatureBody ligature_body )
  {
    GX_MetamorphosisLigatureActionTable ligActionTable = &ligature_body->ligActionTable;
    GX_MetamorphosisComponentTable componentTable = &ligature_body->componentTable;
    GX_MetamorphosisLigatureTable ligatureTable = &ligature_body->ligatureTable;

    FT_FREE (ligatureTable->body);
    ligatureTable->body = NULL;
    FT_FREE (componentTable->body);
    componentTable->body = NULL;
    FT_FREE (ligActionTable->body);
    ligActionTable->body = NULL;
    gx_StateTable_free( &ligature_body->state_table, 
			memory, 
			NULL, /* CHECK, Thu Oct 23 14:14:43 2003 */
			NULL );
    FT_FREE( ligature_body );
  }

  static FT_Error
  gx_mort_load_noncontextual_subtable ( GX_Face face,
					FT_Stream stream,
					FT_UShort length,
					GX_MetamorphosisSubtableBody body )
  {
    return generic_load_noncontextual_subtable( face, stream, length, body, TTAG_mort );
  }

  static void
  gx_mort_free_noncontextual_subtable( FT_Memory memory, 
				       GX_MetamorphosisNoncontextualBody noncontextual_body )
  {
    GX_LookupTable lookup_table = &noncontextual_body->lookup_table;

    GX_LookupTable_FuncsRec funcs = GX_LOOKUP_TABLE_FUNC_ZERO;

    funcs.segment_array_func 	  = generic_lookup_table_segment_array_finalizer;

    if ( lookup_table->format == GX_LOOKUPTABLE_SEGMENT_ARRAY )
      gx_LookupTable_traverse_low( lookup_table, &funcs, memory);

    gx_LookupTable_free( lookup_table, memory );
    FT_FREE(noncontextual_body);
  }

  static FT_Error
  gx_mort_load_insertion_glyphcodes ( GX_Face face,
				      FT_Stream stream,
				      FT_ULong pos,
				      FT_UShort count,
				      FT_UShort * glyphcodes )
  {
    FT_Error error;
    FT_Int i;
    if ( FT_STREAM_SEEK( pos ) )
      return error;
    
    if (FT_FRAME_ENTER( sizeof ( glyphcodes[0] ) * count ) )
      return error;
    for ( i = 0; i < count; i++ )
      glyphcodes[i] = FT_GET_USHORT();
    FT_FRAME_EXIT();
    
    return GX_Err_Ok;;
  }

  static FT_Error 
  gx_mort_load_insertion_subtable_entry( GX_Face face,
					 FT_Stream stream,
					 GX_EntrySubtable entry_subtable,
					 FT_Pointer user )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;

    GX_MetamorphosisInsertionBody insertion_body = user;
    GX_StateTable state_table 			 = &(insertion_body->state_table);
    GX_StateHeader state_header 		 = &(state_table->header);

    GX_MetamorphosisInsertionPerGlyph per_glyph;
    FT_UShort current_count = gx_mask_zero_shift(entry_subtable->flags, 
						 GX_MORT_INSERTION_FLAGS_CURRENT_INSERT_COUNT);
    FT_UShort marked_count  = gx_mask_zero_shift(entry_subtable->flags, 
						 GX_MORT_INSERTION_FLAGS_MARKED_INSERT_COUNT);
    GX_MetamorphosisInsertionList currentInsertList;
    GX_MetamorphosisInsertionList markedInsertList;
    FT_ULong tmp_pos;

    const FT_Frame_Field insertion_subtable_entry_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_MetamorphosisInsertionPerGlyphRec
	FT_FRAME_START ( 4 ),
	    FT_FRAME_USHORT ( currentInsertList.offset ),
	    FT_FRAME_USHORT ( markedInsertList.offset ),
	FT_FRAME_END
      };

    if ( FT_ALLOC ( per_glyph, 
		    sizeof(per_glyph) + 
		    (current_count * sizeof(currentInsertList->glyphcodes[0]) ) +
		    (marked_count *  sizeof(markedInsertList->glyphcodes[0]) )) )
      goto Exit;

    currentInsertList 		  = &per_glyph->currentInsertList;
    markedInsertList 		  = &per_glyph->markedInsertList;
    currentInsertList->glyphcodes = (FT_UShort *)(((char *)per_glyph) + sizeof(per_glyph));
    markedInsertList->glyphcodes  = currentInsertList->glyphcodes + current_count;
    
    if ( FT_STREAM_READ_FIELDS ( insertion_subtable_entry_fields,
				 per_glyph ) )
      goto Failure;
    
    
    /* TODO: Should optimize.
       Being `offset' zero means, there is no insertion.
       But we can know whether the `offset' is zero or not after reading fields.
       Some memory area(->glyphcodes) allocated before reading fields are wasted. */
    if ( !per_glyph->currentInsertList.offset )
      current_count = 0;
    if ( !per_glyph->markedInsertList.offset )
      marked_count = 0;
    
    tmp_pos = FT_STREAM_POS();
    if ( (error = gx_mort_load_insertion_glyphcodes ( face,
						      stream,
						      state_header->position + currentInsertList->offset,
						      current_count,
						      currentInsertList->glyphcodes ) ) )
      goto Failure;
    if ( (error = gx_mort_load_insertion_glyphcodes (face,
						     stream,
						     state_header->position + markedInsertList->offset,
						     marked_count,
						     markedInsertList->glyphcodes) ) )
      goto Failure;
    
    if ( FT_STREAM_SEEK( tmp_pos ) )
      goto Failure;
    entry_subtable->glyphOffsets.insertion = per_glyph;
  Exit:
    return error;
  Failure:
    FT_FREE( per_glyph );
    entry_subtable->glyphOffsets.insertion = NULL;
    return error;
  }

  static void 
  gx_mort_free_insertion_subtable_entry( FT_Memory memory,
					 GX_EntrySubtable entry_subtable,
					 FT_Pointer user )
  {
    FT_FREE( entry_subtable->glyphOffsets.insertion );
    entry_subtable->glyphOffsets.insertion = NULL;
  }

  static FT_Error
  gx_mort_load_insertion_subtable( GX_Face face,
				   FT_Stream stream,
				   FT_UShort length,
				   GX_MetamorphosisSubtableBody body )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_MetamorphosisInsertionBody insertion_body;
    GX_StateTable_Entry_Load_FuncsRec funcs = GX_STATE_TABLE_ENTRY_LOAD_FUNCS_ZERO;

    funcs.loader    = gx_mort_load_insertion_subtable_entry;
    funcs.finalizer = gx_mort_free_insertion_subtable_entry;

    if ( FT_NEW( insertion_body ) )
      goto Exit;
    
    if (( error = gx_face_load_StateTable( face, stream,
					   &insertion_body->state_table,
					   &funcs, insertion_body ) ))
      goto Failure;
    body->insertion = insertion_body;
  Exit:
    return error;
  Failure:
    FT_FREE(insertion_body);
    body->insertion = NULL;
    return error;
    
  }

  static void
  gx_mort_free_insertion_subtable ( FT_Memory memory,
				    GX_MetamorphosisInsertionBody insertion_body )
  {
    gx_StateTable_free( &insertion_body->state_table,
			memory,
			gx_mort_free_insertion_subtable_entry,
			NULL );
    FT_FREE(insertion_body);
  }

  static FT_Error 
  gx_mort_load_subtable_header( GX_Face face,
				FT_Stream stream,
				GX_MetamorphosisSubtableHeader header )
  {
    FT_Error error;
    const FT_Frame_Field mort_subtable_header_fileds[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_MetamorphosisSubtableHeaderRec
	FT_FRAME_START ( 8 ),
	    FT_FRAME_USHORT ( length ),
	    FT_FRAME_USHORT ( coverage ),
	    FT_FRAME_ULONG ( subFeatureFlags ),
	FT_FRAME_END
      };
    header->position = FT_STREAM_POS();
    return FT_STREAM_READ_FIELDS( mort_subtable_header_fileds, header );
  }

  static FT_Error
  gx_mort_load_subtable( GX_Face face,
			 FT_Stream stream,
			 FT_ULong pos,
			 GX_MetamorphosisSubtable chain_Subtbl )
  {
    FT_Error error;
    GX_MetamorphosisSubtableHeader header;
    FT_UShort subtable_type;
    gx_mort_subtable_loader loader;
    FT_UShort header_size = sizeof(header->length)
      + sizeof(header->coverage) 
      + sizeof(header->subFeatureFlags);
      
    if ( FT_STREAM_SEEK ( pos ) )
      goto Exit;
    
    header = &chain_Subtbl->header;
    if (( error = gx_mort_load_subtable_header ( face,
						 stream,
						 header )))
      goto Exit;
    
    subtable_type = header->coverage & GX_MORT_COVERAGE_SUBTABLE_TYPE;
    FT_TRACE2(( " mort subtable format %d\n", subtable_type ));
    switch ( subtable_type )
      {
      case GX_MORT_REARRANGEMENT_SUBTABLE: /* StHeader, newState */
	loader = gx_mort_load_rearrangement_subtable;
	break;
      case GX_MORT_CONTEXTUAL_SUBTABLE: /* StHeader++, newState++ */
	loader = gx_mort_load_contextual_subtable;
	break;
      case GX_MORT_LIGATURE_SUBTABLE: /* StHeader++,, newState */
	loader = gx_mort_load_ligature_subtable;
	break;
      case GX_MORT_NONCONTEXTUAL_SUBTABLE: /* Lookup table */
	loader = gx_mort_load_noncontextual_subtable;
	break;
      case GX_MORT_INSERTION_SUBTABLE: /* StHeader, newState++ */
	loader = gx_mort_load_insertion_subtable;
	break;
      default:
	loader = NULL;
	break;
      }
    FT_ASSERT(loader);
    error = (*loader)(face, stream, header->length - header_size, &chain_Subtbl->body);
  Exit:
    return error;
  }

  static void
  gx_mort_free_subtable( FT_Memory memory, GX_MetamorphosisSubtable chain_Subtbl )
  {
    GX_MetamorphosisSubtableHeader header;
    FT_UShort subtable_type;
    
    header = &chain_Subtbl->header;
    subtable_type = header->coverage & GX_MORT_COVERAGE_SUBTABLE_TYPE;
    switch ( subtable_type )
      {
      case GX_MORT_REARRANGEMENT_SUBTABLE:
	gx_mort_free_rearrangement_subtable( memory, chain_Subtbl->body.rearrangement );
	break;
      case GX_MORT_CONTEXTUAL_SUBTABLE:
	gx_mort_free_contextual_subtable ( memory, chain_Subtbl->body.contextual  );
	break;
      case GX_MORT_LIGATURE_SUBTABLE:
	gx_mort_free_ligature_subtable ( memory, chain_Subtbl->body.ligature );
	break;
      case GX_MORT_NONCONTEXTUAL_SUBTABLE:
	gx_mort_free_noncontextual_subtable( memory, chain_Subtbl->body.noncontextual );
	break;
      case GX_MORT_INSERTION_SUBTABLE:
	gx_mort_free_insertion_subtable ( memory, chain_Subtbl->body.insertion );
	break;
      default:
	break;
      }
    chain_Subtbl->body.any = NULL;
  }


  static FT_Error
  gx_mort_load_feature_table( GX_Face face,
			      FT_Stream stream,
			      GX_MetamorphosisFeatureTable feat_Subtbl )
  {
    FT_Error error;
    const FT_Frame_Field mort_feature_table_fileds[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_MetamorphosisFeatureTableRec
	FT_FRAME_START ( 12 ),
	    FT_FRAME_USHORT ( featureType ),
	    FT_FRAME_USHORT ( featureSetting ),
	    FT_FRAME_ULONG  ( enableFlags ),
	    FT_FRAME_ULONG  ( disableFlags ),
	FT_FRAME_END
      }; 
    
    return FT_STREAM_READ_FIELDS( mort_feature_table_fileds, feat_Subtbl );
  }
       
  static FT_Error
  gx_mort_load_chain_header( GX_Face face,
			     FT_Stream stream,
			     GX_MetamorphosisChainHeader header )
  {
    FT_Error error;
    
    const FT_Frame_Field mort_chain_header_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_MetamorphosisChainHeaderRec
	FT_FRAME_START ( 12 ),
	    FT_FRAME_ULONG  ( defaultFlags ),
	    FT_FRAME_ULONG  ( chainLength ),
	    FT_FRAME_USHORT ( nFeatureEntries ),
	    FT_FRAME_USHORT ( nSubtables ),
	FT_FRAME_END
      };

    return FT_STREAM_READ_FIELDS(mort_chain_header_fields, header);
  }

  static FT_Error
  gx_mort_load_chain( GX_Face face,
		      FT_Stream stream,
		      FT_ULong pos,
		      GX_MetamorphosisChain chain )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_MetamorphosisChainHeader header;
    GX_MetamorphosisFeatureTable feat_Subtbl  = NULL;
    GX_MetamorphosisSubtable     chain_Subtbl = NULL;
    FT_ULong subtbl_start;
    FT_Int i, j;
    
    header = &(chain->header);
    if ( FT_STREAM_SEEK ( pos )                                      ||
	 ( error = gx_mort_load_chain_header(face, stream, header )) ||
	 FT_NEW_ARRAY ( feat_Subtbl, header->nFeatureEntries ) )
      goto Exit;
    
    for ( i = 0; i < header->nFeatureEntries; i++ )
      {
	if (( error = gx_mort_load_feature_table( face, stream, 
						  &feat_Subtbl[i] ) ))
	  goto Failure;
      }

    subtbl_start = FT_STREAM_POS();
    if ( FT_NEW_ARRAY ( chain_Subtbl, header->nSubtables ) )
      goto Failure;

    for ( i = 0; i < header->nSubtables; i++ )
      {
	if (( error = gx_mort_load_subtable( face, stream, subtbl_start,
					     &chain_Subtbl[i]) ))
	  {
	    for ( j = i - 1; j >= 0; j-- )
	      gx_mort_free_subtable( memory,  &chain_Subtbl[j] );
	    goto Failure;
	  }
	subtbl_start += chain_Subtbl[i].header.length;
      }
    chain->feat_Subtbl 	= feat_Subtbl;
    chain->chain_Subtbl = chain_Subtbl;
  Exit:
    return error;
  Failure:
    chain->feat_Subtbl 	= NULL;      
    chain->chain_Subtbl = NULL;
    if ( feat_Subtbl )
      FT_FREE ( feat_Subtbl );
    if ( chain_Subtbl )
      FT_FREE ( chain_Subtbl );
    return error;
  }

  static void
  gx_mort_free_chain( FT_Memory memory,
		      GX_MetamorphosisChain chain )
  {
    GX_MetamorphosisChainHeader header;
    FT_Int i;

    header = &(chain->header);
    for ( i = header->nSubtables; i > 0 ; i-- )
      gx_mort_free_subtable(memory, &chain->chain_Subtbl[i - 1]);
    FT_FREE(chain->chain_Subtbl);
     chain->chain_Subtbl = NULL;
    FT_FREE(chain->feat_Subtbl);
    chain->feat_Subtbl = NULL;
  }

  FT_LOCAL_DEF ( FT_Error )
  gx_face_load_mort( GX_Face   face,
		     FT_Stream stream,
		     GX_Mort   mort )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_MetamorphosisChain chain;
    
    FT_ULong chain_start_pos;
    FT_Int i, j;
    const FT_Frame_Field mort_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_MortRec
	FT_FRAME_START ( 8 ),
	    FT_FRAME_LONG  ( version ),
	    FT_FRAME_ULONG ( nChains ),
	FT_FRAME_END
      };
    /* FT_TRACE2(( "Metamorphosis Table" )); */
    if (( error = gx_table_init( &(mort->root), face, TTAG_mort, stream,
				 (GX_Table_Done_Func)gx_mort_done) ))
      goto Exit;

    mort->chain = NULL;    
    if ( FT_STREAM_READ_FIELDS( mort_fields, mort ) )
      goto Exit;

    if ( FT_NEW_ARRAY ( chain, mort->nChains ) )
      goto Exit;

    chain_start_pos = FT_STREAM_POS();
    for ( i = 0; i < mort->nChains; i++ )
      {
	if (( error = gx_mort_load_chain( face, 
					  stream, chain_start_pos, 
					  &chain[i] ) ))
	  {
	    for ( j = i - 1; j >= 0; j-- )
	      gx_mort_free_chain( memory, &chain[j] );
	    goto Failure;
	  }
	chain_start_pos += chain[i].header.chainLength;
      }
    mort->chain = chain;
  Exit:
    return error;
  Failure:
    FT_FREE( chain );
    return error;
  }

  FT_LOCAL_DEF ( void )
  gx_mort_done( GX_Mort   mort,
		FT_Memory memory )
		     
  {
    FT_Int i;
    
    for ( i = 0; i < mort->nChains; i++ )
      gx_mort_free_chain( memory, &mort->chain[i] );
    FT_FREE( mort->chain );
    mort->chain = NULL;
  }


/******************************MORX************************************/
/* Protype declaration */
static void gx_morx_free_chain( FT_Memory memory, GX_XMetamorphosisChain chain );
static void gx_morx_free_subtable( FT_Memory memory, GX_XMetamorphosisSubtable chain_Subtbl );

#define gx_morx_load_feature_table gx_mort_load_feature_table 
#define gx_morx_free_feature_table gx_mort_free_feature_table 

  typedef FT_Error (* gx_morx_subtable_loader) ( GX_Face face,
						 FT_Stream stream,
						 FT_ULong length,
						 GX_XMetamorphosisSubtableBody body );

  static FT_Error
  gx_morx_load_chain_header( GX_Face face,
			     FT_Stream stream,
			     GX_XMetamorphosisChainHeader header )
  {
    FT_Error error;
    
    const FT_Frame_Field morx_chain_header_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_XMetamorphosisChainHeaderRec
	FT_FRAME_START ( 16 ),
	    FT_FRAME_ULONG  ( defaultFlags ),
	    FT_FRAME_ULONG  ( chainLength ),
	    FT_FRAME_ULONG  ( nFeatureEntries ),
	    FT_FRAME_ULONG  ( nSubtables ),
	FT_FRAME_END
      };

    return FT_STREAM_READ_FIELDS(morx_chain_header_fields, header);
  }

  static FT_Error
  gx_morx_load_rearrangement_subtable( GX_Face face,
				       FT_Stream stream,
				       FT_ULong length,
				       GX_XMetamorphosisSubtableBody body )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_XMetamorphosisRearrangementBody rearrangement_body;

    if ( FT_NEW( rearrangement_body ) )
      goto Exit;
    
    if (( error = gx_face_load_XStateTable ( face, stream, 
					     &rearrangement_body->state_table,
					     NULL,
					     NULL ) ))
      goto Failure;
    body->rearrangement = rearrangement_body;
  Exit:
    return error;
  Failure:
    FT_FREE( rearrangement_body );
    body->rearrangement = NULL;
    return error;
  }

/*
 * Contextual
 */
  static FT_Error 
  gx_morx_load_contextual_subtable_entry( GX_Face face,
					 FT_Stream stream,
					 GX_EntrySubtable entry_subtable,
					 FT_Pointer user )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    
    GX_XMetamorphosisContextualPerGlyph per_glyph;
    const FT_Frame_Field contextual_subtable_entry_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_XMetamorphosisContextualPerGlyphRec
	FT_FRAME_START ( 4 ),
	    FT_FRAME_USHORT ( markIndex ),
	    FT_FRAME_USHORT ( currentIndex ),
	FT_FRAME_END
      };

    if ( FT_NEW( per_glyph ) )
      goto Exit;

    if ( FT_STREAM_READ_FIELDS ( contextual_subtable_entry_fields,
				 per_glyph ) )
      goto Failure;

    entry_subtable->glyphOffsets.xcontextual = per_glyph;
  Exit:
    return error;
  Failure:
    FT_FREE( per_glyph );
    entry_subtable->glyphOffsets.xcontextual = NULL;
    return error;
  }

  static void 
  gx_morx_free_contextual_subtable_entry( FT_Memory memory,
					  GX_EntrySubtable entry_subtable,
					  FT_Pointer user )
  {
    FT_FREE( entry_subtable->glyphOffsets.xcontextual );
    entry_subtable->glyphOffsets.xcontextual = NULL;
  }


  static FT_Error
  morx_max_lookup_table_index( GX_EntrySubtable entry_subtable,
			       FT_Pointer user )
  {
    FT_Long * max_index = (FT_Long *)user;
    FT_UShort local_max_index;
    GX_XMetamorphosisContextualPerGlyph per_glyph = entry_subtable->glyphOffsets.xcontextual;
    FT_UShort markIndex = per_glyph->markIndex;
    FT_UShort currentIndex = per_glyph->currentIndex;
    
    if ( markIndex == GX_MORX_NO_SUBSTITUTION )
      markIndex = 0;
    if ( currentIndex == GX_MORX_NO_SUBSTITUTION )
      currentIndex = 0;
    
    if ( markIndex < currentIndex )
      local_max_index = currentIndex;
    else
      local_max_index = markIndex;
    
    if ( *max_index < local_max_index )
      *max_index = local_max_index;
    return GX_Err_Ok;
  }

  static FT_UShort
  gx_morx_count_lookup_table (GX_Face face,
			      GX_XStateTable state_table)
  {
    FT_Long max_index = -1;
    gx_XStateTable_traverse_entries ( state_table,
				      morx_max_lookup_table_index,
				      &max_index );
    return (FT_UShort)(max_index + 1);
  }

  static FT_Error
  gx_morx_load_lookup_table( GX_Face face,
			     FT_Stream stream,
			     FT_ULong pos,
			     GX_LookupTable * lookup_table )
  {
    FT_Error error;
    FT_Memory memory 		      = stream->memory;
    GX_LookupTable local_lookup_table = NULL;
    GX_LookupTable_FuncsRec funcs = GX_LOOKUP_TABLE_FUNC_ZERO;
    generic_lookup_table_cb_data_rec user_data = GENERIC_LOOKUP_TABLE_CB_DATA_ZERO;

    funcs.segment_array_func 	  = generic_lookup_table_segment_array_loader;
    user_data.face 		  = face;
    user_data.stream 		  = stream;
    /* -----------------------------------------------------------------
     * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 
     * Next code is correct?
     * ----------------------------------------------------------------- */
    user_data.table_tag 	  = 0;
    
    *lookup_table = NULL;
    
    if ( FT_STREAM_SEEK(pos) )
      goto Exit;

    if ( FT_NEW(local_lookup_table) )
      goto Exit;

    if (( error = gx_face_load_LookupTable( face, stream, local_lookup_table )))
      goto Failure;

    if ( local_lookup_table->format == GX_LOOKUPTABLE_SEGMENT_ARRAY )
      {
	if (( error = gx_LookupTable_traverse_low( local_lookup_table,
						   &funcs,
						   &user_data )))
	  goto Failure_Lookup_Table;
      }
    
    *lookup_table = local_lookup_table;
  Exit:
    return error;
  Failure_Lookup_Table:
    gx_LookupTable_free( local_lookup_table, memory );
  Failure:
    FT_FREE(local_lookup_table);
    return error;
  }

  static void
  gx_morx_free_lookup_table( FT_Memory memory,
			     GX_LookupTable lookup_table )
  {
    GX_LookupTable_FuncsRec funcs = GX_LOOKUP_TABLE_FUNC_ZERO;
    
    funcs.segment_array_func = generic_lookup_table_segment_array_finalizer;
    
    if ( lookup_table->format == GX_LOOKUPTABLE_SEGMENT_ARRAY )
      gx_LookupTable_traverse_low ( lookup_table,
				    &funcs,
				    memory );
    gx_LookupTable_free ( lookup_table, memory );
    FT_FREE(lookup_table);
  }

  static FT_Error
  morx_load_lookup_table ( GX_EntrySubtable entry_subtable,
			   FT_Pointer user )
  {
    generic_xstate_table_cb_data data = user;
    FT_Error error 		      = GX_Err_Ok;
    FT_Stream stream = data->stream;
    GX_XMetamorphosisContextualSubstitutionTable substitution_table = data->extra;
    
    GX_XMetamorphosisContextualPerGlyph per_glyph = entry_subtable->glyphOffsets.xcontextual;
    FT_UShort markIndex = per_glyph->markIndex;
    FT_UShort currentIndex = per_glyph->currentIndex;

    GX_LookupTable mark_table 	 = NULL;
    GX_LookupTable current_table = NULL;
    FT_ULong pos;    

    if ( ( markIndex != GX_MORX_NO_SUBSTITUTION )       && 
	 (! substitution_table->lookupTables[markIndex] ) )
      {
	pos = data->base + data->table_offset + (sizeof(mark_table->position) * markIndex);
	if (( error = gx_morx_load_lookup_table ( data->face, stream, pos, &mark_table )))
	  goto Exit;
      }

    if ( currentIndex == markIndex )
      current_table = mark_table;
    else if (( currentIndex != GX_MORX_NO_SUBSTITUTION )     &&
	     (! substitution_table->lookupTables[currentIndex] ))
      {
	pos = data->base + data->table_offset + (sizeof(mark_table->position) * currentIndex);
	if (( error = gx_morx_load_lookup_table ( data->face, stream, pos, &current_table )))
	  goto Failure;
      }
    if ( mark_table )
      substitution_table->lookupTables[markIndex] = mark_table;
    if ( current_table )
      substitution_table->lookupTables[currentIndex] = current_table;
  Exit:
    return error;
  Failure:
    /* Be care if currentIndex == markIndex */
    if ( mark_table )
      {
	gx_morx_free_lookup_table( stream->memory, mark_table );
	mark_table = NULL;
      }
    return error;
  }

  static FT_Error
  gx_morx_load_contextual_substitution_table( GX_Face face,
					      FT_Stream stream,
					      GX_XStateTable state_table,
					      GX_XMetamorphosisContextualSubstitutionTable substitution_table )
  {
    FT_Error error;
    generic_xstate_table_cb_data_rec cb_data = GENERIC_XSTATE_TABLE_CB_DATA_ZERO;
    FT_Int i;
    cb_data.base = state_table->header.position;
    cb_data.table_offset = substitution_table->offset;
    cb_data.stream = stream;
    cb_data.face  = face;
    cb_data.extra = substitution_table;

    if (( error = gx_XStateTable_traverse_entries ( state_table,
						    morx_load_lookup_table,
						    &cb_data )))
      goto Failure;
    return error;
  Failure:
    for ( i = 0; i < substitution_table->nTables; i++ )
      {
	gx_morx_free_lookup_table ( stream->memory, substitution_table->lookupTables[i] );
	substitution_table->lookupTables[i] = NULL;
      }
    return error;
  }

  static FT_Error
  gx_morx_load_contextual_subtable( GX_Face   face,
				    FT_Stream stream,
				    FT_ULong length,
				    GX_XMetamorphosisSubtableBody body )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_XStateTable state_table;
    GX_XMetamorphosisContextualBody contextual_body;
    GX_XStateTable_Entry_Load_FuncsRec funcs = GX_XSTATE_TABLE_ENTRY_LOAD_FUNCS_ZERO;
    FT_ULong  pos;
    GX_XMetamorphosisContextualSubstitutionTable substitution_table;
    FT_ULong i;

    funcs.loader    = gx_morx_load_contextual_subtable_entry;
    funcs.finalizer = gx_morx_free_contextual_subtable_entry;

    if ( FT_NEW( contextual_body ) )
      goto Exit;
    
    state_table = &contextual_body->state_table;
    if  (( error = gx_face_load_XStateTable( face, stream,
					     state_table,
					     &funcs,
					     NULL ) ))
      goto Failure;

    substitution_table = &contextual_body->substitutionTable;
    pos   =     state_table->header.position + GX_XSTATE_HEADER_ADVANCE;
    if ( FT_STREAM_SEEK ( pos ) )
      goto Failure_XStateTable;
    if ( FT_READ_ULONG ( substitution_table->offset ) )
      goto Failure_XStateTable;
    
    substitution_table->nTables = gx_morx_count_lookup_table(face,
							     state_table);
    if ( FT_NEW_ARRAY ( substitution_table->lookupTables,
			substitution_table->nTables ) )
      goto Failure_XStateTable;
    for ( i = 0; i < substitution_table->nTables; i++ )
      substitution_table->lookupTables[i] = NULL;
    
    if (( error = gx_morx_load_contextual_substitution_table( face,
							      stream,
							      state_table,
							      substitution_table ) ))
      goto Failure_LookupTables;
    body->contextual = contextual_body;
  Exit:
    return error;
  Failure_LookupTables:
    FT_FREE( substitution_table->lookupTables );
    substitution_table->lookupTables = NULL;
  Failure_XStateTable:
    gx_XStateTable_free( &contextual_body->state_table,
			 memory,
			 gx_morx_free_contextual_subtable_entry,
			 NULL );
  Failure:
    FT_FREE( contextual_body );
    body->contextual = NULL;
    return error;
  }


  static FT_Error
  gx_morx_load_ligature_subtable_entry ( GX_Face face,
					 FT_Stream stream,
					 GX_EntrySubtable entry_subtable,
					 FT_Pointer user )
  {
    FT_Error  error;
    FT_UShort per_glyph;
    
    /* -----------------------------------------------------------------
     * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 
     * Should I read ushort even if entry_subtable->flags&GX_MORX_LIGATURE_FLAGS_PERFORM_ACTION
     * is 0? 
     * ----------------------------------------------------------------- */    
    if ( FT_READ_USHORT ( per_glyph ) )
      goto Exit;
    entry_subtable->glyphOffsets.ligActionIndex = per_glyph;
  Exit:
    return error;
  }

#if 0
  static FT_Error
  morx_max_ligAction_index( GX_EntrySubtable entry_subtable,
			    FT_Pointer user )
  {
    FT_Long * max_index = (FT_Long *)user;
    if ( ( entry_subtable->flags & GX_MORX_LIGATURE_FLAGS_PERFORM_ACTION ) && 
	 (*max_index < entry_subtable->glyphOffsets.ligActionIndex ) )
      *max_index = (FT_Long)entry_subtable->glyphOffsets.ligActionIndex;
    return GX_Err_Ok;
  }

  static FT_UShort
  gx_morx_count_ligAction_entry(GX_Face face, 
				GX_XMetamorphosisLigatureBody ligature_body)
  {
    FT_Long max_index = -1;
    gx_XStateTable_traverse_entries( &ligature_body->state_table,
				     morx_max_ligAction_index,
				     &max_index );
    
    return (FT_UShort)(max_index + 1);
  }
#endif /* 0 */

  static FT_Error
  gx_morx_load_ligature_subtable( GX_Face   face,
				  FT_Stream stream,
				  FT_ULong length,
				  GX_XMetamorphosisSubtableBody body )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_XMetamorphosisLigatureBody ligature_body;
    GX_XStateHeader state_header;
    GX_XStateTable_Entry_Load_FuncsRec funcs = GX_XSTATE_TABLE_ENTRY_LOAD_FUNCS_ZERO;
    GX_XMetamorphosisLigatureActionTable ligActionTable;
    GX_XMetamorphosisComponentTable componentTable;
    GX_XMetamorphosisLigatureTable ligatureTable;
    FT_Int i;

    funcs.loader = gx_morx_load_ligature_subtable_entry;

    if ( FT_NEW( ligature_body ) )
      goto Exit;

    if (( error = gx_face_load_XStateTable( face, stream,
					    &ligature_body->state_table,
					    &funcs, 
					    NULL )))
      goto Failure;
    state_header   = &ligature_body->state_table.header;
    ligActionTable = &ligature_body->ligActionTable;
    componentTable = &ligature_body->componentTable;
    ligatureTable  = &ligature_body->ligatureTable;
    if ( FT_STREAM_SEEK( state_header->position + GX_XSTATE_HEADER_ADVANCE ) ||
	 FT_FRAME_ENTER( sizeof ( ligActionTable->offset )
			 + sizeof ( componentTable->offset )
			 + sizeof ( ligatureTable->offset ) ) )
      goto Failure_xStateTable;
    ligActionTable->offset = FT_GET_ULONG();
    componentTable->offset = FT_GET_ULONG();
    ligatureTable->offset  = FT_GET_ULONG();
    FT_FRAME_EXIT();

    generic_triple_offset_diff( ligActionTable->offset,
				componentTable->offset,
				ligatureTable->offset,
				length,
				& ligActionTable->nActions,
				& componentTable->nComponent,
				& ligatureTable->nLigature );
    
    /* Load ligActionTable */
#if 0
    ligActionTable->nActions = gx_morx_count_ligAction_entry( face,
							      ligature_body );

#endif /* 0 */

    ligActionTable->body = NULL;
    if ( FT_NEW_ARRAY( ligActionTable->body, ligActionTable->nActions ) )
      goto Failure_xStateTable;
    if ( FT_STREAM_SEEK( state_header->position + ligActionTable->offset ) ||
	 FT_FRAME_ENTER( ligActionTable->nActions * sizeof( ligActionTable->body[0] ) ) )
      goto Failure_ligActionTable;
    for ( i = 0; i < ligActionTable->nActions; i++ )
      ligActionTable->body[i] = FT_GET_ULONG();
    FT_FRAME_EXIT();
    
    /* Load componentTable */
    componentTable->body       = NULL;
    if ( FT_NEW_ARRAY ( componentTable->body, componentTable->nComponent ) )
      goto Failure_ligActionTable;
    if ( FT_STREAM_SEEK ( state_header->position + componentTable->offset ) ||
	 FT_FRAME_ENTER ( componentTable->nComponent * sizeof ( componentTable->body[0] ) ) )
      goto Failure_componentTable;
    for ( i = 0; i < componentTable->nComponent; i++ )
      componentTable->body[i] = FT_GET_USHORT();
    FT_FRAME_EXIT();

    /* Load ligatureTable */

    ligatureTable->body       = NULL;
    if ( FT_NEW_ARRAY ( ligatureTable->body, ligatureTable->nLigature ) )
      goto Failure_componentTable;      
    if ( FT_STREAM_SEEK ( state_header->position + ligatureTable->offset ) )
      goto Failure_ligatureTable;
    if ( FT_FRAME_ENTER ( ligatureTable->nLigature * sizeof ( ligatureTable->body[0] ) ) )
      {
	/* NOTE: morx tables of two ttf fonts in /Library/Fonts/ of
	   MacOSX-10.3 may be broken.  The length of extended state
	   table is incorrect. The length is larger than the font file
	   size. This is the kluge. 6 may be the size of header of
	   chain subtable. */
	ligatureTable->nLigature -= 6;
	if ( FT_FRAME_ENTER ( ligatureTable->nLigature * sizeof ( ligatureTable->body[i] ) ))
	  goto Failure_ligatureTable;
      }
    for ( i = 0; i < ligatureTable->nLigature; i++ )
      ligatureTable->body[i] = FT_GET_USHORT();
    FT_FRAME_EXIT();
    body->ligature = ligature_body;
  Exit:
    return error;
  Failure_ligatureTable:
    FT_FREE ( ligatureTable->body );
    ligatureTable->body = NULL;
  Failure_componentTable:
    FT_FREE (componentTable->body);
    componentTable->body = NULL;
  Failure_ligActionTable:
    FT_FREE (ligActionTable->body);
    ligActionTable->body = NULL;
  Failure_xStateTable:
    gx_XStateTable_free( &ligature_body->state_table, memory, NULL, NULL);
  Failure:
    FT_FREE(ligature_body);
    return error;
  }

/*
 * Noncontextual
 */
  static FT_Error
  gx_morx_load_noncontextual_subtable( GX_Face face,
				       FT_Stream stream,
				       FT_ULong length,
				       GX_XMetamorphosisSubtableBody body )
  {
    FT_Error error;
    GX_MetamorphosisSubtableBodyDesc mort_body;
    
    if (( error = generic_load_noncontextual_subtable ( face,
							stream,
							length,
							&mort_body,
							TTAG_morx) ))
      goto Exit;
    body->noncontextual = mort_body.noncontextual;
  Exit:
    return error;
  }


/*
 * Insertion
 */
  static FT_Error 
  gx_morx_load_insertion_subtable_entry( GX_Face face,
					 FT_Stream stream,
					 GX_EntrySubtable entry_subtable,
					 FT_Pointer user )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_XMetamorphosisInsertionPerGlyph per_glyph;
    FT_UShort current_count = gx_mask_zero_shift(entry_subtable->flags, 
						 GX_MORX_INSERTION_FLAGS_CURRENT_INSERT_COUNT);
    FT_UShort marked_count  = gx_mask_zero_shift(entry_subtable->flags, 
						 GX_MORX_INSERTION_FLAGS_MARKED_INSERT_COUNT);
    GX_XMetamorphosisInsertionList currentInsertList;
    GX_XMetamorphosisInsertionList markedInsertList;

    const FT_Frame_Field insertion_subtable_entry_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_XMetamorphosisInsertionPerGlyphRec
	FT_FRAME_START ( 4 ),
	    FT_FRAME_USHORT ( currentInsertList.offset ),
	    FT_FRAME_USHORT ( markedInsertList.offset ),
	FT_FRAME_END
      };

    /* TODO: Should optimize.
       Being `offset' GX_MORX_NO_INSERTION means, there is no insertion.
       But we can know whether the `offset' is GX_MORX_NO_INSERTION or not after 
       reading fields. Some memory area(->glyphcodes) allocated before reading 
       fields are wasted. See also `morx_load_insertion_glyphcodes'. */
    if ( FT_ALLOC ( per_glyph, 
		    sizeof(per_glyph) + 
		    current_count * sizeof(currentInsertList->glyphcodes[0]) +
		    marked_count  * sizeof(markedInsertList->glyphcodes[0])) )
      goto Exit;

    currentInsertList 		  = &per_glyph->currentInsertList;
    markedInsertList 		  = &per_glyph->markedInsertList;
    currentInsertList->glyphcodes = (FT_UShort *)(((char *)per_glyph) + sizeof(per_glyph));
    markedInsertList->glyphcodes  = currentInsertList->glyphcodes + current_count;
    
    if ( FT_STREAM_READ_FIELDS ( insertion_subtable_entry_fields,
				 per_glyph ) )
      goto Failure;

    entry_subtable->glyphOffsets.insertion = per_glyph;
  Exit:
    return error;
  Failure:
    FT_FREE( per_glyph );
    entry_subtable->glyphOffsets.insertion = NULL;
    return error;
  }

  static void 
  gx_morx_free_insertion_subtable_entry( FT_Memory memory,
					 GX_EntrySubtable entry_subtable,
					 FT_Pointer user )
  {
    entry_subtable->glyphOffsets.insertion->currentInsertList.glyphcodes = NULL;
    entry_subtable->glyphOffsets.insertion->markedInsertList.glyphcodes = NULL;
    FT_FREE( entry_subtable->glyphOffsets.insertion );
    entry_subtable->glyphOffsets.insertion = NULL;
  }

  static FT_Error
  morx_load_insertion_glyphcodes( GX_EntrySubtable entry_subtable,
				  FT_Pointer user )
  {
    FT_Error error;
    generic_xstate_table_cb_data data 		     = user;
    FT_Stream stream 				     = data->stream;
    FT_ULong  pos 				     = data->base + data->table_offset;
    GX_XMetamorphosisInsertionPerGlyph per_glyph     = entry_subtable->glyphOffsets.insertion;
    GX_XMetamorphosisInsertionList currentInsertList = &per_glyph->currentInsertList;
    GX_XMetamorphosisInsertionList markedInsertList  = &per_glyph->markedInsertList;
    FT_UShort current_count = gx_mask_zero_shift(entry_subtable->flags, 
						 GX_MORX_INSERTION_FLAGS_CURRENT_INSERT_COUNT);
    FT_UShort marked_count  = gx_mask_zero_shift(entry_subtable->flags, 
						 GX_MORX_INSERTION_FLAGS_MARKED_INSERT_COUNT);
    FT_Int i;
  
    /* Dirty hack here. See  */
    if ( per_glyph->currentInsertList.offset == GX_MORX_NO_INSERTION )
      current_count = 0;
    if ( per_glyph->markedInsertList.offset == GX_MORX_NO_INSERTION )
      marked_count = 0;

    if ( FT_STREAM_SEEK( pos + currentInsertList->offset ) )
      return error;
    for ( i = 0; i < current_count; i++ )
      {
	if ( FT_READ_USHORT ( currentInsertList->glyphcodes[i] ) )
	  return error;
      }
    
    if ( FT_STREAM_SEEK( pos + markedInsertList->offset ) )
      return error;
    for ( i = 0; i < marked_count; i++ )
      {
	if ( FT_READ_USHORT ( markedInsertList->glyphcodes[i] ) )
	  return error;
      }
    return error;
  }

  static FT_Error
  gx_morx_load_insertion_subtable( GX_Face face,
				   FT_Stream stream,
				   FT_ULong length,
				   GX_XMetamorphosisSubtableBody body )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_XMetamorphosisInsertionBody insertion_body;
    GX_XStateTable state_table;
    GX_XStateHeader state_header;
    GX_XStateTable_Entry_Load_FuncsRec funcs;
    generic_xstate_table_cb_data_rec traverse_data = GENERIC_XSTATE_TABLE_CB_DATA_ZERO;

    funcs.loader    = gx_morx_load_insertion_subtable_entry;
    funcs.finalizer = gx_morx_free_insertion_subtable_entry;
    
    if ( FT_NEW( insertion_body ) )
      goto Exit;

    state_table  = &insertion_body->state_table;
    state_header = &(state_table->header);
    if  (( error = gx_face_load_XStateTable( face, stream,
					     state_table,
					     &funcs,
					     NULL ) ))
      goto Failure;
    if ( FT_STREAM_SEEK( state_header->position + GX_XSTATE_HEADER_ADVANCE ) ||
	 FT_READ_ULONG( insertion_body->insertion_glyph_table ) )
      goto Failure;

    traverse_data.base = state_table->header.position;
    traverse_data.table_offset = insertion_body->insertion_glyph_table;
    traverse_data.stream = stream;
    traverse_data.face = face;
    traverse_data.extra  = NULL;
    if (( error = gx_XStateTable_traverse_entries ( state_table, 
						    morx_load_insertion_glyphcodes,
						    &traverse_data ) ))
      goto Failure_XStateTable;
    body->insertion = insertion_body;
  Exit:
    return error;
  Failure_XStateTable:
    gx_XStateTable_free( &insertion_body->state_table,
			 memory, 
			 gx_morx_free_insertion_subtable_entry,
			 NULL );
  Failure:
    FT_FREE(insertion_body);
    body->insertion = NULL;
    return error;    
  }

  static FT_Error
  gx_morx_load_subtable_header( GX_Face face,
				FT_Stream stream,
				GX_XMetamorphosisSubtableHeader header )
  {
    FT_Error error;
    const FT_Frame_Field morx_subtable_header_fileds[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_XMetamorphosisSubtableHeaderRec
	FT_FRAME_START ( 12 ),
	    FT_FRAME_ULONG ( length ),
	    FT_FRAME_ULONG ( coverage ),
	    FT_FRAME_ULONG ( subFeatureFlags ),
	FT_FRAME_END
      };
    header->position = FT_STREAM_POS();
    return FT_STREAM_READ_FIELDS( morx_subtable_header_fileds, header );
  }

  static FT_Error
  gx_morx_load_subtable( GX_Face face,
			 FT_Stream stream,
			 FT_ULong pos,
			 GX_XMetamorphosisSubtable chain_Subtbl )
  {
    FT_Error error;
    GX_XMetamorphosisSubtableHeader header;
    FT_UShort header_size = sizeof(header->length)
      + sizeof(header->coverage)
      + sizeof(header->subFeatureFlags);
    FT_ULong subtable_type;
    gx_morx_subtable_loader loader;
    
    if ( FT_STREAM_SEEK ( pos ) )
      goto Exit;

    header = &chain_Subtbl->header;
    if (( error = gx_morx_load_subtable_header ( face,
						 stream,
						 header )))
      goto Exit;
    subtable_type = header->coverage & GX_MORX_COVERAGE_SUBTABLE_TYPE;
    FT_TRACE2(( " morx subtable format %d\n", subtable_type ));
    switch ( subtable_type )
      {
      case GX_MORX_REARRANGEMENT_SUBTABLE: /* XStHeader, newState */
	loader = gx_morx_load_rearrangement_subtable;
	break;
      case GX_MORX_CONTEXTUAL_SUBTABLE: /* StHeader++, newState++ */
	loader = gx_morx_load_contextual_subtable;
	break;
      case GX_MORX_LIGATURE_SUBTABLE: /* StHeader++,, newState */
	loader = gx_morx_load_ligature_subtable;
	break;
      case GX_MORX_NONCONTEXTUAL_SUBTABLE: /* Lookup table */
	loader = gx_morx_load_noncontextual_subtable;
	break;
      case GX_MORX_INSERTION_SUBTABLE: /* StHeader, newState++ */
	loader = gx_morx_load_insertion_subtable;
	break;
      default:
	loader = NULL;
	break;
      }
    chain_Subtbl->body.any = NULL;
    if ( !loader )
      {
	error = GX_Err_Invalid_File_Format;
	goto Exit;
      }
    error = (*loader)(face, stream, header->length - header_size, &chain_Subtbl->body);
  Exit:
    return error;
  }

  static FT_Error
  gx_morx_load_chain( GX_Face face,
		      FT_Stream stream,
		      FT_ULong pos,
		      GX_XMetamorphosisChain chain )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_XMetamorphosisChainHeader header;
    GX_XMetamorphosisFeatureTable feat_Subtbl  = NULL;
    GX_XMetamorphosisSubtable     chain_Subtbl = NULL;
    FT_ULong subtbl_start;
    FT_Int i, j;

    header = &(chain->header);
    if ( FT_STREAM_SEEK ( pos ) )
      goto Exit;
    if ( ( error = gx_morx_load_chain_header( face, stream, header ) ) )
      goto Exit;
    if ( FT_NEW_ARRAY ( feat_Subtbl, header->nFeatureEntries ) )
      goto Exit;

    for ( i = 0; i < header->nFeatureEntries; i++ )
      {
	if (( error = gx_morx_load_feature_table ( face, stream,
						   &feat_Subtbl[i] ) ))
	  goto Failure;
      }
    
    subtbl_start = FT_STREAM_POS();
    if ( FT_NEW_ARRAY ( chain_Subtbl, header->nSubtables ) )
      goto Failure;
    for ( i = 0; i < header->nSubtables; i++ )
      {
	if (( error = gx_morx_load_subtable( face, stream, subtbl_start,
					     &chain_Subtbl[i]) ))
	  {
	    for ( j = i - 1; j >= 0; j-- )
	      gx_morx_free_subtable( memory, &chain_Subtbl[j] );
	    goto Failure;
	  }
	subtbl_start += chain_Subtbl[i].header.length;
      }
    chain->feat_Subtbl 	= feat_Subtbl;
    chain->chain_Subtbl = chain_Subtbl;
  Exit:
    return error;
  Failure:
    chain->feat_Subtbl 	= NULL;
    chain->chain_Subtbl = NULL;
    if ( feat_Subtbl )
      FT_FREE ( feat_Subtbl );
    if ( chain_Subtbl )
      FT_FREE ( chain_Subtbl );
    return error;
  }

  FT_LOCAL_DEF( FT_Error )
  gx_face_load_morx( GX_Face   face,
		     FT_Stream stream,
		     GX_Morx   morx )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_XMetamorphosisChain chain;
    
    FT_ULong chain_start_pos;
    const FT_Frame_Field morx_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_MorxRec
	FT_FRAME_START ( 8 ),
	    FT_FRAME_LONG  ( version ),
	    FT_FRAME_ULONG ( nChains ),
	FT_FRAME_END
      };
    FT_ULong i, j;
    
    /* FT_TRACE2(( "MetamorphosisX Table" )); */
    if (( error = gx_table_init( &(morx->root), face, TTAG_morx, stream,
				 (GX_Table_Done_Func)gx_morx_done) ))
      goto Exit;

    morx->chain = NULL;

    if ( FT_STREAM_READ_FIELDS( morx_fields, morx ) )
      goto Exit;
    
    if ( FT_NEW_ARRAY( chain, morx->nChains ) )
      goto Exit;

    chain_start_pos = FT_STREAM_POS();
    for ( i = 0; i < morx->nChains; i++ )
      {
	if (( error = gx_morx_load_chain( face,
					  stream,
					  chain_start_pos,
					  &chain[i] ) ))
	  {
	    /* Rewind to free the resources */
	    for ( j = i ; j > 0; j-- )
	      gx_morx_free_chain ( memory, &chain[j - 1] );
	    goto Failure;
	  }
	chain_start_pos += chain[i].header.chainLength;
      }
    morx->chain = chain;
  Exit:
    return error;
  Failure:
    FT_FREE( chain );
    return error;
  }

  static void
  gx_morx_free_rearrangement_subtable ( FT_Memory memory, GX_XMetamorphosisSubtableBody body )
  {
    GX_XMetamorphosisRearrangementBody rearrangement_body = body->rearrangement;
    gx_XStateTable_free( &rearrangement_body->state_table, memory, NULL, NULL );
    FT_FREE(rearrangement_body);
    body->rearrangement = NULL;
  }

  static void
  gx_morx_free_contextual_substitution_table ( FT_Memory memory, GX_XMetamorphosisContextualSubstitutionTable substitution_table )
  {
    FT_Int i;
    
    for ( i = substitution_table->nTables; i > 0;  i-- )
      {
	if ( !substitution_table->lookupTables[i - 1] )
	  continue ;
	else
	  {    
	    gx_morx_free_lookup_table ( memory, substitution_table->lookupTables[i - 1] );
	    substitution_table->lookupTables[i - 1] = NULL;
	  }
      }
  }

  static void
  gx_morx_free_contextual_subtable ( FT_Memory memory, GX_XMetamorphosisSubtableBody body )
  {
    GX_XMetamorphosisContextualBody contextual_body 		   = body->contextual;
    GX_XMetamorphosisContextualSubstitutionTable substitutionTable = &contextual_body->substitutionTable;
    GX_LookupTable * lookup_tables 				   = substitutionTable->lookupTables;
    gx_morx_free_contextual_substitution_table ( memory, 
						 substitutionTable );
    substitutionTable->lookupTables = NULL;
    FT_FREE( lookup_tables );
    gx_XStateTable_free( &contextual_body->state_table,
			 memory,
			 gx_morx_free_contextual_subtable_entry,
			 NULL );
    FT_FREE( contextual_body );
    body->contextual = NULL;
  }

  static void
  gx_morx_free_ligature_subtable ( FT_Memory memory, GX_XMetamorphosisSubtableBody body )
  {
    GX_XMetamorphosisLigatureBody ligature_body = body->ligature;
    GX_XMetamorphosisLigatureActionTable ligActionTable = &ligature_body->ligActionTable;
    GX_XMetamorphosisComponentTable componentTable = &ligature_body->componentTable;
    GX_XMetamorphosisLigatureTable ligatureTable = &ligature_body->ligatureTable;

    FT_FREE (ligatureTable->body);
    ligatureTable->body = NULL;
    FT_FREE (componentTable->body);
    componentTable->body = NULL;
    FT_FREE (ligActionTable->body);
    ligActionTable->body = NULL;
    gx_XStateTable_free( &ligature_body->state_table, 
			 memory, 
			 NULL, /* CHECK, Thu Oct 23 14:14:43 2003 */
			 NULL);
    FT_FREE( ligature_body );
  }

  static void
  gx_morx_free_noncontextual_subtable ( FT_Memory memory, GX_XMetamorphosisSubtableBody body )
  {
    GX_XMetamorphosisNoncontextualBody noncontextual_body = body->noncontextual;
    
    gx_mort_free_noncontextual_subtable ( memory, noncontextual_body );
    body->noncontextual = NULL;
  }
  
  static void
  gx_morx_free_insertion_subtable ( FT_Memory memory, GX_XMetamorphosisSubtableBody body )
  {
    GX_XMetamorphosisInsertionBody insertion_body = body->insertion;
    GX_XStateTable state_table 			  = &insertion_body->state_table;
  
    gx_XStateTable_free( state_table,
			 memory, 
			 gx_morx_free_insertion_subtable_entry,
			 NULL );
    FT_FREE(insertion_body);
    body->insertion = NULL;
  }

  static void
  gx_morx_free_subtable( FT_Memory memory, GX_XMetamorphosisSubtable chain_Subtbl )
  {
    GX_XMetamorphosisSubtableHeader header;
    FT_ULong subtable_type;
    
    header = &chain_Subtbl->header;
    subtable_type = header->coverage & GX_MORX_COVERAGE_SUBTABLE_TYPE;
    switch ( subtable_type )
      {
      case GX_MORX_REARRANGEMENT_SUBTABLE:
	gx_morx_free_rearrangement_subtable ( memory, &chain_Subtbl->body );
	break;
      case GX_MORX_CONTEXTUAL_SUBTABLE:
	gx_morx_free_contextual_subtable ( memory, &chain_Subtbl->body );
	break;
      case GX_MORX_LIGATURE_SUBTABLE:
	gx_morx_free_ligature_subtable ( memory, &chain_Subtbl->body );
	break;
      case GX_MORX_NONCONTEXTUAL_SUBTABLE:
	gx_morx_free_noncontextual_subtable ( memory, &chain_Subtbl->body );
	break;
      case GX_MORX_INSERTION_SUBTABLE:
	gx_morx_free_insertion_subtable ( memory, &chain_Subtbl->body );
	break;
      default:
	break;
      }
    chain_Subtbl->body.any = NULL;
  }

  static void
  gx_morx_free_chain( FT_Memory memory,
		      GX_XMetamorphosisChain chain )
  {
    GX_XMetamorphosisChainHeader header;
    FT_ULong i;
    
    header = &(chain->header);
    for ( i = header->nSubtables; i > 0; i-- )
      gx_morx_free_subtable( memory, 
			     &chain->chain_Subtbl[i - 1] );
    FT_FREE(chain->chain_Subtbl);
    chain->chain_Subtbl = NULL;
    
    FT_FREE(chain->feat_Subtbl);
    chain->feat_Subtbl = NULL;
  }

  FT_LOCAL_DEF ( void )
  gx_morx_done( GX_Morx   morx,
		FT_Memory memory )
		     
  {
    FT_ULong i;
    
    for ( i = morx->nChains ; i > 0 ; i-- )
      gx_morx_free_chain( memory, &morx->chain[i - 1] );
    FT_FREE( morx->chain );
    morx->chain = NULL;
  }


/******************************FMTX************************************/
  FT_LOCAL_DEF ( FT_Error )
  gx_face_load_fmtx( GX_Face   face,
		     FT_Stream stream,
		     GX_Fmtx   fmtx )
  {
    FT_Error error;
    
    const FT_Frame_Field fmtx_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_FmtxRec
	FT_FRAME_START ( 16 ),
	    FT_FRAME_LONG  ( version ),
	    FT_FRAME_ULONG ( glyphIndex ),
	    FT_FRAME_BYTE  ( horizontalBefore ),
	    FT_FRAME_BYTE  ( horizontalAfter ),
	    FT_FRAME_BYTE  ( horizontalCaretHead ),
	    FT_FRAME_BYTE  ( horizontalCaretBase ),
	    FT_FRAME_BYTE  ( verticalBefore ),
	    FT_FRAME_BYTE  ( verticalAfter ),
	    FT_FRAME_BYTE  ( verticalCaretHead ),
	    FT_FRAME_BYTE  ( verticalCaretBase ),
	FT_FRAME_END
      };
    /* FT_TRACE2(( "Font Metrics Table" ));  */
    if (( error = gx_table_init( &(fmtx->root), face, TTAG_fmtx, stream,
				 (GX_Table_Done_Func)gx_fmtx_done) ))
      goto Exit;

    if ( FT_STREAM_READ_FIELDS( fmtx_fields, fmtx ) )
      goto Exit;
  Exit:
    return error;
  }

  FT_LOCAL_DEF ( void )
  gx_fmtx_done( GX_Fmtx   fmtx,
		     FT_Memory memory )
		     
  {
    /* DO NOTHING */
  }

/******************************FDSC************************************/
  static FT_Error
  gx_fdsc_load_descriptor( GX_Face face,
			   FT_Stream stream,
			   FT_ULong  count,
			   GX_FontDescriptor desc )
  {
    FT_Error error = GX_Err_Ok;
    const FT_Frame_Field fdsc_desc_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_FontDescriptorRec
	FT_FRAME_START ( 8 ),
	    FT_FRAME_ULONG( tag ),
	    FT_FRAME_LONG ( value ),
	FT_FRAME_END
      };

    FT_Int i;

    for ( i = 0; i < count; i++ )
      {
	if ( FT_STREAM_READ_FIELDS( fdsc_desc_fields, &desc[i] ) )
	  goto Exit;
      }
  Exit:
    return error;
  }

  FT_LOCAL_DEF( FT_Error )
  gx_face_load_fdsc( GX_Face   face,
		     FT_Stream stream,
		     GX_Fdsc   fdsc )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;

    GX_FontDescriptor desc;

    const FT_Frame_Field fdsc_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_FdscRec
	FT_FRAME_START ( 8 ),
	    FT_FRAME_LONG  ( version ),
	    FT_FRAME_ULONG ( descriptorCount ),
	FT_FRAME_END
      };

    /* FT_TRACE2(( "Font Descriptors Table" ));  */
    if (( error = gx_table_init( &(fdsc->root), face, TTAG_fdsc, stream,
				 (GX_Table_Done_Func)gx_fdsc_done) ))
      goto Exit;
    
    if ( FT_STREAM_READ_FIELDS( fdsc_fields, fdsc ) )
      goto Exit;
    
    if ( FT_NEW_ARRAY(desc, fdsc->descriptorCount) )
      goto Exit;

    if (( error = gx_fdsc_load_descriptor(face, stream, 
					  fdsc->descriptorCount, desc )))
      goto Failure;
    
    fdsc->descriptor = desc;
  Exit:
    return error;
  Failure:
    FT_FREE(desc);
    return error;
  }

  FT_LOCAL_DEF( void )
  gx_fdsc_done( GX_Fdsc   fdsc,
		     FT_Memory memory )
		     
  {
    if ( fdsc->descriptorCount )
      {
	FT_FREE( fdsc->descriptor );
	fdsc->descriptor = NULL;
      }
    
  }



/****************************JUST***********************************/
  FT_LOCAL_DEF ( FT_Error )
  gx_face_load_just( GX_Face   face,
		     FT_Stream stream,
		     GX_Just   just )
  {
    FT_Error error;

    const FT_Frame_Field just_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_JustRec
	FT_FRAME_START( 10 ),
	    FT_FRAME_ULONG( version ),
	    FT_FRAME_USHORT ( format ),
	    FT_FRAME_USHORT ( horizOffset ),
	    FT_FRAME_USHORT ( vertOffset ),
	FT_FRAME_END
      };
    
    /* FT_TRACE2(( "Justification " )); */
    if (( error = gx_table_init( &(just->root), face, TTAG_just, stream,
				 (GX_Table_Done_Func)gx_just_done) ))
      goto Exit;
    
    if ( FT_STREAM_READ_FIELDS( just_fields, just ) )
      goto Exit;
    /* TODO: just */
    /* FT_TRACE2(( "loaded\n" )); */
  Exit:
    return error;
  }
  
  FT_LOCAL_DEF ( void )
  gx_just_done( GX_Just   just,
		     FT_Memory memory )
		     
  {
    /* TODO */
  }


/****************************KERN***********************************/

  static FT_Error
  gx_kern_load_sutable_header ( GX_Face face,
				FT_Stream stream,
				GX_KerningSubtableHeader header )
  {
    FT_Error error;

    const FT_Frame_Field kern_subtable_header_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_KerningSubtableHeaderRec
	FT_FRAME_START( 8 ),
	    FT_FRAME_ULONG( length ),
	    FT_FRAME_USHORT( coverage ),
	    FT_FRAME_USHORT( tupleIndex ),
	FT_FRAME_END
      };
  
    header->position = FT_STREAM_POS();
    if ( FT_STREAM_READ_FIELDS( kern_subtable_header_fields, 
				header ) )
      goto Exit;
  Exit:    
    return error;
  }

  static FT_Error
  gx_kern_load_fmt0_subtable ( GX_Face face, FT_Stream stream,
			       GX_KerningSubtableHeader header, 
			       GX_KerningSubtableFormat0Body fmt0 )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_KerningSubtableFormat0Entry entries;
    FT_Int i;
    
    const FT_Frame_Field kern_fmt0_subtable_fields [] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_KerningSubtableFormat0BodyRec
	FT_FRAME_START ( 8 ),
	    FT_FRAME_USHORT ( nPairs ),
	    FT_FRAME_USHORT ( searchRange ),
	    FT_FRAME_USHORT ( entrySelector ),
	    FT_FRAME_USHORT ( rangeShift ),
	FT_FRAME_END
      };

    const FT_Frame_Field kern_fmt0_entry_fields [] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_KerningSubtableFormat0EntryRec
	FT_FRAME_START ( 6 ),
	    FT_FRAME_USHORT( left ),
	    FT_FRAME_USHORT( right ),
	    FT_FRAME_SHORT ( value ),
	FT_FRAME_END
      };
    
    if ( FT_STREAM_READ_FIELDS( kern_fmt0_subtable_fields,
				fmt0 ) )
      goto Exit;
    
    if ( FT_NEW_ARRAY ( entries, fmt0->nPairs ) )
      goto Exit;
    for ( i = 0; i < fmt0->nPairs; i++ )
      {
	if ( FT_STREAM_READ_FIELDS( kern_fmt0_entry_fields,
				    &entries[i] ) )
	  goto Failure;
      }
    fmt0->entries = entries;
  Exit:
    return error;
  Failure:
    FT_FREE( entries );
    fmt0->entries = NULL;
    return error;
  }

  static void
  gx_kern_free_fmt0_subtable( FT_Memory memory, GX_KerningSubtableFormat0Body fmt0 )
  {
    FT_FREE ( fmt0->entries );
  }


#if 0
  static FT_Error
  kern_max_value_offset( GX_EntrySubtable entry_subtable,
			 FT_Pointer user )
  {
    FT_UShort * max_value_offset = user;
    FT_UShort tmp;
    tmp = entry_subtable->flags & GX_KERN_ACTION_VALUE_OFFSET;
    if (*max_value_offset < tmp )
      *max_value_offset = tmp;
    return FT_Err_Ok;
  }
#endif /* 0 */

  static FT_Error
  gx_kern_load_fmt1_subtable ( GX_Face face, FT_Stream stream,
			       GX_KerningSubtableHeader header, 
			       GX_KerningSubtableFormat1Body fmt1 )
  {
    FT_Error error;
    FT_Memory memory 	= face->root.driver->root.memory;
    FT_ULong state_table_pos;
    FT_ULong value_table_length; /* in bytes */
    FT_ULong i;

    fmt1->values = NULL;
    state_table_pos = FT_STREAM_POS();
    if (( error = gx_face_load_StateTable ( face, stream, 
					    &fmt1->state_table,
					    NULL, NULL )))
      goto Exit;
    
    if ( FT_STREAM_SEEK( state_table_pos + GX_STATE_HEADER_ADVANCE ) )
      goto Failure;
    if ( FT_READ_USHORT ( fmt1->valueTable ) )
      goto Failure;

#if 0
    FT_UShort max_value_offset;
    max_value = 0;
    gx_StateTable_traverse_entries ( &fmt1->state_table,
				     kern_max_value_offset,
				     &max_value_offset );
#endif /* 0 */
    fmt1->value_absolute_pos = fmt1->valueTable + state_table_pos;
    value_table_length = header->length + header->position;
    fmt1->nValues      = value_table_length / sizeof (*(fmt1->values));
    value_table_length = fmt1->nValues * sizeof (*(fmt1->values)); /* rounding */
    
    if ( FT_NEW_ARRAY ( fmt1->values,  fmt1->nValues ) )
      goto Failure;
    if ( FT_FRAME_ENTER( value_table_length ) )
      goto Failure;
    
    for ( i = 0; i < fmt1->nValues; i++ )
      fmt1->values[i] = FT_GET_USHORT();
    FT_FRAME_EXIT();
  Exit:
    return error;
  Failure:
    if ( fmt1->values )
      FT_FREE ( fmt1->values );
    gx_StateTable_free ( &fmt1->state_table, memory, NULL, NULL);
    return error;
  }

  static void
  gx_kern_free_fmt1_subtable( FT_Memory memory, GX_KerningSubtableFormat1Body fmt1 )
  {
    FT_FREE ( fmt1->values );
    gx_StateTable_free ( &fmt1->state_table, memory, NULL, NULL);
  }

  static FT_Error
  gx_kern_load_fmt2_class_table ( GX_Face face, FT_Stream stream,
				  FT_ULong pos, GX_KerningSubtableFormat2ClassTable class_table)
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    FT_Int i;
    
    const FT_Frame_Field kern_fmt2_class_table_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_KerningSubtableFormat2ClassTableRec
	FT_FRAME_START ( 4 ),
	    FT_FRAME_USHORT ( firstGlyph ),
	    FT_FRAME_USHORT ( nGlyphs ),
	FT_FRAME_END
      };
    
    if ( FT_STREAM_SEEK ( pos ) )
      goto Exit;
    
    if ( FT_STREAM_READ_FIELDS( kern_fmt2_class_table_fields,
				class_table ) )
      goto Exit;
    
    if ( FT_NEW_ARRAY( class_table->classes, class_table->nGlyphs ) )
      goto Exit;

    class_table->max_class = 0;
    if ( FT_FRAME_ENTER( class_table->classes[0] * class_table->nGlyphs ) )
      goto Failure_classes;
    for ( i = 0; i < class_table->nGlyphs; i++ )
      {
	class_table->classes[i] = FT_GET_BYTE();
	if ( class_table->max_class < class_table->classes[i] )
	  class_table->max_class = class_table->classes[i];
      }
    FT_FRAME_EXIT();
  Exit:
    return error;
  Failure_classes:
    FT_FREE( class_table->classes );
    return error;
  }

  static void
  gx_kern_free_fmt2_class_table ( FT_Memory memory, GX_KerningSubtableFormat2ClassTable class_table)
  {
    FT_FREE(class_table->classes);
  }

  static FT_Error
  gx_kern_load_fmt2_subtable ( GX_Face face, FT_Stream stream,
			       GX_KerningSubtableHeader header, 
			       GX_KerningSubtableFormat2Body fmt2 )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    FT_ULong pos;
    FT_Int i;
    FT_Int max_value_index;
    const FT_Frame_Field kern_fmt2_subtable_fields [] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_KerningSubtableFormat2BodyRec	
	FT_FRAME_START( 8 ),
	    FT_FRAME_USHORT ( rowWidth ),
	    FT_FRAME_USHORT ( leftClassTable ),
	    FT_FRAME_USHORT ( rightClassTable ),
	    FT_FRAME_USHORT ( array ),
	FT_FRAME_END
      };

    
    fmt2->leftClass.classes = NULL;
    fmt2->rightClass.classes = NULL;
    fmt2->values = NULL;
    
    if ( FT_STREAM_READ_FIELDS( kern_fmt2_subtable_fields,
				fmt2 ) )
      goto Exit;
    
    pos = header->position + fmt2->leftClassTable;
    if ( ( error = gx_kern_load_fmt2_class_table ( face, stream, 
						   pos, & fmt2->leftClass ) ) )
      goto Exit;
    
    pos = header->position + fmt2->rightClassTable;
    if ( ( error = gx_kern_load_fmt2_class_table ( face, stream, 
						   pos, & fmt2->rightClass ) ) )
      goto Failure_leftClass;
    
    pos = header->position + fmt2->array;
    if ( FT_STREAM_SEEK( pos ) )
      goto Failure_rightClass;
    max_value_index = fmt2->leftClass.max_class + fmt2->rightClass.max_class;
    if ( FT_NEW_ARRAY ( fmt2->values, max_value_index ) )
      goto Failure_rightClass;
    if ( FT_FRAME_ENTER( sizeof( *fmt2->values ) * max_value_index ) )
      goto Failure_values;
    for ( i = 0; i < max_value_index; i++ )
      fmt2->values[i] = FT_GET_SHORT();
    FT_FRAME_EXIT();
  Exit:
    return error;
  Failure_values:
    FT_FREE(fmt2->values);
  Failure_rightClass:
    gx_kern_free_fmt2_class_table( memory, & fmt2->rightClass );
  Failure_leftClass:
    gx_kern_free_fmt2_class_table( memory, & fmt2->leftClass );
    return error;
  }

  static void
  gx_kern_free_fmt2_subtable( FT_Memory memory, GX_KerningSubtableFormat2Body fmt2 )
  {
    FT_FREE(fmt2->values);
    gx_kern_free_fmt2_class_table( memory, & fmt2->rightClass );
    gx_kern_free_fmt2_class_table( memory, & fmt2->leftClass );
  }

  static FT_Error
  gx_kern_load_fmt3_subtable ( GX_Face face, FT_Stream stream,
			       GX_KerningSubtableHeader header, 
			       GX_KerningSubtableFormat3Body fmt3 )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    FT_Int i;
    FT_FWord  * kernValue;	/* [kernValueCount] */
    FT_Byte   * leftClass;	/* [glyphCount] */
    FT_Byte   * rightClass;	/* [glyphCount] */
    FT_Byte   * kernIndex;	/* [leftClassCount * rightClassCount] */    
    FT_ULong    byte_count;
    const FT_Frame_Field kern_fmt3_subtable_fields [] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_KerningSubtableFormat3BodyRec	
	FT_FRAME_START ( 6 ),
	FT_FRAME_USHORT (glyphCount),
	FT_FRAME_BYTE   (kernValueCount),
	FT_FRAME_BYTE   (leftClassCount),
	FT_FRAME_BYTE   (rightClassCount),
	FT_FRAME_BYTE   (flags),
	FT_FRAME_END
      };
    
    if ( FT_STREAM_READ_FIELDS( kern_fmt3_subtable_fields,
				fmt3 ) )
      goto Exit;
    
    byte_count = sizeof(kernValue[0]) * fmt3->kernValueCount 
      + sizeof(FT_Byte) * (fmt3->glyphCount
			   + fmt3->glyphCount
			   + (fmt3->leftClassCount 
			      * fmt3->rightClassCount));
    if ( FT_ALLOC ( kernValue , byte_count ) )
      goto Exit;
    
    leftClass  = (FT_Byte *)(kernValue + fmt3->kernValueCount);
    rightClass = leftClass+fmt3->glyphCount;
    kernIndex  = rightClass+fmt3->glyphCount;
    
    if ( FT_FRAME_ENTER( byte_count ) )
      goto Failure;
    for ( i = 0; i < fmt3->kernValueCount; i++ )
      kernValue[i] = FT_GET_SHORT();
    for ( i = 0; i < fmt3->glyphCount; i++ )
      leftClass[i] = FT_GET_BYTE();
    for ( i = 0; i < fmt3->glyphCount; i++ )
      rightClass[i] = FT_GET_BYTE();
    for ( i = 0; i < fmt3->leftClassCount * fmt3->rightClassCount; i++ )
      kernIndex[i] = FT_GET_BYTE();
    FT_FRAME_EXIT(); 
    fmt3->kernValue  = kernValue;
    fmt3->leftClass  = leftClass;
    fmt3->rightClass = rightClass;
    fmt3->kernIndex  = kernIndex;
  Exit:
    return error;
  Failure:
    FT_FREE( kernValue );
    fmt3->kernValue  = NULL;
    fmt3->leftClass  = NULL;
    fmt3->rightClass = NULL;
    fmt3->kernIndex = NULL;
    return error;
  }

  static void
  gx_kern_free_fmt3_subtable( FT_Memory memory, GX_KerningSubtableFormat3Body fmt3 )
  {
    FT_FREE ( fmt3->kernValue );
    fmt3->kernValue  = NULL;
    fmt3->leftClass  = NULL;
    fmt3->rightClass = NULL;
    fmt3->kernIndex = NULL;
  }

  static FT_Error
  gx_kern_load_subtable( GX_Face face, 
			 FT_Stream stream, 
			 FT_ULong pos,
			 GX_KerningSubtable subtable )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_KerningFormat format;
    GX_KerningSubtableHeader header = &subtable->header;
    GX_KerningSubtableFormat0Body fmt0;
    GX_KerningSubtableFormat1Body fmt1;
    GX_KerningSubtableFormat2Body fmt2;
    GX_KerningSubtableFormat3Body fmt3;
    
    if ( FT_STREAM_SEEK( pos ) )
      goto Exit;
    if (( error = gx_kern_load_sutable_header ( face,
						stream,
						header ) ))
      goto Exit;

#define KERN_FMT_LOAD(fmt)	do {					\
      if ( FT_NEW ( fmt ) )						\
	goto Exit;							\
      if ( ( error = gx_kern_load_##fmt##_subtable ( face, stream, header, fmt ) ) ) \
	{								\
	  FT_FREE(fmt);							\
	  goto Exit;							\
	}								\
      subtable->body.fmt = fmt;						\
    } while (0)
    
    format = header->coverage&GX_KERN_COVERAGE_FORMAT_MASK;
    switch ( format )
      {
      case GX_KERN_FMT_ORDERED_LIST_OF_KERNING_PAIRS:
	KERN_FMT_LOAD(fmt0);
	break;
      case GX_KERN_FMT_STATE_TABLE_FOR_CONTEXTUAL_KERNING:
	KERN_FMT_LOAD(fmt1);
	break;
      case GX_KERN_FMT_SIMPLE_NXM_ARRAY_OF_KERNING_VALUES:
	KERN_FMT_LOAD(fmt2);
	break;
      case GX_KERN_FMT_SIMPLE_NXM_ARRAY_OF_KERNING_INDICES:
	KERN_FMT_LOAD(fmt3);
	break;
      }
  Exit:
    return error;
  }
  
  static void
  gx_kern_free_subtable( FT_Memory memory,
			 GX_KerningSubtable subtable )
  {
    GX_KerningSubtableHeader header = &subtable->header;
    GX_KerningFormat format = header->coverage&GX_KERN_COVERAGE_FORMAT_MASK;
#define KERN_FMT_FREE(fmt) do {						\
      gx_kern_free_##fmt##_subtable( memory, subtable->body.fmt );	\
      FT_FREE(subtable->body.fmt);					\
    } while (0)
    switch ( format )
      {
      case GX_KERN_FMT_ORDERED_LIST_OF_KERNING_PAIRS:
	KERN_FMT_FREE(fmt0);
	break;
      case GX_KERN_FMT_STATE_TABLE_FOR_CONTEXTUAL_KERNING:
	KERN_FMT_FREE(fmt1);
	break;	
      case GX_KERN_FMT_SIMPLE_NXM_ARRAY_OF_KERNING_VALUES:
	KERN_FMT_FREE(fmt2);
	break;	
      case GX_KERN_FMT_SIMPLE_NXM_ARRAY_OF_KERNING_INDICES:
	KERN_FMT_FREE(fmt3);
	break;	
      }
    subtable->body.any = NULL;
  }

  FT_LOCAL_DEF ( FT_Error )
  gx_face_load_kern( GX_Face   face,
		     FT_Stream stream,
		     GX_Kern   kern )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_KerningSubtable subtables;
    FT_ULong subtable_start_pos;
    const FT_Frame_Field kern_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_KernRec
	FT_FRAME_START( 8 ),
	    FT_FRAME_ULONG( version ),
	    FT_FRAME_ULONG ( nTables ),
	FT_FRAME_END
      };
    FT_Int i, j;
    
    /* FT_TRACE2(( "Kerning " )); */
    if (( error = gx_table_init( &(kern->root), face, TTAG_kern, stream,
				 (GX_Table_Done_Func)gx_kern_done) ))
      goto Exit;

    if ( FT_STREAM_READ_FIELDS( kern_fields, kern ) )
      goto Exit;
    if ( kern->version < 0x00010000 )
      {
	error = GX_Err_Old_Kerning_Table;
	goto Exit;		/* old style kern */
      }

    if ( FT_NEW_ARRAY( subtables, kern->nTables ) )
      goto Exit;
    
    subtable_start_pos = FT_STREAM_POS();
    for ( i = 0; i < kern->nTables; i++ )
      {
	if ( ( error = gx_kern_load_subtable( face, 
					      stream, 
					      subtable_start_pos,
					      &subtables[i] ) ) )
	  goto Failure;
	subtable_start_pos += subtables[i].header.length;
      }
    kern->subtables = subtables;
    /* FT_TRACE2(( "loaded\n" )); */
  Exit:
    return error;
  Failure:
    for ( j = i; j > 0; j-- )
      gx_kern_free_subtable( memory, &subtables[j - 1] );
    FT_FREE(subtables);
    kern->subtables = NULL;
    return error;
  }

  FT_LOCAL_DEF ( void )
  gx_kern_done( GX_Kern   kern,
		FT_Memory memory )
		     
  {
    FT_Int i;

    for ( i = kern->nTables; i > 0; i-- )
      gx_kern_free_subtable( memory, &kern->subtables[i - 1] );
    FT_FREE(kern->subtables);
    kern->subtables = NULL;
  }


/****************************FVAR***********************************/

  static FT_Error gx_fvar_load_sfnt_instance ( GX_Face face, FT_Stream stream, 
					       FT_UShort axis_count,
					       GX_FontVariationsSFNTInstance instance );
  static void gx_fvar_free_sfnt_instance ( FT_Memory memory, 
					   GX_FontVariationsSFNTInstance instance );

  FT_LOCAL ( FT_Error )
  gx_face_load_fvar ( GX_Face face,
		      FT_Stream stream,
		      GX_Fvar fvar )
  {
    FT_Error error;
    FT_UShort i, j;
    FT_Memory memory = stream->memory;
    
    const FT_Frame_Field fvar_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_FvarRec	
	FT_FRAME_START ( 14 ),
	    FT_FRAME_LONG   ( version ),
	    FT_FRAME_USHORT ( offsetToData ),
	    FT_FRAME_USHORT ( countSizePairs ),
	    FT_FRAME_USHORT ( axisCount ),
	    FT_FRAME_USHORT ( axisSize ),
	    FT_FRAME_USHORT ( instanceCount ),
	    FT_FRAME_USHORT ( instanceSize ),
	FT_FRAME_END
      };

    const FT_Frame_Field fvar_axis_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_FontVariationsSFNTVariationAxisRec
	FT_FRAME_START ( 20 ),
	    FT_FRAME_ULONG ( axisTag ),
	    FT_FRAME_LONG  ( minValue ),
	    FT_FRAME_LONG  ( defaultValue ),
	    FT_FRAME_LONG  ( maxValue ),
	    FT_FRAME_USHORT( flags ),
	    FT_FRAME_USHORT( nameID ),
	FT_FRAME_END
      };
    
    if (( error = gx_table_init( &(fvar->root), face, TTAG_fvar, stream,
				 (GX_Table_Done_Func)gx_fvar_done) ))
      goto Failure;
    
    if ( FT_STREAM_READ_FIELDS( fvar_fields, fvar ) )
      goto Failure;

    if ( FT_STREAM_SEEK( fvar->root.position + fvar->offsetToData ) )
      goto Failure;
    
    if ( FT_NEW_ARRAY( fvar->axis, fvar->axisCount ) )
      goto Failure;

    if ( FT_NEW_ARRAY( fvar->instance, fvar->instanceCount ) )
      goto Failure;
    
    for ( i = 0; i < fvar->axisCount; i++ )
      {
	if ( FT_STREAM_READ_FIELDS( fvar_axis_fields, &fvar->axis[i] ) )
	  goto Failure;
      }
    
    for ( i = 0; i < fvar->instanceCount; i++ )
      {
	if (( error = gx_fvar_load_sfnt_instance( face, 
						  stream,
						  fvar->axisCount,
						  &fvar->instance[i]) ))
	  {
	    for ( j = i; j > 0; j-- )
	      gx_fvar_free_sfnt_instance( memory,
					  &fvar->instance[j - 1] );
	    goto Failure;
	  }
      }
    return error;
  Failure:
    if ( fvar->axis )
      FT_FREE( fvar->axis );
    if ( fvar->instanceCount )
      FT_FREE ( fvar->instance );
    return error;
  }

  static FT_Error
  gx_fvar_load_sfnt_instance ( GX_Face face, 
			       FT_Stream stream, 
			       FT_UShort axis_count,
			       GX_FontVariationsSFNTInstance instance )
  {
    FT_Error error;
    FT_UShort i;
    FT_Memory memory = stream->memory;
    
    const FT_Frame_Field fvar_sfnt_instance_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_FontVariationsSFNTInstanceRec
	FT_FRAME_START ( 4 ),
	  FT_FRAME_USHORT ( nameID ),
	  FT_FRAME_USHORT ( flags ),
	FT_FRAME_END,
      };

    if (( FT_NEW_ARRAY( instance->coord, axis_count ) ))
      goto Failure;
    
    if ( FT_STREAM_READ_FIELDS( fvar_sfnt_instance_fields, 
				instance ) )
      goto Failure;

    if ( FT_FRAME_ENTER ( axis_count * sizeof( FT_Fixed) ) )
      goto Failure;
    
    for ( i = 0; i < axis_count; i++ )
      instance->coord[i] = FT_GET_LONG();
    
    FT_FRAME_EXIT();
    
    return error;
  Failure:
    if ( instance->coord )
      FT_FREE( instance->coord );
    return error;
  }
  static void
  gx_fvar_free_sfnt_instance ( FT_Memory memory, 
			       GX_FontVariationsSFNTInstance instance )
  {
    FT_FREE( instance->coord );
  }

  static void
  gx_fvar_done( GX_Fvar   fvar,
		FT_Memory memory )
  {
    FT_UShort i;
    
    for ( i = fvar->instanceCount; i > 0; i-- )
      gx_fvar_free_sfnt_instance(memory, &fvar->instance[i - 1] );

    FT_FREE ( fvar->instance );
    FT_FREE ( fvar->axis );
  }


/****************************GENERIC***********************************/
  static FT_Error
  generic_lookup_table_segment_array_loader ( GX_LookupTable_Format format,
					      FT_UShort lastGlyph,
					      FT_UShort firstGlyph,
					      GX_LookupValue value,
					      FT_Pointer user )
  {
    FT_Error error;
    generic_lookup_table_cb_data lookup_data = user;
    GX_Face face 			     = lookup_data->face;
    FT_Stream stream 			     = lookup_data->stream;
    GX_LookupTable lookup_table 	     = lookup_data->lookup_table;
    FT_Int table_tag  = lookup_data->table_tag;
    
    FT_Memory memory = stream->memory;

    FT_Short  value_offset  = value->raw.s;

    FT_UShort segment_count = lastGlyph - firstGlyph + 1;
    FT_UShort * segment;
    
    FT_Int i;
    
    if ( table_tag )
      {
	if ( (error = face->goto_table( face, table_tag, stream, 0 )) || 
	     FT_STREAM_SEEK( FT_STREAM_POS() + value_offset )           )
	  goto Exit;
      }
    else
      {
	if (FT_STREAM_SEEK( lookup_table->position + value_offset ) ) 
	  goto Exit;
      }

    if ( FT_NEW_ARRAY(segment, segment_count ) )
      goto Exit;

    if ( FT_FRAME_ENTER ( sizeof( segment[0] ) * segment_count ) )
      goto Failure;
    for ( i = 0; i < segment_count; i++ )
      segment[i] = FT_GET_USHORT();
    FT_FRAME_EXIT();
    value->extra.word = segment;
  Exit:
    return error;
  Failure:
    /* TODO 
       --------------------------------------------------------------
       Other value->extra.wordS loaded before the visitation to this 
       value->extra.word must be freed if an error is occurred during 
       traverse. */
    FT_FREE(segment);
    return error;
  }

  static FT_Error
  generic_lookup_table_segment_array_finalizer ( GX_LookupTable_Format format,
						 FT_UShort lastGlyph,
						 FT_UShort firstGlyph,
						 GX_LookupValue value,
						 FT_Pointer user )
  {
    FT_Memory memory = user;
    FT_UShort * segment = value->extra.word;
    if ( !segment )
      return GX_Err_Ok;

    value->extra.word = NULL;
    FT_FREE(segment);
    return GX_Err_Ok; 
  }

  static FT_Error
  generic_load_noncontextual_subtable ( GX_Face face,
					FT_Stream stream,
					FT_UShort length,
					GX_MetamorphosisSubtableBody body,
					FT_Int tag )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_MetamorphosisNoncontextualBody noncontextual_body;
    GX_LookupTable lookup_table;
    GX_LookupTable_FuncsRec funcs = GX_LOOKUP_TABLE_FUNC_ZERO;
    generic_lookup_table_cb_data_rec user_data = GENERIC_LOOKUP_TABLE_CB_DATA_ZERO;
    
    funcs.segment_array_func = generic_lookup_table_segment_array_loader;
    user_data.face 	 = face;
    user_data.stream 	 = stream;
    /* -----------------------------------------------------------------
     * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 
     * This code is correct?
     * In spec: "Sometimes they are 16-bit offsets from the start of 
     * the table to the data. " The table? Is the table the lookup table
     * or a table that uses the lookup table?
     * Here I assume the table is the lookup table.
     *
     * The 11th chainSubtable in /Library/Fonts/GujaratiMT.ttf in MacOSX
     * is format 4 lookup table; and it expects the table is the lookup 
     * table.
     * It seems that  pfaedit uses lookup_table_offset + value_offset.
     * ----------------------------------------------------------------- */
    /* user_data.table_tag    = tag; */
    user_data.table_tag = 0;
    
    if ( FT_NEW ( noncontextual_body  ) )
      goto Exit;
    lookup_table 	   = &noncontextual_body->lookup_table;
    user_data.lookup_table = lookup_table;
    if (( error = gx_face_load_LookupTable( face, stream, 
					    lookup_table )))
      goto Failure;

    if ( lookup_table->format == GX_LOOKUPTABLE_SEGMENT_ARRAY )
      {
	if (( error = gx_LookupTable_traverse_low( lookup_table, &funcs, &user_data ) ))
	  goto Failure_Lookup_Table;
      }
    body->noncontextual = noncontextual_body;
  Exit:
    return error;
  Failure_Lookup_Table:
    gx_LookupTable_free( lookup_table, memory  );
  Failure:
    FT_FREE( noncontextual_body );
    body->noncontextual = NULL;
    return error;
  }

  static void
  generic_triple_offset_diff ( FT_ULong ligActionTable_offset,
			       FT_ULong componentTable_offset,
			       FT_ULong ligatureTable_offset,
			       FT_ULong length,
			       FT_UShort *ligActionTable_nAction,
			       FT_UShort *componentTable_nComponent,
			       FT_UShort *ligatureTable_nLigature )
  {
    if (( ligActionTable_offset < componentTable_offset ) &&
	( ligActionTable_offset < ligatureTable_offset ) )
      {				/* 1st: ligActionTable */
	if ( componentTable_offset < ligatureTable_offset )
	  { /* 2nd: componentTable, 3rd: ligatureTable */
	    /* 2nd - 1st */
	    *ligActionTable_nAction = (componentTable_offset- ligActionTable_offset)/4;
	    *componentTable_nComponent = (ligatureTable_offset - componentTable_offset)/2;
	    *ligatureTable_nLigature =   (length - ligatureTable_offset)/2;
	  }
	else
	  { /* 2nd: ligatureTable, 3rd: componentTable */
	    /* 2nd - 1st */
	    *ligActionTable_nAction = (ligatureTable_offset - ligActionTable_offset)/4;
	    *componentTable_nComponent = (length - componentTable_offset)/2;
	    *ligatureTable_nLigature   = (componentTable_offset - ligatureTable_offset)/2;
	  }
      }
    else if (( componentTable_offset < ligActionTable_offset) &&
	     ( componentTable_offset < ligatureTable_offset ))
      {				/* 1st: componentTable */
	if ( ligActionTable_offset < ligatureTable_offset )
	  { /* 2nd: ligActionTable, 3rd: ligatureTable */
	    /* 3rd - 2nd */
	    *ligActionTable_nAction = (ligatureTable_offset - ligActionTable_offset)/4;
	    *componentTable_nComponent = (componentTable_offset - ligActionTable_offset)/2;
	    *ligatureTable_nLigature   = (length - ligatureTable_offset)/2;
	  }
	else
	  { /* 2nd: ligatureTable, 3rd: ligActionTable */
	    /* length - 3rd */
	    *ligActionTable_nAction = (length - ligActionTable_offset)/4;
	    *componentTable_nComponent = (componentTable_offset - ligatureTable_offset)/2;
	    *ligatureTable_nLigature   = (ligActionTable_offset - ligatureTable_offset)/2;
	  }
      }
    else
      {				/* 1st: ligatureTable */
	if ( ligActionTable_offset < componentTable_offset )
	  { /* 2nd: ligActionTable, 3rd: componentTable */
	    /* 3rd - 2nd */
	    *ligActionTable_nAction = (componentTable_offset - ligActionTable_offset)/4;
	    *componentTable_nComponent = (length - componentTable_offset)/2;
	    *ligatureTable_nLigature   = (ligActionTable_offset - ligatureTable_offset)/2;
	  }
	else
	  { /* 2nd: componentTable, 3rd: ligActionTable */
	    /* length - 3rd */
	    *ligActionTable_nAction = (length - ligActionTable_offset)/4;
	    *componentTable_nComponent = (ligActionTable_offset - componentTable_offset)/2;
	    *ligatureTable_nLigature   = (length - ligatureTable_offset)/2;
	  }
      }

  }

  static FT_Error
  gx_table_init(GX_Table table_info, 
		GX_Face face, FT_ULong tag, FT_Stream stream, GX_Table_Done_Func done_table)
  {
    FT_Error error;
    if (( error = face->goto_table( face, tag, stream, 
				    &table_info->length) ))
      return error;
    table_info->position   = FT_STREAM_POS();
    table_info->font 	   = NULL;
    table_info->done_table = done_table;
    return error;
  }

  FT_LOCAL ( FT_Error )
  gx_table_done ( GX_Table table, FT_Memory  memory )
  {
    if ( table && table->done_table)
      table->done_table( table, memory );
    return FT_Err_Ok; 
  }

/* END */
