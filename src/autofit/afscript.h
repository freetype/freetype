/***************************************************************************/
/*                                                                         */
/*  afscript.h                                                             */
/*                                                                         */
/*    Auto-fitter scripts (specification only).                            */
/*                                                                         */
/*  Copyright 2013 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /* The following part can be included multiple times. */
  /* Define `SCRIPT' as needed.                         */


  /* Add new scripts here.  The first and second arguments are the   */
  /* script name in lowercase and uppercase, respectively, followed  */
  /* by a description string.  Then comes the corresponding HarfBuzz */
  /* script name tag, followed by the default character (to derive   */
  /* the standard width and height of stems).                        */

  SCRIPT( cyrl, CYRL,
          "Cyrillic",
          HB_SCRIPT_CYRILLIC,
          0x43E ) /* о */

  SCRIPT( deva, DEVA,
          "Indic scripts",
          HB_SCRIPT_DEVANAGARI,
          'o' ) /* XXX */

  SCRIPT( none, NONE,
          "no script",
          HB_SCRIPT_INVALID,
          '\0' )

  SCRIPT( grek, GREK,
          "Greek",
          HB_SCRIPT_GREEK,
          0x3BF ) /* ο */

  SCRIPT( hani, HANI,
          "CJKV ideographs",
          HB_SCRIPT_HAN,
          0x7530 ) /* 田 */

  SCRIPT( hebr, HEBR,
          "Hebrew",
          HB_SCRIPT_HEBREW,
          0x5DD ) /* ם */

  SCRIPT( latn, LATN,
          "Latin",
          HB_SCRIPT_LATIN,
          'o' )


/* END */
