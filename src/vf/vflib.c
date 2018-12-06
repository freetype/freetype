/****************************************************************************
 *
 * vflib.c
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

#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_OBJECTS_H
#include FT_SYSTEM_H
#include FT_CONFIG_CONFIG_H
#include FT_ERRORS_H
#include FT_TYPES_H

#include "vf.h"
#include "vfdrivr.h"
#include "vferror.h"


  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_vflib

  /**************************************************************************
   *
   * VF font utility functions.
   *
   */

     FT_Long     vf_read_intn( FT_Stream, FT_Int );
     FT_ULong    vf_read_uintn( FT_Stream, FT_Int );
     FT_Long     vf_get_intn( FT_Byte*, FT_Int );
     FT_ULong    vf_get_uintn( FT_Byte*, FT_Int );

#define READ_UINT1( stream )    (FT_Byte)vf_read_uintn( stream, 1 )
#define READ_UINT2( stream )    (FT_Byte)vf_read_uintn( stream, 2 )
#define READ_UINT3( stream )    (FT_Byte)vf_read_uintn( stream, 3 )
#define READ_UINT4( stream )    (FT_Byte)vf_read_uintn( stream, 4 )
#define READ_UINTN( stream, n ) (FT_ULong)vf_read_uintn( stream, n )
#define READ_INT1( stream )    (FT_String)vf_read_intn( stream, 1 )
#define READ_INT4( stream )    (FT_Long)vf_read_intn( stream, 4 )

#define GET_INT1(p)      (FT_Char)vf_get_intn((p), 1)
#define GET_UINT1(p)     (FT_Byte)vf_get_uintn((p), 1)
#define GET_INT2(p)      (FT_Int)vf_get_intn((p), 2)
#define GET_UINT2(p)     (FT_UInt)vf_get_uintn((p), 2)
#define GET_INT3(p)      (FT_Long)vf_get_intn((p), 3)
#define GET_UINT3(p)     (FT_ULong)vf_get_uintn((p), 3)
#define GET_INT4(p)      (FT_Long)vf_get_intn((p), 4)
#define GET_UINT4(p)     (FT_ULong)vf_get_uintn((p), 4)
#define GET_INTN(p,n)    (FT_Long)vf_get_intn((p), (n))
#define GET_UINTN(p,n)   (FT_ULong)vf_get_uintn((p), (n))

