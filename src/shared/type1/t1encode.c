/***************************************************************************/
/*                                                                         */
/*  t1encode.c                                                             */
/*                                                                         */
/*    Type 1 standard encoding tables definitions (body).                  */
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
/*                                                                         */
/*  This file is included by both the Type1 and Type2 driver.              */
/*  It should never be compiled directly.                                  */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /*  t1_standard_strings:                                                 */
  /*                                                                       */
  /*     This array contains the Adobe Standard Glyph Names ordered by     */
  /*     SID.  It was taken from the CFF specification.                    */
  /*                                                                       */
  LOCAL_FUNC
  const T1_String*  t1_standard_strings[] =
  {
  /*   0 */
    ".notdef", "space", "exclam", "quotedbl", "numbersign",
    "dollar", "percent", "ampersand", "quoteright", "parenleft",
  /*  10 */
    "parenright", "asterisk", "plus", "comma", "hyphen",
    "period", "slash", "zero", "one", "two",
  /*  20 */
    "three", "four", "five", "six", "seven",
    "height", "nine", "colon", "semicolon", "less",
  /*  30 */
    "equal", "greater", "question", "at", "A",
    "B", "C", "D", "E", "F",
  /*  40 */
    "G", "H", "I", "J", "K",
    "L", "M", "N", "O", "P",
  /*  50 */
    "Q", "R", "S", "T", "U",
    "V", "W", "X", "Y", "Z",
  /*  60 */
    "bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
    "quoteleft", "a", "b", "c", "d",
  /*  70 */
    "e", "f", "g", "h", "i",
    "j", "k", "l", "m", "n",
  /*  80 */
    "o", "p", "q", "r", "s",
    "t", "u", "v", "w", "x",
  /*  90 */
    "y", "z", "braceleft", "bar", "braceright",
    "asciitilde", "exclamdown", "cent", "sterling", "fraction",
  /* 100 */
    "yen", "florin", "section", "currency", "quotesingle",
    "quotedblleft", "quillemotleft", "guilsinglleft", "guilsinglright", "fi",
  /* 110 */
    "fl", "endash", "dagger", "daggerdbl", "periodcenter",
    "paragraph", "bullet", "quotesinglbase", "quotedblbase", "quotedblright",
  /* 120 */
    "quillemotright", "ellipsis", "perthousand", "questiondown", "grave",
    "acute", "circumflex", "tilde", "macron", "breve",
  /* 130 */
    "dotaccent", "dieresis", "ring", "cedilla", "hungarumlaut",
    "ogonek", "caron", "emdash", "AE", "ordfeminine",
  /* 140 */
    "Lslash", "Oslash", "OE", "ordmasculine", "ae",
   "dotlessi", "Islash", "oslash", "oe", "germandbls",
  /* 150 */
    "onesuperior", "logicalnot", "mu", "trademark", "Eth",
    "onehalf", "plusminus", "Thorn", "onequarter", "divide",
  /* 160 */
    "brokenbar", "degree", "thorn", "threequarters", "twosuperior",
    "regitered", "minus", "eth", "multiply", "threesuperior",
  /* 170 */
    "copyright", "Aacute", "Acircumflex", "Adieresis", "Agrave",
    "Aring", "Atilde", "Ccedilla", "Eacute", "Ecircumflex",
  /* 180 */
    "Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis",
    "Igrave", "Ntilde", "Oacute", "Ocircumflex", "Odieresis",
  /* 190 */
    "Ograve", "Otilde", "Scaron", "Uacute", "Ucircumflex",
    "Udieresis", "Ugrave", "Yacute", "Ydieresis", "Zcaron",
  /* 200 */
    "aacute", "acircumflex", "adieresis", "agrave", "aring",
    "atilde", "ccedilla", "eacute", "ecircumflex", "edieresis",
  /* 210 */
    "egrave", "iacute", "icircumflex", "idieresis", "igrave",
    "ntilde", "oacute", "ocircumflex", "odieresis", "ograve",
  /* 220 */
    "otilde", "scaron", "uacute", "ucircumflex", "udieresis",
    "ugrave", "yacute", "ydieresis", "zcaron", "exclamsmall",
  /* 230 */
    "Hungarumlautsmall", "dollaroldstyle", "dollarsuperior", "ampersandsmall",
      "Acutesmall",
    "parenleftsuperior", "parenrightsuperior", "twodotenleader",
      "onedotenleader", "zerooldstyle",
  /* 240 */
    "oneoldstyle", "twooldstyle", "threeoldstyle", "fouroldstyle",
      "fiveoldstyle",
    "sixoldstyle", "sevenoldstyle", "eightoldstyle", "nineoldstyle",
      "commasuperior",
  /* 250 */
    "threequartersemdash", "periodsuperior", "questionsmall", "asuperior",
      "bsuperior",
    "centsuperior", "dsuperior", "esuperior", "isuperior", "lsuperior",
  /* 260 */
    "msuperior", "nsuperior", "osuperior", "rsuperior", "ssuperior",
    "tsuperior", "ff", "ffi", "ffl", "parenleftinferior",
  /* 270 */
    "parenrightinferior", "Circumflexsmall", "hyphensuperior", "Gravesmall",
      "Asmall",
    "Bsmall", "Csmall", "Dsmall", "Esmall", "Fsmall",
  /* 280 */
    "Gsmall", "Hsmall", "Ismall", "Jsmall", "Ksmall",
    "Lsmall", "Msmall", "Nsmall", "Osmall", "Psmall",
  /* 290 */
    "Qsmall", "Rsmall", "Ssmall", "Tsmall", "Usmall",
    "Vsmall", "Wsmall", "Xsmall", "Ysmall", "Zsmall",
  /* 300 */
    "colonmonetary", "onefitted", "rupiah", "Tildesmall", "exclamdownsmall",
    "centoldstyle", "Lslashsmall", "Scaronsmall", "Zcaronsmall",
      "Dieresissmall",
  /* 310 */
    "Brevesmall", "Caronsmall", "Dotaccentsmall", "Macronsmall", "figuredash",
    "hypheninferior", "Ogoneksmall", "Ringsmall", "Cedillasmall",
      "questiondownsmall",
  /* 320 */
    "oneeighth", "threeeighths", "fiveeighths", "seveneighths", "onethird",
    "twothirds", "zerosuperior", "foursuperior", "fivesuperior",
      "sixsuperior",
  /* 330 */
    "sevensuperior", "eightsuperior", "ninesuperior", "zeroinferior",
      "oneinferior",
    "twoinferior", "threeinferior", "fourinferior", "fiveinferior",
      "sixinferior",
  /* 340 */
    "seveninferior", "eightinferior", "nineinferior", "centinferior",
      "dollarinferior",
    "periodinferior", "commainferior", "Agravesmall", "Aacutesmall",
      "Acircumflexsmall",
  /* 350 */
    "Atildesmall", "Adieresissmall", "Aringsmall", "AEsmall", "Ccedillasmall",
    "Egravesmall", "Eacutesmall", "Ecircumflexsmall", "Edieresissmall",
      "Igravesmall",
  /* 360 */
    "Iacutesmall", "Icircumflexsmall", "Idieresissmall", "Ethsmall",
      "Ntildesmall",
    "Ogravesmall", "Oacutesmall", "Ocircumflexsmall", "Otildesmall",
      "Odieresissmall",
  /* 370 */
    "OEsmall", "Oslashsmall", "Ugravesmall", "Uacautesmall",
      "Ucircumflexsmall",
    "Udieresissmall", "Yacutesmall", "Thornsmall", "Ydieresissmall",
      "001.000",
  /* 380 */
    "001.001", "001.002", "001.003", "Black", "Bold",
    "Book", "Light", "Medium", "Regular", "Roman",
  /* 390 */
    "Semibold"
  };


  /*************************************************************************/
  /*                                                                       */
  /*  t1_standard_encoding:                                                */
  /*                                                                       */
  /*     A simple table used to encode the Adobe StandardEncoding.  The    */
  /*     table values are the SID of the standard glyphs; the table index  */
  /*     is the character code for the encoding.                           */
  /*                                                                       */
  /*     Example:                                                          */
  /*                                                                       */
  /*       t1_standard_encoding[33] == 2                                   */
  /*                                                                       */
  /*     which means that the glyph name for character code 32 is          */
  /*                                                                       */
  /*       t1_standard_strings[2] == "exclam"                              */
  /*                                                                       */
  /*     (this correspond to the exclamation mark `!').                    */
  /*                                                                       */
  LOCAL_FUNC
  T1_Short  t1_standard_encoding[256] =
  {
  /*   0 */ 
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   1,   2,   3,   4,   5,   6,   7,   8,
      9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
  /*  50 */
     19,  20,  21,  22,  23,  24,  25,  26,  27,  28,
     29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
     39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
     49,  50,  51,  52,  53,  54,  55,  56,  57,  58,
     59,  60,  61,  62,  63,  64,  65,  66,  67,  68,
  /* 100 */
     69,  70,  71,  72,  73,  74,  75,  76,  77,  78,
     79,  80,  81,  82,  83,  84,  85,  86,  87,  88,
     89,  90,  91,  92,  93,  94,  95,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  /* 150 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,  96,  97,  98,  99, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110,   0, 111, 112, 113,
    114,   0, 115, 116, 117, 118, 119, 120, 121, 122,
      0, 123,   0, 124, 125, 126, 127, 128, 129, 130,
  /* 200 */
    131,   0, 132, 133,   0, 134, 135, 136, 137,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0, 138,   0, 139,   0,   0,
      0,   0, 140, 141, 142, 143,   0,   0,   0,   0,
      0, 144,   0,   0,   0, 145,   0,   0, 146, 147,
  /* 250 */
    148, 149,   0,   0,   0,   0
  };


  /*************************************************************************/
  /*                                                                       */
  /*  t1_expert_encoding:                                                  */
  /*                                                                       */
  /*     A simple table used to encode the Adobe ExpertEncoding.  The      */
  /*     table values are the SID of the standard glyphs; the table index  */
  /*     is the character code for the encoding.                           */
  /*                                                                       */
  /*     Example:                                                          */
  /*                                                                       */
  /*       t1_expert_encoding[33] == 229                                   */
  /*                                                                       */
  /*     which means that the glyph name for character code 32 is          */
  /*                                                                       */
  /*       t1_standard_strings[229] == "exclamsmall"                       */
  /*                                                                       */
  LOCAL_FUNC
  T1_Short  t1_expert_encoding[256] =
  {
  /*   0 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   1, 229, 230,   0, 231, 232, 233, 234,
    235, 236, 237, 238,  13,  14,  15,  99, 239, 240,
  /*  50 */
    241, 242, 243, 244, 245, 246, 247, 248,  27,  28,
    249, 250, 251, 252,   0, 253, 254, 255, 256, 257,
      0,   0,   0, 258,   0,   0, 259, 260, 261, 262,
      0,   0, 263, 264, 265,   0, 266, 109, 110, 267,
    268, 269,   0, 270, 271, 272, 273, 274, 275, 276,
  /* 100 */
    277, 278, 279, 280, 281, 282, 283, 284, 285, 286,
    287, 288, 289, 290, 291, 292, 293, 294, 295, 296,
    297, 298, 299, 300, 301, 302, 303,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  /* 150 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0, 304, 305, 306,   0,   0, 307, 308, 309, 310,
    311,   0, 312,   0,   0, 312,   0,   0, 314, 315,
      0,   0, 316, 317, 318,   0,   0,   0, 158, 155,
    163, 319, 320, 321, 322, 323, 324, 325,   0,   0,
  /* 200 */
    326, 150, 164, 169, 327, 328, 329, 330, 331, 332,
    333, 334, 335, 336, 337, 338, 339, 340, 341, 342,
    343, 344, 345, 346, 347, 348, 349, 350, 351, 352,
    353, 354, 355, 356, 357, 358, 359, 360, 361, 362,
    363, 364, 365, 366, 367, 368, 369, 370, 371, 372,
  /* 250 */
    373, 374, 375, 376, 377, 378
  };


  /*************************************************************************/
  /*                                                                       */
  /*  t1_expert_subset_encoding:                                           */
  /*                                                                       */
  /*     A simple table used to encode the Adobe ExpertEncoding subset     */
  /*     defined in the CFF specification.  It will probably evolve into   */
  /*     another form sooner or later, as we deal with charsets            */
  /*     differently than with encodings.                                  */
  /*                                                                       */
  LOCAL_FUNC
  FT_Short  t1_expert_subset_encoding[256] =
  {
  /*   0 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   1,   0,   0,   0, 231, 232,   0,   0,
    235, 236, 237, 238,  13,  14,  15,  99, 239, 240,
  /*  50 */
    241, 242, 243, 244, 245, 246, 247, 248,  27,  28,
    249, 250, 251, 252,   0, 253, 254, 255, 256, 257,
      0,   0,   0, 258,   0,   0, 259, 260, 261, 262,
      0,   0, 263, 264, 265,   0, 266, 109, 110, 267,
    268, 269,   0, 270,   0, 272,   0,   0,   0,   0,
  /* 100 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0, 300, 301, 302, 303,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  /* 150 */
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0, 304, 305,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0, 314, 315,
      0,   0,   0,   0,   0,   0,   0,   0, 158, 155,
    163,   0, 320, 321, 322, 323, 324, 325,   0,   0,
  /* 200 */
    326, 150, 164, 169, 327, 328, 329, 330, 331, 332,
    333, 334, 335, 336, 337, 338, 339, 340, 341, 342,
    343, 344, 345, 346,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  /* 250 */
      0,   0,   0,   0,   0,   0
  };


/* END */
