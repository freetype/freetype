/***************************************************************************/
/*                                                                         */
/*  sfnt.c                                                                 */
/*                                                                         */
/*    Single object library component.                                     */
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

#include  <ft2build.h>
#include  FT_SOURCE_FILE( sfnt, ttload.c )
#include  FT_SOURCE_FILE( sfnt, ttcmap.c )
#include  FT_SOURCE_FILE( sfnt, sfobjs.c )
#include  FT_SOURCE_FILE( sfnt, sfdriver.c )

#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS
#include  FT_SOURCE_FILE( sfnt, ttsbit.c )
#endif

#ifdef TT_CONFIG_OPTION_POSTSCRIPT_NAMES
#include  FT_SOURCE_FILE( sfnt, ttpost.c )
#endif


/* END */
