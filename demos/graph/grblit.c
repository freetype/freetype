/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-1999 by                                                  */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*  blitter.c: Support for blitting of bitmaps with various depth.          */
/*                                                                          */
/****************************************************************************/

#include "grblit.h"
#include "grobjs.h"

  static
  int  compute_clips( grBlitter*  blit,
                      int         x_offset,
                      int         y_offset )
  {
    int  xmin, ymin, xmax, ymax, width, height, target_width;

    /* perform clipping and setup variables */
    width  = blit->source.width;
    height = blit->source.rows;

    switch ( blit->source.mode )
    {
    case gr_pixel_mode_mono:
      width = (width + 7) & -8;
      break;

    case gr_pixel_mode_pal4:
      width = (width + 1) & -2;
      break;
      
    default:
      ;
    }

    xmin = x_offset;
    ymin = y_offset;
    xmax = xmin + width-1;
    ymax = ymin + height-1;

    /* clip if necessary */
    if ( width == 0 || height == 0                ||
         xmax < 0   || xmin >= blit->target.width ||
         ymax < 0   || ymin >= blit->target.rows  )
      return 1;

    /* set up clipping and cursors */
    blit->yread = 0;
    if ( ymin < 0 )
    {
      blit->yread  -= ymin;
      height       += ymin;
      blit->ywrite  = 0;
    }
    else
      blit->ywrite  = ymin;

    if ( ymax >= blit->target.rows )
      height -= ymax - blit->target.rows + 1;

    blit->xread = 0;
    if ( xmin < 0 )
    {
      blit->xread  -= xmin;
      width        += xmin;
      blit->xwrite  = 0;
    }
    else
      blit->xwrite  = xmin;

    target_width = blit->target.width;

    switch ( blit->target.mode )
    {
    case gr_pixel_mode_mono:
      target_width = (target_width + 7) & -8;
      break;
    case gr_pixel_mode_pal4:
      target_width = (target_width + 1) & -2;
      break;
      
    default:
      ;
    }

    blit->right_clip = xmax - target_width + 1;
    if ( blit->right_clip > 0 )
      width -= blit->right_clip;
    else
      blit->right_clip = 0;

    blit->width  = width;
    blit->height = height;

    /* set read and write to the top-left corner of the read */
    /* and write areas before clipping.                      */

    blit->read  = (unsigned char*)blit->source.buffer;
    blit->write = (unsigned char*)blit->target.buffer;

    blit->read_line  = blit->source.pitch;
    blit->write_line = blit->target.pitch;
    
    if ( blit->read_line < 0 )
      blit->read -= (blit->source.rows-1) * blit->read_line;

    if ( blit->write_line < 0 )
      blit->write -= (blit->target.rows-1) * blit->write_line;

    /* now go to the start line. Note that we do not move the   */
    /* x position yet, as this is dependent on the pixel format */
    blit->read  += blit->yread * blit->read_line;
    blit->write += blit->ywrite * blit->write_line;

    return 0;
  }


/**************************************************************************/
/*                                                                        */
/* <Function> blit_mono_to_mono                                           */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_mono_to_mono( grBlitter*  blit,
                           grColor     color )
  {
    int    shift, left_clip, x, y;
    byte*  read;
    byte*  write;

    (void)color;   /* unused argument */

    left_clip = ( blit->xread > 0 );
    shift     = ( blit->xwrite - blit->xread ) & 7;

    read  = blit->read  + (blit->xread >> 3);
    write = blit->write + (blit->xwrite >> 3);

    if ( shift == 0 )
    {
      y = blit->height;
      do
      {
        byte*  _read  = read;
        byte*  _write = write;

        x = blit->width;

        do
        {
          *_write++ |= *_read++;
          x -= 8;
        } while ( x > 0 );

        read  += blit->read_line;
        write += blit->write_line;
        y--;
      } while ( y > 0 );
    }
    else
    {
      int  first, last, count;


      first = blit->xwrite >> 3;
      last  = (blit->xwrite + blit->width-1) >> 3;

      count = last - first;

      if ( blit->right_clip )
        count++;

      y = blit->height;

      do
      {
        unsigned char*  _read  = read;
        unsigned char*  _write = write;
        unsigned char   old;
        int             shift2 = (8-shift);

        if ( left_clip )
          old = (*_read++) << shift2;
        else
          old = 0;

        x = count;
        while ( x > 0 )
        {
          unsigned char val;

          val = *_read++;
          *_write++ |= ( (val >> shift) | old );
          old = val << shift2;
          x--;
        }

        if ( !blit->right_clip )
          *_write |= old;

        read  += blit->read_line;
        write += blit->write_line;
        y--;

      } while ( y > 0 );
    }
  }


