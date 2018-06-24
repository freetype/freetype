/****************************************************************************
 *
 * tfmlib.c
 *
 *   FreeType font driver for TeX's TFM FONT files
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

#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_OBJECTS_H
#include FT_SYSTEM_H
#include FT_CONFIG_CONFIG_H
#include FT_ERRORS_H
#include FT_TYPES_H

#include "tfm.h"
#include "tfmdrivr.h"
#include "tfmerror.h"


  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_tfmlib

  /**************************************************************************
   *
   * TFM font utility functions.
   *
   */

#define READ_INT1(fp)    (INT1)vf_tex_read_intn((fp), 1)
#define READ_UINT1(fp)   (UINT1)vf_tex_read_uintn((fp), 1)
#define READ_INT2(fp)    (INT2)vf_tex_read_intn((fp), 2)
#define READ_UINT2(fp)   (UINT2)vf_tex_read_uintn((fp), 2)
#define READ_INT3(fp)    (INT3)vf_tex_read_intn((fp), 3)
#define READ_UINT3(fp)   (UINT3)vf_tex_read_uintn((fp), 3)
#define READ_INT4(fp)    (INT4)vf_tex_read_intn((fp), 4)
#define READ_UINT4(fp)   (UINT4)vf_tex_read_uintn((fp), 4)
#define READ_INTN(fp,n)  (INT4)vf_tex_read_intn((fp), (n))
#define READ_UINTN(fp,n) (UINT4)vf_tex_read_uintn((fp), (n))
#define SKIP_N(fp,k)     vf_tex_skip_n((fp), (k))

Glocal long           vf_tex_read_intn(FILE*,int);
Glocal unsigned long  vf_tex_read_uintn(FILE*,int);
Glocal void           vf_tex_skip_n(FILE*,int);




/*
 * Reading a Number from file
 */
Glocal unsigned long
vf_tex_read_uintn(FILE* fp, int size)
{
  unsigned long  v;

  v = 0L;
  while (size >= 1){
    v = v*256L + (unsigned long)getc(fp);
    --size;
  }
  return v;
}

Glocal long
vf_tex_read_intn(FILE* fp, int size)
{
  long           v;

  v = (long)getc(fp) & 0xffL;
  if (v & 0x80L)
    v = v - 256L;
  --size;
  while (size >= 1){
    v = v*256L + (unsigned long)getc(fp);
    --size;
  }

  return v;
}

Glocal void
vf_tex_skip_n(FILE* fp, int size)
{
  while (size > 0){
    (void)getc(fp);
    --size;
  }
}

Glocal unsigned long
vf_tex_get_uintn(unsigned char *p, int size)
{
  unsigned long  v;

  v = 0L;
  while (size >= 1){
    v = v*256L + (unsigned long) *(p++);
    --size;
  }

  return v;
}

Glocal long
vf_tex_get_intn(unsigned char *p, int size)
{
  long           v;

  v = (long)*(p++) & 0xffL;
  if (v & 0x80L)
    v = v - 256L;
  --size;
  while (size >= 1){
    v = v*256L + (unsigned long) *(p++);
    --size;
  }

  return v;
}




