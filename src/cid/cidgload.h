/***************************************************************************/
/*                                                                         */
/*  cidgload.h                                                             */
/*                                                                         */
/*    OpenType Glyph Loader (specification).                               */
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


#ifndef CIDGLOAD_H
#define CIDGLOAD_H


#ifdef FT_FLAT_COMPILE

#include "cidobjs.h"

#else

#include <cid/cidobjs.h>

#endif


#ifdef __cplusplus
  extern "C" {
#endif


#if 0

  /* Compute the maximum advance width of a font through quick parsing */
  FT_LOCAL
  FT_Error  CID_Compute_Max_Advance( CID_Face  face,
                                     FT_Int*   max_advance );

#endif /* 0 */

  FT_LOCAL
  FT_Error  CID_Load_Glyph( CID_GlyphSlot  glyph,
                            CID_Size       size,
                            FT_Int         glyph_index,
                            FT_Int         load_flags );


#ifdef __cplusplus
  }
#endif


#endif /* CIDGLOAD_H */


/* END */
