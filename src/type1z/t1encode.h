/***************************************************************************/
/*                                                                         */
/*  t1encode.h                                                             */
/*                                                                         */
/*    Type 1 standard encoding tables definitions (specification).         */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*                                                                         */
/*  This file is included by both the Type1 and Type2 driver.              */
/*  It should never be compiled directly.                                  */
/*                                                                         */
/***************************************************************************/


#ifndef T1ENCODE_H
#define T1ENCODE_H

#include <t1types.h>


  /*************************************************************************/
  /*                                                                       */
  /*  t1_standard_strings:                                                 */
  /*                                                                       */
  /*     This array contains the Adobe Standard Glyph Names ordered by     */
  /*     SID.  It was taken from the CFF specification.                    */
  /*                                                                       */
  LOCAL_DEF
  const T1_String*  t1_standard_strings[];


  /*************************************************************************/
  /*                                                                       */
  /*  t1_standard_encoding:                                                */
  /*                                                                       */
  /*     A simple table used to encode the Adobe StandardEncoding.  The    */
  /*     table values are the SID of the standard glyphs; the table index  */
  /*     is the character code for the encoding.                           */
  /*                                                                       */
  /*     Example:                                                          */
  /*                                                                       */
  /*       t1_standard_encoding[33] == 2                                   */
  /*                                                                       */
  /*     which means that the glyph name for character code 33 is          */
  /*                                                                       */
  /*       t1_standard_strings[2] == "exclam"                              */
  /*                                                                       */
  /*     (this correspond to the exclamation mark `!').                    */
  /*                                                                       */
  LOCAL_DEF
  T1_Byte  t1_standard_encoding[256];


  /*************************************************************************/
  /*                                                                       */
  /*  t1_expert_encoding:                                                  */
  /*                                                                       */
  /*     A simple table used to encode the Adobe ExpertEncoding.  The      */
  /*     table values are the SID of the standard glyphs; the table index  */
  /*     is the character code for the encoding.                           */
  /*                                                                       */
  /*     Example:                                                          */
  /*                                                                       */
  /*       t1_expert_encoding[33] == 229                                   */
  /*                                                                       */
  /*     which means that the glyph name for character code 33 is          */
  /*                                                                       */
  /*       t1_standard_strings[229] == "exclamsmall"                       */
  /*                                                                       */
  LOCAL_DEF
  T1_Short  t1_expert_encoding[256];




#endif /* T1ENCODE_H */


/* END */
