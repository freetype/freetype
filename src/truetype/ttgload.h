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

    FT_ULong        load_flags;
    FT_UInt         glyph_index;

    FT_Stream       stream;
    FT_Int          byte_len;
    FT_Int          left_points;
    FT_Int          left_contours;

    FT_BBox         bbox;
    FT_Int          left_bearing;
    FT_Int          advance;
    FT_Bool         preserve_pps;
    FT_Vector       pp1;
    FT_Vector       pp2;

    FT_ULong        glyf_offset;

    /* the zone where we load our glyphs */
    TT_GlyphZone    base;
    TT_GlyphZone    zone;

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
    TT_ExecContext  exec;
    FT_Byte*        instructions;
#endif

  } TT_Loader;


  LOCAL_DEF
  void  TT_Get_Metrics( TT_HoriHeader*  header,
                        FT_UInt         index,
                        FT_Short*       bearing,
                        FT_UShort*      advance );

  LOCAL_DEF
  FT_Error  TT_Load_Glyph( TT_Size       size,
                           TT_GlyphSlot  glyph,
                           FT_UShort     glyph_index,
                           FT_UInt       load_flags );

#ifdef __cplusplus
  }
#endif

#endif /* TTGLOAD_H */


/* END */
