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

#include <ftstream.h>
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
    
  } T1_Loader;

  LOCAL_DEF
  T1_Error  T1_Open_Face( T1_Face  face );


#ifdef __cplusplus
  }
#endif

#endif /* T1LOAD_H */


/* END */
