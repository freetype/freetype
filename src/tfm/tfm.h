/****************************************************************************
 *
 * tfm.h
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


#ifndef TFM_H_
#define TFM_H_

#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_STREAM_H
#include FT_SYSTEM_H


FT_BEGIN_HEADER

/* Temporary TO BE REMOVED */

/* IMPORTANT ASSUMPTION:
 *   char:  at least 8 bits
 *   int:   at least 16 bits
 *   long:  at least 32 bits
 */
typedef  char           INT1;
typedef  unsigned char  UINT1;
typedef  int            INT2;
typedef  unsigned int   UINT2;
typedef  long           INT3;
typedef  unsigned long  UINT3;
typedef  long           INT4;
typedef  unsigned long  UINT4;

FT_END_HEADER


#endif /* TFM_H_ */


/* END */
