/***************************************************************************/
/*                                                                         */
/*  ft2build.h                                                             */
/*                                                                         */
/*    Build macros of the FreeType 2 library.                              */
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


  /*************************************************************************/
  /*                                                                       */
  /* This is a Unix-specific version of <ft2build.h> that should be used   */
  /* exclusively *after* installation of the library.                      */
  /*                                                                       */
  /* Currently, the FreeType 2 root is at "freetype2/freetype", though it  */
  /* will change to "freetype2" in the near future.                        */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FT2_BUILD_UNIX_H__
#define __FT2_BUILD_UNIX_H__

#define FT2_PUBLIC_FILE( x )    <freetype2/freetype/ ## x ## >
#define FT2_CONFIG_FILE( x )    <freetype2/freetype/config/ ## x ## >
#define FT2_INTERNAL_FILE( x )  <freetype2/freetype/internal/ ## x ## >

#include FT2_CONFIG_FILE( ft2build.h )

#endif /* __FT2_BUILD_UNIX_H__ */


/* END */
