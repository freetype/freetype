/***************************************************************************/
/*                                                                         */
/*  gxstatetbl.c                                                           */
/*                                                                         */
/*    AAT/TrueTypeGX state table related types and functions               */
/*    (body).                                                              */
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

#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_MEMORY_H

#include "gxstatetbl.h"
#include "gxlookuptbl.h"
#include "gxerrors.h"

  static FT_Error
  gx_StateTable_load_header( GX_Face face,
			     FT_Stream stream,
			     GX_StateHeader header )
  {
    FT_Error error;
    static const FT_Frame_Field state_header_fields[] =
      {
#undef  FT_STRUCTURE
#define FT_STRUCTURE GX_StateHeaderRec
       FT_FRAME_START ( 8 ),
	   FT_FRAME_USHORT ( stateSize ),
	   FT_FRAME_USHORT ( classTable ),
	   FT_FRAME_USHORT ( stateArray ),
	   FT_FRAME_USHORT ( entryTable ),
       FT_FRAME_END
      };
    header->position    = FT_STREAM_POS();
    return FT_STREAM_READ_FIELDS( state_header_fields, header );
  }

  static FT_Error
  gx_StateTable_load_class_subtable( GX_Face face,
				     FT_Stream stream,
				     FT_ULong pos,
				     GX_ClassSubtable subtable )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    FT_Byte * classArray;
    FT_Int i;
    static const FT_Frame_Field class_subtable_fields[] =
      {
#undef  FT_STRUCTURE
#define FT_STRUCTURE GX_ClassSubtableRec
	FT_FRAME_START ( 4 ),
	    FT_FRAME_USHORT ( firstGlyph ),
	    FT_FRAME_USHORT ( nGlyphs ),
	FT_FRAME_END
      };

    subtable->classArray = NULL;

    if ( FT_STREAM_SEEK(pos) )
      goto Exit;

    if ( FT_STREAM_READ_FIELDS ( class_subtable_fields,  subtable ) )
      goto Exit;

    if ( FT_NEW_ARRAY ( classArray, subtable->nGlyphs ) )
      goto Exit;

    if ( FT_FRAME_ENTER( sizeof (classArray[0]) * subtable->nGlyphs ) )
      goto Failure;

    for ( i = 0; i < subtable->nGlyphs; i++ )
      classArray[i] = FT_GET_BYTE();

    FT_FRAME_EXIT();
    subtable->classArray = classArray;
  Exit:
    return error;
  Failure:
    FT_FREE( classArray );
    return error;
  }

  static void
  gx_StateTable_free_class_subtable( FT_Memory memory,
				     GX_ClassSubtable subtable )
  {
    FT_FREE(subtable->classArray);
    subtable->classArray = NULL;
  }

  static FT_Error
  gx_StateTable_load_state_array( GX_Face   face,
				  FT_Stream stream,
				  FT_ULong  pos,
				  FT_UShort length,
				  FT_Byte * state_array )
  {
    FT_Error error;
    FT_Int i;

    if ( FT_STREAM_SEEK(pos) )
      goto Exit;

    if ( FT_FRAME_ENTER( sizeof (state_array[0]) * length ) )
      goto Exit;

    for ( i = 0; i < length; i++ )
      state_array[i] = FT_GET_BYTE();

    FT_FRAME_EXIT();
  Exit:
    return error;
  }

