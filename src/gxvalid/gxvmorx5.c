/***************************************************************************/
/*                                                                         */
/*  gxvmorx5.c                                                             */
/*                                                                         */
/*    TrueTypeGX/AAT morx table validation                                 */
/*    body for type5 (Contextual Glyph Insertion) subtable.                */
/*                                                                         */
/*  Copyright 2005 by suzuki toshiya, Masatake YAMATO, Red Hat K.K.,       */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* gxvalid is derived from both gxlayout module and otvalid module.        */
/* Development of gxlayout was support of Information-technology Promotion */
/* Agency(IPA), Japan.                                                     */
/***************************************************************************/

#include "gxvmorx.h"

  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxvmorx


/*
 * morx subtable type5 (Contextual Glyph Insertion)
 * has format of StateTable with insertion-glyph-list
 * without name. however, 32bit offset from the head
 * of subtable to the i-g-l is given after "entryTable",
 * without variable name specification (the exist of
 * offset to the table is different from mort type5).
 */


  typedef struct  GXV_morx_subtable_type5_StateOptRec_
  {
    FT_ULong  insertionGlyphList;
    FT_ULong  insertionGlyphList_length;

  }  GXV_morx_subtable_type5_StateOptRec,
    *GXV_morx_subtable_type5_StateOptRecData;

#define  GXV_MORX_SUBTABLE_TYPE5_HEADER_SIZE ( GXV_STATETABLE_HEADER_SIZE + 4 )

  static void
  gxv_morx_subtable_type5_insertionGlyphList_load( FT_Bytes       table,
                                                   FT_Bytes       limit,
                                                   GXV_Validator  valid )
  {
    FT_Bytes  p = table;
    GXV_morx_subtable_type5_StateOptRecData  optdata = valid->xstatetable.optdata;


    GXV_LIMIT_CHECK( 4 );
    optdata->insertionGlyphList = FT_NEXT_ULONG( p );
  }


  static void
  gxv_morx_subtable_type5_subtable_setup( FT_ULong       table_size,
                                          FT_ULong       classTable,
                                          FT_ULong       stateArray,
                                          FT_ULong       entryTable,
                                          FT_ULong*      classTable_length_p,
                                          FT_ULong*      stateArray_length_p,
                                          FT_ULong*      entryTable_length_p,
                                          GXV_Validator  valid )
  {
    FT_ULong   o[4];
    FT_ULong*  l[4];
    FT_ULong   buff[5];
    GXV_morx_subtable_type5_StateOptRecData  optdata = valid->xstatetable.optdata;


    o[0] = classTable;
    o[1] = stateArray;
    o[2] = entryTable;
    o[3] = optdata->insertionGlyphList;
    l[0] = classTable_length_p;
    l[1] = stateArray_length_p;
    l[2] = entryTable_length_p;
    l[3] = &(optdata->insertionGlyphList_length);

    gxv_set_length_by_ulong_offset( o, l, buff, 4, table_size, valid );
  }


  static void
  gxv_morx_subtable_type5_InsertList_validate( FT_UShort      index,
                                               FT_UShort      count,
                                               FT_Bytes       table,
                                               FT_Bytes       limit,
                                               GXV_Validator  valid )
  {
    FT_Bytes p = table + ( index * 2 );

    while ( p < table + ( count * 2 ) + ( index * 2 ) )
    {
      FT_UShort insert_glyphID;


      GXV_LIMIT_CHECK( 2 );
      insert_glyphID = FT_NEXT_USHORT( p );
      GXV_TRACE(( " 0x%04x", insert_glyphID ));
    }

    GXV_TRACE(( "\n" ));
  }


  static void
  gxv_morx_subtable_type5_entry_validate( FT_UShort      state,
                                          FT_UShort      flags,
                                          GXV_StateTable_GlyphOffsetDesc
                                                         glyphOffset,
                                          FT_Bytes       table,
                                          FT_Bytes       limit,
                                          GXV_Validator  valid )
  {
    FT_Bool    setMark;
    FT_Bool    dontAdvance;
    FT_Bool    currentIsKashidaLike;
    FT_Bool    markedIsKashidaLike;
    FT_Bool    currentInsertBefore;
    FT_Bool    markedInsertBefore;
    FT_Bool    currentInsertCount;
    FT_Byte    markedInsertCount;
    FT_Byte    currentInsertList;
    FT_UShort  markedInsertList;


    setMark              = ( flags >> 15 ) & 1;
    dontAdvance          = ( flags >> 14 ) & 1;
    currentIsKashidaLike = ( flags >> 13 ) & 1;
    markedIsKashidaLike  = ( flags >> 12 ) & 1;
    currentInsertBefore  = ( flags >> 11 ) & 1;
    markedInsertBefore   = ( flags >> 10 ) & 1;
    currentInsertCount   = ( flags & 0x03E0 ) / 0x20;
    markedInsertCount    = ( flags & 0x001F );
    currentInsertList    = glyphOffset.ul / 0x00010000;
    markedInsertList     = glyphOffset.ul & 0x0000FFFF;

    if ( currentInsertList && 0 != currentInsertCount )
    {
      gxv_morx_subtable_type5_InsertList_validate( currentInsertList,
                                                   currentInsertCount,
                                                   table, limit,
                                                   valid );
    }

    if ( markedInsertList && 0 != markedInsertCount )
    {
      gxv_morx_subtable_type5_InsertList_validate( markedInsertList,
                                                   markedInsertCount,
                                                   table, limit,
                                                   valid );
    }
  }


  static void
  gxv_morx_subtable_type5_validate( FT_Bytes       table,
                                    FT_Bytes       limit,
                                    GXV_Validator  valid )
  {
    FT_Bytes  p = table;
    GXV_morx_subtable_type5_StateOptRec      et_rec;
    GXV_morx_subtable_type5_StateOptRecData  et = &et_rec;


    GXV_NAME_ENTER( "morx chain subtable type5 (Glyph Insertion)" );

    GXV_LIMIT_CHECK( GXV_MORX_SUBTABLE_TYPE5_HEADER_SIZE );

    valid->xstatetable.optdata               = et;
    valid->xstatetable.optdata_load_func     = gxv_morx_subtable_type5_insertionGlyphList_load;
    valid->xstatetable.subtable_setup_func   = gxv_morx_subtable_type5_subtable_setup;
    valid->xstatetable.entry_glyphoffset_fmt = GXV_GLYPHOFFSET_ULONG;
    valid->xstatetable.entry_validate_func   = gxv_morx_subtable_type5_entry_validate;
    gxv_XStateTable_validate( p, limit, valid );
    GXV_EXIT;
  }


/* END */
