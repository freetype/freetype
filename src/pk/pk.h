/****************************************************************************
 *
 * pk.h
 *
 *   FreeType font driver for TeX's PK FONT files.
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


#ifndef PK_H_
#define PK_H_

#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_STREAM_H
#include FT_SYSTEM_H


FT_BEGIN_HEADER

#define  FONT_DRIVER_PK   1

#define  PK_PRE   247
#define  PK_ID    89
#define  PK_XXX1  240
#define  PK_XXX2  241
#define  PK_XXX3  242
#define  PK_XXX4  243
#define  PK_YYY   244
#define  PK_POST  245
#define  PK_NO_OP 246

#define toint(x)  (int)(((x)>0)?(x+0.5):(x-0.5))

/* Temporary TO BE REMOVED */

typedef  char           INT1;
typedef  unsigned char  UINT1;
typedef  int            INT2;
typedef  unsigned int   UINT2;
typedef  long           INT3;
typedef  unsigned long  UINT3;
typedef  long           INT4;
typedef  unsigned long  UINT4;


FT_END_HEADER


#endif /* PK_H_ */


/* END */
