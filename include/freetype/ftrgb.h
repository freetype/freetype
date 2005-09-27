/***************************************************************************/
/*                                                                         */
/*  ftrgb.h                                                                */
/*                                                                         */
/*    RGB color filtering support for FreeType 2 (specification only).     */
/*                                                                         */
/*  Copyright 2005 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef __FT_RGB_H__
#define __FT_RGB_H__

#include <ft2build.h>
#include FT_FREETYPE_H

FT_BEGIN_HEADER

/**
 * @type: FT_RgbFilter
 *
 * @description:
 *   an opaque handle to an object modelling an RGB color filter.
 *   this is used to process LCD-decimated bitmaps in order to reduce
 *   color fringes during display
 */
typedef struct FT_RgbFilterRec_*   FT_RgbFilter;

/**
 * @func: FT_RgbFilter_ApplyARGB
 *
 * @description:
 *   apply a given color filter to a given bitmap. This function does
 *   nothing if the bitmap's pixel_mode field is not FT_PIXEL_MODE_LCD
 *   or FT_PIXEL_MODE_LCD_V
 *
 * @input:
 *   filter_or_null :: handle to RGB filter object, or NULL to use the
 *                     default one provided with this library
 *
 *   inverted       :: boolean. Set to true if BGR or VBGR filtering is
 *                     needed.
 *
 *   in_mode        :: input bitmap pixel mode. must be either
 *                     @FT_PIXEL_MODE_LCD or @FT_PIXEL_MODE_LCD_V
 *
 *   in_bytes       :: first byte of input bitmap data in memory
 *   in_pitch       :: number of bytes in a row of input pixels. can be negative
 *
 *   out_width      :: width in pixels of output bitmap
 *   out_height     :: width in pixels of output bitmap
 *
 *   out_bytes      :: first byte of output bitmap data in memory, this
 *                     is an array of 32-bit unsigned integer, storing
 *                     pixels values in ARGB8888 format
 *
 *   out_pitch      :: number of _bytes_ in a row of output pixels. can
 *                     be negative. Its absolute value must be greater
 *                     than 4*out_width
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 *   this function returns an error if 'in_mode' isn't set to either
 *   @FT_PIXEL_MODE_LCD or @FT_PIXEL_MODE_LCD_V
 *
 *   when 'in_mode' is @FT_PIXEL_MODE_LCD, this function assumes that the
 *   width of the input bitmap is three times the output's one. otherwise,
 *   it is identical.
 *
 *   when 'in_mode' is @FT_PIXEL_MODE_LCD_V, this function assumes that
 *   the height of the input bitmap is three times the output's one. otherwise
 *   it is identical
 *
 *   the absolute value on 'in_bytes' should always match the number of
 *   _bytes_ in a row of input pixels. It can be negative however to
 *   indicate "inverted" bitmap. See the description of the 'pitch'
 *   field in @FT_Bitmap
 */
FT_EXPORT( FT_Error )
FT_RgbFilter_ApplyARGB( FT_RgbFilter   filter_or_null,
                        FT_Bool        inverted,
                        FT_Pixel_Mode  in_mode,
                        FT_Byte*       in_bytes,
                        FT_Long        in_pitch,
                        FT_Int         out_width,
                        FT_Int         out_height,
                        FT_UInt32*     out_bytes,
                        FT_Long        out_pitch );

/**
 * @func: FT_RgbFilter_ApplyInPlace
 *
 * @description:
 *   a variant of @FT_RgbFilter_ApplyARGB that performs filtering
 *   within the input bitmap. It's up to the caller to convert the
 *   result to a format suitable
 *
 * @input:
 *   filter_or_null :: handle to RGB filter object, or NULL to use the
 *                     default one provided with this library
 *
 *   inverted       :: boolean. Set to true if BGR or VBGR filtering is
 *                     needed.
 *
 *   in_mode        :: input bitmap pixel mode. must be either
 *                     @FT_PIXEL_MODE_LCD or @FT_PIXEL_MODE_LCD_V
 *
 *   in_bytes       :: first byte of input bitmap data in memory
 *   in_pitch       :: number of bytes in a row of input pixels. can be negative
 *
 *   org_width      :: width in pixels of original glyph
 *   org_height     :: height in pixels of original glyph
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 *   this function returns an error if 'in_mode' isn't set to either
 *   @FT_PIXEL_MODE_LCD or @FT_PIXEL_MODE_LCD_V
 *
 *   when 'in_mode' is @FT_PIXEL_MODE_LCD, this function assumes that the
 *   width of the input bitmap is three times 'org_width'. otherwise,
 *   they're identical.
 *
 *   when 'in_mode' is @FT_PIXEL_MODE_LCD_V, this function assumes that
 *   the height of the input bitmap is three times 'org_height'. otherwise
 *   they're identical.
 *
 *   the absolute value on 'in_bytes' should always match the number of
 *   _bytes_ in a row of input pixels. It can be negative however to
 *   indicate "inverted" bitmap. See the description of the 'pitch'
 *   field in @FT_Bitmap
 */
FT_EXPORT( FT_Error )
FT_RgbFilter_ApplyInPlace( FT_RgbFilter   filter_or_null,
                           FT_Bool        inverted,
                           FT_Pixel_Mode  in_mode,
                           FT_Byte*       in_bytes,
                           FT_Long        in_pitch,
                           FT_Int         org_width,
                           FT_Int         org_height );

/**
 * @type: FT_RgbFilter
 *
 * @description:
 *   create a new RGB filter object
 *
 * @input:
 *   library  :: handle to FreeType library instance
 *   values_9 :: an array of 9 16.16 fixed float values
 *
 * @output:
 *   pfilter :: handle to new filter object. NULL in case of error
 *
 * @return:
 *   error code. 0 means success
 *
 * @note:
 *   see the description of 'values_9' in @FT_RgbFilter_Reset.
 */
FT_EXPORT_DEF( FT_Error )
FT_RgbFilter_New( FT_Library     library,
                  FT_Fixed*      values_9,
                  FT_RgbFilter  *pfilter );

/**
 * @type: FT_RgbFilter_Done
 *
 * @description:
 *   destroy a given RGB filter object
 *
 * @input:
 *   filter :: handle to target filter object
 */
FT_EXPORT( void )
FT_RgbFilter_Done( FT_RgbFilter  filter );

/**
 * @type: FT_RgbFilter_Reset
 *
 * @description:
 *    reset a given color filter object
 *
 * @input:
 *   filter   :: handle to target RGB filter
 *   values_9 :: an array of 9 fixed-float multipliers used by the RGB
 *               filter algorithm.
 *
 * @note:
 *   the 9 values are used according to the following scheme, at least
 *   for the 'normal' RGB decimation mode:
 *
 *   {
 *     red   = pix[0]*values[0] + pix[1]*values[1] + pix[2]*values[2];
 *     green = pix[0]*values[3] + pix[1]*values[4] + pix[2]*values[5];
 *     blue  = pix[0]*values[6] + pix[1]*values[7] + pix[2]*values[8];
 *
 *     red   /= 65536;
 *     green /= 65536;
 *     blue  /= 65536;
 *   }
 */
FT_EXPORT( void )
FT_RgbFilter_Reset( FT_RgbFilter  filter,
                    FT_Fixed*     values_9 );

/* */

FT_END_HEADER

#endif /* __FT_RGB_H__ */
