/*******************************************************************
 *
 *  t1load.h                                                    2.0
 *
 *    Type1 Loader.
 *
 *  Copyright 1996-2000 by
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

#include <freetype/internal/ftstream.h>
#include <freetype/internal/t1types.h>
#include <freetype/ftmm.h>
#include <t1parse.h>

#ifdef __cplusplus
  extern "C" {
#endif

  typedef struct T1_Loader_
  {
    T1_Parser        parser;          /* parser used to read the stream */

    T1_Int           num_chars;       /* number of characters in encoding */
    T1_Table         encoding_table;  /* T1_Table used to store the       */
                                /* encoding character names         */

    T1_Int           num_glyphs;
    T1_Table         glyph_names;
    T1_Table         charstrings;

    T1_Int           num_subrs;
    T1_Table         subrs;
    T1_Bool          fontdata;

  } T1_Loader;

  LOCAL_DEF
  T1_Error  T1_Open_Face( T1_Face  face );

#ifndef T1_CONFIG_OPTION_NO_MM_SUPPORT
  LOCAL_DEF
  T1_Error  T1_Get_Multi_Master( T1_Face           face,
                                 FT_Multi_Master*  master );

  LOCAL_DEF
  T1_Error  T1_Set_MM_Blend( T1_Face    face,
                             T1_UInt    num_coords,
                             T1_Fixed*  coords );

  LOCAL_DEF
  T1_Error  T1_Set_MM_Design( T1_Face   face,
                              T1_UInt   num_coords,
                              T1_Long*  coords );

  LOCAL_DEF
  void  T1_Done_Blend( T1_Face  face );
#endif

#ifdef __cplusplus
  }
#endif

#endif /* T1LOAD_H */


/* END */
