/***************************************************************************/
/*                                                                         */
/*  ftlcdfil.h                                                             */
/*                                                                         */
/*    FreeType API for color filtering of subpixel bitmap glyphs           */
/*    (specification).                                                     */
/*                                                                         */
/*  Copyright 2006 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FT_LCD_FILTER_H__
#define __FT_LCD_FILTER_H__

#include <ft2build.h>
#include FT_FREETYPE_H


FT_BEGIN_HEADER


 /****************************************************************************
  *
  * @func:
  *   FT_LcdFilter
  *
  * @description:
  *    a list of values used to identify various types of LCD filters
  *
  * @values:
  *   FT_LCD_FILTER_NONE :: value 0 means do not perform filtering. when
  *     used with subpixel rendering, this will result in sometimes severe
  *     color fringes
  *
  *   FT_LCD_FILTER_DEFAULT ::
  *      the default filter reduces color fringes considerably, at the cost of
  *      a slight bluriness in the output
  *
  *   FT_LCD_FILTER_LIGHT ::
  *      the light filter is a variant that produces less bluriness
  *      at the cost of slightly more color fringes than the default one. It
  *      might be better than the default one, depending on your monitor and
  *      personal vision.
  *
  *   FT_LCD_FILTER_LEGACY ::
  *      this filter corresponds to the original libXft color filter, this
  *      provides high contrast output, but can exhibit really bad color fringes
  *      if your glyphs are not extremely well hinted to the pixel grid. In
  *      other words, it only works well when enabling the TrueType bytecode
  *      interpreter *and* using high-quality hinted fonts. It will suck for
  *      all other cases.
  *
  *      this filter is only provided for comparison purposes, and might be
  *      disabled/unsupported in the future...
  */
  typedef enum
  {
    FT_LCD_FILTER_NONE    = 0,
    FT_LCD_FILTER_DEFAULT = 1,
    FT_LCD_FILTER_LIGHT   = 2,
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
   *   filter_weights ::
   *     A pointer to an array of 5 bytes corresponding to the weights of a
   *     5-tap FIR filter.  Each weight must be positive, and their sum
   *     should be at least 256 to avoid loss of darkness in the rendered
   *     glyphs.  The sum can be greater than 256 to darken the glyphs
   *     (`el-cheapo gamma').
   *
   *     You can use @FT_LCD_FILTER_NONE here to disable this feature, or
   *     @FT_LCD_FILTER_DEFAULT to use a default filter that should work
   *     well on most LCD screens.
   *
   * @return:
   *   FreeType error code.  0 means success.
   *
   * @note:
   *   This feature is always disabled by default.  Clients must make an
   *   explicit call to this function with a `filter_weights' value other
   *   than @FT_LCD_FILTER_NONE in order to enable it.
   *
   *   Due to *PATENTS* covering subpixel rendering, this function doesn't
   *   do anything except returning @FT_Err_Unimplemented_Feature if the
   *   configuration macro FT_CONFIG_OPTION_SUBPIXEL_RENDERING is not
   *   defined in your build of the library, which should correspond to all
   *   default builds of the library.
   *
   *   The filter affects glyph bitmaps rendered through FT_Render_Glyph,
   *   @@FT_Glyph_Get_Bitmap, @FT_Load_Glyph, and FT_Load_Char.
   *
   *   It does _not_ affect the output of @FT_Outline_Render and
   *   @FT_Outline_Get_Bitmap.
   *
   *   If this feature is activated, the dimensions of LCD glyph bitmaps are
   *   either larger or taller than the dimensions of the corresponding
   *   outline with regards to the pixel grid.  For example, for
   *   @FT_RENDER_MODE_LCD, the filter adds up to 3 pixels to the left, and
   *   up to 3 pixels to the right.
   *
   *   The bitmap offset values are adjusted correctly, so clients shouldn't
   *   need to modify their layout and glyph positioning code when enabling
   *   the filter.
   *
   */
  FT_EXPORT( FT_Error )
  FT_Library_SetLcdFilter( FT_Library    library,
                           FT_LcdFilter  filter );

  /* */


FT_END_HEADER

#endif /* __FT_LCD_FILTER_H__ */


/* END */
