/****************************************************************************
 *
 * tfmobjs.c
 *
 *   FreeType auxiliary TFM module.
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
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_TFM_H

#include "tfmobjs.h"
#include "tfmmod.h"
#include "tfmerr.h"





  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_tfmobjs

   /**************************************************************************
   *
   * Global TFM parameters.
   *
   */
#define tfm_size  30000 /* maximum length of tfm data, in bytes */
#define lig_size  5000  /* maximum length of lig kern program, in words */
#define hash_size 5003

  /**************************************************************************
   *
   * TFM font utility functions.
   *
   */

  long           tfm_read_intn(FT_Stream,int);
  unsigned long  tfm_read_uintn(FT_Stream,int);

#define READ_UINT1( stream )    (FT_ULong)tfm_read_uintn( stream, 1)
#define READ_UINT2( stream )    (FT_ULong)tfm_read_uintn( stream, 2)
#define READ_UINT4( stream )    (FT_ULong)tfm_read_uintn( stream, 4)
#define READ_INT4( stream )     (FT_Long)tfm_read_intn( stream, 4)

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
        return 0;
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
        return 0;
    z= (unsigned long)tp;
    v = (long)z & 0xffL;
    if (v & 0x80L)
      v = v - 256L;
    --size;
    while (size >= 1)
    {
      if ( FT_READ_BYTE(tp) )
        return 0;
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

  FT_LOCAL_DEF( FT_Error )
  tfm_init( TFM_Parser  parser,
            FT_Memory   memory,
            FT_Stream   stream )
  {
    parser->memory    = memory;
    parser->stream    = stream;
    parser->FontInfo  = NULL;
    parser->user_data = NULL;

    return FT_Err_Ok;
  }


  FT_LOCAL( void )
  tfm_close( TFM_Parser  parser )
  {
    FT_Memory  memory = parser->memory;

    FT_FREE( parser->stream );
  }


  FT_LOCAL_DEF( FT_Error )
  tfm_parse_metrics( TFM_Parser  parser )
  {
    FT_Memory     memory = parser->memory;
    TFM_FontInfo  fi     = parser->FontInfo;
    FT_Stream     stream = parser->stream;
    FT_Error      error  = FT_ERR( Syntax_Error );

    FT_ULong      lf, lh, nc, nci;
    FT_ULong      offset_char_info, offset_param;
    FT_ULong      nw, nh, nd, ni, nl, nk, ne, np, bc, ec;

    FT_Long       *w,  *h,  *d;
    FT_ULong      *ci, v;

    FT_ULong      i;
    FT_Long       bbxw, bbxh, xoff, yoff;

    if ( !fi )
      return FT_THROW( Invalid_Argument );

    fi->width  = NULL;
    fi->height = NULL;
    fi->depth  = NULL;
    ci         = NULL;
    w          = NULL;
    h          = NULL;
    d          = NULL;

    fi->font_bbx_w = 0.0;
    fi->font_bbx_h = 0.0;
    fi->font_bbx_xoff = 0.0;
    fi->font_bbx_yoff = 0.0;

    if( FT_STREAM_SEEK( 0 ) )
      return error;

    /* Checking the correctness of the TFM file */
    if( READ_UINT1( stream ) > 127 )
    {
      FT_ERROR(( "Malformed TFM file: The first byte of the input file exceeds 127!\n" ));
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    if( FT_STREAM_SEEK( 0 ) )
      return error;

    lf  = READ_UINT2( stream );
    lh  = READ_UINT2( stream );
    bc  = READ_UINT2( stream );
    ec  = READ_UINT2( stream );
    nw  = READ_UINT2( stream );
    nh  = READ_UINT2( stream );
    nd  = READ_UINT2( stream );
    ni  = READ_UINT2( stream );
    nl  = READ_UINT2( stream );
    nk  = READ_UINT2( stream );
    ne  = READ_UINT2( stream );
    np  = READ_UINT2( stream );

    /* Uncomment this to check for the tfm file's header info if this program returns malformed tfm file */
    /*
    FT_TRACE6(( "tfm_parse_metrics: First 24 bytes in the tfm file:\n"
                    "                  lf : %ld\n"
                    "                  lh : %ld\n"
                    "                  bc : %d\n"
                    "                  ec : %d\n"
                    "                  nw : %d\n"
                    "                  nh : %d\n"
                    "                  nd : %d\n"
                    "                  ni : %d\n"
                    "                  nl : %d\n"
                    "                  nk : %d\n"
                    "                  ne : %d\n"
                    "                  np : %d\n", lf, lh, bc, ec, nw, nh, nd, ni, nl, nk, ne, np ));
    */

    if( lf == 0 || ((4*lf) - 1) > tfm_size)
    {
      FT_ERROR(( "Malformed TFM file: The file claims to have length zero, but that's impossible!\n" ));
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    if(lh < 2)
    {
      FT_ERROR(( "Malformed TFM file: The header length is only %ld\n",lh ));
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    if( nl > lig_size )
    {
      FT_ERROR(( "Malformed TFM file: The lig/kern program is longer than I can handle!\n" ));
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    if( ne > 256 )
    {
      FT_ERROR(( "Malformed TFM file: There are %ld extensible recipes!\n",ne ));
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    if ( ((signed)(fi->begin_char-1) > (signed)fi->end_char) ||
       ( fi->end_char > 255) ||
       ( ne > 256 ) )
    {
      FT_ERROR(( "tfm_parse_metrics: Incorrect header information in `tfm' file.\n" ));
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    if ( lf != 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np )
    {
      FT_ERROR(( "tfm_parse_metrics: Incorrect header information in `tfm' file.\n" ));
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    fi->begin_char  = bc;
    fi->end_char    = ec;
    fi->cs          = READ_INT4( stream ); /* Check Sum  */
    fi->ds          = READ_INT4( stream ); /* Design Size */
    fi->design_size = (FT_ULong)((double)(fi->ds)/(double)(1<<20));

    printf("fi->cs is %ld\n",fi->cs);
    if( fi->cs <= 0 )
    {
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    nc  = fi->end_char - fi->begin_char + 1;
    nci = nc;

    ci = (FT_ULong*)calloc(nci, sizeof(FT_ULong));
    w  = (FT_Long*)calloc(nw,  sizeof(FT_ULong));
    h  = (FT_Long*)calloc(nh,  sizeof(FT_ULong));
    d  = (FT_Long*)calloc(nd,  sizeof(FT_ULong));

    if ((ci == NULL) || (w == NULL) || (h == NULL) || (d == NULL))
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    offset_char_info = 4*(6+lh);
    if( FT_STREAM_SEEK( offset_char_info ) ) /* Skip over coding scheme and font family name */
      goto Exit;

    for (i = 0; i < nci; i++)
      ci[i] = READ_UINT4( stream );

    offset_param = stream->pos + 4*(nw + nh + nd + ni + nl + nk + ne);

    for (i = 0; i < nw; i++)
      w[i] = READ_INT4( stream );
    for (i = 0; i < nh; i++)
      h[i] = READ_INT4( stream );
    for (i = 0; i < nd; i++)
      d[i] = READ_INT4( stream );

    fi->width  = (FT_Long*)calloc(nc, sizeof(FT_Long));
    fi->height = (FT_Long*)calloc(nc, sizeof(FT_Long));
    fi->depth  = (FT_Long*)calloc(nc, sizeof(FT_Long));

    if ((fi->width == NULL) || (fi->height == NULL) || (fi->depth == NULL))
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    bbxw = 0;
    bbxh = 0;
    xoff = 0;
    yoff = 0;

    for (i = 0; i < nc; i++)
    {
      v = ci[i] / 0x10000L;
      fi->depth[i]  = d[v & 0xf];  v >>= 4;
      fi->height[i] = h[v & 0xf];  v >>= 4;
      fi->width[i]  = w[v & 0xff];

      if (bbxw < fi->width[i])
	      bbxw = fi->width[i];

      if (bbxh < (fi->height[i] + fi->depth[i]))
	      bbxh = fi->height[i] + fi->depth[i];

      if (yoff > -fi->depth[i])
	      yoff = -fi->depth[i];
    }

    fi->font_bbx_w = (FT_ULong)(fi->design_size * ((double)bbxw / (double)(1<<20)));
    fi->font_bbx_h = (FT_ULong)(fi->design_size * ((FT_ULong)bbxh / (double)(1<<20)));
    fi->font_bbx_xoff = (FT_ULong)(fi->design_size * ((double)xoff / (double)(1<<20)));
    fi->font_bbx_yoff = (FT_ULong)(fi->design_size * ((double)yoff / (double)(1<<20)));

    if( FT_STREAM_SEEK( offset_param ) )
      return error;
    if (FT_READ_ULONG(fi->slant) )
      return error;
    fi->slant = (FT_ULong)((double)fi->slant/(double)(1<<20));

  Exit:
    if( !ci || !w || !h || !d )
    {
      FT_FREE(ci);
      FT_FREE(w);
      FT_FREE(h);
      FT_FREE(d);
    }
    return error;
  }


/* END */
