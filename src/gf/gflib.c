/****************************************************************************
 *
 * gflib.c
 *
 *   FreeType font driver for METAFONT GF FONT files
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

FT_Byte  bit_table[] = {
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

  typedef struct GF_CharOffsetRec_
  {
    FT_Long         char_offset;
    FT_UShort       code;
    FT_Short        gid;

  } GF_CharOffsetRec, *GF_CharOffset;

  /**************************************************************************
   *
   * GF font utility functions.
   *
   */

  FT_Long     gf_read_intn ( FT_Stream, FT_Int );
  FT_ULong    gf_read_uintn( FT_Stream, FT_Int );

#define READ_UINT1( stream )    (FT_Byte)gf_read_uintn( stream, 1 )
#define READ_UINT2( stream )    (FT_Byte)gf_read_uintn( stream, 2 )
#define READ_UINT3( stream )    (FT_Byte)gf_read_uintn( stream, 3 )
#define READ_UINT4( stream )    (FT_Byte)gf_read_uintn( stream, 4 )
#define READ_UINTN( stream, n ) (FT_ULong)gf_read_uintn( stream, n )
#define READ_INT1( stream )    (FT_String)gf_read_intn( stream, 1 )
#define READ_INT4( stream )    (FT_Long)gf_read_intn( stream, 4 )

