/***************************************************************************/
/*                                                                         */
/*  ttsbit.c                                                               */
/*                                                                         */
/*    TrueType and OpenType embedded bitmap support (body).                */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*                                                                         */
/*  WARNING: This file should not be compiled directly, it is meant to be  */
/*           included in the source of several font drivers (i.e., the TTF */
/*           and OTF drivers).                                             */
/*                                                                         */
/***************************************************************************/


#include <ftdebug.h>

#include <ttsbit.h>
#include <tttags.h>
#include <tterrors.h>


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    blit_sbit                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Blits a bitmap from an input stream into a given target.  Supports */
  /*    x and y offsets as well as byte padded lines.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    target      :: The target bitmap/pixmap.                           */
  /*                                                                       */
  /*    source      :: The input packed bitmap data.                       */
  /*                                                                       */
  /*    line_bits   :: The number of bits per line.                        */
  /*                                                                       */
  /*    byte_padded :: A flag which is true if lines are byte-padded.      */
  /*                                                                       */
  /*    x_offset    :: The horizontal offset.                              */
  /*                                                                       */
  /*    y_offset    :: The vertical offset.                                */
  /*                                                                       */
  /* <Note>                                                                */
  /*    IMPORTANT: The x and y offsets are relative to the top corner of   */
  /*               the target bitmap (unlike the normal TrueType           */
  /*               convention).  A positive y offset indicates a downwards */
  /*               direction!                                              */
  /*                                                                       */
  static
  void  blit_sbit( FT_Bitmap*  target,
                   char*       source,
                   FT_Int      line_bits,
                   FT_Bool     byte_padded,
                   FT_Int      x_offset,
                   FT_Int      y_offset )
  {
    FT_Byte*   line_buff;
    FT_Int     line_incr;
    FT_Int     height;

    FT_UShort  acc;
    FT_Byte    loaded;


    /* first of all, compute starting write position */
    line_incr = target->pitch;
    line_buff = target->buffer;
    
    if (line_incr < 0)
      line_buff -= line_incr*(target->rows-1);

    line_buff += (x_offset >> 3) + y_offset * line_incr;

    /***********************************************************************/
    /*                                                                     */
    /* We use the extra-classic `accumulator' trick to extract the bits    */
    /* from the source byte stream.                                        */
    /*                                                                     */
    /* Namely, the variable `acc' is a 16-bit accumulator containing the   */
    /* last `loaded' bits from the input stream.  The bits are shifted to  */
    /* the upmost position in `acc'.                                       */
    /*                                                                     */
    /***********************************************************************/

    acc    = 0;  /* clear accumulator   */
    loaded = 0;  /* no bits were loaded */

    for ( height = target->rows; height > 0; height-- )
    {
      FT_Byte*  cur   = line_buff;    /* current write cursor          */
      FT_Int    count = line_bits;    /* # of bits to extract per line */
      FT_Byte   shift = x_offset & 7; /* current write shift           */
      FT_Byte   space = 8 - shift;


      /* first of all, read individual source bytes */
      if ( count >= 8 )
      {
        count -= 8;
        {
          do
          {
            FT_Byte  val;

            /* ensure that there are at least 8 bits in the accumulator */
            if ( loaded < 8 )
            {
              acc    |= ((FT_UShort)*source++) << (8 - loaded);
              loaded += 8;
            }

            /* now write one byte */
            val     = (FT_Byte)(acc >> 8);
            if (shift)
            {
              cur[0] |= val >> shift;
              cur[1] |= val << space;
            }
            else
              cur[0] = val;

            cur++;
            acc   <<= 8;  /* remove bits from accumulator */
            loaded -= 8;
            count  -= 8;
          }
          while ( count >= 0 );
        }

        /* restore `count' to correct value */
        count += 8;
      }

      /* now write remaining bits (count < 8) */
      if ( count > 0 )
      {
        FT_Byte  val;


        /* ensure that there are at least `count' bits in the accumulator */
        if ( loaded < count )
        {
          acc    |= ((FT_UShort)*source++) << (8 - loaded);
          loaded += 8;
        }

        /* now write remaining bits */
        val     = ((FT_Byte)(acc >> 8)) & ~(0xFF >> count);
        cur[0] |= val >> shift;

        if ( count > space )
          cur[1] |= val << space;

        acc   <<= count;
        loaded -= count;
      }

      /* now, skip to next line */
      if ( byte_padded )
        acc = loaded = 0;   /* clear accumulator on byte-padded lines */

      line_buff += line_incr;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Small_SBit_Metrics                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a small bitmap metrics record.                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream  :: The input stream.                                       */
  /*                                                                       */
  /* <Output>                                                              */
  /*    metrics :: A small metrics structure.                              */
  /*                                                                       */
  static
  void  TT_Load_Small_SBit_Metrics( TT_SBit_Small_Metrics*  metrics,
                                    FT_Stream               stream )
  {
    metrics->height   = GET_Byte();
    metrics->width    = GET_Byte();
    metrics->bearingX = GET_Char();
    metrics->bearingY = GET_Char();
    metrics->advance  = GET_Byte();
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_SBit_Metrics                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a bitmap metrics record.                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream  :: The input stream.                                       */
  /*                                                                       */
  /* <Output>                                                              */
  /*    metrics :: A metrics structure.                                    */
  /*                                                                       */
  static
  void  TT_Load_SBit_Metrics( TT_SBit_Metrics*  metrics,
                              FT_Stream         stream )
  {
    metrics->height       = GET_Byte();
    metrics->width        = GET_Byte();

    metrics->horiBearingX = GET_Char();
    metrics->horiBearingY = GET_Char();
    metrics->horiAdvance  = GET_Byte();

    metrics->vertBearingX = GET_Char();
    metrics->vertBearingY = GET_Char();
    metrics->vertAdvance  = GET_Byte();
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_SBit_Line_Metrics                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a bitmap line metrics record.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream  :: The input stream.                                       */
  /*                                                                       */
  /* <Output>                                                              */
  /*    metrics :: A line metrics structure.                               */
  /*                                                                       */
  static
  void  TT_Load_SBit_Line_Metrics( TT_SBit_Line_Metrics*  metrics,
                                   FT_Stream              stream )
  {
    metrics->ascender  = GET_Char();
    metrics->descender = GET_Char();
    metrics->max_width = GET_Byte();

    metrics->caret_slope_numerator   = GET_Char();
    metrics->caret_slope_denominator = GET_Char();
    metrics->caret_offset            = GET_Char();

    metrics->min_origin_SB  = GET_Char();
    metrics->min_advance_SB = GET_Char();
    metrics->max_before_BL  = GET_Char();
    metrics->min_after_BL   = GET_Char();
    metrics->pads[0]        = GET_Char();
    metrics->pads[1]        = GET_Char();
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_SBit_Const_Metrics                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the metrics for `EBLC' index tables format 2 and 5.          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    range  :: The target range.                                        */
  /*                                                                       */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  static
  TT_Error  Load_SBit_Const_Metrics( TT_SBit_Range*  range,
                                     FT_Stream       stream )
  {
    TT_Error  error;

    if ( !ACCESS_Frame( 12L ) )
    {
      range->image_size = GET_ULong();
      TT_Load_SBit_Metrics( &range->metrics, stream );

      FORGET_Frame();
    }

    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_SBit_Range_Codes                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the range codes for `EBLC' index tables format 4 and 5.      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    range        :: The target range.                                  */
  /*                                                                       */
  /*    stream       :: The input stream.                                  */
  /*                                                                       */
  /*    load_offsets :: A flag whether to load the glyph offset table.     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  static
  TT_Error  Load_SBit_Range_Codes( TT_SBit_Range*  range,
                                   FT_Stream       stream,
                                   TT_Bool         load_offsets )
  {
    TT_Error   error;
    TT_ULong   count, n, size;
    FT_Memory  memory = stream->memory;


    if ( READ_ULong( count ) )
      goto Exit;

    range->num_glyphs = count;

    /* Allocate glyph offsets table if needed */
    if ( load_offsets )
    {
      if ( ALLOC_ARRAY( range->glyph_offsets, count, TT_ULong ) )
        goto Exit;

      size = count * 4L;
    }
    else
      size = count * 2L;

    /* Allocate glyph codes table and access frame */
    if ( ALLOC_ARRAY ( range->glyph_codes, count, TT_UShort ) ||
         ACCESS_Frame( size )                                 )
      goto Exit;

    for ( n = 0; n < count; n++ )
    {
      range->glyph_codes[n] = GET_UShort();

      if (load_offsets)
        range->glyph_offsets[n] = (TT_ULong)range->image_offset + 
                                  GET_UShort();
    }

    FORGET_Frame();

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_SBit_Range                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a given `EBLC' index/range table.                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    range  :: The target range.                                        */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  static
  TT_Error  Load_SBit_Range( TT_SBit_Range*  range,
                             FT_Stream       stream )
  {
    TT_Error   error;
    FT_Memory  memory = stream->memory;


    switch( range->index_format )
    {
    case 1:   /* variable metrics with 4-byte offsets */
    case 3:   /* variable metrics with 2-byte offsets */
      {
        TT_ULong  num_glyphs, n;
        TT_Int    size_elem;
        TT_Bool   large = (range->index_format == 1);


        num_glyphs        = range->last_glyph - range->first_glyph + 1L;
        range->num_glyphs = num_glyphs;
        num_glyphs++;    /* XXX : BEWARE - see spec */

        size_elem  = ( large ? 4 : 2 );

        if ( ALLOC_ARRAY( range->glyph_offsets,
                          num_glyphs, TT_ULong )    ||

             ACCESS_Frame( num_glyphs * size_elem ) )
          goto Exit;

        for ( n = 0; n < num_glyphs; n++ )
          range->glyph_offsets[n] = (TT_ULong)( range->image_offset +
                                     (large ? GET_ULong() : GET_UShort()) );
        FORGET_Frame();
      }
      break;

    case 2:   /* all glyphs have identical metrics */
      error = Load_SBit_Const_Metrics( range, stream );
      break;

    case 4:
      error = Load_SBit_Range_Codes( range, stream, 1 );
      break;

    case 5:
      error = Load_SBit_Const_Metrics( range, stream ) ||
              Load_SBit_Range_Codes( range, stream, 0 );
      break;

    default:
      error = TT_Err_Invalid_File_Format;
    }

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_SBit_Strikes                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the table of embedded bitmap sizes for this face.            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: The target face object.                                  */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_SBit_Strikes( TT_Face    face,
                                  FT_Stream  stream )
  {
    TT_Error   error  = 0;
    FT_Memory  memory = stream->memory;
    TT_Fixed   version;
    TT_ULong   num_strikes;
    TT_ULong   table_base;


    /* this table is optional */
    error = face->goto_table( face, TTAG_EBLC, stream, 0 );
    {
      error = 0;
      goto Exit;
    }

    table_base = FILE_Pos();
    if ( ACCESS_Frame( 8L ) )
      goto Exit;

    version     = GET_Long();
    num_strikes = GET_ULong();

    FORGET_Frame();

    /* check version number and strike count */
    if ( version     != 0x00020000 ||
         num_strikes >= 0x10000    )
    {
      FT_ERROR(( "TT_Load_SBit_Strikes: invalid table version!\n" ));
      error = TT_Err_Invalid_File_Format;

      goto Exit;
    }

    /* allocate the strikes table */
    if ( ALLOC_ARRAY( face->sbit_strikes, num_strikes, TT_SBit_Strike ) )
      goto Exit;

    face->num_sbit_strikes = num_strikes;

    /* now read each strike table separately */
    {
      TT_SBit_Strike*  strike = face->sbit_strikes;
      TT_ULong         count  = num_strikes;

      if ( ACCESS_Frame( 48L * num_strikes ) )
        goto Exit;

      while ( count > 0 )
      {
        TT_ULong  indexTablesSize;


        strike->ranges_offset    = GET_ULong();
        indexTablesSize          = GET_ULong();  /* don't save */

        strike->num_ranges       = GET_ULong();
        strike->color_ref        = GET_ULong();

        TT_Load_SBit_Line_Metrics( &strike->hori, stream );
        TT_Load_SBit_Line_Metrics( &strike->vert, stream );

        strike->start_glyph      = GET_UShort();
        strike->end_glyph        = GET_UShort();
        strike->x_ppem           = GET_Byte();
        strike->y_ppem           = GET_Byte();
        strike->bit_depth        = GET_Byte();
        strike->flags            = GET_Char();

        count--;
        strike++;
      }

      FORGET_Frame();
    }

    /* allocate the index ranges for each strike table */
    {
      TT_SBit_Strike*  strike = face->sbit_strikes;
      TT_ULong         count  = num_strikes;


      while ( count > 0 )
      {
        TT_SBit_Range* range;
        TT_ULong       count2 = strike->num_ranges;


        if ( ALLOC_ARRAY( strike->sbit_ranges,
                          strike->num_ranges,
                          TT_SBit_Range ) )
          goto Exit;

        /* read each range */
        if ( FILE_Seek( table_base + strike->ranges_offset ) ||
             ACCESS_Frame( strike->num_ranges * 8L )         )
          goto Exit;

        range = strike->sbit_ranges;
        while ( count2 > 0 )
        {
          range->first_glyph  = GET_UShort();
          range->last_glyph   = GET_UShort();
          range->table_offset = table_base + strike->ranges_offset
                                 + GET_ULong();
          count2--;
          range++;
        }

        FORGET_Frame();

        /* Now, read each index table */
        count2 = strike->num_ranges;
        range  = strike->sbit_ranges;
        while ( count2 > 0 )
        {
          /* Read the header */
          if ( FILE_Seek( range->table_offset ) ||
               ACCESS_Frame( 8L )               )
            goto Exit;

          range->index_format = GET_UShort();
          range->image_format = GET_UShort();
          range->image_offset = GET_ULong();

          FORGET_Frame();

          error = Load_SBit_Range( range, stream );
          if ( error )
            goto Exit;

          count2--;
          range++;
        }

        count--;
        strike++;
      }
    }

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Free_SBit_Strikes                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases the embedded bitmap tables.                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: The target face object.                                    */
  /*                                                                       */
  LOCAL_FUNC
  void  TT_Free_SBit_Strikes( TT_Face  face )
  {
    FT_Memory        memory       = face->root.memory;
    TT_SBit_Strike*  strike       = face->sbit_strikes;
    TT_SBit_Strike*  strike_limit = strike + face->num_sbit_strikes;


    if ( strike )
    {
      for ( ; strike < strike_limit; strike++ )
      {
        TT_SBit_Range*  range       = strike->sbit_ranges;
        TT_SBit_Range*  range_limit = range + strike->num_ranges;

        if ( range )
        {
          for ( ; range < range_limit; range++ )
          {
            /* release the glyph offsets and codes tables */
            /* where appropriate                          */
            FREE( range->glyph_offsets );
            FREE( range->glyph_codes );
          }
        }
        FREE( strike->sbit_ranges );
        strike->num_ranges = 0;
      }
      FREE( face->sbit_strikes );
    }
    face->num_sbit_strikes = 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Find_SBit_Range                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Scans a given strike's ranges and return, for a given glyph        */
  /*    index, the corresponding sbit range, and `EBDT' offset.            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    glyph_index   :: The glyph index.                                  */
  /*    strike        :: The source/current sbit strike.                   */
  /*                                                                       */
  /* <Output>                                                              */
  /*    arange        :: The sbit range containing the glyph index.        */
  /*    aglyph_offset :: The offset of the glyph data in `EBDT' table.     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means the glyph index was found.           */
  /*                                                                       */
  static
  TT_Error  Find_SBit_Range( TT_UInt          glyph_index,
                             TT_SBit_Strike*  strike,
                             TT_SBit_Range**  arange,
                             TT_ULong*        aglyph_offset )
  {
    TT_SBit_Range  *range, *range_limit;


    /* check whether the glyph index is within this strike's */
    /* glyph range                                           */
    if ( glyph_index < strike->start_glyph ||
         glyph_index > strike->end_glyph   )
      goto Fail;

    /* scan all ranges in strike */
    range       = strike->sbit_ranges;
    range_limit = range + strike->num_ranges;
    if ( !range )
      goto Fail;

    for ( ; range < range_limit; range++ )
    {
      if ( glyph_index >= range->first_glyph &&
           glyph_index <= range->last_glyph  )
      {
        TT_UShort  delta = glyph_index - range->first_glyph;


        switch ( range->index_format )
        {
        case 1:
        case 3:
          *aglyph_offset = range->glyph_offsets[delta];
          break;

        case 2:
          *aglyph_offset = range->image_offset +
                           range->image_size * delta;
          break;

        case 4:
        case 5:
          {
            TT_ULong  n;


            for ( n = 0; n < range->num_glyphs; n++ )
            {
              if ( range->glyph_codes[n] == glyph_index )
              {
                if ( range->index_format == 4 )
                  *aglyph_offset = range->glyph_offsets[n];
                else
                  *aglyph_offset = range->image_offset +
                                   n * range->image_size;
                break;
              }
            }
          }

          /* fall-through */
          default:
            goto Fail;
        }

        /* return successfully! */
        *arange  = range;

        return 0;
      }
    }

  Fail:
    *arange        = 0;
    *aglyph_offset = 0;

    return TT_Err_Invalid_Argument;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Find_SBit_Image                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Checks whether an embedded bitmap (an `sbit') exists for a given   */
  /*    glyph, at given x and y ppems.                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face          :: The target face object.                           */
  /*    glyph_index   :: The glyph index.                                  */
  /*    x_ppem        :: The horizontal resolution in points per EM.       */
  /*    y_ppem        :: The vertical resolution in points per EM.         */
  /*                                                                       */
  /* <Output>                                                              */
  /*    arange        :: The SBit range containing the glyph index.        */
  /*    astrike       :: The SBit strike containing the glyph index.       */
  /*    aglyph_offset :: The offset of the glyph data in `EBDT' table.     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.  Returns                    */
  /*    TT_Err_Invalid_Argument if no sbit exist for the requested glyph.  */
  /*                                                                       */
  static
  TT_Error  Find_SBit_Image( TT_Face           face,
                             TT_UInt           glyph_index,
                             TT_Int            x_ppem,
                             TT_Int            y_ppem,

                             TT_SBit_Range**   arange,
                             TT_SBit_Strike**  astrike,
                             TT_ULong*         aglyph_offset )
  {
    TT_SBit_Strike*  strike = face->sbit_strikes;
    TT_SBit_Strike*  strike_limit = strike + face->num_sbit_strikes;


    if ( !strike)
      goto Fail;

    for ( ; strike < strike_limit; strike++ )
    {
      if ( strike->x_ppem == x_ppem && strike->y_ppem == y_ppem )
      {
        TT_Error  error;


        error = Find_SBit_Range( glyph_index, strike, arange, aglyph_offset );
        if ( error )
          goto Fail;

        *astrike = strike;

        return TT_Err_Ok;
      }
    }

  Fail:
    /* no embedded bitmap for this glyph in face */
    *arange        = 0;
    *astrike       = 0;
    *aglyph_offset = 0;

    return TT_Err_Invalid_Argument;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Load_SBit_Metrics                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Gets the big metrics for a given SBit.                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream      :: The input stream.                                   */
  /*    range       :: The SBit range containing the glyph.                */
  /*                                                                       */
  /* <Output>                                                              */
  /*    big_metrics :: A big SBit metrics structure for the glyph.         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The stream cursor must be positioned at the glyph's offset within  */
  /*    the `EBDT' table before the call.                                  */
  /*                                                                       */
  /*    If the image format uses variable metrics, the stream cursor is    */
  /*    positioned just after the metrics header in the `EBDT' table on    */
  /*    function exit.                                                     */
  /*                                                                       */
  static
  TT_Error  Load_SBit_Metrics( FT_Stream         stream,
                               TT_SBit_Range*    range,
                               TT_SBit_Metrics*  metrics )
  {
    TT_Error  error = TT_Err_Ok;


    switch ( range->index_format )
    {
    case 1:  /* variable metrics */
    case 3:
    case 4:
      {
        switch ( range->image_format )
        {
        case 1:  /* small metrics */
        case 2:
        case 8:
          {
            TT_SBit_Small_Metrics  smetrics;


            /* read small metrics */
            if ( ACCESS_Frame( 5L ) )
              goto Exit;
            TT_Load_Small_SBit_Metrics( &smetrics, stream );
            FORGET_Frame();

            /* convert it to a big metrics */
            metrics->height       = smetrics.height;
            metrics->width        = smetrics.width;
            metrics->horiBearingX = smetrics.bearingX;
            metrics->horiBearingY = smetrics.bearingY;
            metrics->horiAdvance  = smetrics.advance;

            /* these metrics are made up at a higher level when */
            /* needed.                                          */
            metrics->vertBearingX = 0;
            metrics->vertBearingY = 0;
            metrics->vertAdvance  = 0;
          }
          break;

        default:  /* big metrics */
          if ( ACCESS_Frame( 8L ) )
            goto Exit;
          TT_Load_SBit_Metrics( metrics, stream );
          FORGET_Frame();
        }
      }
      break;

    default:  /* constant metrics */
      *metrics = range->metrics;
    }

  Exit:
    return error;
  }



  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Crop_Bitmap                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Crops a bitmap to its tightest bounding box, and adjusts its       */
  /*    metrics.                                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    image   :: The input glyph slot.                                   */
  /*                                                                       */
  /*    metrics :: The corresponding metrics structure.                    */
  /*                                                                       */
  static
  void  Crop_Bitmap( FT_Bitmap*        map,
                     TT_SBit_Metrics*  metrics )
  {
    /***********************************************************************/
    /*                                                                     */
    /* In this situation, some bounding boxes of embedded bitmaps are too  */
    /* large.  We need to crop it to a reasonable size.                    */
    /*                                                                     */
    /*      ---------                                                      */
    /*      |       |                -----                                 */
    /*      |  ***  |                |***|                                 */
    /*      |   *   |                | * |                                 */
    /*      |   *   |    ------>     | * |                                 */
    /*      |   *   |                | * |                                 */
    /*      |   *   |                | * |                                 */
    /*      |  ***  |                |***|                                 */
    /*      ---------                -----                                 */
    /*                                                                     */
    /***********************************************************************/

    TT_Int    rows, count;
    TT_Long   line_len;
    TT_Byte*  line;


    /***********************************************************************/
    /*                                                                     */
    /* first of all, checks the top-most lines of the bitmap, and removes  */
    /* them if they're empty.                                              */
    /*                                                                     */
    {
      line     = (TT_Byte*)map->buffer;
      rows     = map->rows;
      line_len = map->pitch;


      for ( count = 0; count < rows; count++ )
      {
        TT_Byte*  cur   = line;
        TT_Byte*  limit = line + line_len;


        for ( ; cur < limit; cur++ )
          if ( cur[0] )
            goto Found_Top;

        /* the current line was empty - skip to next one */
        line  = limit;
      }

    Found_Top:
      /* check that we have at least one filled line */
      if ( count >= rows )
        goto Empty_Bitmap;

      /* now, crop the empty upper lines */
      if ( count > 0 )
      {
        line = (TT_Byte*)map->buffer;

        MEM_Move( line, line + count * line_len, (rows - count) * line_len );

        metrics->height       -= count;
        metrics->horiBearingY -= count;
        metrics->vertBearingY -= count;

        map->rows -= count;
        rows      -= count;
      }
    }

    /***********************************************************************/
    /*                                                                     */
    /* second, crop the lower lines                                        */
    /*                                                                     */
    {
      line = (TT_Byte*)map->buffer + (rows - 1) * line_len;

      for ( count = 0; count < rows; count++ )
      {
        TT_Byte*  cur   = line;
        TT_Byte*  limit = line + line_len;


        for ( ; cur < limit; cur++ )
          if ( cur[0] )
            goto Found_Bottom;

        /* the current line was empty - skip to previous one */
        line -= line_len;
      }

    Found_Bottom:
      if ( count > 0 )
      {
        metrics->height -= count;
        rows            -= count;
        map->rows       -= count;
      }
    }

    /***********************************************************************/
    /*                                                                     */
    /* third, get rid of the space on the left side of the glyph           */
    /*                                                                     */
    do
    {
      TT_Byte*  limit;


      line  = (TT_Byte*)map->buffer;
      limit = line + rows * line_len;

      for ( ; line < limit; line += line_len )
        if ( line[0] & 0x80 )
          goto Found_Left;

      /* shift the whole glyph one pixel to the left */
      line  = (TT_Byte*)map->buffer;
      limit = line + rows * line_len;

      for ( ; line < limit; line += line_len )
      {
        TT_Int    n, width = map->width;
        TT_Byte   old;
        TT_Byte*  cur = line;


        old = cur[0] << 1;
        for ( n = 8; n < width; n += 8 )
        {
          TT_Byte  val;


          val    = cur[1];
          cur[0] = old | (val >> 7);
          old    = val << 1;
          cur++;
        }
        cur[0] = old;
      }

      map->width--;
      metrics->horiBearingX++;
      metrics->vertBearingX++;
      metrics->width--;
    } while ( map->width > 0 );

  Found_Left:

    /***********************************************************************/
    /*                                                                     */
    /* finally, crop the bitmap width to get rid of the space on the right */
    /* side of the glyph.                                                  */
    /*                                                                     */
    do
    {
      TT_Int    right = map->width-1;
      TT_Byte*  limit;
      TT_Byte   mask;


      line  = (TT_Byte*)map->buffer + (right >> 3);
      limit = line + rows*line_len;
      mask  = 0x80 >> (right & 7);

      for ( ; line < limit; line += line_len )
        if ( line[0] & mask )
          goto Found_Right;

      /* crop the whole glyph to the right */
      map->width--;
      metrics->width--;
    } while ( map->width > 0 );

  Found_Right:
    /* all right, the bitmap was cropped */
    return;

  Empty_Bitmap:
    map->width      = 0;
    map->rows       = 0;
    map->pitch      = 0;
    map->pixel_mode = ft_pixel_mode_mono;
  }


  static
  TT_Error Load_SBit_Single( FT_Bitmap*        map,
                             TT_Int            x_offset,
                             TT_Int            y_offset,
                             TT_Int            pix_bits,
                             TT_UShort         image_format,
                             TT_SBit_Metrics*  metrics,
                             FT_Stream         stream )
  {
    TT_Error  error;


    /* check that the source bitmap fits into the target pixmap */
    if ( x_offset < 0 || x_offset + metrics->width  > map->width ||
         y_offset < 0 || y_offset + metrics->height > map->rows  )
    {
      error = TT_Err_Invalid_Argument;

      goto Exit;
    }

    {
      TT_Int  glyph_width  = metrics->width;
      TT_Int  glyph_height = metrics->height;
      TT_Int  glyph_size;
      TT_Int  line_bits    = pix_bits * glyph_width;
      TT_Bool pad_bytes    = 0;


      /* compute size of glyph image */
      switch ( image_format )
      {
      case 1:  /* byte-padded formats */
      case 6:
        {
          TT_Int  line_length;


          switch ( pix_bits )
          {
          case 1:  line_length = (glyph_width+7) >> 3;   break;
          case 2:  line_length = (glyph_width+3) >> 2;   break;
          case 4:  line_length = (glyph_width+1) >> 1;   break;
          default: line_length =  glyph_width;
          }

          glyph_size = glyph_height * line_length;
          pad_bytes  = 1;
        }
        break;

      case 2:
      case 5:
      case 7:
        line_bits  =  glyph_width * pix_bits;
        glyph_size = (glyph_height * line_bits + 7) >> 3;
        break;

      default:  /* invalid format */
        return TT_Err_Invalid_File_Format;
      }

      /* Now read data and draw glyph into target pixmap       */
      if ( ACCESS_Frame( glyph_size ) )
        goto Exit;

      /* don't forget to multiply `x_offset' by `map->pix_bits' as */
      /* the sbit blitter doesn't make a difference between pixmap */
      /* depths.                                                   */
      blit_sbit( map, stream->cursor, line_bits, pad_bytes,
                 x_offset * pix_bits, y_offset );

      FORGET_Frame();
    }

  Exit:
    return error;
  }


  static
  TT_Error Load_SBit_Image( TT_SBit_Strike*   strike,
                            TT_SBit_Range*    range,
                            TT_ULong          ebdt_pos,
                            TT_ULong          glyph_offset,
                            FT_Bitmap*        map,
                            TT_Int            x_offset,
                            TT_Int            y_offset,
                            FT_Stream         stream,
                            TT_SBit_Metrics*  metrics )
  {
    FT_Memory            memory = stream->memory;
    TT_Error             error;


    /* place stream at beginning of glyph data and read metrics */
    if ( FILE_Seek( ebdt_pos + glyph_offset ) )
      goto Exit;

    error = Load_SBit_Metrics( stream, range, metrics );
    if ( error )
      goto Exit;

    /* this function is recursive.  At the top-level call, the */
    /* field map.buffer is NULL.  We thus begin by finding the */
    /* dimensions of the higher-level glyph to allocate the    */
    /* final pixmap buffer                                     */
    if ( map->buffer == 0 )
    {
      TT_Long  size;


      map->width    = metrics->width;
      map->rows     = metrics->height;

      switch ( strike->bit_depth )
      {
      case 1:
        map->pixel_mode = ft_pixel_mode_mono; 
        map->pitch      = (map->width+7) >> 3;
        break;
      case 2:
        map->pixel_mode = ft_pixel_mode_pal2; 
        map->pitch      = (map->width+3) >> 2;
        break;
      case 4:
        map->pixel_mode = ft_pixel_mode_pal4; 
        map->pitch      = (map->width+1) >> 1;
        break;
      case 8:
        map->pixel_mode = ft_pixel_mode_grays; 
        map->pitch      = map->width;
        break;

      default:
        return TT_Err_Invalid_File_Format;
      }

      size = map->rows * map->pitch;

      /* check that there is no empty image */
      if ( size == 0 )
        goto Exit;     /* exit successfully! */

      if ( ALLOC( map->buffer, size ) )
        goto Exit;
    }

    switch ( range->image_format )
    {
    case 1:  /* single sbit image - load it */
    case 2:
    case 5:
    case 6:
    case 7:
      return Load_SBit_Single( map, x_offset, y_offset, strike->bit_depth,
                               range->image_format, metrics, stream );

    case 8:  /* compound format */
    case 9:
      break;

    default: /* invalid image format */
      return TT_Err_Invalid_File_Format;
    }

    /* All right, we're in a compound format.  First of all, read */
    /* the array of elements                                      */
    {
      TT_SBit_Component*  components;
      TT_SBit_Component*  comp;
      TT_UShort           num_components, count;


      if ( READ_UShort( num_components )                                ||
           ALLOC_ARRAY( components, num_components, TT_SBit_Component ) )
        goto Exit;

      count = num_components;

      if ( ACCESS_Frame( 4L * num_components ) )
        goto Fail_Memory;

      for ( comp = components; count > 0; count--, comp++ )
      {
        comp->glyph_code = GET_UShort();
        comp->x_offset   = GET_Char();
        comp->y_offset   = GET_Char();
      }

      FORGET_Frame();

      /* Now recursively load each element glyph */
      count = num_components;
      comp  = components;
      for ( ; count > 0; count--, comp++ )
      {
        TT_SBit_Range*   elem_range;
        TT_SBit_Metrics  elem_metrics;
        TT_ULong         elem_offset;


        /* find the range for this element */
        error = Find_SBit_Range( comp->glyph_code,
                                 strike,
                                 &elem_range,
                                 &elem_offset );
        if ( error )
          goto Fail_Memory;

        /* now load the element, recursively */
        error = Load_SBit_Image( strike,
                                 elem_range,
                                 ebdt_pos,
                                 elem_offset,
                                 map,
                                 x_offset + comp->x_offset,
                                 y_offset + comp->y_offset,
                                 stream,
                                 &elem_metrics );
        if ( error )
          goto Fail_Memory;
      }

    Fail_Memory:
      FREE( components );
    }

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_SBit_Image                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a given glyph sbit image from the font resource.  This also  */
  /*    returns its metrics.                                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: The target face object.                             */
  /*                                                                       */
  /*    x_ppem      :: The horizontal resolution in points per EM.         */
  /*                                                                       */
  /*    y_ppem      :: The vertical resolution in points per EM.           */
  /*                                                                       */
  /*    glyph_index :: The current glyph index.                            */
  /*                                                                       */
  /*    stream      :: The input stream.                                   */
  /*                                                                       */
  /* <Output>                                                              */
  /*    map         :: The target pixmap.                                  */
  /*    metrics     :: A big sbit metrics structure for the glyph image.   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.  Returns an error if no     */
  /*    glyph sbit exists for the index.                                   */
  /*                                                                       */
  /*  <Note>                                                               */
  /*    The `map.buffer' field is always freed before the glyph is loaded. */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_SBit_Image( TT_Face           face,
                                TT_Int            x_ppem,
                                TT_Int            y_ppem,
                                TT_UInt           glyph_index,
                                FT_Stream         stream,
                                FT_Bitmap*        map,
                                TT_SBit_Metrics*  metrics )
  {
    TT_Error         error;
    FT_Memory        memory = stream->memory;
    TT_ULong         ebdt_pos, glyph_offset;

    TT_SBit_Strike*  strike;
    TT_SBit_Range*   range;


    /* Check whether there is a glyph sbit for the current index */
    error = Find_SBit_Image( face, glyph_index, x_ppem, y_ppem,
                             &range, &strike, &glyph_offset );
    if ( error )
      goto Exit;

    /* now, find the location of the `EBDT' table in */
    /* the font file                                 */
    error = face->goto_table( face, TTAG_EBDT, stream, 0 );
    if (error) goto Exit;
    
    ebdt_pos = FILE_Pos();

    /* clear the bitmap & load the bitmap */
    FREE( map->buffer );
    map->rows = map->pitch = map->width = 0;

    error = Load_SBit_Image( strike, range, ebdt_pos, glyph_offset,
                             map, 0, 0, stream, metrics );
    if ( error )
      goto Exit;

    /* setup vertical metrics if needed */
    if ( strike->flags & 1 )
    {
      /* in case of a horizontal strike only */
      FT_Int  advance;
      FT_Int  top;


      advance = strike->hori.ascender - strike->hori.descender;
      top     = advance / 10;

      metrics->vertBearingX = -metrics->width / 2;
      metrics->vertBearingY =  advance / 10;
      metrics->vertAdvance  =  advance * 12 / 10;
    }

    /* Crop the bitmap now */
    Crop_Bitmap( map, metrics );

  Exit:
    return error;
  }


/* END */
