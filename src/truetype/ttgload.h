/***************************************************************************/
/*                                                                         */
/*  ttgload.h                                                              */
/*                                                                         */
/*    TrueType Glyph Loader (specification).                               */
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


#ifndef TTGLOAD_H
#define TTGLOAD_H

#include <ttobjs.h>

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#include <ttinterp.h>
#endif

#ifdef __cplusplus
  extern "C" {
#endif

  typedef struct  TT_Loader_
  {
    TT_Face         face;
    TT_Size         size;
    TT_GlyphSlot    glyph;

    TT_ULong        load_flags;
    TT_UInt         glyph_index;

    FT_Stream       stream;
    TT_Int          byte_len;
    TT_Int          left_points;
    TT_Int          left_contours;

    TT_BBox         bbox;
    TT_Int          left_bearing;
    TT_Int          advance;
    TT_Bool         preserve_pps;
    TT_Vector       pp1;
    TT_Vector       pp2;

    TT_ULong        glyf_offset;

    /* the zone where we load our glyphs */
    FT_GlyphZone    base;
    FT_GlyphZone    zone;

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
    TT_ExecContext  exec;
    TT_Byte*        instructions;
#endif

  } TT_Loader;


  LOCAL_DEF
  void  TT_Get_Metrics( TT_HoriHeader*  header,
                        TT_UInt         index,
                        TT_Short*       bearing,
                        TT_UShort*      advance );

  LOCAL_DEF
  TT_Error  TT_Load_Glyph( TT_Size       size,
                           TT_GlyphSlot  glyph,
                           TT_UShort     glyph_index,
                           TT_UInt       load_flags );

#ifdef __cplusplus
  }
#endif

#endif /* TTGLOAD_H */


/* END */
