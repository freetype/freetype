/***************************************************************************/
/*                                                                         */
/*  ftcimage.c                                                             */
/*                                                                         */
/*    FreeType Image cache (body).                                         */
/*                                                                         */
/*  Copyright 2000 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef FTCIMAGE_H
#define FTCIMAGE_H


#include <freetype/cache/ftcglyph.h>
#include <freetype/ftglyph.h>


#ifdef __cplusplus
  extern "C" {
#endif

 /* the FT_Glyph image "glyph node" type */
  typedef struct  FTC_GlyphImageRec_
  {
    FTC_GlyphNodeRec   root;
    FT_Glyph           ft_glyph;

  } FTC_GlyphImageRec, *FTC_GlyphImage;


  /* the glyph image queue type */
  typedef struct  FTC_Image_QueueRec_
  {
    FTC_Glyph_QueueRec  root;
    FTC_Image_Desc      description;
  
  } FTC_Image_QueueRec, *FTC_Image_Queue;


  typedef struct  FTC_Image_CacheRec_
  {
    FTC_Glyph_CacheRec  root;
    
  } FTC_Image_CacheRec;


#ifdef __cplusplus
  }
#endif


#endif /* FTCIMAGE_H */


/* END */
