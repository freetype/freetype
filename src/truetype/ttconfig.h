/***************************************************************************/
/*                                                                         */
/*  ttconfig.h                                                             */
/*                                                                         */
/*    TrueType configuration file (specification only).                    */
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


#ifndef TTCONFIG_H
#define TTCONFIG_H


  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_INTERPRETER_SWITCH to compile the TrueType    */
  /* bytecode interpreter with a huge switch statement, rather than a      */
  /* call table.  This results in smaller and faster code for a number of  */
  /* architectures.                                                        */
  /*                                                                       */
  /* Note however that on some compiler/processor combinations, undefining */
  /* this macro will generate a faster, though larger, code.               */
  /*                                                                       */
#define TT_CONFIG_OPTION_INTERPRETER_SWITCH


  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_EMBEDDED_BITMAPS if you want to support       */
  /* embedded bitmaps in the TrueType/OpenType driver.                     */
  /*                                                                       */
#define TT_CONFIG_OPTION_EMBEDDED_BITMAPS


  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_POSTSCRIPT_NAMES if you want to be able to    */
  /* load and enumerate the glyph Postscript names in a TrueType or        */
  /* OpenType file.                                                        */
  /*                                                                       */
#define TT_CONFIG_OPTION_POSTSCRIPT_NAMES


#define  TT_USE_FIXED

#endif /* TTCONFIG_H */


/* END */