/*
 * Reading a Number from file
 */

  FT_ULong
  vf_read_uintn( FT_Stream stream,
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
  vf_read_intn( FT_Stream stream,
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


  FT_ULong
  vf_get_uintn( FT_Byte *p,
                FT_Int  size )
  {
    FT_ULong v;

    v = 0L;
    while (size >= 1)
    {
      v = v*256L + (FT_ULong) *(p++);
      --size;
    }

    return v;
  }

  FT_Long
  vf_get_intn( FT_Byte *p,
                   FT_Int  size )
  {
    FT_Long v;

    v = (FT_Long)*(p++) & 0xffL;
    if (v & 0x80L)
      v = v - 256L;
    --size;
    while (size >= 1)
    {
      v = v*256L + (FT_ULong) *(p++);
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
  vf_read_info(  FT_Stream    stream,
                 FT_Memory    extmemory,
                 VF           vf)
  {
    FT_Byte              id, a, l;
    FT_ULong             k, c, s, d;
    VF_SUBFONT           sf, sf0, sf_next;
    struct s_vf_subfont  subfont;
    FT_ULong             scale;
    FT_Int               fid, name_len, i;
    FT_Char              subfont_name[1024];
    FT_Error             error  = FT_Err_Ok;

    if( READ_UINT1( stream ) != VFINST_PRE )
      goto Exit;

    id = READ_UINT1( stream );

    switch (id)
    {
    case VFINST_ID_BYTE:
      break;
    default:
      goto Exit;
    }

    k = READ_UINT1( stream );
    if ( FT_STREAM_SKIP( k ) )
      goto Exit;

    vf->cs = READ_UINT4( stream );
    vf->ds = READ_UINT4( stream );

    if ((vf->cs != vf->tfm->cs) || (vf->ds != vf->tfm->ds))
    {
      error = FT_THROW( Unknown_File_Format );
      goto Exit;
    }

    vf->design_size     = (FT_ULong)(vf->ds)/(FT_ULong)(1<<20);
    vf->subfonts_opened = 1;
    vf->default_subfont = -1;

    subfont.next = NULL;

    for ( sf0 = &subfont ; ; sf0 = sf )
    {
      if( FT_ALLOC( sf, sizeof(struct s_vf_subfont) ) )
        goto Exit;

      sf0->next = sf;
      switch ( READ_UINT1( stream ) )
      {
      default:
        vf->offs_char_packet = stream->pos - 1;
        sf0->next = NULL;
        FT_FREE( sf );
        goto end_fontdef;
      case VFINST_FNTDEF1:
        k = (FT_ULong)READ_UINT1( stream );
        c = READ_UINT4( stream ); s = READ_UINT4( stream ); d = READ_UINT4( stream );
        a = READ_UINT1( stream ); l = READ_UINT1( stream );
        break;
      case VFINST_FNTDEF2:
        k = (FT_ULong)READ_UINT2( stream );
        c = READ_UINT4( stream ); s = READ_UINT4( stream ); d = READ_UINT4( stream );
        a = READ_UINT1( stream ); l = READ_UINT1( stream );
        break;
      case VFINST_FNTDEF3:
        k = (FT_ULong)READ_UINT3( stream );
        c = READ_UINT4( stream ); s = READ_UINT4( stream ); d = READ_UINT4( stream );
        a = READ_UINT1( stream ); l = READ_UINT1( stream );
        break;
      case VFINST_FNTDEF4:
        k = (FT_ULong)READ_UINT4( stream );
        c = READ_UINT4( stream ); s = READ_UINT4( stream ); d = READ_UINT4( stream );
        a = READ_UINT1( stream ); l = READ_UINT1( stream );
        break;
      }

      name_len = a + l;
      sf->k       = k;
      sf->s       = s;
      sf->d       = d;
      sf->a       = a;
      sf->l       = l;
      sf->next    = NULL;

      scale = (FT_ULong)sf->s/(FT_ULong)(1<<20);

      if ((sf->n = (char*)malloc(name_len + 1)) == NULL)
      {
        goto Exit;
      }
      for (i = 0; i < name_len; i++)
        sf->n[i] = (char)READ_UINT1( stream );
      sf->n[i] = '\0';

      /* sprintf(subfont_name, "%s", &sf->n[sf->a]);

      if (vf_debug('s'))
      {
        printf("VFlib Virtual Font: subfont %d: %s, scaled %f\n",
	     (int)sf->k, subfont_name, scale);
      }
      */  /* To add tracing*/

  end_fontdef:
    if (vf->subfonts_opened == 0)
    {
      /*
      if (open_style == TEX_OPEN_STYLE_REQUIRE)
      {
        if (vf_debug('s'))
	  printf("VFlib Virtual Font: all subfonts are required but failed\n");
        goto error_exit;
      }
      else
      {
        if (vf_debug('s'))
          printf("VFlib Virtual Font: not all fonts are opened; continue.\n");
      }
      */
    }

    vf->subfonts = subfont.next;
    return 0;

  Exit:
    for (sf = subfont.next; sf != NULL; sf = sf_next)
    {
      sf_next = sf->next;
      FT_FREE(sf->n);
      FT_FREE(sf);
    }
    vf->subfonts = NULL;
    return error;
  }


  /**************************************************************************
   *
   * DVI stack utility functions.
   *
   */

  FT_Int
  vf_dvi_stack_init(VF vf, VF_DVI_STACK stack, FT_Memory memory)
  {
    VF_DVI_STACK  top;
    if( FT_ALLOC( top, sizeof(struct s_vf_dvi_stack) ) )
        return 0;
    top->h = top->v = top->w = top->x = top->y = top->z = 0;
    top->f = vf->default_subfont;
    top->font_id = vf->subfonts->font_id;
    top->next    = NULL;
    stack->next = top;
    return 0;
  }

  FT_Int
  vf_dvi_stack_deinit(VF vf, VF_DVI_STACK stack, FT_Memory memory)
  {
    VF_DVI_STACK  elem, elem_next;
    elem = stack->next;
    while (elem != NULL)
    {
      elem_next = elem->next;
      FT_FREE(elem);
      elem = elem_next;
    }
    return 0;
  }

  FT_Int
  vf_dvi_stack_push(VF vf, VF_DVI_STACK stack)
  {
    VF_DVI_STACK  new_elem, top;
    if( FT_ALLOC( new_elem, sizeof(struct s_vf_dvi_stack) ) )
        return 0;
    top = stack->next;
    new_elem->h = top->h;
    new_elem->v = top->v;
    new_elem->w = top->w;
    new_elem->x = top->x;
    new_elem->y = top->y;
    new_elem->z = top->z;
    new_elem->f = top->f;
    new_elem->font_id = top->font_id;
    new_elem->next = top;
    stack->next = new_elem;
    return 0;
  }

  FT_Int
  vf_dvi_stack_pop(VF vf, VF_DVI_STACK stack, FT_Memory memory)
  {
    VF_DVI_STACK  top;

    top = stack->next;
    if (top == NULL)
    {
      /*Add trace and error message here*/
      /* fprintf(stderr, "VFlib warning: VF DVI stack under flow: %s\n",
	   vf->vf_path);*/
      return 0;
    }
    stack->next = top->next;
    FT_FREE(top);
    return 0;
  }


  /**************************************************************************
   *
   * DVI interpreter.
   *
   */

/* END */
