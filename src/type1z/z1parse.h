/***************************************************************************/
/*                                                                         */
/*  z1parse.h                                                              */
/*                                                                         */
/*    Experimental Type 1 parser (specification).                          */
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


#ifndef Z1PARSE_H
#define Z1PARSE_H

#include <freetype/internal/t1types.h>
#include <freetype/internal/ftstream.h>

#ifdef __cplusplus
  extern "C" {
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    Z1_Parser                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A Z1_Parser is an object used to parse a Type 1 fonts very         */
  /*    quickly.                                                           */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    stream       :: The current input stream.                          */
  /*                                                                       */
  /*    memory       :: The current memory object.                         */
  /*                                                                       */
  /*    base_dict    :: A pointer to the top-level dictionary.             */
  /*                                                                       */
  /*    base_len     :: The length in bytes of the top dictionary.         */
  /*                                                                       */
  /*    private_dict :: A pointer to the private dictionary.               */
  /*                                                                       */
  /*    private_len  :: The length in bytes of the private dictionary.     */
  /*                                                                       */
  /*    in_pfb       :: A boolean.  Indicates that we are handling a PFB   */
  /*                    file.                                              */
  /*                                                                       */
  /*    in_memory    :: A boolean.  Indicates a memory-based stream.       */
  /*                                                                       */
  /*    single_block :: A boolean.  Indicates that the private dictionary  */
  /*                    is stored in lieu of the base dictionary.          */
  /*                                                                       */
  /*    cursor       :: The current parser cursor.                         */
  /*                                                                       */
  /*    limit        :: The current parser limit (first byte after the     */
  /*                    current dictionary).                               */
  /*                                                                       */
  /*    error        :: The current parsing error.                         */
  /*                                                                       */
  typedef struct  Z1_Parser_
  {
    T1_Parser  root;
    FT_Stream  stream;

    FT_Byte*   base_dict;
    FT_Int     base_len;

    FT_Byte*   private_dict;
    FT_Int     private_len;

    FT_Byte    in_pfb;
    FT_Byte    in_memory;
    FT_Byte    single_block;

  } Z1_Parser;


#define Z1_Add_Table(p,i,o,l)     (p)->funcs.add( (p), i, o, l )
#define Z1_Done_Table(p)          do { if ((p)->funcs.done) (p)->funcs.done( p ); } while (0)
#define Z1_Release_Table(p)       do { if ((p)->funcs.release) (p)->funcs.release( p ); } while (0)


#define Z1_Skip_Spaces(p)   (p)->root.funcs.skip_spaces( &(p)->root )
#define Z1_Skip_Alpha(p)    (p)->root.funcs.skip_alpha ( &(p)->root )

#define Z1_ToInt(p)        (p)->root.funcs.to_int( &(p)->root )
#define Z1_ToFixed(p,t)    (p)->root.funcs.to_fixed( &(p)->root, t )

#define Z1_ToCoordArray(p,m,c)     (p)->root.funcs.to_coord_array( &(p)->root, m, c )
#define Z1_ToFixedArray(p,m,f,t)   (p)->root.funcs.to_fixed_array( &(p)->root, m, f, t )
#define Z1_ToToken(p,t)            (p)->root.funcs.to_token( &(p)->root, t )
#define Z1_ToTokenArray(p,t,m,c)   (p)->root.funcs.to_token_array( &(p)->root, t, m, c )

#define Z1_Load_Field(p,f,o,m,pf)        (p)->root.funcs.load_field( &(p)->root, f, o, m, pf )
#define Z1_Load_Field_Table(p,f,o,m,pf)  (p)->root.funcs.load_field_table( &(p)->root, f, o, m, pf )

  LOCAL_DEF
  FT_Error  Z1_New_Parser( Z1_Parser*        parser,
                           FT_Stream         stream,
                           FT_Memory         memory,
                           PSAux_Interface*  psaux );

  LOCAL_DEF
  FT_Error  Z1_Get_Private_Dict( Z1_Parser*  parser );

  LOCAL_DEF
  void  Z1_Decrypt( FT_Byte*   buffer,
                    FT_Int     length,
                    FT_UShort  seed );

  LOCAL_DEF
  void  Z1_Done_Parser( Z1_Parser*  parser );

#ifdef __cplusplus
  }
#endif

#endif /* Z1PARSE_H */


/* END */
