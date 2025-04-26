/****************************************************************************
 *
 * ft-hb.h
 *
 *   FreeType-HarfBuzz bridge (specification).
 *
 * Copyright (C) 2025 by
 * Behdad Esfahbod.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef FT_HB_H
#define FT_HB_H

#include <hb.h>

#include <freetype/internal/compiler-macros.h>
#include <freetype/freetype.h>


FT_BEGIN_HEADER

#ifdef FT_CONFIG_OPTION_USE_HARFBUZZ

#define hb( x )  hb_ ## x

#endif /* FT_CONFIG_OPTION_USE_HARFBUZZ */

FT_END_HEADER

#endif /* FT_HB_H */


/* END */
