/****************************************************************************
 *
 * gfdrivr.h
 *
 *   FreeType font driver for TeX's GF FONT files
 *
 * Copyright 1996-2018 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef GFDRIVR_H_
#define GFDRIVR_H_

#include <ft2build.h>
#include FT_INTERNAL_DRIVER_H

#include "gf.h"


FT_BEGIN_HEADER

  /* BitmapRec for GF format specific glyphs  */
  typedef struct GF_BitmapRec_
  {
    FT_UInt              bbx_width, bbx_height;
    FT_UInt              off_x, off_y;
    FT_UInt              mv_x,  mv_y;
    unsigned char        *bitmap;
    FT_UInt              raster;

  } GF_BitmapRec, *GF_Bitmap;


  typedef struct GF_GlyphRec_
  {
    FT_UInt         code_min, code_max;
    GF_Bitmap       bm_table;
    double          ds, hppp, vppp;
    FT_UInt         font_bbx_w, font_bbx_h;
    FT_UInt         font_bbx_xoff, font_bbx_yoff;

  } GF_GlyphRec, *GF_Glyph;


  typedef struct  GF_FaceRec_
  {
    FT_FaceRec        root;
    GF_Glyph          gf_glyph;

  } GF_FaceRec, *GF_Face;


  FT_EXPORT_VAR( const FT_Driver_ClassRec )  gf_driver_class;


FT_END_HEADER


#endif /* GFDRIVR_H_ */


/* END */
