/***************************************************************************/
/*                                                                         */
/*  ftbitmap.c                                                             */
/*                                                                         */
/*    FreeType utility functions for converting 1bpp, 2bpp, 4bpp, and 8bpp */
/*    bitmaps into 8bpp format (body).                                     */
/*                                                                         */
/*  Copyright 2004 by                                                      */
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
#include FT_FREETYPE_H
#include FT_IMAGE_H
#include FT_INTERNAL_OBJECTS_H


  static
  const FT_Bitmap  null_bitmap = { 0, 0, 0, 0, 0, 0, 0, 0 };


  /* documentation is in ftbitmap.h */

  FT_EXPORT_DEF( void )
  FT_Bitmap_New( FT_Bitmap  *abitmap )
  {
    *abitmap = null_bitmap;
  }


  /* documentation is in ftbitmap.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Bitmap_Convert( FT_Library        library,
                     const FT_Bitmap  *source,
                     FT_Bitmap        *target,
                     FT_Int            alignment )
  {
    FT_Error   error = FT_Err_Ok;
    FT_Memory  memory;


    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    memory = library->memory;

    switch ( source->pixel_mode )
    {
    case FT_PIXEL_MODE_MONO:
    case FT_PIXEL_MODE_GRAY:
    case FT_PIXEL_MODE_GRAY2:
    case FT_PIXEL_MODE_GRAY4:
      {
        FT_Int   pad;
        FT_Long  old_size;


        old_size = target->rows * target->pitch;
        if ( old_size < 0 )
          old_size = -old_size;

        target->pixel_mode = FT_PIXEL_MODE_GRAY;
        target->rows       = source->rows;
        target->width      = source->width;

        pad = 0;
        if ( alignment > 0 )
        {
          pad = source->width % alignment;
          if ( pad != 0 )
            pad = alignment - pad;
        }

        target->pitch = source->width + pad;

        if ( target->rows * target->pitch > old_size             &&
             FT_QREALLOC( target->buffer,
                          old_size, target->rows * target->pitch ) )
          return error;
      }
      break;

    default:
      error = FT_Err_Invalid_Argument;
    }

    switch ( source->pixel_mode )
    {
    case FT_PIXEL_MODE_MONO:
      {
        FT_Byte*  s = source->buffer;
        FT_Byte*  t = target->buffer;
        FT_Int    i;


        target->num_grays = 2;

        for ( i = source->rows; i > 0; i-- )
        {
          FT_Byte*  ss = s;
          FT_Byte*  tt = t;
          FT_Int    j;


          /* get the full bytes */
          for ( j = source->width >> 3; j > 0; j-- )
          {
            FT_Int  val = ss[0]; /* avoid a byte->int cast on each line */


            tt[0] = (FT_Byte)( ( val & 0x80 ) >> 7 );
            tt[1] = (FT_Byte)( ( val & 0x40 ) >> 6 );
            tt[2] = (FT_Byte)( ( val & 0x20 ) >> 5 );
            tt[3] = (FT_Byte)( ( val & 0x10 ) >> 4 );
            tt[4] = (FT_Byte)( ( val & 0x08 ) >> 3 );
            tt[5] = (FT_Byte)( ( val & 0x04 ) >> 2 );
            tt[6] = (FT_Byte)( ( val & 0x02 ) >> 1 );
            tt[7] = (FT_Byte)(   val & 0x01 );

            tt += 8;
            ss += 1;
          }

          /* get remaining pixels (if any) */
          j = source->width & 7;
          if ( j > 0 )
          {
            FT_Int  val = *ss;


            for ( ; j > 0; j-- )
            {
              tt[0] = (FT_Byte)( ( val & 0x80 ) >> 7);
              val <<= 1;
              tt   += 1;
            }
          }

          s += source->pitch;
          t += target->pitch;
        }
      }
      break;


    case FT_PIXEL_MODE_GRAY:
      {
        FT_Int    width   = source->width;
        FT_Byte*  s       = source->buffer;
        FT_Byte*  t       = target->buffer;
        FT_Int    s_pitch = source->pitch;
        FT_Int    t_pitch = target->pitch;
        FT_Int    i;


        target->num_grays = 256;

        for ( i = source->rows; i > 0; i-- )
        {
          FT_ARRAY_COPY( t, s, width );

          s += s_pitch;
          t += t_pitch;
        }
      }
      break;


    case FT_PIXEL_MODE_GRAY2:
      {
        FT_Byte*  s = source->buffer;
        FT_Byte*  t = target->buffer;
        FT_Int    i;


        target->num_grays = 4;

        for ( i = source->rows; i > 0; i-- )
        {
          FT_Byte*  ss = s;
          FT_Byte*  tt = t;
          FT_Int    j;


          /* get the full bytes */
          for ( j = source->width >> 2; j > 0; j-- )
          {
            FT_Int  val = ss[0];


            tt[0] = (FT_Byte)( ( val & 0xC0 ) >> 6 );
            tt[1] = (FT_Byte)( ( val & 0x30 ) >> 4 );
            tt[2] = (FT_Byte)( ( val & 0x0C ) >> 2 );
            tt[3] = (FT_Byte)( ( val & 0x03 ) );

            ss += 1;
            tt += 4;
          }

          j = source->width & 3;
          if ( j > 0 )
          {
            FT_Int  val = ss[0];


            for ( ; j > 0; j-- )
            {
              tt[0]  = (FT_Byte)( ( val & 0xC0 ) >> 6 );
              val  <<= 2;
              tt    += 1;
            }
          }

          s += source->pitch;
          t += target->pitch;
        }
      }
      break;


    case FT_PIXEL_MODE_GRAY4:
      {
        FT_Byte*  s = source->buffer;
        FT_Byte*  t = target->buffer;
        FT_Int    i;


        target->num_grays = 16;

        for ( i = source->rows; i > 0; i-- )
        {
          FT_Byte*  ss = s;
          FT_Byte*  tt = t;
          FT_Int    j;


          /* get the full bytes */
          for ( j = source->width >> 1; j > 0; j-- )
          {
            FT_Int  val = ss[0];


            tt[0] = (FT_Byte)( ( val & 0xF0 ) >> 4 );
            tt[1] = (FT_Byte)( ( val & 0x0F ) );

            ss += 1;
            tt += 2;
          }

          if ( source->width & 1 )
            tt[0] = (FT_Byte)( ( ss[0] & 0xF0 ) >> 4 );

          s += source->pitch;
          t += target->pitch;
        }
      }
      break;


    default:
      ;
    }

    return error;
  }


  /* documentation is in ftbitmap.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Bitmap_Done( FT_Library  library,
                  FT_Bitmap  *bitmap )
  {
    FT_Memory  memory;


    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    if ( !bitmap )
      return FT_Err_Invalid_Argument;

    memory = library->memory;

    FT_FREE( bitmap->buffer );
    *bitmap = null_bitmap;

    return FT_Err_Ok;
  }


/* END */
