/****************************************************************************
 *
 * svg.c
 *
 *   FreeType svg renderer module component (body only).
 *
 * Copyright (C) 1996-2019 by
 * David Turner, Robert Wilhelm, Werner Lemberg and Moazin Khatti.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

#define FT_MAKE_OPTION_SINGLE_OBJECT
#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FT_CONFIG_OPTION_DEFAULT_SVG
#include "rsvg_port.c"
#endif

#include "svgtypes.h"
#include "ftsvg.c"



/* END */
