#ifndef __FT_LCD_FILTER_H__
#define __FT_LCD_FILTER_H__

#include <ft2build.h>
#include FT_FREETYPE_H

FT_BEGIN_HEADER

 /**
  * @func: FT_Library_SetLcdFilter
  *
  * @description:
  *   this function is used to apply color filtering to LCD decimated
  *   bitmaps, like the ones used when calling @FT_Render_Glyph with
  *   @FT_RENDER_MODE_LCD or @FT_RENDER_MODE_LCD_V.
  *
  * @input:
  *   library  :: handle to target library instance
  *
  *   filter_weights :: a pointer to an array of 5 bytes corresponding
  *                     to the weights of a 5-tap FIR filter. Each
  *                     weight must be positive, and their sum should
  *                     be at least 256 to avoid loss of darkness
  *                     in the rendered glyphs. The sum can be greater
  *                     than 256 to darken the glyphs (el-cheapo gamma)
  *
  *                     you can use @FT_LCD_FILTER_NONE here to disable
  *                     this feature, or @FT_LCD_FILTER_DEFAULT to use
  *                     a default filter that should work well on most
  *                     LCD screens.
  *
  * @return:
  *   error code. 0 means success
  *
  * @note:
  *   this feature is always disabled by default. Clients must make an
  *   explicit call to this function with a 'filter_weights' value other
  *   than @FT_LCD_FILTER_NONE in order to enable it.
  *
  *   due to *PATENTS* covering subpixel rendering, this function will
  *   not do anything except return @FT_Err_Unimplemented_Feature if the
  *   configuration macro FT_CONFIG_OPTION_SUBPIXEL_RENDERING is not
  *   defined in your build of the library, which should correspond
  *   to all default builds of the library
  *
  *   the filter affects glyph bitmaps rendered through
  *   @FT_Render_Glyph, @FT_Glyph_Get_Bitmap, @FT_Load_Glyph and
  *   @FT_Load_Char.
  *
  *   It does *not* affect the output of @FT_Outline_Render
  *   and @FT_Outline_Get_Bitmap.
  *
  *   if this feature is activated, the dimensions of LCD glyph bitmaps
  *   will be either larger or taller than the dimensions of the corresponding
  *   outline with regards to the pixel grid. For example, for @FT_RENDER_MODE_LCD,
  *   the filter adds up to 3 pixels to the left, and up to 3 pixels to the right.
  *
  *   the bitmap offset values are adjusted correctly, so clients shouldn't need
  *   to modify thei layout / glyph positioning code when enabling the filter.
  */
  FT_EXPORT( FT_Error )
  FT_Library_SetLcdFilter( FT_Library       library,
                           const FT_Byte*   filter_weights );


 /**
  * @enum: FT_LCD_FILTER_XXX
  *
  * @desc: a list of constants correspond to useful lcd filter settings to
  *        be used when calling @FT_Library_SetLcdFilter
  *
  * @values:
  *    FT_LCD_FILTER_NONE :: the value NULL is reserved to indicate that
  *                          LCD color filtering should be disabled.
  *
  *    FT_LCD_FILTER_DEFAULT ::
  *       this value is reserved to indicate a default FIR filter that
  *       should work well on most LCD screen. For the really curious,
  *       it corresponds to the array 0x10, 0x40, 0x70, 0x40, 0x10
  */
#define  FT_LCD_FILTER_NONE        ((const FT_Byte*)NULL )

#define  FT_LCD_FILTER_DEFAULT     ((const FT_Byte*)(void*)(ft_ptrdiff_t)1)

/* */

FT_END_HEADER

#endif /* __FT_LCD_FILTER_H__ */
