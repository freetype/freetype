/****************************************************************************
 *
 * gflib.c
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

#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_OBJECTS_H
#include FT_SYSTEM_H
#include FT_CONFIG_CONFIG_H
#include FT_ERRORS_H
#include FT_TYPES_H

#include "gf.h"
#include "gfdrivr.h"
#include "gferror.h"


  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gflib

unsigned char   bit_table[] = {
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

#define STREAM_FILE( stream )  ( (FT_FILE*)stream->descriptor.pointer )

  /**************************************************************************
   *
   * GF font utility functions.
   *
   */

  long           gf_read_intn(FILE*,int);
  unsigned long  gf_read_uintn(FILE*,int);
  void           gf_skip_n(FILE*,int);

#define READ_INT1(fp)          (INT1)gf_read_intn((fp), 1)
#define READ_UINT1(fp)         (UINT1)gf_read_uintn((fp), 1)
#define READ_INT2(fp)          (INT2)gf_read_intn((fp), 2)
#define READ_UINT2(fp)         (UINT2)gf_read_uintn((fp), 2)
#define READ_INT3(fp)          (INT3)gf_read_intn((fp), 3)
#define READ_UINT3(fp)         (UINT3)gf_read_uintn((fp), 3)
#define READ_INT4(fp)          (INT4)gf_read_intn((fp), 4)
#define READ_UINT4(fp)         (UINT4)gf_read_uintn((fp), 4)
#define READ_INTN(fp,n)        (INT4)gf_read_intn((fp), (n))
#define READ_UINTN(fp,n)       (UINT4)gf_read_uintn((fp), (n))
#define SKIP_N(fp,k)           gf_skip_n((fp), (k))


