/***************************************************************************/
/*                                                                         */
/*  psaux.h                                                                */
/*                                                                         */
/*    Auxiliary functions and data structures related to PostScript fonts  */
/*    (specification).                                                     */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef PSAUX_H
#define PSAUX_H


#include <freetype/internal/ftobjs.h>


#ifdef __cplusplus
  extern "C" {
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                             T1_TABLE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    T1_Table                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A T1_Table is a simple object used to store an array of objects in */
  /*    a single memory block.                                             */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    block     :: The address in memory of the growheap's block.  This  */
  /*                 can change between two object adds, due to            */
  /*                 reallocation.                                         */
  /*                                                                       */
  /*    cursor    :: The current top of the grow heap within its block.    */
  /*                                                                       */
  /*    capacity  :: The current size of the heap block.  Increments by    */
  /*                 1kByte chunks.                                        */
  /*                                                                       */
  /*    max_elems :: The maximum number of elements in table.              */
  /*                                                                       */
  /*    num_elems :: The current number of elements in table.              */
  /*                                                                       */
  /*    elements  :: A table of element addresses within the block.        */
  /*                                                                       */
  /*    lengths   :: A table of element sizes within the block.            */
  /*                                                                       */
  /*    memory    :: The object used for memory operations                 */
  /*                 (alloc/realloc).                                      */
  /*                                                                       */
  typedef struct  T1_Table_
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



  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    T1_Table_Funcs                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A set of function pointers used to manage T1_Table objects..       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    table_init    :: used to initialise a a table                      */
  /*    table_done    :: finalize/destroy a given table                    */
  /*    table_add     :: add one new object to a table                     */
  /*    table_release :: release table data, then finalize it              */
  /*                                                                       */
  typedef  struct T1_Table_Funcs_
  {
    FT_Error   (*init)   ( T1_Table*   table,
                           FT_Int     count,
                           FT_Memory  memory );
  
    void       (*done)   ( T1_Table*  table );                              
  
    FT_Error   (*add)    ( T1_Table*   table,
                           FT_Int      index,
                           void*       object,
                           FT_Int      length );

    void       (*release)( T1_Table*  table );                              
  
  } T1_Table_Funcs;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       T1 FIELDS & TOKENS                      *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* simple enumeration type used to identify token types */
  typedef enum  T1_Token_Type_
  {
    t1_token_none = 0,
    t1_token_any,
    t1_token_string,
    t1_token_array,

    /* do not remove */
    t1_token_max

  } T1_Token_Type;


  /* a simple structure used to identify tokens */
  typedef struct  T1_Token_
  {
    FT_Byte*       start;   /* first character of token in input stream */
    FT_Byte*       limit;   /* first character after the token          */
    T1_Token_Type  type;    /* type of token                            */

  } T1_Token;


  /* enumeration type used to identify object fields */
  typedef enum  T1_Field_Type_
  {
    t1_field_none = 0,
    t1_field_bool,
    t1_field_integer,
    t1_field_fixed,
    t1_field_string,
    t1_field_integer_array,
    t1_field_fixed_array,

    /* do not remove */
    t1_field_max

  } T1_Field_Type;


  /* structure type used to model object fields */
  typedef struct  T1_Field_
  {
    T1_Field_Type  type;          /* type of field                        */
    FT_UInt        offset;        /* offset of field in object            */
    FT_Byte        size;          /* size of field in bytes               */
    FT_UInt        array_max;     /* maximum number of elements for array */
    FT_UInt        count_offset;  /* offset of element count for arrays   */
    FT_Int         flag_bit;      /* bit number for field flag            */

  } T1_Field;


#define T1_FIELD_BOOL( _fname )        \
          {                            \
            t1_field_bool,             \
            FT_FIELD_OFFSET( _fname ), \
            FT_FIELD_SIZE( _fname ),   \
            0, 0, 0                    \
          }

#define T1_FIELD_NUM( _fname )         \
          {                            \
            t1_field_integer,          \
            FT_FIELD_OFFSET( _fname ), \
            FT_FIELD_SIZE( _fname ),   \
            0, 0, 0                    \
          }

#define T1_FIELD_FIXED( _fname, _power ) \
          {                              \
            t1_field_fixed,              \
            FT_FIELD_OFFSET( _fname ),   \
            FT_FIELD_SIZE( _fname ),     \
            0, 0, 0                      \
          }

#define T1_FIELD_STRING( _fname )      \
          {                            \
            t1_field_string,           \
            FT_FIELD_OFFSET( _fname ), \
            FT_FIELD_SIZE( _fname ),   \
            0, 0, 0                    \
          }

#define T1_FIELD_NUM_ARRAY( _fname, _fcount, _fmax )  \
          {                                           \
            t1_field_integer,                         \
            FT_FIELD_OFFSET( _fname ),                \
            FT_FIELD_SIZE_DELTA( _fname ),            \
            _fmax,                                    \
            FT_FIELD_OFFSET( _fcount ),               \
            0                                         \
          }

#define T1_FIELD_FIXED_ARRAY( _fname, _fcount, _fmax ) \
          {                                            \
            t1_field_fixed,                            \
            FT_FIELD_OFFSET( _fname ),                 \
            FT_FIELD_SIZE_DELTA( _fname ),             \
            _fmax,                                     \
            FT_FIELD_OFFSET( _fcount ),                \
            0                                          \
          }

#define T1_FIELD_NUM_ARRAY2( _fname, _fmax ) \
          {                                  \
            t1_field_integer,                \
            FT_FIELD_OFFSET( _fname ),       \
            FT_FIELD_SIZE_DELTA( _fname ),   \
            _fmax,                           \
            0, 0                             \
          }

#define T1_FIELD_FIXED_ARRAY2( _fname, _fmax ) \
          {                                    \
            t1_field_fixed,                    \
            FT_FIELD_OFFSTE( _fname ),         \
            FT_FIELD_SIZE_DELTA( _fname ),     \
            _fmax,                             \
            0, 0                               \
          }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                            T1 PARSER                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    T1_Parser                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A T1_Parser is an object used to parse a Type 1 font very quickly. */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    cursor :: The current position in the text.                        */
  /*                                                                       */
  /*    base   :: Start of the processed text.                             */
  /*                                                                       */
  /*    limit  :: End of the processed text.                               */
  /*                                                                       */
  /*    error  :: The last error returned.                                 */
  /*                                                                       */
  typedef struct  T1_Parser_
  {
    FT_Byte*   cursor;
    FT_Byte*   base;
    FT_Byte*   limit;
    FT_Error   error;
    FT_Memory  memory;
  
  } T1_Parser;


  typedef struct  T1_Parser_Funcs_
  {
    void      (*init)          ( T1_Parser*  parser,
                                 FT_Byte*    base,
                                 FT_Byte*    limit,
                                 FT_Memory   memory );
                       
    void      (*done)          ( T1_Parser*  parser );

    void      (*skip_spaces)   ( T1_Parser*  parser );
    void      (*skip_alpha)    ( T1_Parser*  parser );
  
    FT_Long   (*to_int)        ( T1_Parser*  parser );
    FT_Fixed  (*to_fixed)      ( T1_Parser*  parser,
                                 FT_Int      power_ten );
    FT_Int    (*to_coord_array)( T1_Parser*  parser,
                                 FT_Int      max_coords,
                                 FT_Short*   coords );
    FT_Int    (*to_fixed_array)( T1_Parser*  parser,
                                 FT_Int      max_values,
                                 FT_Fixed*   values,
                                 FT_Int      power_ten );    
  
    void      (*to_token)      ( T1_Parser*  parser,
                                 T1_Token*   token );
    void      (*to_token_array)( T1_Parser*  parser,
                                 T1_Token*   tokens,
                                 FT_UInt     max_tokens,
                                 FT_Int*     pnum_tokens );
                                 
    FT_Error  (*load_field)    ( T1_Parser*       parser,
                                 const T1_Field*  field,
                                 void**           objects,
                                 FT_UInt          max_objects,
                                 FT_ULong*        pflags );
    
    FT_Error  (*load_field_table)( T1_Parser*       parser,
                                   const T1_Field*  field,
                                   void**           objects,
                                   FT_UInt          max_objects,
                                   FT_ULong*        pflags );

  } T1_Parser_Funcs;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                         T1 BUILDER                            *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Structure>                                                           */
  /*    T1_Builder                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*     A structure used during glyph loading to store its outline.       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    memory       :: The current memory object.                         */
  /*                                                                       */
  /*    face         :: The current face object.                           */
  /*                                                                       */
  /*    glyph        :: The current glyph slot.                            */
  /*                                                                       */
  /*    loader       :: XXX                                                */
  /*                                                                       */
  /*    base         :: The base glyph outline.                            */
  /*                                                                       */
  /*    current      :: The current glyph outline.                         */
  /*                                                                       */
  /*    max_points   :: maximum points in builder outline                  */
  /*                                                                       */
  /*    max_contours :: Maximal number of contours in builder outline.     */
  /*                                                                       */
  /*    last         :: The last point position.                           */
  /*                                                                       */
  /*    scale_x      :: The horizontal scale (FUnits to sub-pixels).       */
  /*                                                                       */
  /*    scale_y      :: The vertical scale (FUnits to sub-pixels).         */
  /*                                                                       */
  /*    pos_x        :: The horizontal translation (if composite glyph).   */
  /*                                                                       */
  /*    pos_y        :: The vertical translation (if composite glyph).     */
  /*                                                                       */
  /*    left_bearing :: The left side bearing point.                       */
  /*                                                                       */
  /*    advance      :: The horizontal advance vector.                     */
  /*                                                                       */
  /*    bbox         :: Unused.                                            */
  /*                                                                       */
  /*    path_begun   :: A flag which indicates that a new path has begun.  */
  /*                                                                       */
  /*    load_points  :: If this flag is not set, no points are loaded.     */
  /*                                                                       */
  /*    no_recurse   :: Set but not used.                                  */
  /*                                                                       */
  /*    error        :: An error code that is only used to report memory   */
  /*                    allocation problems.                               */
  /*                                                                       */
  /*    metrics_only :: A boolean indicating that we only want to compute  */
  /*                    the metrics of a given glyph, not load all of its  */
  /*                    points.                                            */
  /*                                                                       */
  typedef struct  T2_Builder_
  {
    FT_Memory        memory;
    FT_Face          face;
    FT_GlyphSlot     glyph;
    FT_GlyphLoader*  loader;
    FT_Outline*      base;
    FT_Outline*      current;

    FT_Vector        last;

    FT_Fixed         scale_x;
    FT_Fixed         scale_y;

    FT_Pos           pos_x;
    FT_Pos           pos_y;

    FT_Vector        left_bearing;
    FT_Vector        advance;

    FT_BBox          bbox;          /* bounding box */
    FT_Bool          path_begun;
    FT_Bool          load_points;
    FT_Bool          no_recurse;

    FT_Error         error;         /* only used for memory errors */
    FT_Bool          metrics_only;

  } T1_Builder;


  typedef FT_Error  (*T1_Builder_Check_Points_Func)( T1_Builder*  builder,
                                                     FT_Int       count );
                                                      
  typedef void  (*T1_Builder_Add_Point_Func)( T1_Builder*  builder,
                                              FT_Pos       x,
                                              FT_Pos       y,
                                              FT_Byte      flag );    
  
  typedef void  (*T1_Builder_Add_Point1_Func)( T1_Builder*  builder,
                                               FT_Pos       x,
                                               FT_Pos       y );
                                                    
  typedef FT_Error  (*T1_Builder_Add_Contour_Func)( T1_Builder*  builder );                                                    

  typedef FT_Error  (*T1_Builder_Start_Point_Func)( T1_Builder*  builder,
                                                    FT_Pos       x,
                                                    FT_Pos       y );

  typedef void  (*T1_Builder_Close_Contour_Func)( T1_Builder*  builder );


  typedef struct  T1_Builder_Funcs_
  {
    FT_Error  (*init)( T1_Builder*   builder,
                       FT_Face       face,
                       FT_Size       size,
                       FT_GlyphSlot  slot );
  
    void      (*done)( T1_Builder*   builder );
    
    T1_Builder_Check_Points_Func   check_points;
    T1_Builder_Add_Points_Func     add_point;
    T1_Builder_Add_Point1_Func     add_point1;
    T1_Builder_Add_Contour_Func    add_contour;
    T1_Builder_Start_Point_Func    start_point;
    T1_Builder_Close_Contour_Func  close_contour;
  
  } T1_Builder_Funcs;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                        PSAux Module Interface                 *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef struct  PSAux_Interface_
  {
    const T1_Table_Funcs*    t1_table_funcs;
    const T1_Parser_Funcs*   t1_parser_funcs;
    const T1_Builder_Funcs*  t1_builder_funcs;

  } PSAux_Interface;


#ifdef __cplusplus
  }
#endif

#endif /* PSAUX_H */


/* END */
