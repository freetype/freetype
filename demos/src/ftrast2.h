/*******************************************************************
 *
 *  ftraster.h                                                 v 2.0
 *
 *  The FreeType glyph scan-line converter (interface)
 *
 *  Copyright 1996-2000 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT. By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *
 ******************************************************************/

#ifndef FTRAST2_H
#define FTRAST2_H

#include <ftimage.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EXPORT_DEF
#define EXPORT_DEF  /* nothing */
#endif

  EXPORT_DEF
  FT_Raster_Funcs   ft_black2_raster;

#ifdef __cplusplus
}
#endif

#endif /* FTRAST2_H */


/* END */
