/****************************************************************************
 *
 * ttsvg.h
 *
 *   OpenType SVG Color (specification).
 *
 * Copyright (C) 2018-2019 by
 * David Turner, Robert Wilhelm, Werner Lemberg and Moazin Khatti.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

#ifndef __TTSVG_H__
#define __TTSVG_H__

#include <ft2build.h>

FT_BEGIN_HEADER

  FT_LOCAL( FT_Error )
  tt_face_load_svg( TT_Face    face,
                    FT_Stream  stream );

  FT_LOCAL( void )
  tt_face_free_svg( TT_Face    face );

  FT_LOCAL( FT_Error )
  tt_face_load_svg_doc( FT_GlyphSlot  glyph,
                        FT_UInt       glyph_index );

FT_END_HEADER

#endif /* __TTSVG_H__ */
/* END */
