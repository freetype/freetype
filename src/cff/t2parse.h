#ifndef T2PARSE_H
#define T2PARSE_H

#include <freetype/internal/t2types.h>
#include <freetype/internal/ftobjs.h>

#define T2_MAX_STACK_DEPTH  96

#define T2CODE_TOPDICT   0x1000
#define T2CODE_PRIVATE   0x2000

  typedef struct T2_Parser_
  {
    FT_Byte*   start;
    FT_Byte*   limit;
    FT_Byte*   cursor;
    
    FT_Byte*   stack[ T2_MAX_STACK_DEPTH+1 ];
    FT_Byte**  top;
    
    FT_UInt    object_code;
    void*      object;
    
  } T2_Parser;


  LOCAL_DEF
  void  T2_Parser_Init( T2_Parser*  parser, FT_UInt code, void*  object );


  LOCAL_DEF
  FT_Error  T2_Parser_Run( T2_Parser*  parser,
                           FT_Byte*    start,
                           FT_Byte*    limit );

#endif /* T2PARSE_H */
