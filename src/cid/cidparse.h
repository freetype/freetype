/*******************************************************************
 *
 *  cidparse.h                                                   2.0
 *
 *    CID-Keyed Type1 parser.
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

#ifndef CIDPARSE_H
#define CIDPARSE_H

#include <freetype/internal/t1types.h>

#ifdef __cplusplus
  extern "C" {
#endif

  

#if 0
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
    FT_Byte*   block;          /* current memory block           */
    FT_Int     cursor;         /* current cursor in memory block */
    FT_Int     capacity;       /* current size of memory block   */
    FT_Long    init;

    FT_Int     max_elems;
    FT_Int     num_elems;
    FT_Byte**  elements;       /* addresses of table elements */
    FT_Int*    lengths;        /* lengths of table elements   */

    FT_Memory  memory;

  } T1_Table;


  LOCAL_DEF
  FT_Error  T1_New_Table( T1_Table*  table,
                          FT_Int     count,
                          FT_Memory  memory );


  LOCAL_DEF
  FT_Error  T1_Add_Table( T1_Table*  table,
                          FT_Int     index,
                          void*      object,
                          FT_Int     length );

  LOCAL_DEF
  void  T1_Release_Table( T1_Table*  table );
#endif

/*************************************************************************
 *
 * <Struct> CID_Parser
 *
 * <Description>
 *    A CID_Parser is an object used to parse a Type 1 fonts very
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

  typedef struct CID_Parser_
  {
    FT_Stream  stream;
    FT_Memory  memory;

    FT_Byte*   postscript;
    FT_Int     postscript_len;

    FT_ULong   data_offset;

    FT_Byte*   cursor;
    FT_Byte*   limit;
    FT_Error   error;

    CID_Info*  cid;
    FT_Int     num_dict;

  } CID_Parser;


  LOCAL_DEF
  FT_Error  CID_New_Parser( CID_Parser*  parser,
                            FT_Stream    stream,
                            FT_Memory    memory );

  LOCAL_DEF
  void  CID_Done_Parser( CID_Parser*  parser );


 /*************************************************************************
  *
  *     PARSING ROUTINES
  *
  *************************************************************************/

  LOCAL_DEF
  FT_Long  CID_ToInt  ( CID_Parser*  parser );

  LOCAL_DEF
  FT_Int  CID_ToCoordArray( CID_Parser* parser,
                            FT_Int     max_coords,
                            FT_Short*  coords );

  LOCAL_DEF
  FT_Int  CID_ToFixedArray( CID_Parser* parser,
                            FT_Int     max_values,
                            T1_Fixed*  values,
                            FT_Int     power_ten );

  LOCAL_DEF
  void      CID_Skip_Spaces( CID_Parser*  parser );



  /* simple enumeration type used to identify token types */
  typedef enum T1_Token_Type_
  {
    t1_token_none = 0,
    t1_token_any,
    t1_token_string,
    t1_token_array,
    
    /* do not remove */
    t1_token_max
    
  } T1_Token_Type;

  /* a simple structure used to identify tokens */
  typedef struct T1_Token_Rec_
  {
    FT_Byte*       start;   /* first character of token in input stream */
    FT_Byte*       limit;   /* first character after the token          */
    T1_Token_Type  type;    /* type of token..                          */
  
  } T1_Token_Rec;  


  LOCAL_DEF
  void      CID_ToToken( CID_Parser*    parser,
                        T1_Token_Rec* token );







  /* enumeration type used to identify object fields */
  typedef enum T1_Field_Type_
  {
    t1_field_none = 0,
    t1_field_bool,
    t1_field_integer,
    t1_field_fixed,
    t1_field_string,
    t1_field_integer_array,
    t1_field_fixed_array,
    t1_field_callback,
        
    /* do not remove */
    t1_field_max
    
  } T1_Field_Type;

  typedef enum T1_Field_Location_
  {
    t1_field_cid_info,
    t1_field_font_dict,
    t1_field_font_info,
    t1_field_private,
    
    /* do not remove */
    t1_field_location_max
  
  } T1_Field_Location;


  typedef FT_Error  (*CID_Field_Parser)( CID_Face     face,
                                         CID_Parser*  parser );

  /* structure type used to model object fields */
  typedef struct T1_Field_Rec_
  {
    const char*        ident;         /* field identifier                     */
    T1_Field_Location  location;
    T1_Field_Type      type;          /* type of field                        */
    CID_Field_Parser   reader;
    T1_UInt            offset;        /* offset of field in object            */
    FT_UInt            size;          /* size of field in bytes               */
    FT_UInt            array_max;     /* maximum number of elements for array */
    FT_UInt            count_offset;  /* offset of element count for arrays   */
   
  } T1_Field_Rec;

