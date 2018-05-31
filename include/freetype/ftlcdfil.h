/***************************************************************************/
/*                                                                         */
/*  ftlcdfil.h                                                             */
/*                                                                         */
/*    FreeType API for color filtering of subpixel bitmap glyphs           */
/*    (specification).                                                     */
/*                                                                         */
/*  Copyright 2006-2018 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef FTLCDFIL_H_
#define FTLCDFIL_H_

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_PARAMETER_TAGS_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER

  /***************************************************************************
   *
   * @section:
   *   lcd_filtering
   *
   * @title:
   *   LCD Filtering
   *
   * @abstract:
   *   Remove color fringes of subpixel-rendered bitmaps.
   *
   * @description:
   *   Should you #define FT_CONFIG_OPTION_SUBPIXEL_RENDERING in your
   *   `ftoption.h', which enables patented ClearType-style rendering,
   *   the LCD-optimized glyph bitmaps should be filtered to reduce color
   *   fringes inherent to this technology.  The default FreeType LCD
   *   rendering uses different technology, and API described below,
   *   although available, does nothing.
   *
   *   ClearType-style LCD rendering exploits the color-striped structure of
   *   LCD pixels, increasing the available resolution in the direction of
   *   the stripe (usually horizontal RGB) by a factor of~3.  Using the
   *   subpixels coverages unfiltered can create severe color fringes
   *   especially when rendering thin features.  Indeed, to produce
   *   black-on-white text, the color components must be dimmed equally.
   *
   *   A good 5-tap FIR filter should be applied to subpixel coverages
   *   regardless of pixel boundaries and should have these properties:
   *
   *   1) It should be symmetrical, like {~a, b, c, b, a~}, to avoid
   *      any shifts in appearance.
   *
   *   2) It should be color-balanced, meaning a~+ b~=~c, to reduce color
   *      fringes by distributing the computed coverage for one subpixel to
   *      all subpixels equally.
   *
   *   3) It should be normalized, meaning 2a~+ 2b~+ c~=~1.0 to maintain
   *      overall brightness.
   *
   *   Use the @FT_Library_SetLcdFilter API to specify a low-pass filter,
   *   which is then applied to subpixel-rendered bitmaps generated through
   *   @FT_Render_Glyph.  The filter affects glyph bitmaps rendered through
   *   @FT_Render_Glyph, @FT_Load_Glyph, and @FT_Load_Char.  It does _not_
   *   affect the output of @FT_Outline_Render and @FT_Outline_Get_Bitmap.
   *
   *   If this feature is activated, the dimensions of LCD glyph bitmaps are
   *   either wider or taller than the dimensions of the corresponding
   *   outline with regard to the pixel grid.  For example, for
   *   @FT_RENDER_MODE_LCD, the filter adds 2~subpixels to the left, and
   *   2~subpixels to the right.  The bitmap offset values are adjusted
   *   accordingly, so clients shouldn't need to modify their layout and
   *   glyph positioning code when enabling the filter.
   *
   *   Only filtering along with gamma-corrected alpha blending can completely
   *   remove color fringes.  Boxy 3-tap filter {0, 1/3, 1/3, 1/3, 0} is
   *   sharper but is less forgiving of non-ideal gamma curves of a screen
   *   (viewing angles!), beveled filters are fuzzier but more tolerant.
   *
   *   Each of the 3~alpha values (subpixels) is independently used to blend
   *   one color channel.  That is, red alpha blends the red channel of the
   *   text color with the red channel of the background pixel.  The
   *   distribution of density values by the color-balanced filter assumes
   *   alpha blending is done in linear space; only then color artifacts
   *   cancel out.
   */


  /****************************************************************************
   *
   * @enum:
   *   FT_LcdFilter
   *
   * @description:
   *   A list of values to identify various types of LCD filters.
   *
   * @values:
   *   FT_LCD_FILTER_NONE ::
   *     Do not perform filtering.  When used with subpixel rendering, this
   *     results in sometimes severe color fringes.
   *
   *   FT_LCD_FILTER_DEFAULT ::
   *     This is a beveled, normalized, and color-balanced five-tap filter
   *     with weights of [0x08 0x4D 0x56 0x4D 0x08] in 1/256th units.
   *
   *   FT_LCD_FILTER_LIGHT ::
   *     this is a boxy, normalized, and color-balanced three-tap filter
   *     with weights of [0x00 0x55 0x56 0x55 0x00] in 1/256th units.
   *
   *   FT_LCD_FILTER_LEGACY ::
   *   FT_LCD_FILTER_LEGACY1 ::
   *     This filter corresponds to the original libXft color filter.  It
   *     provides high contrast output but can exhibit really bad color
   *     fringes if glyphs are not extremely well hinted to the pixel grid.
   *     This filter is only provided for comparison purposes, and might be
   *     disabled or stay unsupported in the future. The second value is
   *     provided for compatibility with FontConfig, which historically
   *     used different enumeration, sometimes incorrectly forwarded to
   *     FreeType.
   *
   * @since:
   *   2.3.0 (`FT_LCD_FILTER_LEGACY1' since 2.6.2)
   */
  typedef enum  FT_LcdFilter_
  {
    FT_LCD_FILTER_NONE    = 0,
    FT_LCD_FILTER_DEFAULT = 1,
    FT_LCD_FILTER_LIGHT   = 2,
    FT_LCD_FILTER_LEGACY1 = 3,
    FT_LCD_FILTER_LEGACY  = 16,

    FT_LCD_FILTER_MAX   /* do not remove */

  } FT_LcdFilter;


  /**************************************************************************
   *
   * @func:
   *   FT_Library_SetLcdFilter
   *
   * @description:
   *   This function is used to apply color filtering to LCD decimated
   *   bitmaps, like the ones used when calling @FT_Render_Glyph with
   *   @FT_RENDER_MODE_LCD or @FT_RENDER_MODE_LCD_V.
   *
   * @input:
   *   library ::
   *     A handle to the target library instance.
   *
   *   filter ::
   *     The filter type.
   *
   *     You can use @FT_LCD_FILTER_NONE here to disable this feature, or
   *     @FT_LCD_FILTER_DEFAULT to use a default filter that should work
   *     well on most LCD screens.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   This feature is always disabled by default.  Clients must make an
   *   explicit call to this function with a `filter' value other than
   *   @FT_LCD_FILTER_NONE in order to enable it.
   *
   *   Due to *PATENTS* covering subpixel rendering, this function doesn't
   *   do anything except returning `FT_Err_Unimplemented_Feature' if the
   *   configuration macro FT_CONFIG_OPTION_SUBPIXEL_RENDERING is not
   *   defined in your build of the library, which should correspond to all
   *   default builds of FreeType.
   *
   * @since:
   *   2.3.0
   */
  FT_EXPORT( FT_Error )
  FT_Library_SetLcdFilter( FT_Library    library,
                           FT_LcdFilter  filter );


  /**************************************************************************
   *
   * @func:
   *   FT_Library_SetLcdFilterWeights
   *
   * @description:
   *   This function can be used to enable LCD filter with custom weights,
   *   instead of using presets in @FT_Library_SetLcdFilter.
   *
   * @input:
   *   library ::
   *     A handle to the target library instance.
   *
   *   weights ::
   *     A pointer to an array; the function copies the first five bytes and
   *     uses them to specify the filter weights in 1/256th units.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   Due to *PATENTS* covering subpixel rendering, this function doesn't
   *   do anything except returning `FT_Err_Unimplemented_Feature' if the
   *   configuration macro FT_CONFIG_OPTION_SUBPIXEL_RENDERING is not
   *   defined in your build of the library, which should correspond to all
   *   default builds of FreeType.
   *
   *   LCD filter weights can also be set per face using @FT_Face_Properties
   *   with @FT_PARAM_TAG_LCD_FILTER_WEIGHTS.
   *
   * @since:
   *   2.4.0
   */
  FT_EXPORT( FT_Error )
  FT_Library_SetLcdFilterWeights( FT_Library      library,
                                  unsigned char  *weights );


  /*
   * @type:
   *   FT_LcdFiveTapFilter
   *
   * @description:
   *   A typedef for passing the five LCD filter weights to
   *   @FT_Face_Properties within an @FT_Parameter structure.
   *
   * @since:
   *   2.8
   *
   */
#define FT_LCD_FILTER_FIVE_TAPS  5

  typedef FT_Byte  FT_LcdFiveTapFilter[FT_LCD_FILTER_FIVE_TAPS];


  /* */


FT_END_HEADER

#endif /* FTLCDFIL_H_ */


/* END */