Private TFM
read_tfm(FILE* fp)
{
  TFM    tfm;
  UINT4  lf, lh, nc, nci, err;
  UINT4  offset_header, offset_char_info, offset_param;
  UINT4  nw,  nh,  nd,  ni, nl, nk, neng, np, dir;
  INT4   *w,  *h,  *d;
  UINT4  *ci, v;
  UINT4  i;
  INT4   bbxw, bbxh, xoff, yoff;

  ALLOC_IF_ERR(tfm, struct s_tfm){
    vf_error = VF_ERR_NO_MEMORY;
    return NULL;
  }

  tfm->width  = NULL;
  tfm->height = NULL;
  tfm->depth  = NULL;
  tfm->ct_kcode = NULL;
  tfm->ct_ctype = NULL;

  tfm->font_bbx_w = 0.0;
  tfm->font_bbx_h = 0.0;
  tfm->font_bbx_xoff = 0.0;
  tfm->font_bbx_yoff = 0.0;

  err = 0;
  rewind(fp);
  lf = (UINT4)READ_UINT2(fp);
  if ((lf == 11) || (lf == 9)){
    /* JFM file of Japanese TeX by ASCII Coop. */
    tfm->type        = METRIC_TYPE_JFM;
    tfm->type_aux    = (lf == 11)?METRIC_TYPE_JFM_AUX_H:METRIC_TYPE_JFM_AUX_V;
    tfm->nt          = (UINT4)READ_UINT2(fp);
    lf               = (UINT4)READ_UINT2(fp);
    lh               = (UINT4)READ_UINT2(fp);
    offset_header    = 4*7;
    offset_char_info = 4*(7+tfm->nt+lh);
  } else if (lf == 0){
    /* Omega Metric File */
    tfm->type        = METRIC_TYPE_OFM;
    tfm->type_aux    = READ_INT2(fp);    /* ofm_level */
    if ((tfm->type_aux < 0) || (1 < tfm->type_aux))
      tfm->type_aux = 0;  /* broken, maybe */
    lf               = READ_UINT4(fp);
    lh               = READ_UINT4(fp);
    if (tfm->type_aux == 0){   /* level 0 OFM */
      offset_header    = 4*14;
      offset_char_info = 4*(14+lh);
    } else {                   /* level 1 OFM: *** NOT SUPPORTED YET *** */
      offset_header    = 4*29;
      offset_char_info = 4*(29+lh);
    }
  } else {
    /* Traditional TeX Metric File */
    tfm->type        = METRIC_TYPE_TFM;
    tfm->type_aux    = 0;
    lh               = (int)READ_UINT2(fp);
    offset_header    = 4*6;
    offset_char_info = 4*(6+lh);
  }

  if (tfm->type == METRIC_TYPE_OFM){
    tfm->begin_char  = READ_UINT4(fp);
    tfm->end_char    = READ_UINT4(fp);
    nw   = READ_UINT4(fp);
    nh   = READ_UINT4(fp);
    nd   = READ_UINT4(fp);

    ni   = READ_UINT4(fp);
    nl   = READ_UINT4(fp);
    nk   = READ_UINT4(fp);
    neng = READ_UINT4(fp);
    np   = READ_UINT4(fp);
    dir  = READ_UINT4(fp);

    if (((signed)(tfm->begin_char-1) > (signed)tfm->end_char) ||
        (tfm->end_char > 65535)){
      vf_error = VF_ERR_INVALID_METRIC;
      return NULL;
    }
  } else {
    tfm->begin_char  = (int)READ_UINT2(fp);
    tfm->end_char    = (int)READ_UINT2(fp);
    nw   = (UINT4)READ_UINT2(fp);
    nh   = (UINT4)READ_UINT2(fp);
    nd   = (UINT4)READ_UINT2(fp);

    ni   = (UINT4)READ_UINT2(fp);
    nl   = (UINT4)READ_UINT2(fp);
    nk   = (UINT4)READ_UINT2(fp);
    neng = (UINT4)READ_UINT2(fp);
    np   = (UINT4)READ_UINT2(fp);

    if (tfm->type == METRIC_TYPE_TFM){
      if (((signed)(tfm->begin_char-1) > (signed)tfm->end_char) ||
          (tfm->end_char > 255)){
        vf_error = VF_ERR_INVALID_METRIC;
        return NULL;
      }
    }
  }

  fseek(fp, offset_header, SEEK_SET);
  tfm->cs          = READ_UINT4(fp);
  tfm->ds          = READ_UINT4(fp);
  tfm->design_size = (double)(tfm->ds)/(double)(1<<20);

  nc  = tfm->end_char - tfm->begin_char + 1;
  nci = nc;
  if (tfm->type == METRIC_TYPE_OFM)
    nci *= 2;
  ci = (UINT4*)calloc(nci, sizeof(UINT4));
  w  = (INT4*)calloc(nw,  sizeof(UINT4));
  h  = (INT4*)calloc(nh,  sizeof(UINT4));
  d  = (INT4*)calloc(nd,  sizeof(UINT4));
  if ((ci == NULL) || (w == NULL) || (h == NULL) || (d == NULL)){
    err = VF_ERR_NO_MEMORY;
    goto Exit;
  }
  fseek(fp, offset_char_info, SEEK_SET);
  for (i = 0; i < nci; i++)
    ci[i] = READ_UINT4(fp);
  offset_param = ftell(fp) + 4*(nw + nh + nd + ni + nl + nk + neng);
  for (i = 0; i < nw; i++)
    w[i] = READ_INT4(fp);
  for (i = 0; i < nh; i++)
    h[i] = READ_INT4(fp);
  for (i = 0; i < nd; i++)
    d[i] = READ_INT4(fp);

  tfm->width  = (INT4*)calloc(nc, sizeof(INT4));
  tfm->height = (INT4*)calloc(nc, sizeof(INT4));
  tfm->depth  = (INT4*)calloc(nc, sizeof(INT4));
  if ((tfm->width == NULL) || (tfm->height == NULL) || (tfm->depth == NULL)){
    err = VF_ERR_NO_MEMORY;
    goto Exit;
  }
  bbxw = 0;
  bbxh = 0;
  xoff = 0;
  yoff = 0;
  if (tfm->type == METRIC_TYPE_OFM){
    for (i = 0; i < nc; i++){
      v = ci[2*i];
      tfm->depth[i]  = d[v & 0xff]; v >>= 8;
      tfm->height[i] = h[v & 0xff]; v >>= 8;
      tfm->width[i]  = w[v & 0xffff];
      if (bbxw < tfm->width[i])
	bbxw = tfm->width[i];
      if (bbxh < (tfm->height[i] + tfm->depth[i]))
	bbxh = tfm->height[i] + tfm->depth[i];
      if (yoff > -tfm->depth[i])
	yoff = -tfm->depth[i];
#if 0
      printf("** %.3f %.3f %.3f\n",
	     (double)tfm->width[i]/(double)(1<<20),
	     (double)tfm->height[i]/(double)(1<<20),
	     (double)tfm->depth[i]/(double)(1<<20));
#endif
    }
  } else {
    for (i = 0; i < nc; i++){
      v = ci[i] / 0x10000L;
      tfm->depth[i]  = d[v & 0xf];  v >>= 4;
      tfm->height[i] = h[v & 0xf];  v >>= 4;
      tfm->width[i]  = w[v & 0xff];
      if (bbxw < tfm->width[i])
	bbxw = tfm->width[i];
      if (bbxh < (tfm->height[i] + tfm->depth[i]))
	bbxh = tfm->height[i] + tfm->depth[i];
      if (yoff > -tfm->depth[i])
	yoff = -tfm->depth[i];
#if 0
      printf("** %.3f %.3f\n",
	     (double)tfm->height[i]/(double)(1<<20),
	     (double)tfm->depth[i]/(double)(1<<20));
#endif
    }
  }
  tfm->font_bbx_w = tfm->design_size * ((double)bbxw / (double)(1<<20));
  tfm->font_bbx_h = tfm->design_size * ((double)bbxh / (double)(1<<20));
  tfm->font_bbx_xoff = tfm->design_size * ((double)xoff / (double)(1<<20));
  tfm->font_bbx_yoff = tfm->design_size * ((double)yoff / (double)(1<<20));

  if (tfm->type == METRIC_TYPE_JFM){
    fseek(fp, 4*(7+lh), SEEK_SET);
    tfm->ct_kcode = (unsigned int*)calloc(tfm->nt+1, sizeof(unsigned int));
    tfm->ct_ctype = (unsigned int*)calloc(tfm->nt+1, sizeof(unsigned int));
    if ((tfm->ct_kcode == NULL) || (tfm->ct_ctype == NULL)){
      err = VF_ERR_NO_MEMORY;
      goto Exit;
    }
    for (i = 0; i < tfm->nt; i++){
      v = READ_UINT4(fp);
      tfm->ct_kcode[i] = v/0x10000L;
      tfm->ct_ctype[i] = v%0x10000L;
    }
    tfm->ct_kcode[tfm->nt] = 0; /* sentinel */
    tfm->ct_ctype[tfm->nt] = 0;
  }

  fseek(fp, offset_param, SEEK_SET);
  tfm->slant = (double)READ_INT4(fp)/(double)(1<<20);

Exit:
  vf_free(ci);
  vf_free(w);
  vf_free(h);
  vf_free(d);

  if (err != 0){
    vf_tfm_free(tfm);
    vf_error = err;
    return NULL;
  }
  return tfm;
}




Glocal void
vf_tfm_free(TFM tfm)
{
  int  tfm_id;

  if (tfm == NULL)
    return;

  tfm_id = (tfm_table->get_id_by_obj)(tfm_table, tfm);
  if (tfm_id < 0)
    return;

  if ((tfm_table->unlink_by_id)(tfm_table, tfm_id) <= 0){
    vf_free(tfm->width);
    vf_free(tfm->height);
    vf_free(tfm->depth);
    vf_free(tfm->ct_kcode);
    vf_free(tfm->ct_ctype);
    vf_free(tfm);
  }
}




/* TO-DO */

/* END */
