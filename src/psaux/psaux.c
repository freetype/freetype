/***************************************************************************/
/*                                                                         */
/*  psaux.c                                                                */
/*                                                                         */
/*    FreeType auxiliary PostScript driver component (body only).          */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#define FT_MAKE_OPTION_SINGLE_OBJECT

#ifdef FT_FLAT_COMPILE

#include "psobjs.c"
#include "psauxmod.c"
#include "t1decode.c"

#else

#include <psaux/psobjs.c>
#include <psaux/psauxmod.c>
#include <psaux/t1decode.c>

#endif


/* END */
