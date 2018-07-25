/****************************************************************************
 *
 * vf.h
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


#ifndef VF_H_
#define VF_H_

#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_STREAM_H
#include FT_SYSTEM_H


FT_BEGIN_HEADER


#define  VFINST_ID_BYTE           202
#define  VFINST_CP_SHORT_CHAR0      0
#define  VFINST_CP_SHORT_CHAR241  241
#define  VFINST_CP_LONG_CHAR      242

#define  VFINST_SETCHAR0        0
#define  VFINST_SETCHAR127    127
#define  VFINST_SET1          128
#define  VFINST_SET2          129
#define  VFINST_SET3          130
#define  VFINST_SET4          131
#define  VFINST_SETRULE       132
#define  VFINST_PUT1          133
#define  VFINST_PUT2          134
#define  VFINST_PUT3          135
#define  VFINST_PUT4          136
#define  VFINST_PUTRULE       137
#define  VFINST_NOP           138
#define  VFINST_PUSH          141
#define  VFINST_POP           142
#define  VFINST_RIGHT1        143
#define  VFINST_RIGHT2        144
#define  VFINST_RIGHT3        145
#define  VFINST_RIGHT4        146
#define  VFINST_W0            147
#define  VFINST_W1            148
#define  VFINST_W2            149
#define  VFINST_W3            150
#define  VFINST_W4            151
#define  VFINST_X0            152
#define  VFINST_X1            153
#define  VFINST_X2            154
#define  VFINST_X3            155
#define  VFINST_X4            156
#define  VFINST_DOWN1         157
#define  VFINST_DOWN2         158
#define  VFINST_DOWN3         159
#define  VFINST_DOWN4         160
#define  VFINST_Y0            161
#define  VFINST_Y1            162
#define  VFINST_Y2            163
#define  VFINST_Y3            164
#define  VFINST_Y4            165
#define  VFINST_Z0            166
#define  VFINST_Z1            167
#define  VFINST_Z2            168
#define  VFINST_Z3            169
#define  VFINST_Z4            170
#define  VFINST_FNTNUM0       171
#define  VFINST_FNTNUM63      234
#define  VFINST_FNT1          235
#define  VFINST_FNT2          236
#define  VFINST_FNT3          237
#define  VFINST_FNT4          238
#define  VFINST_XXX1          239
#define  VFINST_XXX2          240
#define  VFINST_XXX3          241
#define  VFINST_XXX4          242
#define  VFINST_FNTDEF1       243
#define  VFINST_FNTDEF2       244
#define  VFINST_FNTDEF3       245
#define  VFINST_FNTDEF4       246
#define  VFINST_PRE           247
#define  VFINST_POST          248


FT_END_HEADER


#endif /* VF_H_ */


/* END */
