/***************************************************************************/
/*                                                                         */
/*  gxlookuptbl.c                                                          */
/*                                                                         */
/*    AAT/TrueTypeGX lookup table related types and functions              */
/*   (body)                                                                */
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
#include FT_INTERNAL_DEBUG_H
#include "gxlookuptbl.h"
#include "gxerrors.h"

  typedef FT_Error (*gx_LookupTable_loader) ( GX_LookupTable lookup_table,
					      FT_Stream stream );
  typedef void (*gx_LookupTable_finalizer) ( FT_Memory memory,
					     GX_LookupTable lookup_table );
  typedef FT_Error (*gx_LookupTable_traverser) ( GX_LookupTable lookup_table,
						 GX_LookupTable_Funcs funcs,
						 FT_Pointer user );

  static FT_Error
  gx_load_BinSrchHeader( FT_Stream stream,
			 GX_BinSrchHeader header )
  {
    FT_Error error;
    const FT_Frame_Field fields[] =
      {
#undef FT_STRUCTURE
#define FT_STRUCTURE GX_BinSrchHeaderRec
	FT_FRAME_START( 10 ),
	    FT_FRAME_USHORT (unitSize),
	    FT_FRAME_USHORT (nUnits),
	    FT_FRAME_USHORT (searchRange),
	    FT_FRAME_USHORT (entrySelector),
	    FT_FRAME_USHORT (rangeShift),
	FT_FRAME_END
      };

    FT_STREAM_READ_FIELDS( fields, header );
    return error;
  }

  static  FT_Error
  gx_LookupTable_load_raw_values ( FT_Stream stream,
				   FT_Long value_count,
				   GX_LookupValue value_slot )
  {
    FT_Error error;
    FT_Long i;

    for ( i = 0; i < value_count; i++)
      {
	value_slot[i].extra.any = NULL;
	if ( FT_READ_SHORT(value_slot[i].raw.s) )
	  return error;
      }
    return error;
  }

  static FT_Error
  gx_LookupTable_load_segment_generic( GX_LookupTable lookup_table,
				       FT_Stream stream )

  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    GX_LookupTable_Segment segment_table = lookup_table->fsHeader.segment_generic;
    GX_BinSrchHeaderRec binSrchHeader    = segment_table->binSrchHeader;
    GX_LookupSegment segment;
    FT_Long i;

    if ( FT_NEW_ARRAY( segment, binSrchHeader.nUnits ) )
      return error;

    for ( i = 0; i < binSrchHeader.nUnits; i++ )
      {
	if ( FT_READ_USHORT(segment[i].lastGlyph)  ||
	     FT_READ_USHORT(segment[i].firstGlyph) )
	  goto Failure;

	error = gx_LookupTable_load_raw_values(stream, 1, &(segment[i].value));
	if ( error )
	  goto Failure;
      }
    segment_table->segments = segment;
    return error;
  Failure:
    FT_FREE(segment);
    return error;
  }

  static void
  gx_LookupTable_free_segment_generic( FT_Memory memory,
				       GX_LookupTable lookup_table )
  {
    GX_LookupTable_Segment segment_table = lookup_table->fsHeader.segment_generic;
    FT_FREE(segment_table->segments);
    segment_table->segments = NULL;
  }


  static FT_Error
  gx_LookupTable_load_single_table( GX_LookupTable lookup_table,
				    FT_Stream stream )
  {
    FT_Error error;
    FT_Memory memory 			     = stream->memory;
    GX_LookupTable_Single_Table single_table = lookup_table->fsHeader.single_table;
    GX_BinSrchHeaderRec binSrchHeader 	     = single_table->binSrchHeader;
    GX_LookupSingle single;
    FT_Long i;

    if ( FT_NEW_ARRAY( single, binSrchHeader.nUnits ) )
      return error;

    for ( i = 0; i < binSrchHeader.nUnits; i++ )
      {
	if ( FT_READ_USHORT(single[i].glyph) )
	  goto Failure;

	gx_LookupTable_load_raw_values(stream, 1, &(single[i].value));
	if ( error )
	  goto Failure;
      }
    single_table->entries = single;

    return error;
  Failure:
    FT_FREE(single);
    single_table->entries = NULL;
    return error;
  }

  static void
  gx_LookupTable_free_single_table( FT_Memory memory,
				    GX_LookupTable lookup_table )
  {
    GX_LookupTable_Single_Table single_table = lookup_table->fsHeader.single_table;

    FT_FREE(single_table->entries);
    single_table->entries = NULL;
  }

  static FT_Error
  gx_LookupTable_load_binSrch( GX_LookupTable lookup_table,
			       FT_Stream stream )
  {
    FT_Error error = GX_Err_Ok;
    FT_Memory memory = stream->memory;
    GX_LookupTable_BinSrch binSrch;
    gx_LookupTable_loader lookuptable_loader;

    switch (lookup_table->format)
      {
      case GX_LOOKUPTABLE_SEGMENT_SINGLE:
      case GX_LOOKUPTABLE_SEGMENT_ARRAY:
	lookuptable_loader = gx_LookupTable_load_segment_generic;
	break;
      case GX_LOOKUPTABLE_SINGLE_TABLE:
	lookuptable_loader = gx_LookupTable_load_single_table;
	break;
      default:
	return GX_Err_Invalid_Table;
      }

    if ( FT_MEM_NEW (binSrch) )
      return error;

    binSrch->dummy = NULL;
    error = gx_load_BinSrchHeader( stream, &(binSrch->binSrchHeader) );
    if ( error )
      goto Failure;

    lookup_table->fsHeader.bin_srch = binSrch;
    error = lookuptable_loader ( lookup_table, stream );
    if ( error )
      {
	lookup_table->fsHeader.bin_srch = NULL;
	goto Failure;
      }

    return error;

  Failure:
    FT_FREE ( binSrch );
    return error;
  }

  static void
  gx_LookupTable_free_binSrch ( FT_Memory memory,
				GX_LookupTable lookup_table )
  {
    GX_LookupTable_BinSrch binSrch = lookup_table->fsHeader.bin_srch;
    gx_LookupTable_finalizer finalizer;

    switch (lookup_table->format)
      {
      case GX_LOOKUPTABLE_SEGMENT_SINGLE:
      case GX_LOOKUPTABLE_SEGMENT_ARRAY:
	finalizer = gx_LookupTable_free_segment_generic;
	break;
      case GX_LOOKUPTABLE_SINGLE_TABLE:
	finalizer = gx_LookupTable_free_single_table;
	break;
      default:
	finalizer = NULL;
      }
    FT_ASSERT(finalizer);

    finalizer ( memory, lookup_table );
    binSrch->dummy 	   = NULL;
    FT_FREE ( lookup_table->fsHeader.bin_srch );
    lookup_table->fsHeader.bin_srch = NULL;
  }

  static FT_Error
  gx_LookupTable_load_simple_array( GX_LookupTable lookup_table,
				    FT_Stream stream )

  {
    FT_Error error;
    FT_Memory memory   = stream->memory;
    GX_LookupValue value_slot;

    if ( FT_NEW_ARRAY (value_slot, lookup_table->num_glyphs) )
      return error;
    error =  gx_LookupTable_load_raw_values ( stream,
					      lookup_table->num_glyphs,
					      value_slot );
    if ( error )
      goto Failure;

    lookup_table->fsHeader.simple_array = value_slot;
    return error;

  Failure:
    FT_FREE( value_slot );
    return error;
  }

  static void
  gx_LookupTable_free_simple_array( FT_Memory memory,
				    GX_LookupTable lookup_table )
  {
    FT_FREE ( lookup_table->fsHeader.simple_array );
    lookup_table->fsHeader.simple_array = NULL;
  }

  static FT_Error
  gx_LookupTable_load_trimmed_array( GX_LookupTable lookup_table,
				     FT_Stream stream )
  {
    FT_Error error;
    FT_Memory memory = stream->memory;
    FT_Short firstGlyph, glyphCount;
    GX_LookupTable_Trimmed_Array trimmed_array;
    GX_LookupValue value_slot;

    if ( FT_READ_USHORT(firstGlyph) ||
         FT_READ_USHORT(glyphCount) )
      return error;

    if ( FT_ALLOC (trimmed_array,
		   sizeof (*trimmed_array) + sizeof(*trimmed_array->valueArray)  * glyphCount) )
      return error;
    trimmed_array->firstGlyph = firstGlyph;
    trimmed_array->glyphCount = glyphCount;
    trimmed_array->valueArray = NULL;
    value_slot =(GX_LookupValue)(((char *)trimmed_array) + sizeof (*trimmed_array));
    error = gx_LookupTable_load_raw_values ( stream, glyphCount, value_slot );
    if ( error )
      goto Failure;

    lookup_table->fsHeader.trimmed_array = trimmed_array;
    trimmed_array->valueArray = value_slot;
    return error;

  Failure:
    FT_FREE( trimmed_array );
    lookup_table->fsHeader.trimmed_array = NULL;
    return error;
  }

  static void
  gx_LookupTable_free_trimmed_array( FT_Memory memory,
				     GX_LookupTable lookup_table )
  {
    GX_LookupTable_Trimmed_Array trimmed_array;

    trimmed_array = lookup_table->fsHeader.trimmed_array;
    trimmed_array->valueArray = NULL;
    FT_FREE(trimmed_array);
    lookup_table->fsHeader.trimmed_array = NULL;
  }


  FT_LOCAL_DEF( FT_Error )
  gx_face_load_LookupTable ( GX_Face face,
			     FT_Stream stream,
			     GX_LookupTable lookup_table )
  {
    FT_Error error;
    gx_LookupTable_loader loader;

    lookup_table->position   = FT_STREAM_POS();
    lookup_table->num_glyphs = face->root.num_glyphs;
    lookup_table->fsHeader.any = NULL;
    if ( FT_READ_SHORT(lookup_table->format) )
      return error;

    switch ( lookup_table->format )
      {
      case GX_LOOKUPTABLE_SIMPLE_ARRAY:
	loader = gx_LookupTable_load_simple_array;
	break;
      case GX_LOOKUPTABLE_TRIMMED_ARRAY:
	loader = gx_LookupTable_load_trimmed_array;
	break;
      default:
	loader = gx_LookupTable_load_binSrch;
      }

    error = (*loader)( lookup_table, stream );
    return error;
  }

  FT_LOCAL_DEF ( void )
  gx_LookupTable_free ( GX_LookupTable lookup_table,
			FT_Memory memory )
  {
    gx_LookupTable_finalizer finalizer;

    switch ( lookup_table->format )
      {
     case GX_LOOKUPTABLE_SIMPLE_ARRAY:
       finalizer = gx_LookupTable_free_simple_array;
       break;
     case GX_LOOKUPTABLE_TRIMMED_ARRAY:
       finalizer = gx_LookupTable_free_trimmed_array;
       break;
     default:
       finalizer = gx_LookupTable_free_binSrch;
     }
    finalizer ( memory, lookup_table );
  }


  static FT_Error
  gx_LookupTable_traverse_simple_array( GX_LookupTable lookup_table,
					GX_LookupTable_Funcs funcs,
					FT_Pointer user )
  {
    FT_Long i;
    FT_Error error = GX_Err_Ok;

    if ( funcs->simple_array_func )
      {
	for ( i = 0; i < lookup_table->num_glyphs; i++ )
	  {
	    error = (* funcs->simple_array_func)(lookup_table->format,
						 i,
						 &((lookup_table->fsHeader.simple_array)[i]),
						 user);
	    if ( error )
	      return error;
	  }
      }
    else if ( funcs->generic_func )
      {
        for ( i = 0; i < lookup_table->num_glyphs; i++ )
	  {
	    error = (* funcs->generic_func)(lookup_table->format,
					    &((lookup_table->fsHeader.simple_array)[i]),
					    user);
	    if ( error )
	      return error;
	  }
      }
    else
      error = GX_Err_Ok;
    return error;
  }

  static FT_Error
  gx_LookupTable_traverse_segment_generic( GX_LookupTable lookup_table,
					   GX_LookupTable_Funcs funcs,
					   FT_Pointer user )
  {
    FT_Error error = GX_Err_Ok;
    GX_LookupTable_Segment segment_table = lookup_table->fsHeader.segment_generic;
    GX_BinSrchHeader header = &(segment_table->binSrchHeader);
    GX_LookupSegment segment;
    GX_LookupTable_Segment_Func segment_func;
    FT_Long i;

    if ( lookup_table->format == GX_LOOKUPTABLE_SEGMENT_SINGLE )
      segment_func = funcs->segment_single_func;
    else if ( lookup_table->format == GX_LOOKUPTABLE_SEGMENT_ARRAY)
      segment_func = funcs->segment_array_func;
    else
      segment_func = NULL;

    if ( segment_func )
      {
	for ( i = 0; i < header->nUnits; i++ )
	  {
	    segment = &(segment_table->segments[i]);
	    error 	= (*segment_func)( lookup_table->format,
					   segment->lastGlyph,
					   segment->firstGlyph,
					   &(segment->value),
					   user );
	    if ( error )
	      return error;
	  }
      }
    else if ( funcs->generic_func )
      {
	for ( i = 0; i < header->nUnits; i++ )
	  {
	    segment = &(segment_table->segments[i]);
	    error 	= (*funcs->generic_func)( lookup_table->format,
						  &(segment->value),
						  user );
	    if ( error )
	      return error;
	  }
      }
    else
      error = GX_Err_Ok;
    return error;
  }

  static FT_Error
  gx_LookupTable_traverse_single_table ( GX_LookupTable lookup_table,
					 GX_LookupTable_Funcs funcs,
					 FT_Pointer user )
  {
    FT_Error error = GX_Err_Ok;
    GX_LookupTable_Single_Table single_table;
    GX_LookupSingle entries;
    GX_BinSrchHeader binSrchHeader;
    FT_Long i;

    single_table  = lookup_table->fsHeader.single_table;
    entries 	  = single_table->entries;
    binSrchHeader = &(single_table->binSrchHeader);

    if ( funcs->single_table_func )
      {
	for ( i = 0; i < binSrchHeader->nUnits; i++ )
	  {
	    error = (*funcs->single_table_func) ( lookup_table->format,
						  entries[i].glyph,
						  &(entries[i].value),
						  user );
	      if ( error )
		return error;
	  }
      }
    else if ( funcs->generic_func )
      {
	for ( i = 0; i < binSrchHeader->nUnits; i++ )
	  {
	    error = (* funcs->generic_func) ( lookup_table->format,
					      &(entries[i].value),
					      user );
	    if ( error )
	      return error;
	  }
      }
    else
      error = GX_Err_Ok;
    return error;
  }

  static FT_Error
  gx_LookupTable_traverse_trimmed_array( GX_LookupTable lookup_table,
					 GX_LookupTable_Funcs funcs,
					 FT_Pointer user )
  {
    FT_Error error = GX_Err_Ok;
    FT_Long i;
    GX_LookupTable_Trimmed_Array trimmed_array = lookup_table->fsHeader.trimmed_array;

    if ( funcs->trimmed_array_func )
      {
	for ( i = 0; i < trimmed_array->glyphCount; i++ )
	  {
	    error = (* funcs->trimmed_array_func)(lookup_table->format,
						  i,
						  trimmed_array->firstGlyph,
						  trimmed_array->glyphCount,
						  &(trimmed_array->valueArray[i]),
						  user);
	    if ( error )
	      return error;
	  }
      }
    else if ( funcs->generic_func )
      {
	for ( i = 0; i < trimmed_array->glyphCount; i++ )
	  {
	    error = (* funcs->generic_func)(lookup_table->format,
					    &(trimmed_array->valueArray[i]),
					    user);
	    if ( error )
	      return error;
	  }
      }
    else
      error = GX_Err_Ok;
    return error;
  }

  FT_LOCAL_DEF ( FT_Error )
  gx_LookupTable_traverse_low( GX_LookupTable lookup_table,
			   GX_LookupTable_Funcs funcs,
			   FT_Pointer user )
  {
    gx_LookupTable_traverser traverser;

    switch (lookup_table->format)
      {
      case GX_LOOKUPTABLE_SIMPLE_ARRAY:
	traverser = gx_LookupTable_traverse_simple_array;
	break;
      case GX_LOOKUPTABLE_SEGMENT_SINGLE:
      case GX_LOOKUPTABLE_SEGMENT_ARRAY:
	traverser = gx_LookupTable_traverse_segment_generic;
	break;
      case GX_LOOKUPTABLE_SINGLE_TABLE:
	traverser = gx_LookupTable_traverse_single_table;
	break;
      case GX_LOOKUPTABLE_TRIMMED_ARRAY:
	traverser = gx_LookupTable_traverse_trimmed_array;
	break;
      default:
	traverser = NULL;
      }
    FT_ASSERT(traverser);
    return (*traverser) ( lookup_table, funcs, user );
  }

  static int
  lookup_lookup_segment (const void *keyval, const void *datum)
  {
    const GX_LookupSegment segment = (const GX_LookupSegment)datum;
    FT_UShort glyph 	     = *(FT_UShort*)keyval;

    if ( glyph < segment->firstGlyph )
      return -1;
    else if ( segment->lastGlyph < glyph )
      return 1;
    else
      return 0;
  }


  static int
  lookup_lookup_single(const void *keyval, const void *datum)
  {
    const GX_LookupSingle entry = (const GX_LookupSingle)datum;
    FT_UShort glyph 	  = *(FT_UShort*)keyval;
    if ( glyph < entry->glyph )
      return -1;
    else if ( entry->glyph < glyph )
      return 1;
    else
      return 0;
  }

  FT_LOCAL_DEF( GX_LookupResultRec )
  gx_LookupTable_lookup ( GX_LookupTable lookup_table,
			  FT_UShort glyph )
  {
    GX_LookupResultRec result;

    GX_LookupTable_Segment segment_table;
    GX_LookupSegment segment;

    GX_LookupTable_Single_Table single_table;
    GX_LookupSingle entry;

    GX_LookupTable_Trimmed_Array trimmed_array;
    FT_Long trimmed_index;

    void * bs_key = &glyph;
    void * bs_base;
    size_t bs_n;
    size_t bs_size;

    int (* bs_cmp)(const void* keyval, const void* datum);

    result.firstGlyph = GX_LOOKUP_RESULT_NO_FIRST_GLYPH;
    result.value      = NULL;
    switch ( lookup_table->format )
      {
      case GX_LOOKUPTABLE_SIMPLE_ARRAY:
	if ( glyph < lookup_table->num_glyphs )
	  result.value = &((lookup_table->fsHeader.simple_array) [glyph]);
	break;
      case GX_LOOKUPTABLE_SEGMENT_SINGLE:
      case GX_LOOKUPTABLE_SEGMENT_ARRAY:
	segment_table 	  = lookup_table->fsHeader.segment_generic;
	bs_base       	  = segment_table->segments;
	bs_n          	  = segment_table->binSrchHeader.nUnits;
	bs_size       	  = sizeof (*segment_table->segments);
	bs_cmp        	  = lookup_lookup_segment;
	segment       	  = ft_bsearch( bs_key, bs_base, bs_n, bs_size, bs_cmp );
	if ( segment )
	  {
	    result.value = &segment->value;
	    if ( ( lookup_table->format == GX_LOOKUPTABLE_SEGMENT_ARRAY ) )
	      result.firstGlyph = segment->firstGlyph;
	  }
	return result;
      case GX_LOOKUPTABLE_SINGLE_TABLE:
	single_table = lookup_table->fsHeader.single_table;
	bs_base      = single_table->entries;
	bs_n 	     = single_table->binSrchHeader.nUnits;
	bs_size      = sizeof (*single_table->entries);
	bs_cmp 	     = lookup_lookup_single;
	entry 	     = ft_bsearch( bs_key, bs_base, bs_n, bs_size, bs_cmp );
	if ( entry )
	  result.value = &(entry->value);
	break;
      case GX_LOOKUPTABLE_TRIMMED_ARRAY:
	trimmed_array = lookup_table->fsHeader.trimmed_array;
	if ( glyph < trimmed_array->firstGlyph )
	  break;

	trimmed_index = glyph - trimmed_array->firstGlyph;
	if ( trimmed_index < trimmed_array->glyphCount )
	  result.value = &(trimmed_array->valueArray[trimmed_index]);
	break;
      }
    return result;
  }

  FT_LOCAL ( FT_Error )
  gx_LookupTable_traverse_high ( GX_LookupTable lookup_table,
				 GX_LookupTable_Glyph_Func func,
				 FT_Pointer user)
  {
    FT_Error error;
    FT_UShort glyph;
    GX_LookupResultRec result;

    for ( glyph = 0; glyph < 0xFFFF; glyph++ )
      {
	result = gx_LookupTable_lookup ( lookup_table, glyph );
	if ( result.value == NULL)
	  continue ;

	if (( error = func( glyph, result.value, result.firstGlyph, user ) ))
	  return error;
      }
    return FT_Err_Ok;
  }


/* END */