/**************************************************************************/
/*                                                                        */
/* <Function> blit_mono_to_pal8                                           */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_mono_to_pal8( grBlitter*  blit,
                           grColor     color )
  {
    int             x, y;
    unsigned int    left_mask;
    unsigned char*  read;
    unsigned char*  write;

    read  = blit->read  + (blit->xread >> 3);
    write = blit->write +  blit->xwrite;

    left_mask = 0x80 >> (blit->xread & 7);

    y = blit->height;
    do
    {
      unsigned char*  _read  = read;
      unsigned char*  _write = write;
      unsigned int    mask   = left_mask;
      unsigned int    val    = *_read;


      x = blit->width;
      do
      {
        if ( mask == 0x80 )
          val = *_read++;
          
        if ( val & mask )
          *_write = (unsigned char)color.value;

        mask >>= 1;
        if ( mask == 0 )
          mask = 0x80;

        _write++;
        x--;
      } while ( x > 0 );

      read  += blit->read_line;
      write += blit->write_line;
      y--;
    } while ( y > 0 );
  }


/**************************************************************************/
/*                                                                        */
/* <Function> blit_mono_to_pal4                                           */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_mono_to_pal4( grBlitter*  blit,
                           grColor     color )
  {
    int             x, y, phase;
    unsigned int    left_mask;
    unsigned char*  read;
    unsigned char*  write;
    unsigned char   col;


    col   = color.value & 15;
    read  = blit->read  + (blit->xread >> 3);
    write = blit->write + (blit->xwrite >> 1);

    /* now begin blit */
    left_mask = 0x80 >> (blit->xread & 7);
    phase     = blit->xwrite & 1;

    y = blit->height;
    do
    {
      unsigned char*  _read  = read;
      unsigned char*  _write = write;
      unsigned int    mask   = left_mask;
      int             _phase = phase;
      unsigned int    val    = *_read;

      x = blit->width;
      do
      {
        if ( mask == 0x80 )
          val = *_read++;

        if ( val & mask )
        {
          if ( _phase )
            *_write = (*_write & 0xF0) | col;
          else
            *_write = (*_write & 0x0F) | (col << 4);
        }

        mask >>= 1;
        if ( mask == 0 )
          mask = 0x80;

        _write += _phase;
        _phase ^= 1;
        x--;
      } while ( x > 0 );

      read  += blit->read_line;
      write += blit->write_line;
      y--;
    } while ( y > 0 );
  }


/**************************************************************************/
/*                                                                        */
/* <Function> blit_mono_to_rgb16                                          */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_mono_to_rgb16( grBlitter*  blit,
                            grColor     color )
  {
    int              x, y;
    unsigned int     left_mask;
    unsigned char*   read;
    unsigned char*   write;
    
    read  = blit->read + (blit->xread >> 3);
    write = blit->write + blit->xwrite*2;

    left_mask = 0x80 >> (blit->xread & 7);

    y = blit->height;
    do
    {
      unsigned char*  _read  = read;
      unsigned char*  _write = write;
      unsigned int    mask   = left_mask;
      unsigned int    val    = *_read;

      x = blit->width;
      do
      {
        if ( mask == 0x80 )
          val = *_read++;

        if ( val & mask )
          *(short*)_write = (short)color.value;

        mask >>= 1;
        if ( mask == 0 )
          mask = 0x80;

        _write +=2;
        x--;
      } while ( x > 0 );

      read  += blit->read_line;
      write += blit->write_line;
      y--;
    } while ( y > 0 );
  }


