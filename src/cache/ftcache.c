/***************************************************************************/
/*                                                                         */
/*  ftcache.c                                                              */
/*                                                                         */
/*    The FreeType Caching sub-system (body only).                         */
/*                                                                         */
/*  Copyright 2000 by                                                      */
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

#ifdef FT_FLAT_COMPILE

#include "ftlru.c"
#include "ftcmanag.c"
#include "ftcglyph.c"
#include "ftcchunk.c"
#include "ftcimage.c"
#include "ftcsbits.c"

#else

#include <cache/ftlru.c>
#include <cache/ftcmanag.c>
#include <cache/ftcglyph.c>
#include <cache/ftcchunk.c>
#include <cache/ftcimage.c>
#include <cache/ftcsbits.c>

#endif


/* END */
