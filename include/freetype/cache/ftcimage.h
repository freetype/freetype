/***************************************************************************/
/*                                                                         */
/*  ftcimage.c                                                             */
/*                                                                         */
/*    FreeType Image cache (body).                                         */
/*                                                                         */
/*  Each image cache really manages FT_Glyph objects :-)                   */
/*                                                                         */
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


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       IMAGE CACHE OBJECT                      *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


#define  FTC_IMAGE_FORMAT( x )  ( (x) & 7 )

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FTC_Image_Type                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration used to list the types of glyph images found in a   */
  /*    glyph image cache.                                                 */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    ftc_image_mono     :: Monochrome bitmap glyphs.                    */
  /*                                                                       */
  /*    ftc_image_grays    :: Anti-aliased bitmap glyphs.                  */
  /*                                                                       */
  /*    ftc_image_outline  :: Scaled (and hinted) outline glyphs.          */
  /*                                                                       */
  /*    ftc_master_outline :: Unscaled original outline glyphs.            */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Other types may be defined in the future.                          */
  /*                                                                       */
  typedef enum  FTC_Image_Type_
  {
    ftc_image_format_bitmap   = 0,
    ftc_image_format_outline  = 1,

    ftc_image_flag_monochrome = 16,
    ftc_image_flag_unhinted   = 32,
    ftc_image_flag_autohinted = 64,
    ftc_image_flag_unscaled   = 128,
    ftc_image_flag_no_sbits   = 256,

    ftc_image_mono    = ftc_image_format_bitmap |
                        ftc_image_flag_monochrome, /* monochrome bitmap   */
    ftc_image_grays   = ftc_image_format_bitmap,   /* anti-aliased bitmap */
    ftc_image_outline = ftc_image_format_outline   /* scaled outline */

  } FTC_Image_Type;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FTC_Image_Desc                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to describe a given glyph image category.  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    size       :: An FTC_SizeRec used to describe the glyph's face &   */
  /*                  size.                                                */
  /*                                                                       */
  /*    image_type :: The glyph image's type.                              */
  /*                                                                       */
  typedef struct  FTC_Image_Desc_
  {
    FTC_FontRec  font;
    FT_UInt      image_type;

  } FTC_Image_Desc;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FTC_Image_Cache                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an glyph image cache object.  They are designed to     */
  /*    hold many distinct glyph images, while not exceeding a certain     */
  /*    memory threshold.                                                  */
  /*                                                                       */
  typedef struct FTC_Image_CacheRec_*  FTC_Image_Cache;


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_Image_Cache_New                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new glyph image cache.                                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    manager   :: The parent manager for the image cache.               */
  /*                                                                       */
  /* <Output>                                                              */
  /*    acache    :: A handle to the new glyph image cache object.         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_EXPORT_DEF( FT_Error )  FTC_Image_Cache_New( FTC_Manager       manager,
                                                  FTC_Image_Cache*  acache );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_Image_Cache_Lookup                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Retrieves a given glyph image from a glyph image cache.            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    cache  :: A handle to the source glyph image cache.                */
  /*                                                                       */
  /*    desc   :: A pointer to a glyph image descriptor.                   */
  /*                                                                       */
  /*    gindex :: The glyph index to retrieve.                             */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aglyph :: The corresponding FT_Glyph object.  0 in case of         */
  /*              failure.                                                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    error code, 0 means success                                        */
  /*                                                                       */
  /* <Note>                                                                */
  /*    the returned glyph is owned and manager by the glyph image cache.  */
  /*    Never try to transform or discard it manually!  You can however    */
  /*    create a copy with FT_Glyph_Copy() and modify the new one.         */
  /*                                                                       */
  /*    Because the glyph image cache limits the total amount of memory    */
  /*    taken by the glyphs it holds, the returned glyph might disappear   */
  /*    on a later invocation of this function!  It's a cache after all... */
  /*                                                                       */
  FT_EXPORT_DEF( FT_Error )
  FTC_Image_Cache_Lookup( FTC_Image_Cache  cache,
                          FTC_Image_Desc*  desc,
                          FT_UInt          gindex,
                          FT_Glyph*        aglyph );



#ifdef __cplusplus
  }
#endif


#endif /* FTCIMAGE_H */


/* END */
