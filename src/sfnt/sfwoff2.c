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


  static FT_Error
  ReadBase128( FT_Stream  stream,
               FT_ULong*  value )
  {
    FT_ULong  result = 0;
    FT_Int    i;
    FT_Byte   code;
    FT_Byte*  p = stream->cursor;

    for ( i = 0; i < 5; ++i ) {
      code = 0;
      code = FT_NEXT_BYTE( p );

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


  /* Replace `face->root.stream' with a stream containing the extracted */
  /* SFNT of a WOFF2 font.                                              */

  FT_LOCAL_DEF( FT_Error )
  woff2_open_font( FT_Stream  stream,
                   TT_Face    face )
  {
    FT_Memory        memory = stream->memory;
    FT_Error         error  = FT_Err_Ok;
    FT_Byte*         p      = stream->cursor;
    FT_Byte*         limit  = stream->limit;

    WOFF2_HeaderRec  woff2;
    WOFF2_Table      tables  = NULL;
    WOFF2_Table*     indices = NULL;

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

    FT_UNUSED( p );
    FT_UNUSED( limit );
    FT_UNUSED( tables );
    FT_UNUSED( indices );
    FT_UNUSED( memory );

    /* DEBUG - Remove later */
    FT_TRACE2(("woff2_open_font: Received Data.\n"));

    FT_ASSERT( stream == face->root.stream );
    FT_ASSERT( FT_STREAM_POS() == 0 );

    /* Read WOFF2 Header. */
    if ( FT_STREAM_READ_FIELDS( woff2_header_fields, &woff2 ) )
      return error;

    /* DEBUG - Remove later. */
    FT_TRACE2(("signature  -> 0x%X\n", woff2.signature));
    FT_TRACE2(("flavor     -> 0x%X\n", woff2.flavor));
    FT_TRACE2(("length     -> %lu\n", woff2.length));
    FT_TRACE2(("num_tables -> %hu\n", woff2.num_tables));
    FT_TRACE2(("metaOffset -> %hu\n", woff2.metaOffset));
    FT_TRACE2(("metaLength -> %hu\n", woff2.metaLength));
    FT_TRACE2(("privOffset -> %hu\n", woff2.privOffset));
    FT_TRACE2(("privLength -> %hu\n", woff2.privLength));

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
      FT_TRACE2(("WOFF2 Header is valid.\n"));
    }

    /* TODO Read table directory. */

    error = FT_THROW( Unimplemented_Feature );
    goto Exit;

  Exit:
    return error;
  }


/* END */
