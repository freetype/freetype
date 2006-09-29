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
  FT_Library_SetLcdFilter( FT_Library      library,
                           const FT_Byte*  filter_weights );



  /**************************************************************************
   *
   * @enum:
   *   FT_LCD_FILTER_XXX
   *
   * @description:
   *   A list of constants which correspond to useful lcd filter settings
   *   for the @FT_Library_SetLcdFilter function.
   *
   * @values:
   *   FT_LCD_FILTER_NONE ::
   *     The value NULL is reserved to indicate that LCD color filtering
   *     should be disabled.
   *
   *   FT_LCD_FILTER_DEFAULT ::
   *     This value is reserved to indicate a default FIR filter that should
   *     work well on most LCD screen.  It corresponds to the array 0x10,
   *     0x40, 0x70, 0x40, 0x10.
   *
   */
#define FT_LCD_FILTER_NONE     ( (const FT_Byte*)NULL )
 
#define FT_LCD_FILTER_DEFAULT  ( (const FT_Byte*)(void*)(ft_ptrdiff_t)1 )

  /* */


FT_END_HEADER

#endif /* __FT_LCD_FILTER_H__ */


/* END */
