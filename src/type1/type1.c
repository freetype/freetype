/***************************************************************************/
/*                                                                         */
/*  type1.c                                                                */
/*                                                                         */
/*    FreeType Type 1 driver component (body only).                        */
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

#include <ft2build.h>
#include FT_SOURCE_FILE( type1, t1parse.c )
#include FT_SOURCE_FILE( type1, t1load.c )
#include FT_SOURCE_FILE( type1, t1objs.c )
#include FT_SOURCE_FILE( type1, t1driver.c )
#include FT_SOURCE_FILE( type1, t1gload.c )

#ifndef T1_CONFIG_OPTION_NO_AFM
#include FT_SOURCE_FILE( type1, t1afm.c )
#endif


/* END */