/* - gx_StateTable_load_entry_subtable
   I assume FUNCS is not NULL. Set any kind of dummy in the caller if
   necessary.
   To support both a state table and an extended state table, the type of nEntries
   is FT_ULong. */
  static FT_Error
  gx_StateTable_load_entry_subtable ( GX_Face   face,
				      FT_Stream stream,
				      FT_ULong  pos,
				      FT_ULong  nEntries,
				      GX_EntrySubtable entry_subtable,
				      GX_StateTable_Entry_Load_Funcs funcs,
				      FT_Pointer user )
  {
    FT_Error error;
    FT_Memory memory = face->root.driver->root.memory;
    FT_ULong i, j;

    static const FT_Frame_Field entry_subtable_fields[] =
      {
#undef  FT_STRUCTURE
#define FT_STRUCTURE GX_EntrySubtableRec
	FT_FRAME_START ( 4 ),
	    FT_FRAME_USHORT ( newState ),
	    FT_FRAME_USHORT ( flags ),
	FT_FRAME_END
      };

    if ( FT_STREAM_SEEK( pos ) )
      goto Exit;

    for ( i = 0; i < nEntries; i++ )
      {
	if ( FT_STREAM_READ_FIELDS ( entry_subtable_fields,  &entry_subtable[i] ) )
	  goto Failure;
	if (( error = funcs->loader(face, stream, &entry_subtable[i], user) ))
	  goto Failure;
      }
  Exit:
    return error;
  Failure:
    for ( j = i; j > 0; j-- )
      funcs->finalizer(memory, &entry_subtable[j - 1], user);
    return error;
  }

  static void
  gx_StateTable_free_entry_subtable ( FT_Memory memory,
				      FT_Byte   nEntries,
				      GX_EntrySubtable entry_subtable,
				      GX_StateTable_Entry_Finalizer finalizer,
				      FT_Pointer user )
  {
    FT_Int i;
    for ( i = 0; i < nEntries; i++ )
      finalizer(memory, &entry_subtable[i], user);
  }

  static FT_Error
  gx_StateTable_Entry_default_loader ( GX_Face face,
				       FT_Stream stream,
				       GX_EntrySubtable entry_subtable,
				       FT_Pointer user )
  {
    entry_subtable->glyphOffsets.any = NULL;
    return GX_Err_Ok;
  }

  static void
  gx_StateTable_Entry_default_finalizer ( FT_Memory memory,
					  GX_EntrySubtable entry_subtable,
					  FT_Pointer user )
 {
   /* Do nothing */;
 }

  static FT_Byte
  gx_StateTable_find_state_array_max_index (FT_UShort array_length, FT_Byte state_array[])
  {
    FT_Int i;
    FT_Byte max_index = 0;
    for ( i = 0; i < array_length; i++ )
      {
	/* fprintf(stderr, "->%u\n", state_array[i]); */
	if ( state_array[i] > max_index )
	  max_index = state_array [i];
      }
    return max_index;
  }

  FT_LOCAL_DEF ( FT_Error )
  gx_face_load_StateTable ( GX_Face face,
			    FT_Stream stream,
			    GX_StateTable state_table,
			    GX_StateTable_Entry_Load_Funcs funcs,
			    FT_Pointer user )

  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_StateHeader header;
    GX_ClassSubtable class_subtable;
    FT_Byte * state_array = NULL;
    FT_UShort state_array_len;
    GX_EntrySubtable entry_subtable = NULL;
    FT_ULong pos;
    GX_StateTable_Entry_Load_FuncsRec default_funcs = GX_STATE_TABLE_ENTRY_LOAD_FUNCS_ZERO;

    default_funcs.loader    = gx_StateTable_Entry_default_loader;
    default_funcs.finalizer = gx_StateTable_Entry_default_finalizer;

    state_table->state_array = NULL;
    state_table->entry_subtable = NULL;

    /* 1. Header */
    header = &state_table->header;
    if (( error  = gx_StateTable_load_header( face, stream,
					      header ) ))
      goto Exit;

    /* 2. class subtable */
    pos 	   = header->position + header->classTable;
    class_subtable = &state_table->class_subtable;
    if (( error = gx_StateTable_load_class_subtable( face,
						     stream,
						     pos,
						     class_subtable ) ))
      goto Exit;


    /* 3. state array */
    /* To calculate the length of stateArray, we assume
       the order of fields placement is classTable, stateArray,
       entryTable */
    FT_ASSERT( header->classTable < header->stateArray );
    FT_ASSERT( header->stateArray < header->entryTable );

    pos 	    = header->position + header->stateArray;
    state_array_len = header->entryTable - header->stateArray;
    state_table->nStates = state_array_len / header->stateSize;
    /* Calculate state_array_len again.
       state_array_len must be a multiple of header->stateSize. */
    state_array_len = state_table->nStates  * header->stateSize;
    if (( FT_NEW_ARRAY ( state_array, state_array_len ) ))
      goto Failure;

    if (( error = gx_StateTable_load_state_array( face,
						  stream,
						  pos,
						  state_array_len,
						  state_array ) ))
      goto Failure;

    /* 4. entry subtable */
    if ( funcs )
      {
	if (! funcs->loader )
	  funcs->loader = gx_StateTable_Entry_default_loader;
	if (! funcs->finalizer )
	  funcs->finalizer = gx_StateTable_Entry_default_finalizer;
      }
    else
      funcs = &default_funcs;

    pos = header->position + header->entryTable;
    /* gx_StateTable_find_state_array_max_index returns the max index into an array
       which starts from 0. By adding 1 to the max index, get the length of the array. */
    state_table->nEntries = 1+ gx_StateTable_find_state_array_max_index(state_array_len,
									state_array);

    if (( FT_NEW_ARRAY( entry_subtable, state_table->nEntries) ))
      goto Failure;
    if (( error = gx_StateTable_load_entry_subtable( face,
						     stream,
						     pos,
						     state_table->nEntries,
						     entry_subtable,
						     funcs,
						     user) ))
      goto Failure;

    state_table->state_array 	= state_array;
    state_table->entry_subtable = entry_subtable;
  Exit:
    return error;
  Failure:
    if ( entry_subtable )
      FT_FREE ( entry_subtable );
    if ( state_array )
      FT_FREE(state_array);
    gx_StateTable_free_class_subtable (memory, class_subtable);
    return error;
  }

  FT_LOCAL_DEF ( void )
  gx_StateTable_free ( GX_StateTable state_table,
		       FT_Memory memory,
		       GX_StateTable_Entry_Finalizer finalizer,
		       FT_Pointer user )
  {
    if ( finalizer == NULL )
      finalizer = gx_StateTable_Entry_default_finalizer;
    gx_StateTable_free_entry_subtable ( memory,
					state_table->nEntries,
					state_table->entry_subtable,
					finalizer,
					user );
    FT_FREE( state_table->entry_subtable );
    state_table->entry_subtable = NULL;

    FT_FREE( state_table->state_array );
    state_table->state_array = NULL;

    gx_StateTable_free_class_subtable( memory, &state_table->class_subtable );
  }

  static FT_Error
  gx_EntrySubtable_traverse( GX_EntrySubtable entries,
			     FT_Long nEntries,
			     GX_StateTable_Entry_Action action,
			     FT_Pointer user )
  {
    FT_Error error = GX_Err_Ok;
    FT_Int i;

    if (!action)
      return error;

    for ( i = 0; i < nEntries; i++ )
      {
	error = action( &entries[i], user );
	if ( error )
	  return error;
      }
    return error;
  }