/**************************************************************************/
/*                                                                        */
/* <Function> blit_mono_to_rgb24                                          */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_mono_to_rgb24( grBlitter*  blit,
                            grColor     color )
  {
    int             x, y;
    unsigned int    left_mask;
    unsigned char*  read;
    unsigned char*  write;
    
    read  = blit->read  + (blit->xread >> 3);
    write = blit->write + blit->xwrite*3;

    left_mask = 0x80 >> (blit->xread & 7);

    y = blit->height;
    do
    {
      unsigned char*  _read  = read;
      unsigned char*  _write = write;
      unsigned int    mask   = left_mask;
      unsigned int    val    = *_read;

      x = blit->width;
      do
      {
        if ( mask == 0x80 )
          val = *_read++;

        if ( val & mask )
        {
          _write[0] = color.chroma[0];
          _write[1] = color.chroma[1];
          _write[2] = color.chroma[2];
        }

        mask >>= 1;
        if ( mask == 0 )
          mask = 0x80;

        _write += 3;
        x--;
      } while ( x > 0 );

      read  += blit->read_line;
      write += blit->write_line;
      y--;
    } while ( y > 0 );
  }


/**************************************************************************/
/*                                                                        */
/* <Function> blit_mono_to_rgb32                                          */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_mono_to_rgb32( grBlitter*  blit,
                            grColor     color )
  {
    int             x, y;
    unsigned int    left_mask;
    unsigned char*  read;
    unsigned char*  write;
     
    read  = blit->read  + (blit->xread >> 3);
    write = blit->write + blit->xwrite*4;

    left_mask = 0x80 >> (blit->xread & 7);

    y = blit->height;
    do
    {
      unsigned char*  _read  = read;
      unsigned char*  _write = write;
      unsigned int    mask   = left_mask;
      unsigned int    val    = *_read;

      x = blit->width;
      do
      {
        if ( mask == 0x80 )
          val = *_read++;

        if ( val & mask )
        {
   /* this could be greatly optimised as a *(long*)_write = color.value */
   /* but this wouldn't work on 64-bits systems... stupid C types!      */
          _write[0] = color.chroma[0];
          _write[1] = color.chroma[1];
          _write[2] = color.chroma[2];
          _write[3] = color.chroma[3];
        }

        mask >>= 1;
        if ( mask == 0 )
          mask = 0x80;

        _write += 4;
        x--;
      } while ( x > 0 );

      read  += blit->read_line;
      write += blit->write_line;
      y--;
    } while ( y > 0 );
  }



  static
  const grBlitterFunc  gr_mono_blitters[gr_pixel_mode_max] =
  {
    0,
    blit_mono_to_mono,
    blit_mono_to_pal4,
    blit_mono_to_pal8,
    blit_mono_to_pal8,
    blit_mono_to_rgb16,
    blit_mono_to_rgb16,
    blit_mono_to_rgb24,
    blit_mono_to_rgb32
  };


  /*******************************************************************/
  /*                                                                 */
  /*                    Saturation tables                            */
  /*                                                                 */
  /*******************************************************************/

  typedef struct grSaturation_
  {
    int          count;
    const byte*  table;
  
  } grSaturation;
  
  
  static
  const byte  gr_saturation_5[8] = { 0, 1, 2, 3, 4, 4, 4, 4 };
  

  static
  const byte  gr_saturation_17[32] =
  {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 
  };


  static
  grSaturation  gr_saturations[ GR_MAX_SATURATIONS ] =
  {
    {  5, gr_saturation_5  },
    { 17, gr_saturation_17 }
  };

  static
  int  gr_num_saturations = 2;

  static
  grSaturation*  gr_last_saturation = gr_saturations;


  extern
  const byte*  grGetSaturation( int  num_grays )
  {
    /* first of all, scan the current saturations table */
    grSaturation*  sat   = gr_saturations;
    grSaturation*  limit = sat + gr_num_saturations;

    if ( num_grays < 2 )
    {
      grError = gr_err_bad_argument;
      return 0;
    }
     
    for ( ; sat < limit; sat++ )
    {
      if ( sat->count == num_grays )
      {
        gr_last_saturation = sat;
        return sat->table;
      }
    }
    
    /* not found, simply create a new entry if there is room */
    if (gr_num_saturations < GR_MAX_SATURATIONS)
    {
      int          i;
      const byte*  table;
      
      table = (const byte*)grAlloc( (3*num_grays-1)*sizeof(byte) );
      if (!table) return 0;
        
      sat->count = num_grays;
      sat->table = table;
      
      for ( i = 0; i < num_grays; i++, table++ )
        *(unsigned char*)table = (unsigned char)i;  
      
      for ( i = 2*num_grays-1; i > 0; i--, table++ )
        *(unsigned char*)table = (unsigned char)(num_grays-1);   
        
      gr_num_saturations++;
      gr_last_saturation = sat;
      return sat->table;
    }
    grError = gr_err_saturation_overflow;
    return 0;
  }



  /*******************************************************************/
  /*                                                                 */
  /*                    conversion tables                            */
  /*                                                                 */
  /*******************************************************************/

  typedef struct grConversion_
  {
    int          target_grays;
    int          source_grays;
    const byte*  table;
  
  } grConversion;
  
  
  
  static
  const byte  gr_gray5_to_gray17[5] = { 0, 4, 8, 12, 16 };


  static
  const byte  gr_gray5_to_gray128[5] = { 0, 32, 64, 96, 127 };


  static
  const unsigned char  gr_gray17_to_gray128[17] =
  {
    0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 127
  };

  static
  grConversion  gr_conversions[ GR_MAX_CONVERSIONS ] =
  {
    {  17,  5, gr_gray5_to_gray17   },
    { 128,  5, gr_gray5_to_gray128  },
    { 128, 17, gr_gray17_to_gray128 }
  };

  static
  int  gr_num_conversions = 3;

  static
  grConversion*  gr_last_conversion = gr_conversions;


  extern
  const byte*  grGetConversion( int  target_grays,
                                int  source_grays )
  {
    grConversion*  conv  = gr_conversions;
    grConversion*  limit = conv + gr_num_conversions;
   
    if ( target_grays < 2 || source_grays < 2 )
    {
      grError = gr_err_bad_argument;
      return 0;
    }
    
    /* otherwise, scan table */
    for ( ; conv < limit; conv++ )
    {
      if ( conv->target_grays == target_grays &&
           conv->source_grays == source_grays )
      {
        gr_last_conversion = conv;
        return conv->table;
      }
    }
    
    /* not found, add a new conversion to the table */
    if (gr_num_conversions < GR_MAX_CONVERSIONS)
    {
      const byte*  table;
      int          n;

      table = (const byte*)grAlloc( source_grays*sizeof(byte) );
      if (!table)
        return 0;
        
      conv->target_grays = target_grays;
      conv->source_grays = source_grays;
      conv->table        = table;

      for ( n = 0; n < source_grays; n++ )
        ((unsigned char*)table)[n] = (unsigned char)(n*(target_grays-1)) / 
                                         (source_grays-1);
            
      gr_num_conversions++;
      gr_last_conversion = conv;
      return table;
    }
    grError = gr_err_conversion_overflow;
    return 0;
  }




