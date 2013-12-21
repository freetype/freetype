/***************************************************************************/
/*                                                                         */
/*  afcover.h                                                              */
/*                                                                         */
/*    Auto-fitter coverages (specification only).                          */
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


  /* This header file can be included multiple times. */
  /* Define `COVERAGE_{1,2,3}' as needed.             */


  /* Add new coverages here.  The first and second arguments are the   */
  /* coverage name in lowercase and uppercase, respectively, followed  */
  /* by a description string.  The remaining arguments the             */
  /* corresponding OpenType features (with four characters a feature). */

  COVERAGE_1( oldstyle_figures, OLDSTYLE_FIGURES,
              "oldstyle figures",
              'o', 'n', 'u', 'm' ) /* Oldstyle Figures */

  COVERAGE_2( petite_capitals, PETITE_CAPITALS,
              "petite capitals",
              'c', '2', 'c', 'p',  /* Petite Capitals from Capitals */
              'p', 'c', 'a', 'p' ) /* Petite Capitals               */

  COVERAGE_2( small_capitals, SMALL_CAPITALS,
              "small capitals",
              'c', '2', 's', 'c',  /* Small Capitals from Capitals */
              's', 'm', 'c', 'p' ) /* Small Capitals               */

  COVERAGE_1( titling, TITLING,
              "titling",
              't', 'i', 't', 'l' ) /* Titling */

  COVERAGE_2( sub_superscript_1, SUB_SUPERSCRIPT_1,
              "sub- and superscripts group 1",
              's', 'u', 'b', 's',  /* Subscript   */
              's', 'u', 'p', 's' ) /* Superscript */

  COVERAGE_2( sub_superscript_2, SUB_SUPERSCRIPT_2,
              "sub- and superscripts group 2",
              'o', 'r', 'd', 'n',  /* Ordinals             */
              's', 'i', 'n', 'f' ) /* Scientific Inferiors */

  COVERAGE_3( fractions, FRACTIONS,
              "fractions",
              'd', 'n', 'o', 'm',  /* Denominators */
              'f', 'r', 'a', 'c',  /* Fractions    */
              'n', 'u', 'm', 'r' ) /* Numerators   */

  COVERAGE_1( alternative_fractions, ALTERNATIVE_FRACTIONS,
              "alternative fractions",
              'a', 'f', 'r', 'c' ) /* Alternative Fractions */

/* END */
