/*******************************************************************
 *
 *  t1config.h
 *
 *    Type 1 driver configuration file
 *
 *  Copyright 1996-1998 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used,
 *  modified, and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute 
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 ******************************************************************/

#ifndef T1CONFIG_H
#define T1CONFIG_H

/* T1_MAX_STACK_DEPTH is the maximal depth of the token stack used */
/* by the Type 1 parser (see t1load.c). A minimum of 16 is required */
/*                                                                 */
#define T1_MAX_STACK_DEPTH  16

/* T1_MAX_DICT_DEPTH is the maximal depth of nest dictionaries and */
/* arrays in the Type 1 stream (see t1load.c). A minimum of 4 is   */
/* required                                                        */
#define T1_MAX_DICT_DEPTH   5

/* T1_MAX_SUBRS_CALLS details the maximum number of nested sub-routine */
/* calls during glyph loading                                          */
#define T1_MAX_SUBRS_CALLS   8


/* T1_MAX_CHARSTRING_OPERANDS is the charstring stack's capacity */
#define T1_MAX_CHARSTRINGS_OPERANDS  32


/* Define T1_CONFIG_OPTION_DISABLE_HINTER if you want to generate  */
/* a driver with no hinter. This can be useful to debug the parser */
/*                                                                 */
#define T1_CONFIG_OPTION_DISABLE_HINTER

/* Define this configuration macro if you want to prevent the      */
/* compilation of "t1afm", which is in charge of reading Type1     */
/* AFM files into an existing face. Note that when set, the T1     */
/* driver will be unable to produce kerning distances..            */
/*                                                                 */
#define T1_CONFIG_OPTION_NO_AFM

#endif /* T1CONFIG_H */
