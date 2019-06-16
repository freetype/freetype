/****************************************************************************
 *
 * sfwoff2.c
 *
 *   WOFF2 format management (base).
 *
 * Copyright (C) 2019 by
 * Nikhil Ramakrishnan, David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

#include <ft2build.h>
#include "sfwoff2.h"
#include "woff2tags.h"
#include FT_TRUETYPE_TAGS_H
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H


  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  sfwoff2


#define READ_255USHORT( var )  Read255UShort( stream, &var )
#define READ_BASE128( var )    ReadBase128( stream, &var )
#define ROUND4( var )          ( var + 3 ) & ~3


  static FT_Error
  Read255UShort( FT_Stream   stream,
                 FT_UShort*  value )
  {
    static const FT_Int  oneMoreByteCode1 = 255;
    static const FT_Int  oneMoreByteCode2 = 254;
    static const FT_Int  wordCode         = 253;
    static const FT_Int  lowestUCode      = 253;

    FT_Error             error;
    FT_Byte              code;
    FT_Byte              result_byte      = 0;
    FT_UShort            result_short     = 0;

    if( FT_READ_BYTE( code ) )
      return FT_THROW( Invalid_Table );
    if( code == wordCode )
    {
      /* Read next two bytes and store UInt16 value */
      if( FT_READ_USHORT( result_short ) )
        return FT_THROW( Invalid_Table );
      *value = result_short;
      return FT_Err_Ok;
    }
    else if( code == oneMoreByteCode1 )
    {
      if( FT_READ_BYTE( result_byte ) )
        return FT_THROW( Invalid_Table );
      *value = result_byte + lowestUCode;
      return FT_Err_Ok;
    }
    else if( code == oneMoreByteCode2 )
    {
      if( FT_READ_BYTE( result_byte ) )
        return FT_THROW( Invalid_Table );
      *value = result_byte + lowestUCode * 2;
      return FT_Err_Ok;
    }
    else
    {
      *value = code;
      return FT_Err_Ok;
    }
  }


  static FT_Error
  ReadBase128( FT_Stream  stream,
               FT_ULong*  value )
  {
    FT_ULong  result = 0;
    FT_Int    i;
    FT_Byte   code;
    FT_Error  error;

    for ( i = 0; i < 5; ++i ) {
      code = 0;
      if( FT_READ_BYTE( code ) )
        return FT_THROW( Invalid_Table );

      /* Leading zeros are invalid. */
      if ( i == 0 && code == 0x80 ) {
        return FT_THROW( Invalid_Table );
      }

      /* If any of top seven bits are set then we're about to overflow. */
      if ( result & 0xfe000000 ){
        return FT_THROW( Invalid_Table );
      }

      result = ( result << 7 ) | ( code & 0x7f );

      /* Spin until most significant bit of data byte is false. */
      if ( (code & 0x80) == 0 ) {
        *value = result;
        return FT_Err_Ok;
      }
    }
    /* Make sure not to exceed the size bound. */
    return FT_THROW( Invalid_Table );
  }


  static FT_Offset
  CollectionHeaderSize( FT_ULong header_version,
                        FT_UShort num_fonts )
  {
    FT_Offset size = 0;
    if (header_version == 0x00020000)
      size += 12;             /* ulDsig{Tag,Length,Offset} */
    if (header_version == 0x00010000 || header_version == 0x00020000) {
      size += 12              /* TTCTag, Version, numFonts */
        + ( 4 * num_fonts );  /* OffsetTable[numFonts]     */
    }
    return size;
  }


  static FT_Error
  compute_first_table_offset( const WOFF2_Header  woff2 )
  {
    FT_Int  nn;
    FT_Offset  offset = WOFF2_SFNT_HEADER_SIZE +
                        ( WOFF2_SFNT_ENTRY_SIZE *
                        (FT_Offset)( woff2->num_tables ) );
    if(woff2->header_version)
    {
      offset = CollectionHeaderSize( woff2->header_version,
                                     woff2->num_fonts )
               + WOFF2_SFNT_HEADER_SIZE * woff2->num_fonts;
      for ( nn = 0; nn< woff2->num_fonts; nn++ ) {
        offset += WOFF2_SFNT_ENTRY_SIZE * woff2->ttc_fonts[nn].num_tables;
      }
    }
    return offset;
  }


  /* Replace `face->root.stream' with a stream containing the extracted */
  /* SFNT of a WOFF2 font.                                              */

  FT_LOCAL_DEF( FT_Error )
  woff2_open_font( FT_Stream  stream,
                   TT_Face    face )
  {
    FT_Memory        memory = stream->memory;
    FT_Error         error  = FT_Err_Ok;

    WOFF2_HeaderRec  woff2;
    WOFF2_Table      tables  = NULL;
    WOFF2_Table*     indices = NULL;
    WOFF2_Table      last_table;

    FT_Int           nn;
    FT_ULong         j;
    FT_ULong         flags;
    FT_UShort        xform_version;
    FT_ULong         src_offset = 0;

    FT_UShort        ttc_num_tables;
    FT_UInt          glyf_index;
    FT_UInt          loca_index;
    FT_UInt64        first_table_offset;
    FT_UInt64        file_offset;

    static const FT_Frame_Field  woff2_header_fields[] =
    {
#undef  FT_STRUCTURE
#define FT_STRUCTURE  WOFF2_HeaderRec

      FT_FRAME_START( 48 ),
        FT_FRAME_ULONG ( signature ),
        FT_FRAME_ULONG ( flavor ),
        FT_FRAME_ULONG ( length ),
        FT_FRAME_USHORT( num_tables ),
        FT_FRAME_SKIP_BYTES( 2 + 4 ),
        FT_FRAME_ULONG ( totalCompressedSize ),
        FT_FRAME_SKIP_BYTES( 2 * 2 ),
        FT_FRAME_ULONG ( metaOffset ),
        FT_FRAME_ULONG ( metaLength ),
        FT_FRAME_ULONG ( metaOrigLength ),
        FT_FRAME_ULONG ( privOffset ),
        FT_FRAME_ULONG ( privLength ),
      FT_FRAME_END
    };


    /* DEBUG - Remove later */
    FT_TRACE2(( "woff2_open_font: Received Data.\n" ));

    FT_ASSERT( stream == face->root.stream );
    FT_ASSERT( FT_STREAM_POS() == 0 );

    /* Read WOFF2 Header. */
    if ( FT_STREAM_READ_FIELDS( woff2_header_fields, &woff2 ) )
      return error;

    /* DEBUG - Remove later. */
    FT_TRACE2(( "signature  -> 0x%X\n", woff2.signature ));
    FT_TRACE2(( "flavor     -> 0x%X\n", woff2.flavor ));
    FT_TRACE2(( "length     -> %lu\n", woff2.length ));
    FT_TRACE2(( "num_tables -> %hu\n", woff2.num_tables ));
    FT_TRACE2(( "metaOffset -> %hu\n", woff2.metaOffset ));
    FT_TRACE2(( "metaLength -> %hu\n", woff2.metaLength ));
    FT_TRACE2(( "privOffset -> %hu\n", woff2.privOffset ));
    FT_TRACE2(( "privLength -> %hu\n", woff2.privLength ));

    /* Make sure we don't recurse back here. */
    if ( woff2.flavor == TTAG_wOF2 )
      return FT_THROW( Invalid_Table );

    /* Miscellaneous checks. */
    if ( woff2.length != stream->size                               ||
         woff2.num_tables == 0                                      ||
         48 + woff2.num_tables * 20UL >= woff2.length               ||
         ( woff2.metaOffset == 0 && ( woff2.metaLength != 0     ||
                                      woff2.metaOrigLength != 0 ) ) ||
         ( woff2.metaLength != 0 && woff2.metaOrigLength == 0 )     ||
         ( woff2.metaOffset >= woff2.length )                       ||
         ( woff2.length - woff2.metaOffset < woff2.metaLength )     ||
         ( woff2.privOffset == 0 && woff2.privLength != 0 )         ||
         ( woff2.privOffset >= woff2.length )                       ||
         ( woff2.length - woff2.privOffset < woff2.privLength )     )
    {
      FT_ERROR(( "woff_font_open: invalid WOFF2 header\n" ));
      return FT_THROW( Invalid_Table );
    }
    /* DEBUG - Remove later. */
    else{
      FT_TRACE2(( "WOFF2 Header is valid.\n" ));
    }

    /* Read table directory. */
    if ( FT_NEW_ARRAY( tables, woff2.num_tables )  ||
         FT_NEW_ARRAY( indices, woff2.num_tables ) )
      goto Exit;

    FT_TRACE2(( "\n"
                "  tag    flags    transform   origLen   transformLen\n"
                "  --------------------------------------------------\n" ));

    /* TODO check whether there is sufficient input before FT_READ_*. */
    for ( nn = 0; nn < woff2.num_tables; nn++ )
    {
      WOFF2_Table  table = tables + nn;
      if( FT_READ_BYTE( table->FlagByte ) )
        goto Exit;

      if ( ( table->FlagByte & 0x3f ) == 0x3f )
      {
        if( FT_READ_ULONG( table->Tag ) )
          goto Exit;
      }
      else
        table->Tag = KnownTags[table->FlagByte & 0x3f];

      flags = 0;
      xform_version = ( table->FlagByte >> 6 ) & 0x03;

      /* 0 means xform for glyph/loca, non-0 for others. */
      if ( table->Tag == TTAG_glyf || table->Tag == TTAG_loca )
      {
        if ( xform_version == 0 )
          flags |= WOFF2_FLAGS_TRANSFORM;
      }
      else if ( xform_version != 0 )
        flags |= WOFF2_FLAGS_TRANSFORM;

      flags |= xform_version;

      if( READ_BASE128( table->OrigLength ) )
        goto Exit;

      table->TransformLength = table->OrigLength;

      if ( ( flags & WOFF2_FLAGS_TRANSFORM ) != 0 )
      {
        if( READ_BASE128( table->TransformLength ) )
          goto Exit;
        if( table->Tag == TTAG_loca && table->TransformLength )
        {
          FT_ERROR(( "woff_font_open: Invalid loca `transformLength'.\n" ));
          return FT_THROW( Invalid_Table );
        }
      }

      if ( src_offset + table->TransformLength < src_offset )
      {
        FT_ERROR(( "woff_font_open: invalid WOFF2 table directory.\n" ));
        return FT_THROW( Invalid_Table );
      }

      table->src_offset = src_offset;
      table->src_length = table->TransformLength;
      src_offset += table->TransformLength;
      table->flags = flags;

      FT_TRACE2(( "  %c%c%c%c  %08d  %08d    %08ld  %08ld\n",
                  (FT_Char)( table->Tag >> 24 ),
                  (FT_Char)( table->Tag >> 16 ),
                  (FT_Char)( table->Tag >> 8  ),
                  (FT_Char)( table->Tag       ),
                  table->FlagByte & 0x3f,
                  ( table->FlagByte >> 6 ) & 0x03,
                  table->OrigLength,
                  table->TransformLength,
                  table->src_length,
                  table->src_offset ));

      indices[nn] = table;
    }

    /* End of last table is uncompressed size. */
    last_table = indices[woff2.num_tables - 1];
    woff2.uncompressed_size = last_table->src_offset
                              + last_table->src_length;
    if( woff2.uncompressed_size < last_table->src_offset )
        return FT_THROW( Invalid_Table );
    /* DEBUG - Remove later. */
    FT_TRACE2(( "Uncompressed size: %ld\n", woff2.uncompressed_size ));

    FT_TRACE2(( "Table directory successfully parsed.\n" ));

    /* Check for and read collection directory. */
    woff2.header_version = 0;
    if( woff2.flavor == TTAG_ttcf ){
      FT_TRACE2(( "Font is a TTC, reading collection directory.\n" ));
      if( FT_READ_ULONG( woff2.header_version ) )
        goto Exit;
      /* DEBUG - Remove later */
      FT_TRACE2(( "Header version: %lx\n", woff2.header_version ));
      if( woff2.header_version != 0x00010000 &&
          woff2.header_version != 0x00020000 )
        return FT_THROW( Invalid_Table );

      if( READ_255USHORT( woff2.num_fonts ) )
        goto Exit;
      /* DEBUG - Remove later */
      FT_TRACE2(( "Number of fonts in TTC: %ld\n", woff2.num_fonts ));

      if( FT_NEW_ARRAY( woff2.ttc_fonts, woff2.num_fonts ) )
        goto Exit;

      for ( nn = 0; nn < woff2.num_fonts; nn++ )
      {
        WOFF2_TtcFont  ttc_font = woff2.ttc_fonts + nn;

        if( READ_255USHORT( ttc_num_tables ) )
          goto Exit;
        if( FT_READ_ULONG( ttc_font->flavor ) )
          goto Exit;

        if( FT_NEW_ARRAY( ttc_font->table_indices, ttc_num_tables ) )
          goto Exit;
        /* DEBUG - Change to TRACE4 */
        FT_TRACE2(( "Number of tables in font %d: %ld\n",
                    nn, ttc_num_tables ));
        /* DEBUG - Change to TRACE5 */
        FT_TRACE2(( "  Indices: " ));

        glyf_index = 0;
        loca_index = 0;

        for ( j = 0; j < ttc_num_tables; j++ )
        {
          FT_UShort    table_index;
          WOFF2_Table  table;

          if( READ_255USHORT( table_index ) )
            goto Exit;
          /* DEBUG - Change to TRACE5 */
          FT_TRACE2(("%hu ", table_index));
          ttc_font->table_indices[j] = table_index;

          table = indices[table_index];
          if( table->Tag == TTAG_loca )
            loca_index = table_index;
          if( table->Tag == TTAG_glyf )
            glyf_index = table_index;
        }
        /* DEBUG - Change to TRACE5 */
        FT_TRACE2(( "\n" ));

        /* glyf and loca must be consecutive */
        if( glyf_index > 0 || loca_index > 0 )
        {
          if(glyf_index > loca_index || loca_index - glyf_index != 1)
            return FT_THROW( Invalid_Table );
          /* DEBUG - Remove later */
          else
            FT_TRACE2(( "Glyf and loca are valid.\n" ));
        }
      }
      /* Collection directory reading complete. */
      FT_TRACE2(( "WOFF2 collection dirtectory is valid.\n" ));

    }

    first_table_offset = compute_first_table_offset( &woff2 );
    FT_TRACE2(( "Offset to first table: %ld\n", first_table_offset ));

    woff2.compressed_offset = FT_STREAM_POS();
    file_offset = ROUND4( woff2.compressed_offset +
                            woff2.totalCompressedSize );

    /* Few more checks before we start reading the tables */
    if( file_offset > woff2.length )
      return FT_THROW( Invalid_Table );

    if ( woff2.metaOffset )
    {
      if ( file_offset != woff2.metaOffset )
        return FT_THROW( Invalid_Table );
      file_offset = ROUND4(woff2.metaOffset + woff2.metaLength);
    }

    if( woff2.privOffset )
    {
      if( file_offset != woff2.privOffset )
        return FT_THROW( Invalid_Table );
      file_offset = ROUND4(woff2.privOffset + woff2.privLength);
    }

    if( file_offset != ( ROUND4( woff2.length ) ) )
      return FT_THROW( Invalid_Table );

    error = FT_THROW( Unimplemented_Feature );
    FT_TRACE2(( "Reached end without errors.\n" ));
    goto Exit;

  Exit:
    return error;
  }


/* END */