/*
 * Reading a Number from file
 */
  unsigned long
  gf_read_uintn(FILE* fp, int size)
  {
    unsigned long  v ;
    v = 0L;
    while (size >= 1)
    {
      v = v*256L + (unsigned long)getc(fp);
      --size;
    }
    return v;
  }

  long
  gf_read_intn(FILE* fp, int size)
  {
    long           v;
    v = (long)getc(fp) & 0xffL;
    if (v & 0x80L)
      v = v - 256L;
    --size;
    while (size >= 1)
    {
      v = v*256L + (unsigned long)getc(fp);
      --size;
		}
    return v;
  }

  void
  gf_skip_n(FILE* fp, int size)
  {
    while (size > 0)
    {
      (void)getc(fp);
      --size;
    }
  }

  /**************************************************************************
   *
   * API.
   *
   */

  FT_LOCAL_DEF( FT_Error )
  gf_read_glyph(FT_FILE* fp, GF_Bitmap bm)
  {
    long           m, n;
    int            paint_sw;
    int            instr;
    INT4           min_m, max_m, min_n, max_n, del_m, del_n;
    long           w, h, d;
    int            m_b, k;
    unsigned char  *ptr;
    FT_Error        error  = FT_Err_Ok;

    switch (READ_UINT1(fp))
    {
    case GF_BOC:
      SKIP_N(fp, 4);
      SKIP_N(fp, 4);
      min_m = READ_INT4(fp);
      max_m = READ_INT4(fp);
      min_n = READ_INT4(fp);
      max_n = READ_INT4(fp);
      break;
    case GF_BOC1:
      SKIP_N(fp, 1);
      del_m = (INT4)READ_UINT1(fp);
      max_m = (INT4)READ_UINT1(fp);
      del_n = (INT4)READ_UINT1(fp);
      max_n = (INT4)READ_UINT1(fp);
      min_m = max_m - del_m;
      min_n = max_n - del_n;
      break;
    default:
      goto Exit;
    }

    w = max_m - min_m + 1;
    h = max_n - min_n + 1;
    if ((w < 0) || (h < 0))
    {
      error = FT_THROW( Invalid_File_Format );
      goto Exit;
    }

    if ((bm->bitmap = (unsigned char*)malloc(h*((w+7)/8))) == NULL)
    {
      error = FT_THROW( Invalid_File_Format );
      goto Exit;
    }

    memset(bm->bitmap, 0, h*((w+7)/8));
    bm->raster     = (w+7)/8;
    bm->bbx_width  = w;
    bm->bbx_height = h;
    bm->off_x      = -min_m;
    bm->off_y      = max_n;
    #if 0
      bm->mv_x       = -min_m;
      bm->mv_y       = max_n;
    #endif

    m        = min_m;
    n        = max_n;
    paint_sw = 0;
    while ((instr = (int)READ_UINT1(fp)) != GF_EOC)
    {
      if (instr == GF_PAINT_0)
      {
        paint_sw = 1 - paint_sw;
      }
      else if ((GF_NEW_ROW_0 <= instr) && (instr <= GF_NEW_ROW_164))
      {
        m        = min_m + (instr - GF_NEW_ROW_0);
        n        = n - 1;
        paint_sw = 1;
      }
      else if ((GF_PAINT_1 <= instr) && (instr <= GF_PAINT_63))
      {
        d = (instr - GF_PAINT_1 + 1);
        goto Paint;
      }
      else
      {
        switch ((int)instr)
        {
        case GF_PAINT1:
        case GF_PAINT2:
        case GF_PAINT3:
          d = (UINT4)READ_UINTN(fp, (instr - GF_PAINT1 + 1));
          Paint:
            if (paint_sw == 0)
            {
              m = m + d;
            }
            else
            {
              ptr = &bm->bitmap[(max_n - n) * bm->raster + (m - min_m)/8];
              m_b = (m - min_m) % 8;
              while (d > 0)
              {
                *ptr |= bit_table[m_b];
                m++;
                if (++m_b >= 8)
                {
                  m_b = 0;
                  ++ptr;
                }
                d--;
              }
            }
            paint_sw = 1 - paint_sw;
          break;
			  case GF_SKIP0:
          m = min_m;
          n = n - 1;
          paint_sw = 0;
          break;
        case GF_SKIP1:
        case GF_SKIP2:
        case GF_SKIP3:
          m = min_m;
          n = n - (UINT4)READ_UINTN(fp, (instr - GF_SKIP1 + 1)) - 1;
          paint_sw = 0;
          break;
        case GF_XXX1:
        case GF_XXX2:
        case GF_XXX3:
        case GF_XXX4:
          k = READ_UINTN(fp, instr - GF_XXX1 + 1);
          SKIP_N(fp, k);
          break;
        case GF_YYY:
          SKIP_N(fp, 4);
          break;
        case GF_NO_OP:
          break;
        default:
          /* FT_FREE(bm->bitmap); */ /* Returning unnecessary errors TO BE CHECKED */
          bm->bitmap = NULL;
          error = FT_THROW( Invalid_File_Format );
          goto Exit;
         }
      }
    }

    Exit:
      return error;
  }


  FT_LOCAL_DEF( FT_Error )
  gf_load_font(  FT_Stream       stream,
                 FT_Memory       extmemory,
                 GF_Glyph        *goptr  )
  {
    GF_Glyph        go;
    GF_Bitmap       bm;
    UINT1           instr, d;
    UINT4           ds, check_sum, hppp, vppp;
    INT4            min_m, max_m, min_n, max_n;
    INT4            w;
    UINT4           code;
    double          dx, dy;
    long            ptr_post, ptr_p, ptr, optr, gptr,tp;
    int             bc, ec, nchars, i;
    FT_Error        error  = FT_Err_Ok;
    FT_Memory       memory = extmemory; /* needed for FT_NEW */
    FT_ULong        offset;

    FT_FILE *fp = STREAM_FILE( stream ) ;

    go = NULL;
    nchars = -1;

    char* st =  (char*)stream->pathname.pointer;
    fp=fopen(st,"rb");
printf("\nHi I am here in gflibfp\n");
    /* seek to post_post instr. */
    fseek(fp, -5, SEEK_END);

    while ((d = READ_UINT1(fp)) == 223)
      fseek(fp, -2, SEEK_CUR);

    if (d != GF_ID)
    {
      error = FT_THROW( Invalid_File_Format );
      goto ErrExit;
    }

    fseek(fp, -6, SEEK_CUR);

    /* check if the code is post_post */
    if (READ_UINT1(fp) != GF_POST_POST)
    {
      error = FT_THROW( Invalid_File_Format );
      goto ErrExit;
    }

    /* read pointer to post instr. */
    if ((ptr_post = READ_UINT4(fp)) == -1)
    {
      error = FT_THROW( Invalid_File_Format );
      goto ErrExit;
    }

    /* goto post instr. and read it */
    fseek(fp, ptr_post, SEEK_SET);
    if (READ_UINT1(fp) != GF_POST)
    {
      error = FT_THROW( Invalid_File_Format );
      goto ErrExit;
    }

    ptr_p     = READ_UINT4(fp);printf("\nptr_p is %d\n",ptr_p);
    ds        = READ_UINT4(fp);printf("\nds is %d\n",ds);
    check_sum = READ_UINT4(fp);printf("\ncheck_sum is %d\n",check_sum);
    hppp      = READ_UINT4(fp);printf("\nhppp is %d\n",hppp);
    vppp      = READ_UINT4(fp);printf("\nvppp is %d\n",vppp);
    min_m     = READ_INT4(fp);printf("\nmin_m is %d\n",min_m);
    max_m     = READ_INT4(fp);printf("\nmax_m is %d\n",max_m);
    min_n     = READ_INT4(fp);printf("\nmin_n is %d\n",min_n);
    max_n     = READ_INT4(fp);printf("\nmax_n is %d\n",max_n);

    gptr = ftell(fp);

    #if 0
      /* read min & max char code */
      bc = 256;
      ec = -1;
      for (  ;  ;  )
      {
        instr = READ_UINT1(fp);
        if (instr == GF_POST_POST)
        {
          break;
        }
        else if (instr == GF_CHAR_LOC)
        {
          code = READ_UINT1(fp);
          (void)SKIP_N(fp, 16);
        }
        else if (instr == GF_CHAR_LOC0)
        {
          code = READ_UINT1(fp);
          (void)SKIP_N(fp, 9);
        }
        else
        {
          error = FT_THROW( Invalid_File_Format );
          goto ErrExit;
        }
        if (code < bc)
          bc = code;
        if (code > ec)
          ec = code;
      }
    #else
      bc = 0;
      ec = 255;
    #endif

    nchars = ec - bc + 1;
    go= malloc(sizeof(GF_GlyphRec)); /* FT_ALLOC(go, sizeof(GF_GlyphRec)); goto ErrExit; */
                                               /* Returning unnecessary errors TO BE CHECKED */

    go->bm_table = (GF_Bitmap)malloc(nchars* sizeof(GF_BitmapRec));/* FT_ALLOC_MULT(go->bm_table, sizeof(GF_BitmapRec), nchars); goto ErrExit; */
                                                                   /* Returning unnecessary errors TO BE CHECKED */

    for (i = 0; i < nchars; i++)
      go->bm_table[i].bitmap = NULL;

    go->ds   = (double)ds/(1<<20);printf("\ngo->ds is %d\n",go->ds);
    go->hppp = (double)hppp/(1<<16);printf("\ngo->hppp is %d\n",go->hppp);
    go->vppp = (double)vppp/(1<<16);printf("\ngo->vppp is %d\n",go->vppp);
    go->font_bbx_w = max_m - min_m;printf("\ngo->font_bbx_w is %d\n",go->font_bbx_w);
    go->font_bbx_h = max_n - min_n;printf("\ngo->font_bbx_h is %d\n",go->font_bbx_h);
    go->font_bbx_xoff = min_m;printf("\ngo->font_bbx_xoff is %d\n",go->font_bbx_xoff);
    go->font_bbx_yoff = min_n;printf("\ngo->font_bbx_yoff is %d\n",go->font_bbx_yoff);
    go->code_min = bc;printf("\ngo->code_min is %d\n",go->code_min);
    go->code_max = ec;printf("\ngo->code_max is %d\n",go->code_max);

    /* read glyph */
    #if 0
      fseek(fp, gptr, SEEK_SET);
    #endif

    for (  ;  ;  )
    {
      if ((instr = READ_UINT1(fp)) == GF_POST_POST)
        break;
      switch ((int)instr)
      {
      case GF_CHAR_LOC:
        code = READ_UINT1(fp);
        dx   = (double)READ_INT4(fp)/(double)(1<<16);
        dy   = (double)READ_INT4(fp)/(double)(1<<16);
        w    = READ_INT4(fp);
        ptr  = READ_INT4(fp);
        break;
      case GF_CHAR_LOC0:
        code = READ_UINT1(fp);printf("code is %d\n",code);
        dx   = (double)READ_INT1(fp);printf("dx is %d\n",dx);
        dy   = (double)0;
        w    = READ_INT4(fp);printf("w is %d\n",w);
        ptr  = READ_INT4(fp);printf("ptr is %d\n",ptr);
        break;
      default:
        error = FT_THROW( Invalid_File_Format );
        goto ErrExit;
      }

      optr = ft_ftell(fp);
      ft_fseek(fp, ptr, SEEK_SET);

      bm = &go->bm_table[code - bc];
      if (gf_read_glyph(fp, bm) < 0)
        goto ErrExit;

      bm->mv_x = dx;
      bm->mv_y = dy;
      ft_fseek(fp, optr, SEEK_SET);
    }
    *goptr          = go;
        tp = ( go->code_max );
        printf("tp go->code_max %d\n",go->code_max);
        printf("\nHi I was here in gf_load_font\n");
return error;

		ErrExit:
      printf("*ERROR\n");
      if (go != NULL)
      {
        if (go->bm_table != NULL)
        {
          for (i = 0; i < nchars; i++){}
          /*  FT_FREE(go->bm_table[i].bitmap);*/
        }
       /* FT_FREE(go->bm_table);*/ /* Returning unnecessary errors TO BE CHECKED */
      }
     /* FT_FREE(go); *//* Returning unnecessary errors TO BE CHECKED */
  }


  FT_LOCAL_DEF( void )
  gf_free_font( GF_Glyph  go, FT_Memory memory )
  {
    int i=0, nchars =sizeof(go->bm_table);
    if (go != NULL)
    {
      if (go->bm_table != NULL)
      {
        for (i = 0; i < nchars; i++){}
          /* FT_FREE(go->bm_table[i].bitmap); */ /* To be verified from Vflib */
      }
      /* FT_FREE(go->bm_table); */ /* Returning unnecessary errors TO BE CHECKED */
    }
    /* FT_FREE(go); */ /* Returning unnecessary errors TO BE CHECKED */
  }


/* END */
