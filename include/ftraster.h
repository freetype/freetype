/***************************************************************************/
/*                                                                         */
/*  ftraster.h                                                             */
/*                                                                         */
/*    The FreeType glyph rasterizer (specification).                       */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef FTRASTER_H
#define FTRASTER_H

#ifdef __cplusplus
  extern "C" {
#endif


  /*************************************************************************/
  /*                                                                       */
  /* Uncomment the following line if you are using ftraster.c as a         */
  /* standalone module, fully independent of FreeType.                     */
  /*                                                                       */
/* #define _STANDALONE_ */

#include <ftimage.h>

#ifndef EXPORT_DEF
#define EXPORT_DEF  /* nothing */
#endif

  EXPORT_DEF
  FT_Raster_Funcs  ft_raster_funcs;

#ifdef __cplusplus
  }
#endif

#endif /* FTRASTER_H */


/* END */
