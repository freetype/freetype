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
  int  FT_Raster_Init( FT_Raster    raster,
                       const char*  pool_base,
                       long         pool_size );

  EXPORT_DEF
  int  FT_Raster_Render( FT_Raster    raster,
                         FT_Outline*  outline,
                         FT_Bitmap*   target_map );

  EXPORT_DEF
  long  FT_Raster_ObjSize( void );

  /* FT_Raster_SetPalette() is currently unused by FreeType 2 */

  EXPORT_DEF
  int  FT_Raster_SetPalette( FT_Raster    raster,
                             int          count,
                             const char*  palette );


#ifdef __cplusplus
  }
#endif

#endif /* FTRASTER_H */


/* END */
