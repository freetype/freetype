/***************************************************************************/
/*                                                                         */
/*  truetype.c                                                             */
/*                                                                         */
/*    FreeType TrueType driver component (body only).                      */
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
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /*  This file is used to compile the FreeType TrueType font driver.  It  */
  /*  relies on all components included in the `base' layer (see the file  */
  /*  `ftbase.c').  The source code is located in `freetype/ttlib' and     */
  /*  contains:                                                            */
  /*                                                                       */
  /*  - a driver interface                                                 */
  /*  - an object manager                                                  */
  /*  - a table loader                                                     */
  /*  - a glyph loader                                                     */
  /*  - a glyph hinter/bytecode interpreter                                */
  /*  - a charmap processor                                                */
  /*  - an extension manager (only used for some tools)                    */
  /*                                                                       */
  /*  Note that the engine extensions found in `freetype/ttlib/extend' are */
  /*  reserved to specific tools and/or font servers; they're not part of  */
  /*  the `core' TrueType driver, even though they are separately linkable */
  /*  to it.                                                               */
  /*                                                                       */
  /*************************************************************************/


#define FT_MAKE_OPTION_SINGLE_OBJECT

#include <ttdriver.c>    /* driver interface     */
#include <ttpload.c>     /* tables loader        */
#include <ttgload.c>     /* glyph loader         */
#include <ttobjs.c>      /* object manager       */

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#include <ttinterp.c>    /* bytecode interpreter */
#endif
/* END */
