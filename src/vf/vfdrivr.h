/****************************************************************************
 *
 * vfdrivr.h
 *
 *   FreeType font driver for TeX's VF FONT files.
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


#ifndef VFDRIVR_H_
#define VFDRIVR_H_

#include <ft2build.h>
#include FT_INTERNAL_DRIVER_H

#include "vf.h"


FT_BEGIN_HEADER

  typedef struct VF_BitmapRec_
  {
    FT_UInt              bbx_width, bbx_height;
    FT_UInt              off_x, off_y;
    FT_UInt              mv_x,  mv_y;
    FT_Byte              *bitmap;
    FT_UInt              raster;

  } VF_BitmapRec, *VF_Bitmap;


  /* Bitmap list */
  struct vf_s_bitmaplist {
    FT_Long              off_x, off_y;
    VF_Bitmap            bitmap;
    struct vf_s_bitmaplist *next;
  };
  typedef struct vf_s_bitmaplist *VF_BITMAPLIST;

  typedef struct TFM_Rec_
  {
    /* Font Info */
    FT_ULong        cs;
    /* Metrics */
    FT_ULong        ds;
    FT_ULong        design_size;
    FT_ULong        slant;
    unsigned int    begin_char, end_char;
    FT_Long         *width, *height, *depth;
    /* Font bounding box */
    FT_ULong        font_bbx_w, font_bbx_h;
    FT_ULong        font_bbx_xoff, font_bbx_yoff;

  } TFM_Rec, *TFM;

  typedef struct VF_Rec_
  {
    FT_Char       *vf_path;
    FT_ULong      cs;
    FT_ULong      ds;
    FT_ULong      design_size;
    FT_ULong      point_size;
    FT_ULong      dpi_x, dpi_y;
    FT_ULong      mag_x, mag_y;
    /* TFM */
    FT_Char       *tfm_path;
    TFM           tfm;
    /* subfotns */
    struct s_vf_subfont  *subfonts;
    FT_Int        subfonts_opened;
    FT_Int        default_subfont;
    /* file offset to character packets (offset in vf file) */
    FT_Long       offs_char_packet;
  }VF_Rec, *VF;

  typedef struct s_vf_char_packet
  {
    FT_ULong    pl;
    FT_ULong    cc;
    FT_ULong    tfm;
    FT_Byte     *dvi;
  }s_vf_char_packet, *VF_CHAR_PACKET;

  typedef struct s_vf_char_packet_tbl
  {
    int                npackets;
    VF_CHAR_PACKET     packets;
  }s_vf_char_packet_tbl, *VF_CHAR_PACKET_TBL;

  typedef struct s_vf_subfont
  {
    FT_ULong         k;
    FT_ULong         s;
    FT_ULong         d;
    FT_ULong         a;
    FT_ULong         l;
    char             *n;
    struct s_vf_subfont *next;
  }s_vf_subfont, *VF_SUBFONT;


  struct s_vf_dvi_stack
  {
    long    h, v, w, x, y, z;
    int     f;
    int     font_id;
    struct s_vf_dvi_stack  *next;
  };
  typedef struct s_vf_dvi_stack  *VF_DVI_STACK;

#define   STACK(X)     dvi_stack->next->X

  typedef struct  VF_FaceRec_
  {
    FT_FaceRec        root;
    VF                vf;
  } VF_FaceRec, *VF_Face;


  FT_EXPORT_VAR( const FT_Driver_ClassRec )  vf_driver_class;


FT_END_HEADER


#endif /* VFDRIVR_H_ */


/* END */
