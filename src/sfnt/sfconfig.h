/***************************************************************************/
/*                                                                         */
/*  sfconfig.h                                                             */
/*                                                                         */
/*    the `sfnt' driver configuration file.                                */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This file is used to configure various aspects of the TrueType        */
  /* driver.                                                               */
  /*                                                                       */
  /*************************************************************************/


#ifndef SFCONFIG_H
#define SFCONFIG_H


#if 0
  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_EMBEDDED_BITMAPS if you want to support       */
  /* embedded bitmaps in the TrueType/OpenType driver.                     */
  /*                                                                       */
#define  TT_CONFIG_OPTION_EMBEDDED_BITMAPS


  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_POSTSCRIPT_NAMES if you want to be able to    */
  /* load and enumerate the glyph Postscript names in a TrueType or        */
  /* OpenType file.                                                        */
  /*                                                                       */
  /* Note that if FT_CONFIG_OPTION_POSTSCRIPT_NAMES is also defined,       */
  /* the TrueType driver will use the "psnames" module to fetch the        */
  /* glyph names.                                                          */
  /*                                                                       */
  /* Otherwise, the driver will provide its own set of glyph names to      */
  /* be built without the "psnames" module.                                */
  /*                                                                       */
#define  TT_CONFIG_OPTION_POSTSCRIPT_NAMES
#endif


#endif /* SFCONFIG_H */


/* END */
