/***************************************************************************/
/*                                                                         */
/*  truetype.c                                                             */
/*                                                                         */
/*    FreeType TrueType driver component (body only).                      */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#define FT_MAKE_OPTION_SINGLE_OBJECT

#include <ft2build.h>
#include FT_SOURCE_FILE(truetype,ttdriver.c)   /* driver interface */
#include FT_SOURCE_FILE(truetype,ttpload.c)    /* tables loader    */
#include FT_SOURCE_FILE(truetype,ttgload.c)    /* glyph loader     */
#include FT_SOURCE_FILE(truetype,ttobjs.c)     /* object manager   */

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#include FT_SOURCE_FILE(truetype,ttinterp.c)
#endif

/* END */