/**************************************************************************/
/*                                                                        */
/* <Function> blit_gray_to_gray                                           */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_gray_to_gray( grBlitter*   blit,
                           const byte*  saturation,
                           const byte*  conversion )
  {
    int             y;
    unsigned char*  read;
    unsigned char*  write;
    unsigned char   max1;
    unsigned char   max2;

    max1  = (unsigned char)(blit->source.grays-1);
    max2  = (unsigned char)(blit->target.grays-1);
    
    read  = (unsigned char*)blit->read  + blit->xread;
    write = (unsigned char*)blit->write + blit->xwrite;

    y = blit->height;
    do
    {
      unsigned char*  _read  = read;
      unsigned char*  _write = write;
      int             x      = blit->width;

      while (x > 0)
      {
#ifdef GR_CONFIG_GRAY_SKIP_WHITE
        unsigned char val = *_read;
        
        if (val)
        {
          if (val == max)
            *_write = max2;
          else
            *_write = saturation[ (int)*_write + conversion[ *_read ] ];
        }
#else
        *_write = saturation[ (int)*_write + conversion[ *_read ] ];
#endif
        _write++;
        _read++;
        x--;
      }
      
      read  += blit->read_line;
      write += blit->write_line;
      y--;
    }
    while (y > 0);
  }


