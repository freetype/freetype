/***************************************************************************/
/*                                                                         */
/*  ftautoh.h                                                              */
/*                                                                         */
/*    FreeType API for setting and accessing auto-hinter properties        */
/*    (specification).                                                     */
/*                                                                         */
/*  Copyright 2012 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTAUTOH_H__
#define __FTAUTOH_H__

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    autohinter                                                         */
  /*                                                                       */
  /* <Title>                                                               */
  /*    The Auto-hinter                                                    */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    Controlling the behaviour of the auto-hinting engine.              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains the declaration of functions specific to the */
  /*    auto-hinter.                                                       */
  /*                                                                       */
  /*************************************************************************/


  /**********************************************************************
   *
   * @enum:
   *    FT_AUTOHINTER_XXX
   *
   * @description:
   *    A list of bit-field constants used with
   *    @FT_Library_GetAutohinterProperties,
   *    @FT_Face_GetAutohinterProperties,
   *    @FT_Library_SetAutohinterProperties, and
   *    @FT_Face_SetAutohinterProperties to control the behaviour of the
   *    auto-hinter.
   *
   * @values:
   *    FT_AUTOHINTER_DEFAULT::
   *      Corresponding to~0, this value indicates the default value.
   *
   *    FT_AUTOHINTER_INCREASE_GLYPH_HEIGHTS ::
   *      For glyphs in the size range 5 < PPEM < 15, round up the glyph
   *      height much more often than normally.
   */
#define FT_AUTOHINTER_DEFAULT                 0x0
#define FT_AUTOHINTER_INCREASE_GLYPH_HEIGHTS  ( 1L << 0 )


 /**********************************************************************
  *
  * @function:
  *    FT_Library_GetAutohinterProperties
  *
  * @description:
  *    Retrieve the global property flags which control the behaviour of the
  *    auto-hinter.
  *
  * @output:
  *    properties ::
  *       The current global auto-hinter property flags, consisting of
  *       @FT_AUTOHINTER_XXX constants which are ORed together.
  *
  *       Use @FT_Face_GetAutohinterProperties to retrieve the local
  *       properties of a face.
  *
  * @return:
  *   FreeType error code.  0~means success.
  */
  FT_EXPORT( FT_Error )
  FT_Library_GetAutohinterProperties( FT_Library  library,
                                      FT_Int32   *properties );


 /**********************************************************************
  *
  * @function:
  *    FT_Library_SetAutohinterProperties
  *
  * @description:
  *    Set the global property flags which control the behaviour of the
  *    auto-hinter.
  *
  * @input:
  *    properties ::
  *       The auto-hinter property flags to be set globally, consisting of
  *       @FT_AUTOHINTER_XXX constants which are ORed together.  All faces
  *       created after a call to this function inherit the new auto-hinter
  *       properties.
  *
  *       Use @FT_Face_SetAutohinterProperties to override the properties
  *       locally.
  *
  * @return:
  *   FreeType error code.  0~means success.
  */
  FT_EXPORT( FT_Error )
  FT_Library_SetAutohinterProperties( FT_Library  library,
                                      FT_Int32    properties );


 /**********************************************************************
  *
  * @function:
  *    FT_Face_GetAutohinterProperties
  *
  * @description:
  *    Retrieve the property flags which control the behaviour of the
  *    auto-hinter for the given face.
  *
  * @input:
  *    face ::
  *       A handle to the input face.
  *
  * @output:
  *    properties ::
  *       The current auto-hinter property flags of the given face,
  *       consisting of @FT_AUTOHINTER_XXX constants which are ORed
  *       together.
  *
  * @return:
  *   FreeType error code.  0~means success.
  *
  */
  FT_EXPORT( FT_Error )
  FT_Face_GetAutohinterProperties( FT_Face    face,
                                   FT_Int32  *properties );


 /**********************************************************************
  *
  * @function:
  *    FT_Face_SetAutohinterProperties
  *
  * @description:
  *    Set the property flags which control the behaviour of the autolhinter
  *    for the given face.
  *
  * @input:
  *    face ::
  *       A handle to the input face.
  *
  *    properties ::
  *       The auto-hinter property flags to be set for the given face,
  *       consisting of @FT_AUTOHINTER_XXX constants which are ORed
  *       together.
  *
  *       By default, a face inherits the global auto-hinter properties (set
  *       with @FT_Library_SetAutohinterProperties, if any) at the time of
  *       its creation.  Using this function you can override them locally.
  *
  * @return:
  *   FreeType error code.  0~means success.
  */
  FT_EXPORT( FT_Error )
  FT_Face_SetAutohinterProperties( FT_Face   face,
                                   FT_Int32  properties );

 /* */

FT_END_HEADER

#endif /* __FTAUTOH_H__ */


/* END */
