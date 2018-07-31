/****************************************************************************
 *
 * pkdrivr.h
 *
 *   FreeType font driver for TeX's PK FONT files.
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


#ifndef PKDRIVR_H_
#define PKDRIVR_H_

#include <ft2build.h>
#include FT_INTERNAL_DRIVER_H

#include "pk.h"


FT_BEGIN_HEADER

  typedef struct PK_BitmapRec_
  {
    FT_UInt              bbx_width, bbx_height;
    FT_UInt              off_x, off_y;
    FT_UInt              mv_x,  mv_y;
    FT_Byte              *bitmap;
    FT_UInt              raster;

  } PK_BitmapRec, *PK_Bitmap;

  typedef struct PK_GlyphRec_
  {
    FT_UInt         code_min, code_max;
    PK_Bitmap       bm_table;
    FT_UInt         ds, hppp, vppp;
    FT_UInt         font_bbx_w, font_bbx_h;
    FT_UInt         font_bbx_xoff, font_bbx_yoff;

  } PK_GlyphRec, *PK_Glyph;

  typedef struct  PK_FaceRec_
  {
    FT_FaceRec      root;
    PK_Glyph        pk_glyph;

    const void*     tfm;
    const void*     tfm_data;

  } PK_FaceRec, *PK_Face;

  FT_EXPORT_VAR( const FT_Driver_ClassRec )  pk_driver_class;


FT_END_HEADER


#endif /* PKDRIVR_H_ */


/* END */