/**************************************************************************/
/*                                                                        */
/* <Function> blit_gray_to_gray_simple                                    */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_gray_to_gray_simple( grBlitter*   blit,
                                  const byte*  saturation )
  {
    int             y;
    unsigned char*  read;
    unsigned char*  write;
    unsigned char   max;

    max   = (unsigned char)(blit->source.grays-1);
    
    read  = (unsigned char*)blit->read  + blit->xread;
    write = (unsigned char*)blit->write + blit->xwrite;

    y = blit->height;
    do
    {
      unsigned char*  _read  = read;
      unsigned char*  _write = write;
      int             x      = blit->width;

      while (x > 0)
      {
#ifdef GR_CONFIG_GRAY_SKIP_WHITE
        unsigned char val = *_read;
        
        if (val)
        {
          if (val == max)
            *_write = val;
          else
            *_write = saturation[ (int)*_write + *_read ];
        }
#else
        *_write = saturation[ (int)*_write + *_read ];
#endif
        _write++;
        _read++;
        x--;
      }
      
      read  += blit->read_line;
      write += blit->write_line;
      y--;
    }
    while (y > 0);
  }



#define  compose_pixel( a, b, n, max )       \
   {                                         \
     int  d, half = max >> 1;                \
                                             \
     d = (int)b.chroma[0] - a.chroma[0]; \
     a.chroma[0] += (n*d + half)/max;      \
                                             \
     d = (int)b.chroma[1] - a.chroma[1]; \
     a.chroma[1] += (n*d + half)/max;      \
                                             \
     d = (int)b.chroma[2] - a.chroma[2]; \
     a.chroma[2] += (n*d + half)/max;      \
   }


#define  extract555( pixel, color )           \
   color.chroma[0] = (pixel >> 10) & 0x1F;  \
   color.chroma[1] = (pixel >>  5) & 0x1F;  \
   color.chroma[2] = (pixel      ) & 0x1F;
   

#define  extract565( pixel, color )           \
   color.chroma[0] = (pixel >> 11) & 0x1F;  \
   color.chroma[1] = (pixel >>  5) & 0x3F;  \
   color.chroma[2] = (pixel      ) & 0x1F;


#define  inject555( color )                           \
   ( ( (unsigned short)color.chroma[0] << 10 ) |    \
     ( (unsigned short)color.chroma[1] <<  5 ) |    \
       color.chroma[2]                         )
      

#define  inject565( color )         \
   ( ( (unsigned short)color.chroma[0] << 11 ) |    \
     ( (unsigned short)color.chroma[1] <<  5 ) |    \
       color.chroma[2]                         )
      

/**************************************************************************/
/*                                                                        */
/* <Function> blit_gray_to_555                                            */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_gray_to_555( grBlitter*  blit,
                          grColor     color,
                          int         max )
  {
    int             y;
    unsigned char*  read;
    unsigned char*  write;
    long            color2;

    read   = blit->read  + blit->xread;
    write  = blit->write + 2*blit->xwrite;
    
    /* convert color to R:G:B triplet */
    color2 = color.value;
    extract555( color2, color );

    y = blit->height;
    do
    {
      unsigned char*  _read  = read;
      unsigned char*  _write = write;
      int             x      = blit->width;

      while (x > 0)
      {
        unsigned char   val;
        
        val = *_read;
        if (val)
        {
          unsigned short* pixel = (unsigned short*)_write;
          
          if (val == max)
          {
            pixel[0] = (short)color2;
          }
          else
          {
            /* compose gray value */
            unsigned short  pix16 = *pixel;
            grColor         pix;
            
            extract555( pix16, pix );

            compose_pixel( pix, color, val, max );
            *pixel = inject555(pix);
          }  
        }
        _write += 2;
        _read  ++;
        x--;
      }
      
      read  += blit->read_line;
      write += blit->write_line;
      y--;
    }
    while (y > 0);
  }


