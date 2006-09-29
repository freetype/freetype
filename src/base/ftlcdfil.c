/***************************************************************************/
/*                                                                         */
/*  ftlcdfil.c                                                             */
/*                                                                         */
/*    FreeType API for color filtering of subpixel bitmap glyphs (body).   */
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


#include <ft2build.h>
#include FT_LCD_FILTER_H
#include FT_IMAGE_H
#include FT_INTERNAL_OBJECTS_H


#ifdef FT_CONFIG_OPTION_SUBPIXEL_RENDERING

  /* The smooth rasterizer invokes this function. */
  static void
  _ft_lcd_filter( FT_Bitmap*      bitmap,
                  FT_Render_Mode  mode,
                  FT_Byte*        weights )
  {
    FT_UInt  width  = (FT_UInt)bitmap->width;
    FT_UInt  height = (FT_UInt)bitmap->rows;


    /* horizontal in-place FIR filter */
    if ( mode == FT_RENDER_MODE_LCD && width >= 4 )
    {
      FT_Byte*  line = bitmap->buffer;


      for ( ; height > 0; height--, line += bitmap->pitch )
      {
        FT_UInt  fir[5];
        FT_UInt  val1, xx;


        val1   = line[0];
        fir[0] = weights[2] * val1;
        fir[1] = weights[3] * val1;
        fir[2] = weights[4] * val1;
        fir[3] = 0;
        fir[4] = 0;

        val1    = line[1];
        fir[0] += weights[1] * val1;
        fir[1] += weights[2] * val1;
        fir[2] += weights[3] * val1;
        fir[3] += weights[4] * val1;

        for ( xx = 2; xx < width; xx++ )
        {
          FT_UInt  val, pix;


          val    = line[xx];
          pix    = fir[0] + weights[0] * val;
          fir[0] = fir[1] + weights[1] * val;
          fir[1] = fir[2] + weights[2] * val;
          fir[2] = fir[3] + weights[3] * val;
          fir[3] =          weights[4] * val;

          pix        >>= 8;
          pix         |= -( pix >> 8 );
          line[xx - 2] = (FT_Byte)pix;
        }

        {
          FT_UInt  pix;


          pix          = fir[0] >> 8;
          pix         |= -( pix >> 8 );
          line[xx - 2] = (FT_Byte)pix;

          pix          = fir[1] >> 8;
          pix         |= -( pix >> 8 );
          line[xx - 1] = (FT_Byte)pix;
        }
      }
    }

    /* vertical in-place FIR filter */
    else if ( mode == FT_RENDER_MODE_LCD_V && height >= 4 )
    {
      FT_Byte*  column = bitmap->buffer;
      FT_Int    pitch  = bitmap->pitch;


      for ( ; width > 0; width--, column++ )
      {
        FT_Byte*  col = column;
        FT_UInt   fir[5];
        FT_UInt   val1, yy;


        val1   = col[0];
        fir[0] = weights[2] * val1;
        fir[1] = weights[3] * val1;
        fir[2] = weights[4] * val1;
        fir[3] = 0;
        fir[4] = 0;
        col   += pitch;

        val1    = col[0];
        fir[0] += weights[1] * val1;
        fir[1] += weights[2] * val1;
        fir[2] += weights[3] * val1;
        fir[3] += weights[4] * val1;
        col    += pitch;

        for ( yy = 2; yy < height; yy++ )
        {
          FT_UInt  val, pix;


          val    = col[0];
          pix    = fir[0] + weights[0] * val;
          fir[0] = fir[1] + weights[1] * val;
          fir[1] = fir[2] + weights[2] * val;
          fir[2] = fir[3] + weights[3] * val;
          fir[3] =          weights[4] * val;

          pix           >>= 8;
          pix            |= -( pix >> 8 );
          col[-2 * pitch] = (FT_Byte)pix;
          col            += pitch;
        }

        {
          FT_UInt  pix;


          pix             = fir[0] >> 8;
          pix            |= -( pix >> 8 );
          col[-2 * pitch] = (FT_Byte)pix;

          pix         = fir[1] >> 8;
          pix        |= -( pix >> 8 );
          col[-pitch] = (FT_Byte)pix;
        }
      }
    }
  }


  FT_EXPORT( FT_Error )
  FT_Library_SetLcdFilter( FT_Library      library,
                           const FT_Byte*  filter_weights )
  {
    static const FT_Byte  default_filter[5] = { 0x10, 0x40, 0x70, 0x40, 0x10 };


    if ( library == NULL )
      return FT_Err_Invalid_Argument;

    if ( filter_weights == FT_LCD_FILTER_NONE )
    {
      library->lcd_filter = NULL;
      return 0;
    }

    if ( filter_weights == FT_LCD_FILTER_DEFAULT )
      filter_weights = default_filter;

    memcpy( library->lcd_filter_weights, filter_weights, 5 );
    library->lcd_filter = _ft_lcd_filter;

    return 0;
  }

#else /* !FT_CONFIG_OPTION_SUBPIXEL_RENDERING */

  FT_EXPORT( FT_Error )
  FT_Library_SetLcdFilter( FT_Library      library,
                           const FT_Byte*  filter_weights )
  {
    FT_UNUSED( library );
    FT_UNUSED( filter_weights );

    return FT_Err_Unimplemented_Feature;
  }

#endif /* !FT_CONFIG_OPTION_SUBPIXEL_RENDERING */


/* END */
