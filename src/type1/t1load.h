/***************************************************************************/
/*                                                                         */
/*  t1load.h                                                               */
/*                                                                         */
/*    Type 1 font loader (specification).                                  */
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


#ifndef T1LOAD_H
#define T1LOAD_H

#include <freetype/internal/ftstream.h>
#include <freetype/internal/psaux.h>
#include <freetype/ftmm.h>


#ifdef FT_FLAT_COMPILE

#include "t1parse.h"

#else

#include <type1/t1parse.h>

#endif


#ifdef __cplusplus
  extern "C" {
#endif

  typedef struct  T1_Loader_
  {
    T1_ParserRec  parser;          /* parser used to read the stream */

    FT_Int        num_chars;       /* number of characters in encoding */
    PS_Table      encoding_table;  /* PS_Table used to store the       */
                                   /* encoding character names         */

    FT_Int        num_glyphs;
    PS_Table      glyph_names;
    PS_Table      charstrings;

    FT_Int        num_subrs;
    PS_Table      subrs;
    FT_Bool       fontdata;

  } T1_Loader;


  FT_LOCAL
  FT_Error  T1_Open_Face( T1_Face  face );

#ifndef T1_CONFIG_OPTION_NO_MM_SUPPORT

  FT_LOCAL
  FT_Error  T1_Get_Multi_Master( T1_Face           face,
                                 FT_Multi_Master*  master );

  FT_LOCAL
  FT_Error  T1_Set_MM_Blend( T1_Face    face,
                             FT_UInt    num_coords,
                             FT_Fixed*  coords );

  FT_LOCAL
  FT_Error  T1_Set_MM_Design( T1_Face   face,
                              FT_UInt   num_coords,
                              FT_Long*  coords );

  FT_LOCAL
  void  T1_Done_Blend( T1_Face  face );

#endif /* !T1_CONFIG_OPTION_NO_MM_SUPPORT */


#ifdef __cplusplus
  }
#endif

#endif /* T1LOAD_H */


/* END */