/**************************************************************************/
/*                                                                        */
/* <Function> blit_gray_to_565                                            */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_gray_to_565( grBlitter*  blit,
                          grColor     color,
                          int         max )
  {
    int             y;
    unsigned char*  read;
    unsigned char*  write;
    long            color2;

    read   = blit->read  + blit->xread;
    write  = blit->write + 2*blit->xwrite;
    
    color2 = color.value;
    extract565( color2, color );

    y = blit->height;
    do
    {
      unsigned char*  _read  = read;
      unsigned char*  _write = write;
      int             x      = blit->width;

      while (x > 0)
      {
        unsigned char    val;
        
        val = *_read;
        if (val)
        {
          unsigned short* pixel = (unsigned short*)_write;
          
          if (val == max)
          {
            pixel[0] = (short)color2;
          }
          else
          {
            /* compose gray value */
            unsigned short  pix16 = *pixel;
            grColor         pix;

            extract565( pix16, pix );            

            compose_pixel( pix, color, val, max );
            *pixel = inject565( pix );
          }  
        }
        _write +=2;
        _read  ++;
        x--;
      }
      
      read  += blit->read_line;
      write += blit->write_line;
      y--;
    }
    while (y > 0);
  }


/**************************************************************************/
/*                                                                        */
/* <Function> blit_gray_to_24                                             */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_gray_to_24( grBlitter*  blit,
                         grColor     color,
                         int         max )
  {
    int             y;
    unsigned char*  read;
    unsigned char*  write;

    read   = blit->read  + blit->xread;
    write  = blit->write + 3*blit->xwrite;
    
    y = blit->height;
    do
    {
      unsigned char*  _read  = read;
      unsigned char*  _write = write;
      int             x      = blit->width;

      while (x > 0)
      {
        unsigned char    val;
        
        val = *_read;
        if (val)
        {
          if (val == max)
          {
            _write[0] = color.chroma[0];
            _write[1] = color.chroma[1];
            _write[2] = color.chroma[2];
          }
          else
          {
            /* compose gray value */
            grColor pix;
            
            pix.chroma[0] = _write[0];
            pix.chroma[1] = _write[1];
            pix.chroma[2] = _write[2];

            compose_pixel( pix, color, val, max );
            
            _write[0] = pix.chroma[0];
            _write[1] = pix.chroma[1];
            _write[2] = pix.chroma[2];
          }  
        }
        _write += 3;
        _read  ++;
        x--;
      }
      
      read  += blit->read_line;
      write += blit->write_line;
      y--;
    }
    while (y > 0);
  }


