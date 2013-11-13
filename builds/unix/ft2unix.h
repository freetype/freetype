/***************************************************************************/
/*                                                                         */
/*  ft2build.h                                                             */
/*                                                                         */
/*    Build macros of the FreeType 2 library.                              */
/*                                                                         */
/*  Copyright 1996-2001, 2003, 2006 by                                     */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This is a Unix-specific version of <ft2build.h> that should be used   */
  /* exclusively *after* installation of the library.                      */
  /*                                                                       */
  /* It assumes that `/usr/local/include/freetype2' (or whatever is        */
  /* returned by the `freetype-config --cflags' or `pkg-config --cflags'   */
  /* command) is in your compilation include path.                         */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FT2_BUILD_UNIX_H__
#define __FT2_BUILD_UNIX_H__

  /* `<prefix>/include/freetype2' must be in your current inclusion path */
#include <config/ftheader.h>

#endif /* __FT2_BUILD_UNIX_H__ */


/* END */