#if 0
  FT_LOCAL_DEF ( FT_Error )
  gx_StateTable_traverse_entries( GX_StateTable state_table,
				  GX_StateTable_Entry_Action action,
				  FT_Pointer user )
 {
      return gx_EntrySubtable_traverse( state_table->entry_subtable,
					state_table->nEntries,
					action,
					user );
  }
#endif /* 0 */

  FT_LOCAL_DEF ( FT_Byte )
  gx_StateTable_get_class ( GX_StateTable state_table,
			    FT_UShort glyph )
  {
    GX_ClassSubtable class_subtable;
    FT_Byte class_code;
    FT_UShort first_glyph, last_glyph;

    class_subtable = &state_table->class_subtable;
    first_glyph    = class_subtable->firstGlyph;
    last_glyph     = first_glyph + class_subtable->nGlyphs;

    if ( glyph == GX_DELETED_GLYPH_INDEX )
      class_code = GX_CLASS_DELETED_GLYPH;
    else if ( ( first_glyph <= glyph ) && ( glyph < last_glyph ) )
      class_code = class_subtable->classArray[glyph - first_glyph];
    else
      class_code = GX_CLASS_OUT_OF_BOUNDS;
    return class_code;
  }

  FT_LOCAL_DEF ( GX_EntrySubtable )
  gx_StateTable_get_entry_subtable ( GX_StateTable state_table,
				     FT_UShort current_state,
				     FT_Byte class_code )
  {
    GX_StateHeader header 	    = &state_table->header;
    FT_Byte * state_array 	    = state_table->state_array;
    GX_EntrySubtable entry_subtable = state_table->entry_subtable;
    FT_Byte * state_array_for_current_state;
    FT_Byte entry_index;

    state_array_for_current_state = &state_array[current_state - header->stateArray];
    entry_index 		  = state_array_for_current_state[class_code];
    return &entry_subtable[entry_index];
  }