/**************************************************************************/
/*                                                                        */
/* <Function> blit_gray_to_32                                             */
/*                                                                        */
/**************************************************************************/

  static
  void  blit_gray_to_32( grBlitter*  blit,
                         grColor     color,
                         int         max )
  {
    int             y;
    unsigned char*  read;
    unsigned char*  write;

    read   = blit->read  + blit->xread;
    write  = blit->write + 4*blit->xwrite;
    
    y = blit->height;
    do
    {
      unsigned char*  _read  = read;
      unsigned char*  _write = write;
      int             x      = blit->width;

      while (x > 0)
      {
        unsigned char  val;
        
        val = *_read;
        if (val)
        {
          if (val == max)
          {
            _write[0] = color.chroma[0];
            _write[1] = color.chroma[1];
            _write[2] = color.chroma[2];
            _write[3] = color.chroma[3];
          }
          else
          {
            /* compose gray value */
            grColor pix;
            
            pix.chroma[0] = _write[0];
            pix.chroma[1] = _write[1];
            pix.chroma[2] = _write[2];

            compose_pixel( pix, color, val, max );
            
            _write[0] = pix.chroma[0];
            _write[1] = pix.chroma[1];
            _write[2] = pix.chroma[2];
          }  
        }
        _write += 4;
        _read  ++;
        x--;
      }
      
      read  += blit->read_line;
      write += blit->write_line;
      y--;
    }
    while (y > 0);
  }


 /**********************************************************************
  *
  * <Function>
  *    grBlitGlyphBitmap
  *
  * <Description>
  *    writes a given glyph bitmap to a target surface.
  *
  * <Input>
  *    surface :: handle to target surface
  *    x       :: position of left-most pixel of glyph image in surface
  *    y       :: position of top-most pixel of glyph image in surface
  *    bitmap  :: source glyph image
  *
  * <Return>
  *   Error code. 0 means success
  *
  **********************************************************************/
    
  typedef  void (*grColorGlyphBlitter)( grBlitter*  blit,
                                        grColor     color,
                                        int         max_gray );

  static
  const grColorGlyphBlitter  gr_color_blitters[gr_pixel_mode_max] =
  {
    0,
    0,
    0,
    0,
    blit_gray_to_555,
    blit_gray_to_565,
    blit_gray_to_24,
    blit_gray_to_32
  };  

   
  extern int  grBlitGlyphToBitmap( grBitmap*  target,
                                   grBitmap*  glyph,
                                   grPos      x,
                                   grPos      y,
                                   grColor    color )
  {
    grBlitter    blit;
    grPixelMode  mode;
  
    /* check arguments */
    if (!target || !glyph)
    {
      grError = gr_err_bad_argument;
      return -1;
    }
    

    /* set up blitter and compute clipping. Return immediately if needed */
    blit.source = *glyph;
    blit.target = *target;
    mode        = target->mode;

    if ( compute_clips( &blit, x, y ) )
      return 0;
    

    /* handle monochrome bitmap blitting */
    if (glyph->mode == gr_pixel_mode_mono)
    {
      if ( mode <= gr_pixel_mode_none || mode >= gr_pixel_mode_max )
      {
        grError = gr_err_bad_source_depth;
        return -1;
      }
      
      gr_mono_blitters[mode]( &blit, color );
      goto End;     
    }
    
    /* handle gray bitmap composition */
    if (glyph->mode == gr_pixel_mode_gray &&
        glyph->grays > 1                  )
    {
      int          target_grays = target->grays;
      int          source_grays = glyph->grays;
      const byte*  saturation;
      
      if ( mode == gr_pixel_mode_gray && target_grays > 1 )
      {
        /* rendering into a gray target - use special composition */
        /* routines..                                             */
        if ( gr_last_saturation->count == target_grays )
          saturation = gr_last_saturation->table;
        else
        {
          saturation = grGetSaturation( target_grays );
          if (!saturation) return -3;
        }


        if ( target_grays == source_grays )
          blit_gray_to_gray_simple( &blit, saturation );
        else
        {
          const byte*  conversion;

          if ( gr_last_conversion->target_grays == target_grays &&
               gr_last_conversion->source_grays == source_grays )
            conversion = gr_last_conversion->table;
          else
          {
            conversion = grGetConversion( target_grays, source_grays );
            if (!conversion) return -3;
          };
              
          blit_gray_to_gray( &blit, saturation, conversion );
        }
      }
      else
      {
        /* rendering into a color target */
        if ( mode <= gr_pixel_mode_gray ||
             mode >= gr_pixel_mode_max  )
        {
          grError = gr_err_bad_target_depth;
          return -1;
        }
        
        gr_color_blitters[mode]( &blit, color, source_grays-1 );
      }
      goto End;      
    }
    
    /* we don't support the blitting of bitmaps of the following  */
    /* types : pal4, pal8, rgb555, rgb565, rgb24, rgb32           */
    /*                                                            */
    grError = gr_err_bad_source_depth;
    return -2;
    
  End:
    return 0;
  }


/* End */
