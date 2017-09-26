/***************************************************************************/
/*                                                                         */
/*  ftt1drv.h                                                              */
/*                                                                         */
/*    FreeType API for controlling the Type 1 driver (specification only). */
/*                                                                         */
/*  Copyright 2017 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef FTT1DRV_H_
#define FTT1DRV_H_

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
   * @property:
   *   hinting-engine[type1]
   *
   * @description:
   *   Thanks to Adobe, which contributed a new hinting (and parsing)
   *   engine, an application can select between `freetype' and `adobe' if
   *   compiled with T1_CONFIG_OPTION_OLD_ENGINE.  If this configuration
   *   macro isn't defined, `hinting-engine' does nothing.
   *
   *   The default engine is `freetype' if T1_CONFIG_OPTION_OLD_ENGINE is
   *   defined, and `adobe' otherwise.
   *
   *   The following example code demonstrates how to select Adobe's hinting
   *   engine (omitting the error handling).
   *
   *   {
   *     FT_Library  library;
   *     FT_UInt     hinting_engine = FT_T1_HINTING_ADOBE;
   *
   *
   *     FT_Init_FreeType( &library );
   *
   *     FT_Property_Set( library, "type1",
   *                               "hinting-engine", &hinting_engine );
   *   }
   *
   * @note:
   *   This property can be used with @FT_Property_Get also.
   *
   *   This property can be set via the `FREETYPE_PROPERTIES' environment
   *   variable (using values `adobe' or `freetype').
   *
   * @since:
   *   2.8.2
   *
   */


  /**************************************************************************
   *
   * @enum:
   *   FT_T1_HINTING_XXX
   *
   * @description:
   *   A list of constants used for the @hinting-engine[type1] property to
   *   select the hinting engine for Type 1 fonts.
   *
   * @values:
   *   FT_T1_HINTING_FREETYPE ::
   *     Use the old FreeType hinting engine.
   *
   *   FT_T1_HINTING_ADOBE ::
   *     Use the hinting engine contributed by Adobe.
   *
   * @since:
   *   2.8.2
   *
   */
#define FT_T1_HINTING_FREETYPE  0
#define FT_T1_HINTING_ADOBE     1


  /**************************************************************************
   *
   * @constant:
   *   FT_PARAM_TAG_RANDOM_SEED
   *
   * @description:
   *   An @FT_Parameter tag to be used with @FT_Face_Properties.  The
   *   corresponding 32bit signed integer argument overrides the CFF
   *   module's random seed value with a face-specific one; see
   *   @random-seed.
   *
   */
#define FT_PARAM_TAG_RANDOM_SEED \
          FT_MAKE_TAG( 's', 'e', 'e', 'd' )

  /* */


FT_END_HEADER


#endif /* FTT1DRV_H_ */


/* END */