#define T1_FIELD_REF(s,f)  (((s*)0)->f)

#define T1_NEW_SIMPLE_FIELD( _ident, _type, _fname )   \
   { _ident, T1CODE, _type,                            \
     0,                                                \
     (FT_UInt)(char*)&T1_FIELD_REF(T1TYPE,_fname),     \
     sizeof(T1_FIELD_REF(T1TYPE,_fname)),              \
     0, 0 },

#define T1_NEW_CALLBACK_FIELD( _ident, _reader )  \
   { _ident, T1CODE, t1_field_callback,           \
     _reader,                                     \
     0, 0, 0, 0 },

#define T1_NEW_TABLE_FIELD( _ident, _type, _fname, _max )   \
   { _ident, T1CODE, _type,                                 \
     0,                                                     \
     (FT_UInt)(char*)&T1_FIELD_REF(T1TYPE,_fname),          \
     sizeof(T1_FIELD_REF(T1TYPE,_fname)[0]),                \
     _max,                                                  \
     (FT_UInt)(char*)&T1_FIELD_REF(T1TYPE,num_ ## _fname) },

#define T1_NEW_TABLE_FIELD2( _ident, _type, _fname, _max )   \
   { _ident, T1CODE, _type,                                  \
     0,                                                      \
     (FT_UInt)(char*)&T1_FIELD_REF(T1TYPE,_fname),           \
     sizeof(T1_FIELD_REF(T1TYPE,_fname)[0]),                 \
     _max, 0 },


#define T1_FIELD_BOOL( _ident, _fname )       \
           T1_NEW_SIMPLE_FIELD( _ident, t1_field_bool, _fname )

#define T1_FIELD_NUM( _ident, _fname )       \
           T1_NEW_SIMPLE_FIELD( _ident, t1_field_integer, _fname )

#define T1_FIELD_FIXED( _ident, _fname )       \
           T1_NEW_SIMPLE_FIELD( _ident, t1_field_fixed, _fname )

#define T1_FIELD_STRING( _ident, _fname )       \
           T1_NEW_SIMPLE_FIELD( _ident, t1_field_string, _fname )

#define T1_FIELD_NUM_TABLE( _ident, _fname, _fmax )   \
           T1_NEW_TABLE_FIELD( _ident, t1_field_integer_array, _fname, _fmax )
           
#define T1_FIELD_FIXED_TABLE( _ident, _fname, _fmax )   \
           T1_NEW_TABLE_FIELD( _ident, t1_field_fixed_array, _fname, _fmax )

#define T1_FIELD_NUM_TABLE2( _ident, _fname, _fmax )  \
           T1_NEW_TABLE_FIELD2( _ident, t1_field_integer_array, _fname, _fmax )

#define T1_FIELD_FIXED_TABLE2( _ident, _fname, _fmax )   \
           T1_NEW_TABLE_FIELD2( _ident, t1_field_fixed_array, _fname, _fmax )

#define T1_FIELD_CALLBACK( _ident, _name )  \
           T1_NEW_CALLBACK_FIELD( _ident, parse_ ## _name )

  LOCAL_DEF
  FT_Error  CID_Load_Field( CID_Parser*           parser,
                            const T1_Field_Rec*  field,
                            void*                object );

  LOCAL_DEF
  FT_Error  CID_Load_Field_Table( CID_Parser*           parser,
                                  const T1_Field_Rec*  field,
                                  void*                object );




#ifdef __cplusplus
  }
#endif

#endif /* CIDPARSE_H */


/* END */

