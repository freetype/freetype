/****************************************************************************
 *
 * ttgpos.c
 *
 *   Load the TrueType GPOS table.  The only GPOS layout feature this
 *   currently supports is kerning, from x advances in the pair adjustment
 *   layout feature.
 *
 *   Parts of the implementation were adapted from:
 *   https://github.com/nothings/stb/blob/master/stb_truetype.h
 *
 *   GPOS spec reference available at:
 *   https://learn.microsoft.com/typography/opentype/spec/gpos
 *
 * Copyright (C) 2024 by
 * David Saltzman
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 */

#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftstream.h>
#include <freetype/tttags.h>
#include "freetype/fttypes.h"
#include "freetype/internal/ftobjs.h"
#include "ttgpos.h"


#ifdef TT_CONFIG_OPTION_GPOS_KERNING

  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  ttgpos


  FT_LOCAL_DEF( FT_Error )
  tt_face_load_gpos( TT_Face    face,
                     FT_Stream  stream )
  {
    return FT_Err_Ok;
  }


  FT_LOCAL_DEF( void )
  tt_face_done_gpos( TT_Face  face )
  {
  }


  FT_LOCAL_DEF( FT_Int )
  tt_face_get_gpos_kerning( TT_Face  face,
                            FT_UInt  left_glyph,
                            FT_UInt  right_glyph )
  {
    return 0;
  }

#else /* !TT_CONFIG_OPTION_GPOS_KERNING */

  /* ANSI C doesn't like empty source files */
  typedef int  tt_gpos_dummy_;

#endif /* !TT_CONFIG_OPTION_GPOS_KERNING */


/* END */
