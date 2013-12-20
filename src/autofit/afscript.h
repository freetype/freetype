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


  /* Add new scripts here. */

  SCRIPT( cyrl, CYRL,
          "Cyrillic",
          0x43E ) /* о */

  SCRIPT( deva, DEVA,
          "Indic scripts",
          'o' ) /* XXX */

  SCRIPT( none, NONE,
          "no script",
          '\0' )

  SCRIPT( grek, GREK,
          "Greek",
          0x3BF ) /* ο */

  SCRIPT( hani, HANI,
          "CJKV ideographs",
          0x7530 ) /* 田 */

  SCRIPT( hebr, HEBR,
          "Hebrew",
          0x5DD ) /* ם */

  SCRIPT( latn, LATN,
          "Latin",
          'o' )


/* END */