/*
 * Extended State Table
 */

#define gx_XStateTable_load_entry_subtable gx_StateTable_load_entry_subtable
#define gx_XStateTable_free_entry_subtable gx_StateTable_free_entry_subtable

#define gx_XStateTable_Entry_default_loader gx_StateTable_Entry_default_loader
#define gx_XStateTable_Entry_default_finalizer gx_StateTable_Entry_default_finalizer
#define gx_XStateTable_Entry_default_finalizer gx_StateTable_Entry_default_finalizer

  static FT_Error
  gx_XStateTable_load_header( GX_Face face,
			     FT_Stream stream,
			     GX_XStateHeader header )
  {
    FT_Error error;
    static const FT_Frame_Field xstate_header_fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_XStateHeaderRec
	FT_FRAME_START ( 16 ),
	    FT_FRAME_ULONG( nClasses ),
	    FT_FRAME_ULONG( classTableOffset ),
	    FT_FRAME_ULONG( stateArrayOffset ),
	    FT_FRAME_ULONG( entryTableOffset ),
	FT_FRAME_END
      };
    header->position = FT_STREAM_POS();
    return FT_STREAM_READ_FIELDS( xstate_header_fields, header );
  }

  static FT_Error
  gx_XStateTable_load_state_array( GX_Face   face,
				   FT_Stream  stream,
				   FT_ULong   pos,
				   FT_ULong   length,
				   FT_UShort* state_array )
  {
    FT_Error error;
    FT_Int i;

    if ( FT_STREAM_SEEK(pos) )
      goto Exit;

    if ( FT_FRAME_ENTER( sizeof (state_array[0]) * length ) )
      goto Exit;

    for ( i = 0; i < length; i++ )
      state_array[i] = FT_GET_USHORT();

    FT_FRAME_EXIT();
  Exit:
    return error;
  }

  static FT_UShort
  gx_XStateTable_find_state_array_max_index (FT_ULong array_length, FT_UShort state_array[])
  {
    FT_ULong i;
    FT_UShort max_index = 0;
    for ( i = 0; i < array_length; i++ )
      {
	if ( state_array[i] > max_index )
	  max_index = state_array [i];
      }
    return max_index;
  }

