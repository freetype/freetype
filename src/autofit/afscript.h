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
          AF_BLUE_STRINGSET_CYRL,
          AF_WRITING_SYSTEM_LATIN,
          0x43E ) /* о */

  SCRIPT( deva, DEVA,
          "Indic scripts",
          (AF_Blue_Stringset)0, /* XXX */
          AF_WRITING_SYSTEM_INDIC,
          'o' ) /* XXX */

  SCRIPT( none, NONE,
          "no script",
          (AF_Blue_Stringset)0,
          AF_WRITING_SYSTEM_DUMMY,
          '\0' )

  SCRIPT( grek, GREK,
          "Greek",
          AF_BLUE_STRINGSET_GREK,
          AF_WRITING_SYSTEM_LATIN,
          0x3BF ) /* ο */

  SCRIPT( hani, HANI,
          "CJKV ideographs",
          AF_BLUE_STRINGSET_HANI,
          AF_WRITING_SYSTEM_CJK,
          0x7530 ) /* 田 */

  SCRIPT( hebr, HEBR,
          "Hebrew",
          AF_BLUE_STRINGSET_HEBR,
          AF_WRITING_SYSTEM_LATIN,
          0x5DD ) /* ם */

  SCRIPT( latn, LATN,
          "Latin",
          AF_BLUE_STRINGSET_LATN,
          AF_WRITING_SYSTEM_LATIN,
          'o' )

#ifdef FT_OPTION_AUTOFIT2
  SCRIPT( ltn2, LTN2,
          "Latin 2",
          AF_BLUE_STRINGSET_LATN,
          AF_WRITING_SYSTEM_LATIN2,
          'o' )
#endif


/* END */
