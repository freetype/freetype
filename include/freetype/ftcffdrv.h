/***************************************************************************/
/*                                                                         */
/*  ftcffdrv.h                                                             */
/*                                                                         */
/*    FreeType API for controlling the CFF driver (specification only).    */
/*                                                                         */
/*  Copyright 2013 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTCFFDRV_H__
#define __FTCFFDRV_H__

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER


  /**************************************************************************
   *
   * @section:
   *   cff_driver
   *
   * @title:
   *   The CFF driver
   *
   * @abstract:
   *   Controlling the CFF driver module.
   *
   * @description:
   *   While FreeType's CFF driver doesn't expose API functions by itself,
   *   it is possible to control its behaviour with @FT_Property_Set and
   *   @FT_Property_Get.  The following lists the available properties
   *   together with the necessary macros and structures.
   *
   *   The CFF driver's module name is `cff'.
   *
   */


  /**************************************************************************
   *
   * @property:
   *   hinting-engine
   *
   * @description:
   *   Thanks to Adobe, which contributed a new hinting (and parsing)
   *   engine, an application can select between `freetype' and `adobe' if
   *   compiled with CFF_CONFIG_OPTION_OLD_ENGINE.  If this configuration
   *   macro isn't defined, `hinting-engine' does nothing.
   *
   *   The default engine is `freetype' if CFF_CONFIG_OPTION_OLD_ENGINE is
   *   defined, and `adobe' otherwise.
   *
   *   The following example code demonstrates how to select Adobe's hinting
   *   engine (omitting the error handling).
   *
   *   {
   *     FT_Library  library;
   *     FT_UInt     hinting_engine = FT_CFF_HINTING_ADOBE;
   *
   *
   *     FT_Init_FreeType( &library );
   *
   *     FT_Property_Set( library, "cff",
   *                               "hinting-engine", &hinting_engine );
   *   }
   *
   * @note:
   *   This property can be used with @FT_Property_Get also.
   *
   */


  /**************************************************************************
   *
   * @enum:
   *   FT_CFF_HINTING_XXX
   *
   * @description:
   *   A list of constants used for the @hinting-engine property to select
   *   the hinting engine for CFF fonts.
   *
   * @values:
   *   FT_CFF_HINTING_FREETYPE ::
   *     Use the old FreeType hinting engine.
   *
   *   FT_CFF_HINTING_ADOBE ::
   *     Use the hinting engine contributed by Adobe.
   *
   */
#define FT_CFF_HINTING_FREETYPE  0
#define FT_CFF_HINTING_ADOBE     1


  /**************************************************************************
   *
   * @property:
   *   no-stem-darkening
   *
   * @description:
   *   By default, the Adobe CFF engine darkens stems at smaller sizes,
   *   regardless of hinting, to enhance contrast.  Setting this property,
   *   stem darkening gets switched off.
   *
   *   Note that stem darkening is never applied if @FT_LOAD_NO_SCALE is set.
   *
   *   {
   *     FT_Library  library;
   *     FT_Bool     no_stem_darkening = TRUE;
   *
   *
   *     FT_Init_FreeType( &library );
   *
   *     FT_Property_Set( library, "cff",
   *                               "no-stem-darkening", &no_stem_darkening );
   *   }
   *
   * @note:
   *   This property can be used with @FT_Property_Get also.
   *
   */


  /**************************************************************************
   *
   * @property:
   *   darkening-parameters
   *
   * @description:
   *   By default, the Adobe CFF engine darkens stems as follows (if the
   *   `no-stem-darkening' property isn't set):
   *
   *   {
   *     stem width <= 0.5px:   darkening amount = 0.4px
   *     stem width  = 1px:     darkening amount = 0.275px
   *     stem width  = 1.667px: darkening amount = 0.275px
   *     stem width >= 2.333px: darkening amount = 0px
   *   }
   *
   *   and piecewise linear in-between.  Using the `darkening-parameters'
   *   property, these four control points can be changed, as the following
   *   example demonstrates.
   *
   *   {
   *     FT_Library  library;
   *     FT_Int      darken_params[8] = {  500, 300,   // x1, y1
   *                                      1000, 200,   // x2, y2
   *                                      1500, 100,   // x3, y3
   *                                      2000,   0 }; // x4, y4
   *
   *
   *     FT_Init_FreeType( &library );
   *
   *     FT_Property_Set( library, "cff",
   *                               "darkening-parameters", darken_params );
   *   }
   *
   *   The x~values give the stem width, and the y~values the darkening
   *   amount.  The unit is 1000th of pixels.  All coordinate values must be
   *   positive; the x~values must be monotonically increasing, and the
   *   y~values smaller than or equal to 500 (corresponding to half a
   *   pixel).
   *
   * @note:
   *   This property can be used with @FT_Property_Get also.
   *
   */


 /* */

FT_END_HEADER


#endif /* __FTCFFDRV_H__ */


/* END */
