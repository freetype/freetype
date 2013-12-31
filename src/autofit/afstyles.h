/***************************************************************************/
/*                                                                         */
/*  afstyles.h                                                             */
/*                                                                         */
/*    Auto-fitter styles (specification only).                             */
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
  /* Define `STYLE' as needed.                          */


  /* Add new styles here.  The first and second arguments are the  */
  /* style name in lowercase and uppercase, respectively, followed */
  /* by a description string.  The next arguments are the          */
  /* corresponding writing system, script, blue stringset, and     */
  /* coverage.                                                     */

  STYLE( cyrl_dflt, CYRL_DFLT,
         "Cyrillic default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_CYRL,
         AF_BLUE_STRINGSET_CYRL,
         AF_COVERAGE_DEFAULT )

  STYLE( deva_dflt, DEVA_DFLT,
         "Indic scripts default style",
         AF_WRITING_SYSTEM_INDIC,
         AF_SCRIPT_DEVA,
         (AF_Blue_Stringset)0, /* XXX */
         AF_COVERAGE_DEFAULT )

  STYLE( grek_dflt, GREK_DFLT,
         "Greek default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_GREK,
         AF_BLUE_STRINGSET_GREK,
         AF_COVERAGE_DEFAULT )

  STYLE( hani_dflt, HANI_DFLT,
         "CJKV ideographs default style",
         AF_WRITING_SYSTEM_CJK,
         AF_SCRIPT_HANI,
         AF_BLUE_STRINGSET_HANI,
         AF_COVERAGE_DEFAULT )

  STYLE( hebr_dflt, HEBR_DFLT,
         "Hebrew default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_HEBR,
         AF_BLUE_STRINGSET_HEBR,
         AF_COVERAGE_DEFAULT )

  STYLE( latn_dflt, LATN_DFLT,
         "Latin default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_LATN,
         AF_BLUE_STRINGSET_LATN,
         AF_COVERAGE_DEFAULT )

#ifdef FT_OPTION_AUTOFIT2
  STYLE( ltn2_dflt, LTN2_DFLT,
         "Latin 2 default style",
         AF_WRITING_SYSTEM_LATIN2,
         AF_SCRIPT_LATN,
         AF_BLUE_STRINGSET_LATN,
         AF_COVERAGE_DEFAULT )
#endif

  STYLE( none_dflt, NONE_DFLT,
         "no style",
         AF_WRITING_SYSTEM_DUMMY,
         AF_SCRIPT_NONE,
         (AF_Blue_Stringset)0,
         AF_COVERAGE_DEFAULT )


/* END */
