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

  /**************************************************************************
   *
   * GF font utility functions.
   *
   */

  long           gf_read_intn(FT_Stream,int);
  unsigned long  gf_read_uintn(FT_Stream,int);
  void           gf_skip_n(FT_Stream,int);

#define READ_INT1( stream )          (INT1)gf_read_intn(( stream ), 1)
#define READ_UINT1( stream )         (UINT1)gf_read_uintn(( stream ), 1)
#define READ_INT2( stream )          (INT2)gf_read_intn(( stream ), 2)
#define READ_UINT2( stream )         (UINT2)gf_read_uintn(( stream ), 2)
#define READ_INT3( stream )          (INT3)gf_read_intn(( stream ), 3)
#define READ_UINT3( stream )         (UINT3)gf_read_uintn(( stream ), 3)
#define READ_INT4( stream )          (INT4)gf_read_intn(( stream ), 4)
#define READ_UINT4( stream )         (UINT4)gf_read_uintn(( stream ), 4)
#define READ_INTN( stream ,n)        (INT4)gf_read_intn(( stream ), (n))
#define READ_UINTN( stream ,n)       (UINT4)gf_read_uintn(( stream ), (n))
#define SKIP_N( stream ,k)           gf_skip_n(( stream ), (k))


/*
 * Reading a Number from stream
 */
  unsigned long
  gf_read_uintn(FT_Stream stream, int size)
  {
    unsigned long  v;
    FT_Error error = FT_Err_Ok;
    v = 0L;
    while (size >= 1)
    {
      v = v*256L + (unsigned long)FT_Stream_ReadULong(stream, &error);
      stream->pos-=3;
      --size;
    }
    return v;
  }

