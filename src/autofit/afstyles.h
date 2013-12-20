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


  /* Add new styles here. */

  STYLE( cyrl_default, CYRL_DEFAULT,
         "Cyrillic default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_CYRL,
         AF_BLUE_STRINGSET_CYRL )

  STYLE( deva_default, DEVA_DEFAULT,
         "Indic scripts default style",
         AF_WRITING_SYSTEM_INDIC,
         AF_SCRIPT_DEVA,
         (AF_Blue_Stringset)0 ) /* XXX */

  STYLE( none_default, NONE_DEFAULT,
         "no style",
         AF_WRITING_SYSTEM_DUMMY,
         AF_SCRIPT_NONE,
         (AF_Blue_Stringset)0 )

  STYLE( grek_default, GREK_DEFAULT,
         "Greek default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_GREK,
         AF_BLUE_STRINGSET_GREK )

  STYLE( hani_default, HANI_DEFAULT,
         "CJKV ideographs default style",
         AF_WRITING_SYSTEM_CJK,
         AF_SCRIPT_HANI,
         AF_BLUE_STRINGSET_HANI )

  STYLE( hebr_default, HEBR_DEFAULT,
         "Hebrew default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_HEBR,
         AF_BLUE_STRINGSET_HEBR )

  STYLE( latn_default, LATN_DEFAULT,
         "Latin default style",
         AF_WRITING_SYSTEM_LATIN,
         AF_SCRIPT_LATN,
         AF_BLUE_STRINGSET_LATN )

#ifdef FT_OPTION_AUTOFIT2
  STYLE( ltn2_default, LTN2_DEFAULT,
         "Latin 2 default style",
         AF_WRITING_SYSTEM_LATIN2,
         AF_SCRIPT_LATN,
         AF_BLUE_STRINGSET_LATN )
#endif


/* END */
