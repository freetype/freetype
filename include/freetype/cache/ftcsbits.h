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


#ifndef __FTCSBITS_H__
#define __FTCSBITS_H__


#include <ft2build.h>
#include FT_CACHE_H
#include FT_CACHE_INTERNAL_CHUNK_H
#include FT_CACHE_IMAGE_H



FT_BEGIN_HEADER

  /***
   * <Section> cache_subsystem
   */

 /***********************************************************************
  *
  * <Type> FTC_SBit
  *
  * <Description>
  *    handle to a small bitmap descriptor. see the FTC_SBitRec
  *    structure for details..
  */
  typedef struct FTC_SBitRec_*  FTC_SBit;

 /***********************************************************************
  *
  * <Type> FTC_SBit_Cache
  *
  * <Description>
  *    handle to a small bitmap cache. These are special cache objects
  *    used to store small glyph bitmaps (and anti-aliased pixmaps) in
  *    a much more efficient way than the traditional glyph image cache
  *    implemented by FTC_Image_Cache
  */
  typedef struct FTC_SBit_CacheRec_*  FTC_SBit_Cache;

 /***********************************************************************
  *
  * <Struct> FTC_SBitRec
  *
  * <Description>
  *    a very compact structure used to describe a small glyph bitmap
  *
  * <Fields>
  *    width    :: bitmap width in pixels
  *    height   :: bitmap height in pixels
  *
  *    left     :: horizontal distance from pen position to left bitmap
  *                border (a.k.a. "left side bearing", or "lsb")
  *
  *    top      :: vertical distance from pen position (on the baseline)
  *                to the upper bitmap border (a.k.a. "top side bearing")
  *                the distance is positive for upwards Y coordinates.
  *
  *    format   :: format of glyph bitmap (mono or gray)
  *
  *    pitch    :: number of bytes per bitmap lines. may be positive or
  *                negative
  *
  *    xadvance :: horizontal advance width in pixels
  *    yadvance :: vertical advance height in pixels
  *
  *    buffer   :: pointer to bitmap pixels
  */
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


 /*************************************************************************
  *
  * <Section> FTC_SBit_Cache_New
  *
  * <Description>
  *    Create a new cache to store small glyph bitmaps
  *
  * <Input>
  *    manager :: handle to source cache manager
  *
  * <Output>
  *    acache  :: handle to new sbit cache. NULL in case of error
  *
  * <Return>
  *    error code. 0 means success
  */
  FT_EXPORT( FT_Error )  FTC_SBit_Cache_New( FTC_Manager      manager,
                                             FTC_SBit_Cache  *acache );

 /*************************************************************************
  *
  * <Section> FTC_SBit_Cache_Lookup
  *
  * <Description>
  *    Lookup a given small glyph bitmap in a given sbit cache
  *
  * <Input>
  *    cache  :: handle to source sbit cache
  *    desc   :: pointer to glyph image descriptor
  *    gindex :: glyph index
  *
  * <Output>
  *    sbit   :: handle to a small bitmap descriptor
  *
  * <Return>
  *    error code. 0 means success
  *
  * <Note>
  *    the small bitmap descriptor, and its bit buffer are owned by the
  *    cache and should never be freed by the application. They might
  *    as well disappear from memory on the next cache lookup, so don't
  *    treat them like persistent data..
  *
  *    the descriptor's "buffer" field is set to 0 to indicate a missing
  *    glyph bitmap.
  */
  FT_EXPORT( FT_Error )  FTC_SBit_Cache_Lookup( FTC_SBit_Cache   cache,
                                                FTC_Image_Desc*  desc,
                                                FT_UInt          gindex,
                                                FTC_SBit        *sbit );

  /* */

FT_END_HEADER

#endif /* __FTCSBITS_H__ */


/* END */
