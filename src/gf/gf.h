/****************************************************************************
 *
 * gf.h
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


#ifndef GF_H_
#define GF_H_


#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_STREAM_H
#include FT_SYSTEM_H


FT_BEGIN_HEADER

#define  FONT_DRIVER_GF   1

#define  GF_PRE          247
#define  GF_ID           131
#define  GF_POST         248
#define  GF_CHAR_LOC     245
#define  GF_CHAR_LOC0    246
#define  GF_POST_POST    249

#define  GF_PAINT_0        0
#define  GF_PAINT_1        1
#define  GF_PAINT_63      63
#define  GF_PAINT1        64
#define  GF_PAINT2        65
#define  GF_PAINT3        66
#define  GF_BOC           67
#define  GF_BOC1          68
#define  GF_EOC           69
#define  GF_SKIP0         70
#define  GF_SKIP1         71
#define  GF_SKIP2         72
#define  GF_SKIP3         73
#define  GF_NEW_ROW_0     74
#define  GF_NEW_ROW_164  238
#define  GF_XXX1         239
#define  GF_XXX2         240
#define  GF_XXX3         241
#define  GF_XXX4         242
#define  GF_YYY          243
#define  GF_NO_OP        244


typedef  char           INT1;
typedef  unsigned char  UINT1;
typedef  int            INT2;
typedef  unsigned int   UINT2;
typedef  long           INT3;
typedef  unsigned long  UINT3;
typedef  long           INT4;
typedef  unsigned long  UINT4;

FT_END_HEADER


#endif /* GF_H_ */


/* END */