/* Preliminary */
  unsigned long
  gf_read_uint4(FT_Stream stream, int size)
  {
    unsigned long  v;
    FT_Error error = FT_Err_Ok;
    v = 0L;
    while (size >= 1)
    {
      v = v*256L + (unsigned long)FT_Stream_ReadULong(stream, &error);
      --size;
    }
    return v;
  }


  long
  gf_read_intn(FT_Stream stream, int size)
  {
    long           v;
    FT_Error error = FT_Err_Ok;
    v = (long)FT_Stream_ReadULong(stream, &error) & 0xffL;
    if (v & 0x80L)
      v = v - 256L;
    --size;
    while (size >= 1)
    {
      v = v*256L + (unsigned long)FT_Stream_ReadULong(stream, &error);
      --size;
		}
    return v;
  }

  void
  gf_skip_n(FT_Stream stream, int size)
  {
    FT_Stream_Skip(stream, (FT_Long)size );
    #if 0
    while (size > 0)
    {
      (void)getc(fp);
      --size;
    }
    #endif
  }

  /**************************************************************************
   *
   * API.
   *
   */

  FT_LOCAL_DEF( FT_Error )
  gf_read_glyph(FT_Stream stream, GF_Bitmap bm)
  {
    long           m, n;
    int            paint_sw;
    int            instr;
    INT4           min_m, max_m, min_n, max_n, del_m, del_n;
    long           w, h, d;
    int            m_b, k;
    unsigned char  *ptr;
    FT_Error        error  = FT_Err_Ok;

    switch (READ_UINT1( stream ))
    {
    case GF_BOC:
      SKIP_N( stream , 4);
      SKIP_N( stream , 4);
      min_m = READ_INT4( stream );
      max_m = READ_INT4( stream );
      min_n = READ_INT4( stream );
      max_n = READ_INT4( stream );
      break;
    case GF_BOC1:
      SKIP_N( stream , 1);
      del_m = (INT4)READ_UINT1( stream );
      max_m = (INT4)READ_UINT1( stream );
      del_n = (INT4)READ_UINT1( stream );
      max_n = (INT4)READ_UINT1( stream );
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
    while ((instr = (int)READ_UINT1( stream )) != GF_EOC)
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
          d = (UINT4)READ_UINTN( stream, (instr - GF_PAINT1 + 1));
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
          n = n - (UINT4)READ_UINTN( stream, (instr - GF_SKIP1 + 1)) - 1;
          paint_sw = 0;
          break;
        case GF_XXX1:
        case GF_XXX2:
        case GF_XXX3:
        case GF_XXX4:
          k = READ_UINTN( stream, instr - GF_XXX1 + 1);
          SKIP_N( stream, k);
          break;
        case GF_YYY:
          SKIP_N( stream, 4);
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

    go = NULL;
    nchars = -1;
printf("\nHi I am here in gf_load_font -1\n\n");
    /* seek to post_post instr. */
    /* fseek(fp, -5, SEEK_END); */
    FT_STREAM_SEEK( stream->size - 5 );

printf("\nHi I am here in gf_load_font 0\n\n");
    while ((d = READ_UINT1( stream )) == 223)
    {
      FT_STREAM_SEEK( stream->pos -2 );
      /* fseek(fp, -2, SEEK_CUR); */
    }

printf("\nHi I am here in gf_load_font 1\n\n");
    if (d != GF_ID)
    {
      error = FT_THROW( Invalid_File_Format );
      goto ErrExit;
    }

printf("\nHi I am here in gf_load_font 2\n\n");
    /* fseek(fp, -6, SEEK_CUR); */
    FT_STREAM_SEEK( stream->pos -6 );

printf("\nHi I am here in gf_load_font 3\n\n");
    /* check if the code is post_post */
    if (READ_UINT1( stream ) != GF_POST_POST)
    {
      error = FT_THROW( Invalid_File_Format );
      goto ErrExit;
    }

printf("\nHi I am here in gf_load_font 4\n\n");
    /* read pointer to post instr. */
    if ((ptr_post = gf_read_uint4( stream,4 )) == -1)
    {
      error = FT_THROW( Invalid_File_Format );
      goto ErrExit;
    }

printf("\nHi I am here in gf_load_font 5\n\n");
    /* goto post instr. and read it */
    /* fseek(fp, ptr_post, SEEK_SET); */
    FT_STREAM_SEEK( ptr_post );

    if (d=READ_UINT1( stream ) != GF_POST)
    {
      error = FT_THROW( Invalid_File_Format );
      goto ErrExit;
    }

printf("\nHi I am here in gf_load_font 6\n\n");
    ptr_p     = READ_UINT4( stream );
    ds        = READ_UINT4( stream );
    check_sum = READ_UINT4( stream );
    hppp      = READ_UINT4( stream );
    vppp      = READ_UINT4( stream );
    min_m     = READ_INT4( stream );
    max_m     = READ_INT4( stream );
    min_n     = READ_INT4( stream );
    max_n     = READ_INT4( stream );
printf("\nHi I am here in gf_load_font 7\n\n");
    #if 0
      gptr = ftell(fp);
    #endif

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

    go->ds   = (double)ds/(1<<20);
    go->hppp = (double)hppp/(1<<16);
    go->vppp = (double)vppp/(1<<16);
    go->font_bbx_w = max_m - min_m;
    go->font_bbx_h = max_n - min_n;
    go->font_bbx_xoff = min_m;
    go->font_bbx_yoff = min_n;
    go->code_min = bc;
    go->code_max = ec;

    /* read glyph */
    #if 0
      fseek(fp, gptr, SEEK_SET);
    #endif

    for (  ;  ;  )
    {
      if ((instr = READ_UINT1( stream )) == GF_POST_POST)
        break;
      switch ((int)instr)
      {
      case GF_CHAR_LOC:
        code = READ_UINT1( stream );
        dx   = (double)READ_INT4( stream )/(double)(1<<16);
        dy   = (double)READ_INT4( stream )/(double)(1<<16);
        w    = READ_INT4( stream );
        ptr  = READ_INT4( stream );
        break;
      case GF_CHAR_LOC0:
        code = READ_UINT1( stream );
        dx   = (double)READ_INT1( stream );
        dy   = (double)0;
        w    = READ_INT4( stream );
        ptr  = READ_INT4( stream );
        break;
      default:
        error = FT_THROW( Invalid_File_Format );
        goto ErrExit;
      }

      /* optr = ft_ftell(fp); */
      optr = stream->pos;
      /* ft_fseek(fp, ptr, SEEK_SET); */
      FT_STREAM_SEEK( ptr );

      bm = &go->bm_table[code - bc];
      if (gf_read_glyph( stream, bm ) < 0)
        goto ErrExit;

      bm->mv_x = dx;
      bm->mv_y = dy;
      /* ft_fseek(fp, optr, SEEK_SET); */
      FT_STREAM_SEEK( ptr );
    }

    *goptr          = go;
    tp = ( go->code_max );
    printf("tp go->code_max %d\n",go->code_max);
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
  gf_free_font( GF_Glyph  go )
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
