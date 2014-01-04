/***************************************************************************/
/*                                                                         */
/*  afscript.h                                                             */
/*                                                                         */
/*    Auto-fitter scripts (specification only).                            */
/*                                                                         */
/*  Copyright 2013, 2014 by                                                */
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

  SCRIPT( grek, GREK,
          "Greek",
          HB_SCRIPT_GREEK,
          0x3BF ) /* ο */

  SCRIPT( hebr, HEBR,
          "Hebrew",
          HB_SCRIPT_HEBREW,
          0x5DD ) /* ם */

  SCRIPT( latn, LATN,
          "Latin",
          HB_SCRIPT_LATIN,
          'o' )

  SCRIPT( none, NONE,
          "no script",
          HB_SCRIPT_INVALID,
          '\0' )

#ifdef AF_CONFIG_OPTION_INDIC

  SCRIPT( beng, BENG,
          "Bengali",
          HB_SCRIPT_BENGALI,
          'o' ) /* XXX */

  SCRIPT( deva, DEVA,
          "Devanagari",
          HB_SCRIPT_DEVANAGARI,
          'o' ) /* XXX */

  SCRIPT( gujr, GUJR,
          "Gujarati",
          HB_SCRIPT_GUJARATI,
          'o' ) /* XXX */

  SCRIPT( guru, GURU,
          "Gurmukhi",
          HB_SCRIPT_GURMUKHI,
          'o' ) /* XXX */

  SCRIPT( knda, KNDA,
          "Kannada",
          HB_SCRIPT_KANNADA,
          'o' ) /* XXX */

  SCRIPT( limb, LIMB,
          "Limbu",
          HB_SCRIPT_LIMBU,
          'o' ) /* XXX */

  SCRIPT( mlym, MLYM,
          "Malayalam",
          HB_SCRIPT_MALAYALAM,
          'o' ) /* XXX */

  SCRIPT( orya, ORYA,
          "Oriya",
          HB_SCRIPT_ORIYA,
          'o' ) /* XXX */

  SCRIPT( sinh, SINH,
          "Sinhala",
          HB_SCRIPT_SINHALA,
          'o' ) /* XXX */

  SCRIPT( sund, SUND,
          "Sundanese",
          HB_SCRIPT_SUNDANESE,
          'o' ) /* XXX */

  SCRIPT( sylo, SYLO,
          "Syloti Nagri",
          HB_SCRIPT_SYLOTI_NAGRI,
          'o' ) /* XXX */

  SCRIPT( taml, TAML,
          "Tamil",
          HB_SCRIPT_TAMIL,
          'o' ) /* XXX */

  SCRIPT( telu, TELU,
          "Telugu",
          HB_SCRIPT_TELUGU,
          'o' ) /* XXX */

  SCRIPT( tibt, TIBT,
          "Tibetan",
          HB_SCRIPT_TIBETAN,
          'o' ) /* XXX */

#endif /* AF_CONFIG_OPTION_INDIC */

#ifdef AF_CONFIG_OPTION_CJK

  SCRIPT( hani, HANI,
          "CJKV ideographs",
          HB_SCRIPT_HAN,
          0x7530 ) /* 田 */

#endif /* AF_CONFIG_OPTION_CJK */


/* END */
