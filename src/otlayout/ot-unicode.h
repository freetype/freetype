/* ot-unicode.[ch] are copied from gunicode.[ch] */

/* gunicode.h - Unicode manipulation functions
 *
 *  Copyright (C) 1999, 2000 Tom Tromey
 *  Copyright 2000 Red Hat, Inc.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *   Boston, MA 02111-1307, USA.
 */

#ifndef __OT_UNICODE_H__
#define __OT_UNICODE_H__

#include <ft2build.h>
#include FT_FREETYPE_H

FT_BEGIN_HEADER

/* 3: Mark glyph (non-spacing combining glyph),
   1: Base glyph (single character, spacing glyph) */

FT_LOCAL(FT_UShort) ot_get_glyph_class(FT_ULong charcode);

FT_END_HEADER
#endif /* Not def: __OT_UNICODE_H__ */

