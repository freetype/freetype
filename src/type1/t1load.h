/*******************************************************************
 *
 *  t1load.h                                                    1.0
 *
 *    Type1 Loader.                          
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
 ******************************************************************/

#ifndef T1LOAD_H
#define T1LOAD_H

#include <ftstream.h>
#include <t1parse.h>

#ifdef __cplusplus
  extern "C" {
#endif

  /*************************************************************************/
  /*                                                                       */
  /* <Function> Init_T1_Parser                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initialise a given parser object to build a given T1_Face          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    parser  :: handle to the newly built parser object                 */
  /*    face    :: handle to target T1 face object                         */
  /*                                                                       */
   LOCAL_DEF
   void  Init_T1_Parser( T1_Parser*    parser,
                         T1_Face       face,
                         T1_Tokenizer  tokenizer );


  /*************************************************************************/
  /*                                                                       */
  /* <Function> Parse_T1_FontProgram                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Parses a given Type 1 font file and builds its face object         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    parser  :: handle to target parser object                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code. 0 means success..                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The parser contains a handle to the target face object.            */
  /*                                                                       */
   LOCAL_DEF
   T1_Error  Parse_T1_FontProgram( T1_Parser*  parser );


#ifdef __cplusplus
  }
#endif

#endif /* T1LOAD_H */


/* END */
