/***************************************************************************/
/*                                                                         */
/*  gxstatetbl.h                                                           */
/*                                                                         */
/*    AAT/TrueTypeGX state table related types and functions               */
/*    (specification).                                                     */
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


#ifndef __GXSTATETBL_H_
#define __GXSTATETBL_H_ 

#include <ft2build.h>
#include FT_TYPES_H
#include "gxtypes.h"

FT_BEGIN_HEADER
  
/* - GX_StateTable_Entry_Loader
   Fill entry_subtable->glyphOffsets and 
   update stream to read next entry_subtable. */
  typedef FT_Error 
  (* GX_StateTable_Entry_Loader) ( GX_Face face,
				   FT_Stream stream,
				   GX_EntrySubtable entry_subtable,
				   FT_Pointer user );
  typedef void
  (* GX_StateTable_Entry_Finalizer) ( FT_Memory memory,
				      GX_EntrySubtable entry_subtable,
				      FT_Pointer user );

  typedef FT_Error
  (* GX_StateTable_Entry_Action) ( GX_EntrySubtable entry_subtable,
				   FT_Pointer user );

#define  GX_STATE_TABLE_ENTRY_LOAD_FUNCS_ZERO {NULL, NULL}
  typedef struct GX_StateTable_Entry_Load_FuncsRec_
  {
    GX_StateTable_Entry_Loader loader;
    GX_StateTable_Entry_Finalizer finalizer;
  } GX_StateTable_Entry_Load_FuncsRec, *GX_StateTable_Entry_Load_Funcs;

  FT_LOCAL ( FT_Error )
  gx_face_load_StateTable ( GX_Face face,
			    FT_Stream stream,
			    GX_StateTable state_table,
			    GX_StateTable_Entry_Load_Funcs funcs,
			    FT_Pointer user );

  FT_LOCAL ( void )
  gx_StateTable_free ( GX_StateTable state_table,
		       FT_Memory memory,
		       GX_StateTable_Entry_Finalizer finalizer,
		       FT_Pointer user );

/* If action returns value other than FT_Err_Ok, the traverse stops
   at that point and returns. */
#if 0
  FT_LOCAL ( FT_Error )
  gx_StateTable_traverse_entries( GX_StateTable state_table,
				  GX_StateTable_Entry_Action action,
				  FT_Pointer user );
#endif /* 0 */


  FT_LOCAL ( FT_Byte )
  gx_StateTable_get_class ( GX_StateTable state_table,
			    FT_UShort glyph );

  FT_LOCAL ( GX_EntrySubtable )
  gx_StateTable_get_entry_subtable ( GX_StateTable state_table,
				     FT_UShort current_state,
				     FT_Byte class_code );

#define  GX_XSTATE_TABLE_ENTRY_LOAD_FUNCS_ZERO {NULL, NULL}
  typedef GX_StateTable_Entry_Load_FuncsRec GX_XStateTable_Entry_Load_FuncsRec;
  typedef GX_StateTable_Entry_Load_Funcs GX_XStateTable_Entry_Load_Funcs;
  typedef GX_StateTable_Entry_Loader GX_XStateTable_Entry_Loader;
  typedef GX_StateTable_Entry_Finalizer GX_XStateTable_Entry_Finalizer;
  typedef GX_StateTable_Entry_Action GX_XStateTable_Entry_Action;

  FT_LOCAL ( FT_Error )
  gx_face_load_XStateTable ( GX_Face face,
			     FT_Stream stream,
			     GX_XStateTable state_table,
			     GX_XStateTable_Entry_Load_Funcs funcs,
			     FT_Pointer user );

  FT_LOCAL ( FT_Error )
  gx_XStateTable_traverse_entries( GX_XStateTable state_table,
				   GX_XStateTable_Entry_Action action,
				   FT_Pointer user );
				       
  FT_LOCAL ( void )
  gx_XStateTable_free ( GX_XStateTable state_table,
			FT_Memory memory,
			GX_XStateTable_Entry_Finalizer finalizer,
			FT_Pointer user );

  FT_LOCAL ( FT_UShort )
  gx_XStateTable_get_class ( GX_XStateTable state_table,
			     FT_UShort glyph );

  FT_LOCAL ( GX_EntrySubtable )
  gx_XStateTable_get_entry_subtable ( GX_XStateTable state_table,
				     FT_UShort current_state,
				     FT_UShort class_code );

FT_END_HEADER

#endif /* Not def: __GXSTATETBL_H_ */

/* END */