#define LOOKUP_TABLE_CB_DATA_ZERO {NULL, NULL}
  typedef struct lookup_table_cb_data_rec_
  {
    FT_Stream stream;
    GX_LookupTable lookup_table;
    FT_Long        table_end;
  } lookup_table_cb_data_rec, *lookup_table_cb_data;

  static FT_Error
  gx_XStateTable_LookupTable_segment_array_loader( GX_LookupTable_Format format,
						   FT_UShort lastGlyph,
						   FT_UShort firstGlyph,
						   GX_LookupValue value,
						   FT_Pointer user )
  {
    /* generic_lookup_table_segment_array_loader */
    FT_Error error;
    lookup_table_cb_data lookup_data = user;
    FT_Stream stream                 = lookup_data->stream;
    GX_LookupTable lookup_table      = lookup_data->lookup_table;
    FT_Memory memory = stream->memory;
    FT_Short  value_offset  = value->raw.s;
    FT_UShort segment_count = lastGlyph - firstGlyph + 1;
    FT_UShort * segment;

    FT_Int i;

    /* -----------------------------------------------------------------
     * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
     * This code is correct?
     * In spec: "Sometimes they are 16-bit offsets from the start of
     * the table to the data. " The table? Is the table the lookup table
     * or a table that uses the lookup table?
     * Here I assume the table is the table that uses the lookup table.
     * However, I have no conviction.
     * It seems that  pfaedit uses lookup_table_offset + value_offset.
     * ----------------------------------------------------------------- */
    FT_ASSERT(lookup_table->position + value_offset < lookup_data->table_end);
    if (FT_STREAM_SEEK( lookup_table->position + value_offset ) )
      goto Exit;

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
       Other value->extra.wordS loaded before the visitation to this
       value->extra.word must be freed if an error is occurred during
       traverse. */
    FT_FREE(segment);
    return error;
  }

  FT_LOCAL_DEF ( FT_Error )
  gx_face_load_XStateTable ( GX_Face face,
			     FT_Stream stream,
			     GX_XStateTable state_table,
			     GX_XStateTable_Entry_Load_Funcs funcs,
			     FT_Pointer user )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_XStateHeader header;
    FT_ULong state_array_len;	/* in word */
    FT_UShort * state_array = NULL;
    GX_EntrySubtable entry_subtable = NULL;
    FT_ULong pos;
    GX_XStateTable_Entry_Load_FuncsRec default_XStateTable_funcs = GX_XSTATE_TABLE_ENTRY_LOAD_FUNCS_ZERO;
    GX_LookupTable_FuncsRec default_LookupTable_funcs = GX_LOOKUP_TABLE_FUNC_ZERO;
    lookup_table_cb_data_rec lookup_table_cb_data = LOOKUP_TABLE_CB_DATA_ZERO;

    default_XStateTable_funcs.loader = gx_XStateTable_Entry_default_loader;
    default_XStateTable_funcs.finalizer = gx_XStateTable_Entry_default_finalizer;

    default_LookupTable_funcs.segment_array_func = gx_XStateTable_LookupTable_segment_array_loader;

    state_table->state_array 			  = NULL;
    state_table->entry_subtable 		  = NULL;


    /* 1. Header */
    header = &state_table->header;
    if (( error = gx_XStateTable_load_header( face, stream, header ) ))
      goto Exit;

    /* 2. class subtable */
    pos = header->position + header->classTableOffset;
    if ( FT_STREAM_SEEK( pos ) )
      goto Exit;

    if (( error = gx_face_load_LookupTable( face,
					    stream,
					    &state_table->class_subtable ) ))
      goto Exit;

    if ( state_table->class_subtable.format == GX_LOOKUPTABLE_SEGMENT_ARRAY )
      {

	lookup_table_cb_data.stream       = stream;
	lookup_table_cb_data.lookup_table = &state_table->class_subtable;
	lookup_table_cb_data.table_end = header->position + header->stateArrayOffset;
	if (( error = gx_LookupTable_traverse_low( & state_table->class_subtable,
						    & default_LookupTable_funcs,
						    & lookup_table_cb_data ) ))
	  goto Failure;
      }

    /* 3. state array */
    /* To calculate the length of stateArray, we assume
       the order of fields placement is classTable, stateArray,
       entryTable */
    FT_ASSERT( header->classTableOffset < header->stateArrayOffset );
    FT_ASSERT( header->stateArrayOffset < header->entryTableOffset );

    pos = header->position + header->stateArrayOffset;
    state_array_len = (header->entryTableOffset - header->stateArrayOffset) / sizeof( state_array[0] );
    state_table->nStates = state_array_len / header->nClasses;
    /* Calculate state_array_len again.
       state_array_len must be a multiple of header->nClasses. */
    state_array_len = state_table->nStates  * header->nClasses;

    if (( FT_NEW_ARRAY ( state_array, state_array_len ) ))
      goto Failure;

    if (( error = gx_XStateTable_load_state_array( face,
						   stream,
						   pos,
						   state_array_len,
						   state_array ) ))
      goto Failure;

    /* 4. entry subtable */
    if ( funcs )
      {
	if (! funcs->loader )
	  funcs->loader = gx_XStateTable_Entry_default_loader;
	if (! funcs->finalizer )
	  funcs->finalizer = gx_XStateTable_Entry_default_finalizer;
      }
    else
      funcs = &default_XStateTable_funcs;

    pos = header->position + header->entryTableOffset;
    state_table->nEntries = 1+ gx_XStateTable_find_state_array_max_index( state_array_len,
									  state_array );
    if (( FT_NEW_ARRAY( entry_subtable, state_table->nEntries) ))
      goto Failure;
    if (( error = gx_XStateTable_load_entry_subtable( face,
						      stream,
						      pos,
						      state_table->nEntries,
						      entry_subtable,
						      funcs,
						      user) ))
      goto Failure;
    state_table->state_array 	= state_array;
    state_table->entry_subtable = entry_subtable;
  Exit:
    return error;
  Failure:
    if ( entry_subtable )
      FT_FREE ( entry_subtable );
    if ( state_array )
      FT_FREE(state_array);
    gx_LookupTable_free( & state_table->class_subtable, memory );
    return error;
  }

  FT_LOCAL_DEF ( FT_Error )
  gx_XStateTable_traverse_entries( GX_XStateTable state_table,
				   GX_XStateTable_Entry_Action action,
				   FT_Pointer user )
  {
    return gx_EntrySubtable_traverse( state_table->entry_subtable,
				      state_table->nEntries,
				      action,
				      user );
  }

  static FT_Error
  gx_XStateTable_LookupTable_segment_array_finalizer ( GX_LookupTable_Format format,
						       FT_UShort lastGlyph,
						       FT_UShort firstGlyph,
						       GX_LookupValue value,
						       FT_Pointer user )
  {
    /* TODO: Merge generic_lookup_table_segment_array_finalizer */
    FT_Memory memory = user;
    FT_UShort * segment = value->extra.word;
    if ( !segment )
      return GX_Err_Ok;

    value->extra.word = NULL;
    FT_FREE(segment);
    return GX_Err_Ok;
  }

  FT_LOCAL_DEF ( void )
  gx_XStateTable_free ( GX_XStateTable state_table,
			FT_Memory memory,
			GX_XStateTable_Entry_Finalizer finalizer,
			FT_Pointer user )
  {
    GX_LookupTable_FuncsRec lookup_table_funcs = GX_LOOKUP_TABLE_FUNC_ZERO;

    lookup_table_funcs.segment_array_func = gx_XStateTable_LookupTable_segment_array_finalizer;

    if ( finalizer == NULL )
      finalizer = gx_StateTable_Entry_default_finalizer;
    gx_XStateTable_free_entry_subtable ( memory,
					 state_table->nEntries,
					 state_table->entry_subtable,
					 finalizer,
					 user );
    FT_FREE( state_table->entry_subtable );
    state_table->entry_subtable = NULL;

    FT_FREE( state_table->state_array );
    state_table->state_array = NULL;


    if ( state_table->class_subtable.format == GX_LOOKUPTABLE_SEGMENT_ARRAY )
      gx_LookupTable_traverse_low( & state_table->class_subtable,
				   &lookup_table_funcs,
				   memory );
    gx_LookupTable_free( & state_table->class_subtable, memory );
  }

  FT_LOCAL_DEF ( FT_UShort )
  gx_XStateTable_get_class ( GX_XStateTable state_table,
			     FT_UShort glyph )
  {
    GX_LookupTable class_subtable;
    GX_LookupResultRec result;
    FT_UShort class_code;

    class_subtable = &state_table->class_subtable;
    result 	   = gx_LookupTable_lookup ( class_subtable,
					     glyph );

    if ( result.value == NULL )
      class_code = GX_CLASS_OUT_OF_BOUNDS;
    else if ( result.firstGlyph == GX_LOOKUP_RESULT_NO_FIRST_GLYPH )
      class_code = result.value->raw.u;
    else
      class_code = result.value->extra.word[glyph - result.firstGlyph];
    return class_code;
  }

  FT_LOCAL ( GX_EntrySubtable )
  gx_XStateTable_get_entry_subtable ( GX_XStateTable state_table,
				     FT_UShort current_state,
				     FT_UShort class_code )
 {
    GX_XStateHeader header  = &state_table->header;
    FT_UShort * state_array = state_table->state_array;
    FT_UShort * state_array_for_current_state;
    FT_UShort entry_index;
    GX_EntrySubtable entry_subtable;

    FT_ASSERT( current_state < state_table->nStates );
    FT_ASSERT( class_code < header->nClasses );
    state_array_for_current_state = &state_array[current_state * header->nClasses];
    entry_index = state_array_for_current_state[class_code];
    FT_ASSERT( entry_index < state_table->nEntries );
    entry_subtable = &state_table->entry_subtable[entry_index];
    return entry_subtable;
  }

/* END */
