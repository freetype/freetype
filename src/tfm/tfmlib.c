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

  long           tfm_read_intn(FT_Stream,int);
  unsigned long  tfm_read_uintn(FT_Stream,int);

#define READ_UINT2( stream )    (UINT1)tfm_read_uintn( stream, 2)
#define READ_UINT4( stream )    (UINT1)tfm_read_uintn( stream, 4)
#define READ_INT4( stream )     (INT4)tfm_read_intn( stream, 4)

/*
 * Reading a Number from file
 */
  unsigned long
  tfm_read_uintn(FT_Stream stream, int size)
  {
    unsigned long  v,k;
    FT_Error error = FT_Err_Ok;
    FT_Byte tp;
    v = 0L;
    while (size >= 1)
    {
      if ( FT_READ_BYTE(tp) )
        return 0; /* To be changed */
      k =(unsigned long)tp;
      v = v*256L + k;
      --size;
    }
    return v;
  }

  long
  tfm_read_intn(FT_Stream stream, int size)
  {
    long           v;
    FT_Byte tp;
    FT_Error error= FT_Err_Ok;
    unsigned long z ;
    if ( FT_READ_BYTE(tp) )
        return 0; /* To be changed */
    z= (unsigned long)tp;
    v = (long)z & 0xffL;
    if (v & 0x80L)
      v = v - 256L;
    --size;
    while (size >= 1)
    {
      if ( FT_READ_BYTE(tp) )
        return 0; /* To be changed */
      z= (unsigned long)tp;
      v = v*256L + z;
      --size;
		}
    return v;
  }

  /**************************************************************************
   *
   * API.
   *
   */

  FT_LOCAL_DEF( void )
  tfm_free_font( TFM_Glyph tfm, FT_Memory memory )
  {
    if (tfm == NULL)
      return;

      FT_FREE(tfm->width);
      FT_FREE(tfm->height);
      FT_FREE(tfm->depth);
      FT_FREE(tfm);
  }

  FT_LOCAL_DEF( FT_Error )
  tfm_load_font(  FT_Stream       stream,
                  FT_Memory       extmemory,
                  TFM_Glyph       *tfmptr  )
  {
    TFM_Glyph  tfm;
    UINT4  lf, lh, nc, nci, err;
    UINT4  offset_header, offset_char_info, offset_param;
    UINT4  nw,  nh,  nd,  ni, nl, nk, neng, np, dir;
    INT4   *w,  *h,  *d;
    UINT4  *ci, v;
    UINT4  i;
    INT4   bbxw, bbxh, xoff, yoff;
    FT_Error        error  =FT_Err_Ok;
    FT_Memory       memory = extmemory; /* needed for FT_NEW */

    if( FT_ALLOC(tfm, sizeof(TFM_GlyphRec)) )
      goto Exit;

    tfm->width  = NULL;
    tfm->height = NULL;
    tfm->depth  = NULL;

    tfm->font_bbx_w = 0.0;
    tfm->font_bbx_h = 0.0;
    tfm->font_bbx_xoff = 0.0;
    tfm->font_bbx_yoff = 0.0;

    err = 0;
    /* rewind(fp); */
    if( FT_STREAM_SEEK( 0 ) )
      return error;
    lf = (UINT4)READ_UINT2( stream );
    #if 0
    if ((lf == 11) || (lf == 9))
    {
      /* JFM file of Japanese TeX by ASCII Coop. */
      tfm->type        = METRIC_TYPE_JFM;
      tfm->type_aux    = (lf == 11)?METRIC_TYPE_JFM_AUX_H:METRIC_TYPE_JFM_AUX_V;
      tfm->nt          = (UINT4)READ_UINT2(fp);
      lf               = (UINT4)READ_UINT2(fp);
      lh               = (UINT4)READ_UINT2(fp);
      offset_header    = 4*7;
      offset_char_info = 4*(7+tfm->nt+lh);
    }
    else if (lf == 0)
    {
      /* Omega Metric File */
      tfm->type        = METRIC_TYPE_OFM;
      tfm->type_aux    = READ_INT2(fp);    /* ofm_level */
      if ((tfm->type_aux < 0) || (1 < tfm->type_aux))
        tfm->type_aux = 0;  /* broken, maybe */
      lf               = READ_UINT4(fp);
      lh               = READ_UINT4(fp);
      if (tfm->type_aux == 0)
      {   /* level 0 OFM */
        offset_header    = 4*14;
        offset_char_info = 4*(14+lh);
      }
      else
      {                   /* level 1 OFM: *** NOT SUPPORTED YET *** */
        offset_header    = 4*29;
        offset_char_info = 4*(29+lh);
      }
    }
    else
    { }
    #endif
    /* Traditional TeX Metric File */
    tfm->type_aux    = 0;
    lh               = (int)READ_UINT2( stream );
    offset_header    = 4*6;
    offset_char_info = 4*(6+lh);

    #if 0
    if (tfm->type == METRIC_TYPE_OFM)
    {
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
        (tfm->end_char > 65535))
      {
        error = FT_THROW( Invalid_Argument );
        goto Exit;
      }
    }
    else
    { }
    #endif
    tfm->begin_char  = (int)READ_UINT2( stream );
    tfm->end_char    = (int)READ_UINT2( stream );
    nw   = (UINT4)READ_UINT2( stream );
    nh   = (UINT4)READ_UINT2( stream );
    nd   = (UINT4)READ_UINT2( stream );

    ni   = (UINT4)READ_UINT2( stream );
    nl   = (UINT4)READ_UINT2( stream );
    nk   = (UINT4)READ_UINT2( stream );
    neng = (UINT4)READ_UINT2( stream );
    np   = (UINT4)READ_UINT2( stream );

    #if 0
      if (tfm->type == METRIC_TYPE_TFM)
        {}
    #endif
    if (((signed)(tfm->begin_char-1) > (signed)tfm->end_char) ||
       (tfm->end_char > 255))
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* fseek(fp, offset_header, SEEK_SET); */
    if (FT_STREAM_SEEK( offset_header ) )
      goto Exit;
    tfm->cs          = READ_UINT4( stream );
    tfm->ds          = READ_UINT4( stream );
    tfm->design_size = (double)(tfm->ds)/(double)(1<<20);

    nc  = tfm->end_char - tfm->begin_char + 1;
    nci = nc;
    #if 0
    if (tfm->type == METRIC_TYPE_OFM)
      nci *= 2;
    #endif
    ci = (UINT4*)calloc(nci, sizeof(UINT4));
    w  = (INT4*)calloc(nw,  sizeof(UINT4));
    h  = (INT4*)calloc(nh,  sizeof(UINT4));
    d  = (INT4*)calloc(nd,  sizeof(UINT4));
    if ((ci == NULL) || (w == NULL) || (h == NULL) || (d == NULL))
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }
    /* fseek(fp, offset_char_info, SEEK_SET); */
    if( FT_STREAM_SEEK( offset_char_info ) )
      goto Exit;
    for (i = 0; i < nci; i++)
      ci[i] = READ_UINT4( stream );

    /* offset_param = ftell(fp) + 4*(nw + nh + nd + ni + nl + nk + neng); */
    offset_param = stream->pos + 4*(nw + nh + nd + ni + nl + nk + neng);

    for (i = 0; i < nw; i++)
      w[i] = READ_INT4( stream );
    for (i = 0; i < nh; i++)
      h[i] = READ_INT4( stream );
    for (i = 0; i < nd; i++)
      d[i] = READ_INT4( stream );

    tfm->width  = (INT4*)calloc(nc, sizeof(INT4));
    tfm->height = (INT4*)calloc(nc, sizeof(INT4));
    tfm->depth  = (INT4*)calloc(nc, sizeof(INT4));
    if ((tfm->width == NULL) || (tfm->height == NULL) || (tfm->depth == NULL))
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }
    bbxw = 0;
    bbxh = 0;
    xoff = 0;
    yoff = 0;
    #if 0
    if (tfm->type == METRIC_TYPE_OFM)
    {
      for (i = 0; i < nc; i++)
      {
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
    }
    else
    { }
    #endif
    for (i = 0; i < nc; i++)
    {
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
    tfm->font_bbx_w = tfm->design_size * ((double)bbxw / (double)(1<<20));
    tfm->font_bbx_h = tfm->design_size * ((double)bbxh / (double)(1<<20));
    tfm->font_bbx_xoff = tfm->design_size * ((double)xoff / (double)(1<<20));
    tfm->font_bbx_yoff = tfm->design_size * ((double)yoff / (double)(1<<20));

    #if 0
    if (tfm->type == METRIC_TYPE_JFM)
    {
      fseek(fp, 4*(7+lh), SEEK_SET);
      tfm->ct_kcode = (unsigned int*)calloc(tfm->nt+1, sizeof(unsigned int));
      tfm->ct_ctype = (unsigned int*)calloc(tfm->nt+1, sizeof(unsigned int));
      if ((tfm->ct_kcode == NULL) || (tfm->ct_ctype == NULL))
      {
        error = FT_THROW( Invalid_Argument );
        goto Exit;
      }
      for (i = 0; i < tfm->nt; i++)
      {
        v = READ_UINT4(fp);
        tfm->ct_kcode[i] = v/0x10000L;
        tfm->ct_ctype[i] = v%0x10000L;
      }
      tfm->ct_kcode[tfm->nt] = 0; /* sentinel */
      tfm->ct_ctype[tfm->nt] = 0;
    }
    #endif

    /* fseek(fp, offset_param, SEEK_SET); */
    if( FT_STREAM_SEEK( offset_param ) )
      return error; /* To be changed */
    if (FT_READ_ULONG(tfm->slant) )
      return error;
    tfm->slant = (double)tfm->slant/(double)(1<<20);
    *tfmptr          = tfm;
  Exit:
    FT_FREE(ci);
    FT_FREE(w);
    FT_FREE(h);
    FT_FREE(d);

    if (err != 0)
    {
      tfm_free_font(tfm, memory);
      error = err;
    }
    return error;
  }

/* END */
