/***************************************************************************/
/*                                                                         */
/*  gx.c                                                                   */
/*                                                                         */
/*    FreeType AAT/TrueTypeGX driver component (body only).                */
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/
#define FT_MAKE_OPTION_SINGLE_OBJECT

#include <ft2build.h>

#include "gxdriver.c"		/* low level driver interface */
#include "gxobjs.c"		/* low level object manager */
#include "gxload.c"		/* tables loader */

#include "gxlookuptbl.c"
#include "gxstatetbl.c"
#include "gxaccess.c"
#include "gxutils.c"
#include "gxfeatreg.c"

#include "gxlayout.c"
#include "gxlfeatreg.c"
#include "gxvm.c"

#include "gxdump.c"		/* only for debug */

/* END */