/*
 * Reading a Number from file
 */

  FT_ULong
  gf_read_uintn( FT_Stream stream,
		 FT_Int size )
  {
    FT_ULong  v,k;
    FT_Error  error;
    FT_Byte   tp;

    v = 0L;

    while ( size >= 1 )
    {
      if ( FT_READ_BYTE(tp) )
        return 0;
      k = (FT_ULong) tp;
      v = v*256L + k;
      --size;
    }
    return v;
  }


  FT_Long
  gf_read_intn( FT_Stream stream,
		FT_Int size )
  {
    FT_Long   v;
    FT_Byte   tp;
    FT_Error  error;
    FT_ULong  z;

    if ( FT_READ_BYTE(tp) )
      return 0;
    z = (FT_ULong) tp;
    v = (FT_Long) z & 0xffL;

    if( v & 0x80L )
      v = v - 256L;
    --size;

    while ( size >= 1 )
    {
      if ( FT_READ_BYTE(tp) )
        return 0;
      z = (FT_ULong) tp;
      v = v*256L + z;
      --size;
    }
    return v;
  }


  static int
  compare( FT_Long*  a,
           FT_Long*  b )
  {
    if ( *a < *b )
      return -1;
    else if ( *a > *b )
      return 1;
    else
      return 0;
  }


  /**************************************************************************
   *
   * API.
   *
   */


  static FT_Error
  gf_set_encodings( GF_CharOffset of,
                    FT_Int        ngphs,
                    GF_Glyph      go,
                    FT_Memory     memory )
  {
    FT_Error      error;
    FT_ULong      nencoding;
    FT_Int        i, j;
    FT_ULong      k;
    GF_Encoding   encoding = NULL;
    FT_Long       *tosort;

    nencoding = ngphs;

    if ( FT_NEW_ARRAY( encoding, nencoding ) )
      return error;

    if ( FT_NEW_ARRAY( tosort, nencoding ) )
      return error;

    for ( i = 0 ; i < ngphs ; i++ )
    {
      if ( of[i].char_offset >= 0 )
        tosort[i] = of[i].char_offset;
    }

    ft_qsort( (void*)tosort, ngphs, sizeof(FT_Long),
              (int(*)(const void*, const void*))compare );

    k = 0;
    for ( i = 0; i < ngphs; i++ )
    {
      for ( j = 0; j < ngphs; j++ )
      {
        if ( of[j].char_offset == tosort[i] )
          break;
      }
      encoding[k].enc   = of[j].code;
      encoding[k].glyph = k;
      of[j].gid         = k;
      k++;
    }

    FT_FREE(tosort);

    go->nencodings = k;
    go->encodings  = encoding;

    return error;
  }

  FT_LOCAL_DEF( FT_Error )
  gf_read_glyph( FT_Stream    stream,
                 GF_Bitmap    bm,
                 FT_Memory    memory )
  {
    FT_Long   m, n;
    FT_Int    paint_sw;
    FT_Int    instr,inst;
    FT_Long   min_m, max_m, min_n, max_n, del_m, del_n;
    FT_Long   w, h, d;
    FT_Int    m_b, k;
    FT_Byte   *ptr;
    FT_Error  error  = FT_Err_Ok;

    for (  ;  ;  )
    {
      inst = READ_UINT1( stream );
      switch ( (FT_Int) inst )
      {
      case GF_BOC:
        if ( FT_STREAM_SKIP( 4 ) )
          return -1;
        if ( FT_STREAM_SKIP( 4 ) )
          return -1;
        min_m = READ_INT4( stream );
        max_m = READ_INT4( stream );
        min_n = READ_INT4( stream );
        max_n = READ_INT4( stream );
        goto BOC;
        break;
      case GF_BOC1:
        if ( FT_STREAM_SKIP( 1 ) )
          return -1;
        del_m = (FT_Long)READ_UINT1( stream );
        max_m = (FT_Long)READ_UINT1( stream );
        del_n = (FT_Long)READ_UINT1( stream );
        max_n = (FT_Long)READ_UINT1( stream );
        min_m = max_m - del_m;
        min_n = max_n - del_n;
        goto BOC;
        break;
      case GF_XXX1:
        k = (FT_ULong)READ_UINT1( stream );
        if ( FT_STREAM_SKIP( k ) )
          return -1;
        break;
      case GF_XXX2:
        k = (FT_ULong)READ_UINT2( stream );
        if ( FT_STREAM_SKIP( k ) )
          return -1;
        break;
      case GF_XXX3:
        k = (FT_ULong)READ_UINT3( stream );
        if ( FT_STREAM_SKIP( k ) )
          return -1;
        break;
      case GF_XXX4:
        k = (FT_ULong)READ_UINT4( stream );
        if ( FT_STREAM_SKIP( k ) )
          return -1;
        break;
      case GF_YYY :
        if ( FT_STREAM_SKIP( 4 ) )
          return -1;
        break;
      case GF_NO_OP:
        break;
      default:
        return -1;
      }
    }
    return 0;

  BOC:
    if ( error != FT_Err_Ok )
      return -1;
    w = max_m - min_m + 1;
    h = max_n - min_n + 1;
    if ( (w < 0) || (h < 0) )
    {
      FT_ERROR(( "gf_read_glyph: invalid w and h values\n" ));
      error = FT_THROW( Invalid_File_Format );
      return -1;
    }

    /* allocate and build bitmap */
    if ( (bm->bitmap = (FT_Byte*)malloc(h*((w+7)/8))) == NULL )
    {
      error = FT_THROW( Invalid_File_Format );
      return -1;
    }

    memset(bm->bitmap, 0, h*((w+7)/8));
    bm->raster     = (FT_UInt)(w+7)/8;
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
    while ( (instr = (FT_Int)READ_UINT1( stream )) != GF_EOC )
    {
      if ( instr == GF_PAINT_0 )
      {
        paint_sw = 1 - paint_sw;
      }
      else if ( (GF_NEW_ROW_0 <= instr) && (instr <= GF_NEW_ROW_164) )
      {
        m        = min_m + (instr - GF_NEW_ROW_0);
        n        = n - 1;
        paint_sw = 1;
      }
      else if ( (GF_PAINT_1 <= instr) && (instr <= GF_PAINT_63) )
      {
        d = instr - GF_PAINT_1 + 1;
        goto Paint;
      }
      else
      {
        switch ( (FT_Int)instr )
        {
        case GF_PAINT1:
        case GF_PAINT2:
        case GF_PAINT3:
          d = (FT_ULong)READ_UINTN( stream, (instr - GF_PAINT1 + 1));
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
          n = n - (FT_ULong)READ_UINTN( stream, (instr - GF_SKIP1 + 1)) - 1;
          paint_sw = 0;
          break;
        case GF_XXX1:
        case GF_XXX2:
        case GF_XXX3:
        case GF_XXX4:
          k = READ_UINTN( stream, instr - GF_XXX1 + 1);
          if ( FT_STREAM_SKIP( k ) )
            return -1;
          break;
        case GF_YYY:
          if ( FT_STREAM_SKIP( 4 ) )
            return -1;
          break;
        case GF_NO_OP:
          break;
        default:
          FT_FREE( bm->bitmap );
          bm->bitmap = NULL;
          error = FT_THROW( Invalid_File_Format );
          return -1;
         }
      }
    }
  return 0;
  }


  FT_LOCAL_DEF( FT_Error )
  gf_load_font(  FT_Stream    stream,
                 FT_Memory    extmemory,
                 GF_Glyph     *goptr  )
  {
    GF_Glyph        go;
    GF_Bitmap       bm;
    GF_CharOffset   of;
    FT_Byte         instr, d, pre, id, k, code;
    FT_Long         ds, check_sum, hppp, vppp;
    FT_Long         min_m, max_m, min_n, max_n, w;
    FT_UInt         dx, dy;
    FT_Long         ptr_post, ptr_p, ptr, optr, rptr;
    FT_Int          bc, ec, nchars, i, ngphs, idx;
    FT_Error        error  = FT_Err_Ok;
    FT_Memory       memory = extmemory; /* needed for FT_NEW */

    go = NULL;
    nchars = -1;
    idx = 0;

    if( FT_STREAM_SEEK( 0 ) )
      goto Exit;

    pre = READ_UINT1( stream );
    if ( pre != GF_PRE )
    {
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    id = READ_UINT1( stream );
    if ( id != GF_ID )
    {
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    k = READ_UINT1( stream );
    if ( FT_STREAM_SKIP( k ) )
      goto Exit;

    /* seek to post_post instr. */
    if( FT_STREAM_SEEK( stream->size - 1 ) )
      goto Exit;

    if( FT_STREAM_SEEK( stream->size - 1 ) )
      goto Exit;

    while ( READ_UINT1( stream ) == 223)
    {
      if( FT_STREAM_SEEK( stream->pos -2 ) )
        goto Exit;
    }

    if( FT_STREAM_SEEK( stream->pos -1 ) )
        goto Exit;
    d= READ_UINT1( stream );

    if ( d != GF_ID )
    {
      FT_ERROR(( "gf_load_font: missing GF_ID(131) field\n" ));
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    FT_TRACE2(( "gf_load_font: GF_ID(131) found\n" ));

    if( FT_STREAM_SEEK( stream->pos -6 ) )
      goto Exit;

    /* check if the code is post_post */
    if ( READ_UINT1( stream ) != GF_POST_POST )
    {
      FT_ERROR(( "gf_load_font: missing GF_POST_POST(249) field\n" ));
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    FT_TRACE2(( "gf_load_font: GF_POST_POST(249) found\n" ));

    /* read pointer to post instr. */
    if( FT_READ_ULONG( ptr_post ) )
      goto Exit;

    if ( ptr_post == -1 )
    {
      FT_ERROR(( "gf_load_font: invalid postamble pointer\n" ));
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    /* goto post instr. and read it */
    if( FT_STREAM_SEEK( ptr_post ) )
      goto Exit;

    if ( READ_UINT1( stream ) != GF_POST )
    {
      FT_ERROR(( "gf_load_font: missing GF_POST(248) field\n" ));
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    FT_TRACE2(( "gf_load_font: GF Postamble found\n" ));

    if ( FT_READ_ULONG( ptr_p ) )
      goto Exit;
    if ( FT_READ_ULONG( ds ) )
      goto Exit;
    if ( FT_READ_ULONG( check_sum ) )
      goto Exit;
    if ( FT_READ_ULONG( hppp ) )
      goto Exit;
    if ( FT_READ_ULONG( vppp ) )
      goto Exit;

    min_m     = READ_INT4( stream );
    max_m     = READ_INT4( stream );
    min_n     = READ_INT4( stream );
    max_n     = READ_INT4( stream );

    FT_TRACE5(( "gf_load_font: checksum is %ld\n",check_sum ));

    if ( ptr_p < 0 )     /* Defined to use ptr_p */
    {
      FT_ERROR(( "gf_load_font: invalid pointer in postamble\n" ));
      goto Exit;
    }

    if ( check_sum < 0 ) /* Defined to use check_sum */
    {
      FT_ERROR(( "gf_load_font: invalid check sum value\n" ));
      goto Exit;
    }

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
        if ( instr == GF_POST_POST )
        {
          break;
        }
        else if ( instr == GF_CHAR_LOC )
        {
          code = READ_UINT1(fp);
          (void)SKIP_N(fp, 16);
        }
        else if ( instr == GF_CHAR_LOC0 )
        {
          code = READ_UINT1(fp);
          (void)SKIP_N(fp, 9);
        }
        else
        {
          error = FT_THROW( Invalid_File_Format );
          goto Exit;
        }
        if ( code < bc )
          bc = code;
        if ( code > ec )
          ec = code;
      }
    #else
      bc = 0;
      ec = 255;
    #endif

    nchars = ec - bc + 1;

    if( FT_ALLOC( go, sizeof(GF_GlyphRec) ) )
      goto Exit;

    FT_TRACE2(( "gf_load_font: Allocated bitmap table\n" ));

    go->ds             = (FT_UInt)ds/(1<<20);
    go->hppp           = (FT_UInt)hppp/(1<<16);
    go->vppp           = (FT_UInt)vppp/(1<<16);
    go->font_bbx_w     = max_m - min_m;
    go->font_bbx_h     = max_n - min_n;
    go->font_bbx_xoff  = min_m;
    go->font_bbx_yoff  = min_n;
    go->code_min       = bc;
    go->code_max       = ec;

    go->nglyphs = 0;

    if( FT_ALLOC_MULT( of, sizeof(GF_CharOffsetRec), nchars ) )
      goto Exit;

    rptr = stream->pos;
    i    =0;
    ngphs=0;

    for (  ;  ;  )
    {
      if ( (instr = READ_UINT1( stream )) == GF_POST_POST )
        break;
      switch ( (FT_Int)instr )
      {
      case GF_CHAR_LOC:
        code = READ_UINT1( stream );
        dx   = (FT_UInt)READ_INT4( stream )/(FT_UInt)(1<<16);
        dy   = (FT_UInt)READ_INT4( stream )/(FT_UInt)(1<<16);
        w    = READ_INT4( stream );
        ptr  = READ_INT4( stream );
        break;
      case GF_CHAR_LOC0:
        code = READ_UINT1( stream );
        dx   = (FT_UInt)READ_INT1( stream );
        dy   = (FT_UInt)0;
        w    = READ_INT4( stream );
        ptr  = READ_INT4( stream );
        break;
      default:
        FT_ERROR(( "gf_load_font: missing character locators in postamble\n" ));
        error = FT_THROW( Unknown_File_Format );
        goto Exit;
      }

      of[i].char_offset = (FT_ULong) ptr;
      of[i].code        = (FT_UShort) code;
      of[i].gid         = -1;
      ngphs            += 1;
      i++;
    }

    if( FT_ALLOC_MULT( go->bm_table, sizeof(GF_BitmapRec), ngphs ) )
      goto Exit;

    error = gf_set_encodings( of, ngphs, go, memory );
    if( error )
      goto Exit;

    if( FT_STREAM_SEEK( rptr ) )
        goto Exit;

    for (  ;  ;  )
    {
      if ( (instr = READ_UINT1( stream )) == GF_POST_POST )
        break;
      switch ( (FT_Int) instr )
      {
      case GF_CHAR_LOC:
        code = READ_UINT1( stream );
        dx   = (FT_UInt)READ_INT4( stream )/(FT_UInt)(1<<16);
        dy   = (FT_UInt)READ_INT4( stream )/(FT_UInt)(1<<16);
        w    = READ_INT4( stream );
        ptr  = READ_INT4( stream );
        break;
      case GF_CHAR_LOC0:
        code = READ_UINT1( stream );
        dx   = (FT_UInt)READ_INT1( stream );
        dy   = (FT_UInt)0;
        w    = READ_INT4( stream );
        ptr  = READ_INT4( stream );
        break;
      default:
        FT_ERROR(( "gf_load_font: missing character locators in postamble\n" ));
        error = FT_THROW( Unknown_File_Format );
        goto Exit;
      }

      optr = stream->pos;
      if ( FT_STREAM_SEEK( ptr ) )
        goto Exit;

      for ( i = 0; i < 256; i++ )
      {
        if( of[i].code == code && of[i].char_offset == ptr )
        {
          idx = of[i].gid;
          break;
        }
      }

      bm = &go->bm_table[idx];

      bm->mv_x     = dx;
      bm->mv_y     = dy;
      bm->code     = code;
      go->nglyphs += 1;

      if ( gf_read_glyph( stream, bm, memory ) < 0 )
        goto Exit;

      if ( FT_STREAM_SEEK( optr ) )
        goto Exit;
    }

    FT_FREE(of);
    *goptr = go;
    return error;

  Exit:
    if ( go != NULL )
    {
      if ( go->bm_table )
      {
        for ( i = 0; i < nchars; i++ )
          FT_FREE(go->bm_table[i].bitmap);
      }
      FT_FREE( go->bm_table );
      FT_FREE( go );
    }
    return error;
  }


  FT_LOCAL_DEF( void )
  gf_free_font( GF_Face face )
  {
    FT_Memory  memory = FT_FACE( face )->memory;
    GF_Glyph   go     = face->gf_glyph;
    FT_UInt    nchars = FT_FACE( face )->num_glyphs,i;

    if ( !go )
      return;

    if( go->bm_table )
    {
      for ( i = 0; i < nchars; i++ )
        FT_FREE(go->bm_table[i].bitmap);
    }
    FT_FREE( go->bm_table );
    FT_FREE( go );
  }


/* END */
