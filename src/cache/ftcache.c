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

#include <ft2build.h>
#include FT_SOURCE_FILE( cache, ftlru.c )
#include FT_SOURCE_FILE( cache, ftcmanag.c )
#include FT_SOURCE_FILE( cache, ftcglyph.c )
#include FT_SOURCE_FILE( cache, ftcchunk.c )
#include FT_SOURCE_FILE( cache, ftcimage.c )
#include FT_SOURCE_FILE( cache, ftcsbits.c )


/* END */
