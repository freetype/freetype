/****************************************************************************
 *
 * woff2tags.h
 *
 *   WOFFF2 Font table tags (specification).
 *
 * Copyright (C) 1996-2019 by
 * Nikhil Ramakrishnan, David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef WOFF2TAGS_H
#define WOFF2TAGS_H


#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H


FT_BEGIN_HEADER


/* Leave the first byte open to store flag_byte. */
#define WOFF2_FLAGS_TRANSFORM   1 << 8

#define WOFF2_SFNT_HEADER_SIZE  12
#define WOFF2_SFNT_ENTRY_SIZE   16

/* Known table tags. */
extern const FT_ULong KnownTags[];

FT_END_HEADER

#endif /* WOFF2TAGS_H */


/* END */
