/*******************************************************************
 *
 *  t1parse.h                                                   2.0
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
 *  The Type 1 parser is in charge of the following:
 *
 *   - provide an implementation of a growing sequence of
 *     objects called a T1_Table (used to build various tables
 *     needed by the loader).
 *
 *   - opening .pfb and .pfa files to extract their top-level
 *     and private dictionaries
 *
 *   - read numbers, arrays & strings from any dictionary
 *
 *  See "t1load.c" to see how data is loaded from the font file
 *
 ******************************************************************/

#ifndef T1PARSE_H
#define T1PARSE_H

#include <t1types.h>

#ifdef __cplusplus
  extern "C" {
#endif

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
 *    init      :: boolean. set when the table has been initialized
 *                 (the table user should set this field)
 *
 *    max_elems :: maximum number of elements in table
 *    num_elems :: current number of elements in table
 *
 *    elements  :: table of element addresses within the block
 *    lengths   :: table of element sizes within the block
 *
 *    memory    :: memory object used for memory operations (alloc/realloc)
 */

  typedef struct T1_Table_
  {
    T1_Byte*   block;          /* current memory block           */
    T1_Int     cursor;         /* current cursor in memory block */
    T1_Int     capacity;       /* current size of memory block   */
    T1_Long    init;

    T1_Int     max_elems;
    T1_Int     num_elems;
    T1_Byte**  elements;       /* addresses of table elements */
    T1_Int*    lengths;        /* lengths of table elements   */

    FT_Memory  memory;

  } T1_Table;


/*************************************************************************
 *
 * <Struct> T1_Parser
 *
 * <Description>
 *    A T1_Parser is an object used to parse a Type 1 fonts very
 *    quickly.
 *
 * <Fields>
 *    stream        :: current input stream
 *    memory        :: current memory object
 *
 *    base_dict     :: pointer to top-level dictionary
 *    base_len      :: length in bytes of top dict
 *
 *    private_dict  :: pointer to private dictionary
 *    private_len   :: length in bytes of private dict
 *
 *    in_pfb        :: boolean. Indicates that we're in a .pfb file
 *    in_memory     :: boolean. Indicates a memory-based stream
 *    single_block  :: boolean. Indicates that the private dict
 *                     is stored in lieu of the base dict
 *
 *    cursor        :: current parser cursor
 *    limit         :: current parser limit (first byte after current
 *                     dictionary).
 *
 *    error         :: current parsing error
 */
  typedef struct T1_Parser_
  {
    FT_Stream  stream;
    FT_Memory  memory;
    
    T1_Byte*   base_dict;
    T1_Int     base_len;
    
    T1_Byte*   private_dict;
    T1_Int     private_len;
    
    T1_Byte    in_pfb;
    T1_Byte    in_memory;
    T1_Byte    single_block;

    T1_Byte*   cursor;
    T1_Byte*   limit;
    T1_Error   error;
    
  } T1_Parser;


  LOCAL_DEF
  T1_Error  T1_New_Table( T1_Table*  table,
                          T1_Int     count,
                          FT_Memory  memory );


  LOCAL_DEF
  T1_Error  T1_Add_Table( T1_Table*  table,
                          T1_Int     index,
                          void*      object,
                          T1_Int     length );

#if 0
  LOCAL_DEF
  void  T1_Done_Table( T1_Table*  table );
#endif

  LOCAL_DEF
  void  T1_Release_Table( T1_Table*  table );

  LOCAL_DEF
  T1_Long  T1_ToInt  ( T1_Parser*  parser );

  LOCAL_DEF
  T1_Long  T1_ToFixed( T1_Parser*  parser, T1_Int  power_ten );

  LOCAL_DEF
  T1_Int  T1_ToCoordArray( T1_Parser* parser,
                           T1_Int     max_coords,
                           T1_Short*  coords );

  LOCAL_DEF
  T1_Int  T1_ToFixedArray( T1_Parser* parser,
                           T1_Int     max_values,
                           T1_Fixed*  values,
                           T1_Int     power_ten );

  LOCAL_DEF
  T1_String*  T1_ToString( T1_Parser* parser );


  LOCAL_DEF
  T1_Bool   T1_ToBool( T1_Parser* parser );

#if 0
  LOCAL_DEF
  T1_Int  T1_ToImmediate( T1_Parser*  parser );
#endif

  LOCAL_DEF
  T1_Error  T1_New_Parser( T1_Parser*  parser,
                           FT_Stream   stream,
                           FT_Memory   memory );

  LOCAL_DEF
  T1_Error  T1_Get_Private_Dict( T1_Parser*  parser );

  LOCAL_DEF
  void  T1_Decrypt( T1_Byte*   buffer,
                    T1_Int     length,
                    T1_UShort  seed );

  LOCAL_DEF
  void  T1_Done_Parser( T1_Parser*  parser );

#ifdef __cplusplus
  }
#endif

#endif /* T1PARSE_H */


/* END */

