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
    FT_Error   error;
    FT_Memory  memory;
    FT_Int     i, j, old_size;
    FT_Byte    *s, *ss, *t, *tt;


    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    memory = library->memory;

    switch ( source->pixel_mode )
    {
    case FT_PIXEL_MODE_MONO:
    case FT_PIXEL_MODE_GRAY:
    case FT_PIXEL_MODE_GRAY2:
    case FT_PIXEL_MODE_GRAY4:
      old_size = target->rows * target->pitch;

      target->pixel_mode = FT_PIXEL_MODE_GRAY;
      target->rows       = source->rows;
      target->width      = source->width;
      target->pitch      = ( source->width + alignment - 1 )
                             / alignment * alignment;

      if ( target->rows * target->pitch > old_size )
        if ( FT_QREALLOC( target->buffer,
                          old_size, target->rows * target->pitch ) )
          return error;

      break;

    default:
      error = FT_Err_Invalid_Argument;
    }

    s = source->buffer;
    t = target->buffer;

    switch ( source->pixel_mode )
    {
    case FT_PIXEL_MODE_MONO:
      target->num_grays = 2;

      for ( i = 0; i < source->rows; i++ )
      {
        ss = s;
        tt = t;

        /* get the full bytes */
        for ( j = 0; j < ( source->width >> 3 ); j++ )
        {
          *(tt++) = (FT_Byte)( ( *ss & 0x80 ) >> 7 );
          *(tt++) = (FT_Byte)( ( *ss & 0x40 ) >> 6 );
          *(tt++) = (FT_Byte)( ( *ss & 0x20 ) >> 5 );
          *(tt++) = (FT_Byte)( ( *ss & 0x10 ) >> 4 );
          *(tt++) = (FT_Byte)( ( *ss & 0x08 ) >> 3 );
          *(tt++) = (FT_Byte)( ( *ss & 0x04 ) >> 2 );
          *(tt++) = (FT_Byte)( ( *ss & 0x02 ) >> 1 );
          *(tt++) = (FT_Byte)(   *ss & 0x01 );

          ss++;
        }

        /* get remaining pixels (if any) */
        switch ( source->width & 7 )
        {
        case 7:
          *(tt++) = (FT_Byte)( ( *ss & 0x80 ) >> 7 );
          /* fall through */
        case 6:
          *(tt++) = (FT_Byte)( ( *ss & 0x40 ) >> 6 );
          /* fall through */
        case 5:
          *(tt++) = (FT_Byte)( ( *ss & 0x20 ) >> 5 );
          /* fall through */
        case 4:
          *(tt++) = (FT_Byte)( ( *ss & 0x10 ) >> 4 );
          /* fall through */
        case 3:
          *(tt++) = (FT_Byte)( ( *ss & 0x08 ) >> 3 );
          /* fall through */
        case 2:
          *(tt++) = (FT_Byte)( ( *ss & 0x04 ) >> 2 );
          /* fall through */
        case 1:
          *(tt++) = (FT_Byte)( ( *ss & 0x02 ) >> 1 );
          /* fall through */
        case 0:
          break;
        }

        s += source->pitch;
        t += target->pitch;
      }
      break;

    case FT_PIXEL_MODE_GRAY:
      target->num_grays = 256;

      for ( i = 0; i < source->rows; i++ )
      {
        ss = s;
        tt = t;

        for ( j = 0; j < source->width; j++ )
          *(tt++) = *(ss++);

        s += source->pitch;
        t += target->pitch;
      }
      break;

    case FT_PIXEL_MODE_GRAY2:
      target->num_grays = 4;

      for ( i = 0; i < source->rows; i++ )
      {
        ss = s;
        tt = t;

        /* get the full bytes */
        for ( j = 0; j < ( source->width >> 2 ); j++ )
        {
          *(tt++) = (FT_Byte)( ( *ss & 0xC0 ) >> 6 );
          *(tt++) = (FT_Byte)( ( *ss & 0x30 ) >> 4 );
          *(tt++) = (FT_Byte)( ( *ss & 0x0C ) >> 2 );
          *(tt++) = (FT_Byte)(   *ss & 0x03 );

          ss++;
        }

        /* get remaining pixels (if any) */
        switch ( source->width & 3 )
        {
        case 3:
          *(tt++) = (FT_Byte)( ( *ss & 0xC0 ) >> 6 );
          /* fall through */
        case 2:
          *(tt++) = (FT_Byte)( ( *ss & 0x30 ) >> 4 );
          /* fall through */
        case 1:
          *(tt++) = (FT_Byte)( ( *ss & 0x0C ) >> 2 );
          /* fall through */
        case 0:
          break;
        }

        s += source->pitch;
        t += target->pitch;
      }
      break;

    case FT_PIXEL_MODE_GRAY4:
      target->num_grays = 16;

      for ( i = 0; i < source->rows; i++ )
      {
        ss = s;
        tt = t;

        /* get the full bytes */
        for ( j = 0; j < ( source->width >> 1 ); j++ )
        {
          *(tt++) = (FT_Byte)( ( *ss & 0xF0 ) >> 4 );
          *(tt++) = (FT_Byte)(   *ss & 0x0F );

          ss++;
        }

        /* get remaining pixels (if any) */
        switch ( source->width & 1 )
        {
        case 1:
          *(tt++) = (FT_Byte)( ( *ss & 0xF0 ) >> 4 );
          /* fall through */
        case 0:
          break;
        }

        s += source->pitch;
        t += target->pitch;
      }
      break;

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
