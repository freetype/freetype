/***************************************************************************/
/*                                                                         */
/*  ftcsbits.h                                                             */
/*                                                                         */
/*    A small-bitmap cache (specification).                                */
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


#ifndef FTCSBITS_H
#define FTCSBITS_H


#include <freetype/cache/ftcchunk.h>
#include <freetype/cache/ftcimage.h>


#ifdef __cplusplus
  extern "C" {
#endif


  /* handle to small bitmap */
  typedef struct FTC_SBitRec_*  FTC_SBit;

  /* handle to small bitmap cache */
  typedef struct FTC_SBit_CacheRec_*  FTC_SBit_Cache;

  /* a compact structure used to hold a single small bitmap */
  typedef struct  FTC_SBitRec_
  {
    FT_Byte   width;
    FT_Byte   height;
    FT_Char   left;
    FT_Char   top;

    FT_Byte   format;
    FT_Char   pitch;
    FT_Char   xadvance;
    FT_Char   yadvance;

    FT_Byte*  buffer;

  } FTC_SBitRec;


  FT_EXPORT( FT_Error )  FTC_SBit_Cache_New( FTC_Manager      manager,
                                             FTC_SBit_Cache  *acache );

  FT_EXPORT( FT_Error )  FTC_SBit_Cache_Lookup( FTC_SBit_Cache   cache,
                                                FTC_Image_Desc*  desc,
                                                FT_UInt          gindex,
                                                FTC_SBit        *sbit );


#ifdef __cplusplus
  }
#endif


#endif /* FTCSBITS_H */


/* END */
