/***************************************************************************/
/*                                                                         */
/*  gxlookuptbl.h                                                          */
/*                                                                         */
/*    AAT/TrueTypeGX lookup table related types and functions              */
/*   (specification)                                                       */
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

#ifndef __GXLOOKUPTBL_H__
#define __GXLOOKUPTBL_H__ 

#include <ft2build.h>
#include FT_INTERNAL_TRUETYPE_TYPES_H
#include "gxtypes.h"

FT_BEGIN_HEADER


  typedef FT_Error 
  (* GX_LookupTable_Generic_Func) ( GX_LookupTable_Format format,
				    GX_LookupValue value,
				    FT_Pointer user );

  typedef FT_Error 
  (* GX_LookupTable_Simple_Array_Func) ( GX_LookupTable_Format format,
					 FT_UShort index,
					 GX_LookupValue value,
					 FT_Pointer user );
  typedef FT_Error
  (* GX_LookupTable_Segment_Func) ( GX_LookupTable_Format format,
				    FT_UShort lastGlyph,
				    FT_UShort firstGlyph,
				    GX_LookupValue value,
				    FT_Pointer user );
  typedef FT_Error
  (* GX_LookupTable_Single_Table_Func) ( GX_LookupTable_Format format,
					 FT_UShort glyph,
					 GX_LookupValue value,
					 FT_Pointer user );
  typedef FT_Error
  (* GX_LookupTable_Trimmed_Array_Func) ( GX_LookupTable_Format format,
					  FT_UShort index,
					  FT_UShort firstGlyph,
					  FT_UShort lastGlyph,
					  GX_LookupValue value,
					  FT_Pointer user );

#define GX_LOOKUP_TABLE_FUNC_ZERO {NULL, NULL, NULL, NULL, NULL, NULL}
  typedef struct GX_LookupTable_FuncsRec_
  {
    /* If a function suitable for the lookup format is given, use
       it. If not, use `generic_func'. If both a function suitable for
       the lookup format and `generic_func' are not given(= NULL), do
       nothing */
    GX_LookupTable_Generic_Func       generic_func;
    /* -------------------------------------------------------------- */
    GX_LookupTable_Simple_Array_Func  simple_array_func;
    GX_LookupTable_Segment_Func       segment_single_func;
    GX_LookupTable_Segment_Func       segment_array_func;
    GX_LookupTable_Single_Table_Func  single_table_func;
    GX_LookupTable_Trimmed_Array_Func trimmed_array_func;
  } GX_LookupTable_FuncsRec, *GX_LookupTable_Funcs;

  typedef struct GX_LookupResultRec_
  {
    /* `value' is NULL if a value for the glyph cannot be found. */
    GX_LookupValue value;

    /* `firstGlyph' is given only if the target lookup table 
       format is Segement array. If the format is other than
       Segement array, GX_LOOKUP_RESULT_NO_FIRST_GLYPH is set 
       to `firstGlyph'. */
#define GX_LOOKUP_RESULT_NO_FIRST_GLYPH -1
    FT_Long firstGlyph;
  } GX_LookupResultRec, *GX_LookupResult;

  typedef FT_Error
  (* GX_LookupTable_Glyph_Func) ( FT_UShort glyph,
				  GX_LookupValue value,
				  FT_Long firstGlyph,
				  FT_Pointer user );
				  
  FT_LOCAL( FT_Error )
  gx_face_load_LookupTable ( GX_Face face,
			     FT_Stream stream,
			     GX_LookupTable lookup_table );
		    
  FT_LOCAL( void )
  gx_LookupTable_free ( GX_LookupTable lookup_table,
			FT_Memory memory );

  FT_LOCAL ( FT_Error )
  gx_LookupTable_traverse_low( GX_LookupTable lookup_table,
			       GX_LookupTable_Funcs funcs,
			       FT_Pointer user);

  FT_LOCAL ( GX_LookupResultRec )
  gx_LookupTable_lookup ( GX_LookupTable lookup_table,
			  FT_UShort glyph );

  FT_LOCAL ( FT_Error )
  gx_LookupTable_traverse_high ( GX_LookupTable lookup_table,
				 GX_LookupTable_Glyph_Func func,
				 FT_Pointer user);

FT_END_HEADER

#endif /* Not def: __GXLOOKUPTBL_H__ */

/* END */
