/***************************************************************************/
/*                                                                         */
/*  t2parse.h                                                              */
/*                                                                         */
/*    OpenType parser (specification).                                     */
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


#ifndef T2PARSE_H
#define T2PARSE_H

#include <freetype/internal/t2types.h>
#include <freetype/internal/ftobjs.h>

#define T2_MAX_STACK_DEPTH  96

#define T2CODE_TOPDICT  0x1000
#define T2CODE_PRIVATE  0x2000


#ifdef __cplusplus
  extern "C" {
#endif


  typedef struct  T2_Parser_
  {
    TT_Byte*   start;
    TT_Byte*   limit;
    TT_Byte*   cursor;

    TT_Byte*   stack[T2_MAX_STACK_DEPTH + 1];
    TT_Byte**  top;

    TT_UInt    object_code;
    void*      object;

  } T2_Parser;


  LOCAL_DEF
  void  T2_Parser_Init( T2_Parser*  parser,
                        TT_UInt     code,
                        void*       object );

  LOCAL_DEF
  TT_Error  T2_Parser_Run( T2_Parser*  parser,
                           TT_Byte*    start,
                           TT_Byte*    limit );


#ifdef __cplusplus
  }
#endif


#endif /* T2PARSE_H */


/* END */
