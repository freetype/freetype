/***************************************************************************/
/*                                                                         */
/*  t1gload.h                                                              */
/*                                                                         */
/*    Type 1 Glyph Loader (specification).                                 */
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


#ifndef T1GLOAD_H
#define T1GLOAD_H


#ifdef FT_FLAT_COMPILE

#include "t1objs.h"

#else

#include <type1/t1objs.h>

#endif


#ifdef __cplusplus
  extern "C" {
#endif


  FT_LOCAL
  FT_Error  T1_Compute_Max_Advance( T1_Face  face,
                                    FT_Int*  max_advance );

  FT_LOCAL
  FT_Error  T1_Load_Glyph( T1_GlyphSlot  glyph,
                           T1_Size       size,
                           FT_Int        glyph_index,
                           FT_Int        load_flags );


#ifdef __cplusplus
  }
#endif


#endif /* T1GLOAD_H */


/* END */
