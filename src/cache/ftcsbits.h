/***************************************************************************/
/*                                                                         */
/*  ftcsbits.h                                                             */
/*                                                                         */
/*    a small-bitmaps cache (specification).                               */
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

#ifdef __cplusplus
  extern "C" {
#endif

 /* handle to small bitmap */
  typedef struct FTC_SBitRec_*   FTC_SBit;


 /* handle to small bitmap cache */
  typedef struct FTC_SBit_CacheRec_*   FTC_SBit_Cache;


 /* format of small bitmaps */  
  typedef enum FTC_SBit_Format_
  {
    ftc_sbit_format_mono  = 0,
    ftc_sbit_format_aa256 = 1,

  } FTC_SBit_Format;


 /* a compact structure used to hold a single small bitmap */  
  typedef struct FTC_SBitRec_
  {
    FT_Byte   width;
    FT_Byte   height;
    FT_SChar  left;
    FT_SChar  top;

    FT_Byte   format;
    FT_SChar  pitch;
    FT_SChar  xadvance;
    FT_SChar  yadvance;

    FT_Byte*  buffer;
  
  } FTC_SBitRec;


  typedef struct FTC_SBitSetRec_
  {
    FTC_ChunkSetRec   root;
    FTC_Image_Desc    desc;

  } FTC_SBitSet;


  typedef struct FTC_SBit_CacheRec_
  {
    FTC_Chunk_CacheRec    root;
    
  } FTC_SBit_CacheRec;



  FT_EXPORT_DEF( FT_Error )
  FTC_SBit_Cache_New( FTC_Manager      manager,
                      FTC_SBit_Cache  *acache );


  FT_EXPORT_DEF( FT_Error )
  FTC_SBit_Cache_Lookup( FTC_SBit_Cache   cache,
                         FTC_Image_Desc*  desc,
                         FTC_SBit        *sbit );


#ifdef __cplusplus
  }
#endif


#endif /* FTCSBITS_H */

/* END */

