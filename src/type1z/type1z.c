/***************************************************************************/
/*                                                                         */
/*  type1.c                                                                */
/*                                                                         */
/*  FreeType Type 1 driver component                                       */
/*                                                                         */
/*  Copyright 1996-1998 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*                                                                         */
/*  This file is used to compile the FreeType Type 1  font driver.         */
/*  It relies on all components included in the "base" layer (see          */
/*  the file "ftbase.c"). Source code is located in "freetype/ttlib"       */
/*  and contains :                                                         */
/*                                                                         */
/*     - a driver interface                                                */
/*     - an object manager                                                 */
/*     - a table loader                                                    */
/*     - a glyph loader                                                    */
/*     - a glyph hinter                                                    */
/*                                                                         */
/***************************************************************************/

#define FT_MAKE_OPTION_SINGLE_OBJECT

#include <t1parse.c>
#include <t1load.c>
#include <t1objs.c>
#include <t1driver.c>
#include <t1gload.c>

#ifndef T1_CONFIG_OPTION_NO_AFM
#include <t1afm.c>
#endif

