/*******************************************************************
 *
 *  t1parse.h                                                   1.0
 *
 *    Type1 parser.
 *
 *  Copyright 1996-1998 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *
 *  The Type1 parser component is in charge of simply parsing
 *  the font input stream and convert simple tokens and elements
 *  into integers, floats, matrices, strings, etc..
 *
 *  It is used by the Type1 loader..
 *
 ******************************************************************/

#ifndef T1PARSE_H
#define T1PARSE_H

#include <ftstream.h>
#include <t1tokens.h>

#ifdef __cplusplus
  extern "C" {
#endif

/*************************************************************************
 *
 * <Enum> T1_DictState
 *
 * <Description>
 *    An enumeration used to describe the Type 1 parser's state, i.e.
 *    which dictionary (or array) it is scanning and processing at the
 *    current moment..
 *
 */
  typedef enum  T1_DictState_
  {
    dict_none = 0,
    dict_font,          /* parsing the font dictionary              */
    dict_fontinfo,      /* parsing the font info dictionary         */
    dict_none2,         /* beginning to parse the encrypted section */
    dict_private,       /* parsing the private dictionary           */
    dict_encoding,      /* parsing the encoding array               */
    dict_subrs,         /* parsing the subrs array                  */
    dict_othersubrs,    /* parsing the othersubrs array (?)         */
    dict_charstrings,   /* parsing the charstrings dictionary       */
    dict_unknown_array, /* parsing/ignoring an unknown array        */
    dict_unknown_dict,  /* parsing/ignoring an unknown dictionary   */

    dict_max    /* do not remove from list */

  } T1_DictState;


/*************************************************************************
 *
 * <Struct> T1_Table
 *
 * <Description>
 *    A T1_Table is a simple object used to store an array of objects
 *    in a single memory block.
 *
 * <Fields>
 *    block     :: address in memory of the growheap's block. This
 *                 can change between two object adds, due to the use
 *                 of 'realloc'.
 *
 *    cursor    :: current top of the grow heap within its block
 *
 *    capacity  :: current size of the heap block. Increments by 1 Kb
 *
 *    max_elems :: maximum number of elements in table
 *    num_elems :: current number of elements in table
 *
 *    elements  :: table of element addresses within the block
 *    lengths   :: table of element sizes within the block
 *
 *    system   :: system object used for memory operations (alloc/realloc)
 */

  typedef struct T1_Table_
  {
    T1_Byte*   block;          /* current memory block           */
    T1_Int     cursor;         /* current cursor in memory block */
    T1_Int     capacity;       /* current size of memory block   */

    T1_Int     max_elems;
    T1_Int     num_elems;
    T1_Byte**  elements;       /* addresses of table elements */
    T1_Int*    lengths;        /* lengths of table elements   */

    FT_Memory  memory;

  } T1_Table;




/*************************************************************************/
/*                                                                       */
/* <Struct> T1_Parser                                                    */
/*                                                                       */
/* <Description>                                                         */
/*    A Type 1 parser. This object is in charge of parsing Type 1        */
/*    ASCII streams and builds dictionaries for a T1_Face object.        */
/*                                                                       */
/* <Fields>                                                              */
/*    error ::                                                           */
/*       current error code. 0 means success                             */
/*                                                                       */
/*    face  ::                                                           */
/*       the target T1_Face object being built                           */
/*                                                                       */
/*    tokenizer ::                                                       */
/*       the tokenizer (lexical analyser) used for processing the        */
/*       input stream.                                                   */
/*                                                                       */
/*    stack ::                                                           */
/*       the current token stack. Note that we don't use intermediate    */
/*       Postscript objects here !                                       */
/*                                                                       */
/*    top ::                                                             */
/*       current top of token stack                                      */
/*                                                                       */
/*    limit ::                                                           */
/*       current upper bound of the token stack. Used for overflow       */
/*       checks..                                                        */
/*                                                                       */
/*    args ::                                                            */
/*       arguments of a given operator. used and increased by the        */
/*       Copy.... functions..                                            */
/*                                                                       */
/*    state_index ::                                                     */
/*       index of top of the dictionary state stack                      */
/*                                                                       */
/*    state_stack ::                                                     */
/*       dictionary states stack                                         */
/*                                                                       */
/*    table ::                                                           */
/*       a T1_Table object used to record various kinds of               */
/*       dictionaries or arrays (like /Encoding, /Subrs, /CharStrings)   */
/*                                                                       */
/*                                                                       */
  typedef  struct  T1_Parser_
  {
    T1_Error      error;
    T1_Face       face;

    T1_Tokenizer  tokenizer;
    T1_Bool       dump_tokens;

    T1_Token      stack[ T1_MAX_STACK_DEPTH ];
    T1_Token*     top;
    T1_Token*     limit;
    T1_Token*     args;

    T1_Int        state_index;
    T1_DictState  state_stack[ T1_MAX_DICT_DEPTH ];

	T1_Table      table;

	T1_Int        cur_name;

	T1_EncodingType  encoding_type;
    T1_Byte*         encoding_names;
    T1_Int*          encoding_lengths;
    T1_Byte**        encoding_offsets;

    T1_Byte*      subrs;
    T1_Byte*      charstrings;

  } T1_Parser;



/*************************************************************************/
/*                                                                       */
/* <Function> T1_New_Table                                               */
/*                                                                       */
/* <Description>                                                         */
/*    Initialise a T1_Table.                                             */
/*                                                                       */
/* <Input>                                                               */
/*    table  :: address of target table                                  */
/*    count  :: table size = maximum number of elements                  */
/*    system :: system object to use for all subsequent reallocations    */
/*                                                                       */
/* <Return>                                                              */
/*    Error code. 0 means success                                        */
/*                                                                       */
  LOCAL_DEF
  T1_Error  T1_New_Table( T1_Table*  table,
                          T1_Int     count,
                          FT_Memory  memory );


/*************************************************************************/
/*                                                                       */
/* <Function> T1_Add_Table                                               */
/*                                                                       */
/* <Description>                                                         */
/*    Adds an object to a T1_Table, possibly growing its memory block    */
/*                                                                       */
/* <Input>                                                               */
/*    table  :: target table                                             */
/*    index  :: index of object in table                                 */
/*    object :: address of object to copy in memory                      */
/*    length :: length in bytes of source object                         */
/*                                                                       */
/* <Return>                                                              */
/*    Error code. 0 means success. An error is returned when a           */
/*    realloc failed..                                                   */
/*                                                                       */
  LOCAL_DEF
  T1_Error  T1_Add_Table( T1_Table*  table,
                          T1_Int     index,
                          void*      object,
                          T1_Int     length );


/*************************************************************************/
/*                                                                       */
/* <Function> T1_Done_Table                                              */
/*                                                                       */
/* <Description>                                                         */
/*    Finalise a T1_Table. (realloc it to its current cursor).           */
/*                                                                       */
/* <Input>                                                               */
/*    table :: target table                                              */
/*                                                                       */
/* <Note>                                                                */
/*    This function does NOT release the heap's memory block. It is up   */
/*    to the caller to clean it, or reference it in its own structures.  */
/*                                                                       */
  LOCAL_DEF
  void  T1_Done_Table( T1_Table*  table );




  LOCAL_DEF
  T1_String*   CopyString( T1_Parser*  parser );


  LOCAL_DEF
  T1_Long      CopyInteger( T1_Parser*  parser );


  LOCAL_DEF
  T1_Bool      CopyBoolean( T1_Parser*  parser );


  LOCAL_DEF
  T1_Long      CopyFloat( T1_Parser*  parser,
                          T1_Int      scale );

  LOCAL_DEF
  void         CopyBBox( T1_Parser*  parser,
                         T1_BBox*    bbox );

  LOCAL_DEF
  void         CopyMatrix( T1_Parser*  parser,
                           T1_Matrix*  matrix );

  LOCAL_DEF
  void  CopyArray( T1_Parser*  parser,
                   T1_Byte*    num_elements,
                   T1_Short*   elements,
                   T1_Int      max_elements );

#ifdef __cplusplus
  }
#endif

#endif /* T1PARSE_H */


/* END */

