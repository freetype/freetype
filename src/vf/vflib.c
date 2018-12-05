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

   /* TO-DO */

/* END */
