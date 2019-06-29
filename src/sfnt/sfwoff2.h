/****************************************************************************
 *
 * sfwoff2.h
 *
 *   WOFFF2 format management (specification).
 *
 * Copyright (C) 2019 by
 * Nikhil Ramakrishnan, David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef SFWOFF2_H_
#define SFWOFF2_H_


#include <ft2build.h>
#include FT_INTERNAL_SFNT_H
#include FT_INTERNAL_OBJECTS_H


FT_BEGIN_HEADER


  FT_LOCAL( FT_Error )
  woff2_open_font( FT_Stream  stream,
                   TT_Face    face,
                   FT_Int     face_index );


FT_END_HEADER

#endif /* SFWOFF2_H_ */


/* END */
