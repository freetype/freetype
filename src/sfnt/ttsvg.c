/****************************************************************************
 *
 * ttsvg.c
 *
 *   OpenType SVG Color (specification).
 *
 * Copyright (C) 2018-2019 by
 * David Turner, Robert Wilhelm, Werner Lemberg and Moazin Khatti.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


  /**************************************************************************
   *
   * `SVG' table specification:
   *
   *    https://docs.microsoft.com/en-us/typography/opentype/spec/svg
   *
   */

#include <ft2build.h>
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_OBJECTS_H
#include FT_TRUETYPE_TAGS_H
#include FT_GZIP_H
#include FT_SVG_RENDER_H

#ifdef FT_CONFIG_OPTION_SVG

#include "ttsvg.h"

  /* TODO: (OT-SVG) Decide whether to add documentation here or not */

  typedef struct Svg_
  {
    FT_UShort  version;           /* Table version (starting at 0)         */
    FT_UShort  num_entries;        /* Number of SVG document records       */
    /* Pointer to the starting of SVG Document List */
    FT_Byte*   svg_doc_list;
    /* Memory that backs up SVG */
    void*     table;
    FT_ULong  table_size;
  } Svg;


  FT_LOCAL_DEF( FT_Error )
  tt_face_load_svg( TT_Face    face,
                    FT_Stream  stream )
  {
    FT_Error   error;
    FT_Memory  memory = face->root.memory;

    FT_ULong   table_size;
    FT_Byte*   table = NULL;
    FT_Byte*   p     = NULL;

    Svg*       svg   = NULL;

    FT_ULong  offsetToSVGDocumentList;


    error = face->goto_table( face, TTAG_SVG, stream, &table_size );
    if( error )
      goto NoSVG;

    if( FT_FRAME_EXTRACT( table_size, table ))
      goto NoSVG;

    /* Allocate the memory for the Svg object */
    if( FT_NEW( svg ) )
      goto NoSVG;

    p = table;
    svg->version =            FT_NEXT_USHORT( p );
    offsetToSVGDocumentList = FT_NEXT_ULONG( p );

    if( offsetToSVGDocumentList == 0 )
      goto InvalidTable;

    svg->svg_doc_list = (FT_Byte*)( table + offsetToSVGDocumentList );

    p = svg->svg_doc_list;
    svg->num_entries = FT_NEXT_USHORT( p );

    FT_TRACE3(( "version: %d\n", svg->version ));
    FT_TRACE3(( "num entiries: %d\n", svg->num_entries ));

    svg->table =      table;
    svg->table_size = table_size;

    face->svg = svg;

    face->root.face_flags |= FT_FACE_FLAG_SVG;

    return FT_Err_Ok;

  InvalidTable:
    error = FT_THROW( Invalid_Table );

  NoSVG:
    FT_FRAME_RELEASE( table );
    FT_FREE( svg );
    face->svg = NULL;

    return error;
  }

  FT_LOCAL_DEF( void )
  tt_face_free_svg( TT_Face  face )
  {
    FT_Memory  memory = face->root.memory;
    FT_Stream  stream = face->root.stream;
    Svg*       svg    = (Svg*) face->svg;


    if( svg )
    {
      FT_FRAME_RELEASE( svg->table );
      FT_FREE( svg );
    }
  }

  typedef struct Svg_doc_
  {
    FT_UShort  start_glyph_id;
    FT_UShort  end_glyph_id;
    FT_ULong   offset;
    FT_ULong   length;
  } Svg_doc;

  static Svg_doc
  extract_svg_doc( FT_Byte*  stream )
  {
    Svg_doc  doc;
    doc.start_glyph_id = FT_NEXT_USHORT( stream );
    doc.end_glyph_id   = FT_NEXT_USHORT( stream );
    doc.offset = FT_NEXT_ULONG( stream );
    doc.length = FT_NEXT_ULONG( stream );
    return doc;
  }

  static FT_Int
  compare_svg_doc( Svg_doc  doc,
                   FT_UInt  glyph_index )
  {
    if ( glyph_index < doc.start_glyph_id )
      return -1;
    else if ( glyph_index > doc.end_glyph_id )
      return 1;
    else
      return 0;
  }

  static FT_Error
  find_doc( FT_Byte*   stream,
            FT_UShort  num_entries,
            FT_UInt    glyph_index,
            FT_ULong   *doc_offset,
            FT_ULong   *doc_length,
            FT_UShort  *start_glyph,
            FT_UShort  *end_glyph )
  {
    FT_Error   error;
    Svg_doc    start_doc;
    Svg_doc    mid_doc;
    Svg_doc    end_doc;

    FT_Bool  found       = FALSE;
    FT_UInt  i           = 0;
    FT_UInt  start_index = 0;
    FT_UInt  end_index   = num_entries - 1;
    FT_Int   comp_res;


    /* search algo */
    if ( num_entries == 0 )
    {
      error = FT_THROW( Invalid_Table );
      return error;
    }

    FT_TRACE6(( "--- binary search glyph id: %d ---\n", glyph_index ));

    start_doc = extract_svg_doc( stream + start_index * 12 );
    end_doc   = extract_svg_doc( stream + end_index * 12 );

    FT_TRACE6(( "--- start glyph ---\n" ));
    FT_TRACE6(( "start_id: %d\n", start_doc.start_glyph_id ));
    FT_TRACE6(( "end_id: %d\n", start_doc.end_glyph_id ));
    FT_TRACE6(( "--- end glyph ---\n" ));
    FT_TRACE6(( "start_id: %d\n", end_doc.start_glyph_id ));
    FT_TRACE6(( "end_id: %d\n", end_doc.end_glyph_id ));
    if ( ( compare_svg_doc( start_doc, glyph_index ) == -1 ) ||
         ( compare_svg_doc( end_doc, glyph_index ) == 1 ) )
    {
      error = FT_THROW( Invalid_Glyph_Index );
      return error;
    }

    while ( start_index <= end_index )
    {
      i = ( start_index + end_index ) / 2;
      mid_doc = extract_svg_doc( stream + i * 12 );
      FT_TRACE6(( "--- current glyph ---\n" ));
      FT_TRACE6(( "start_id: %d\n", mid_doc.start_glyph_id ));
      FT_TRACE6(( "end_id: %d\n", mid_doc.end_glyph_id ));
      comp_res = compare_svg_doc( mid_doc, glyph_index );
      if ( comp_res == 1 )
      {
        start_index = i + 1;
        start_doc = extract_svg_doc( stream + start_index * 4 );
        FT_TRACE6(( "RIGHT\n" ));
      }
      else if ( comp_res == -1 )
      {
        end_index = i - 1;
        end_doc = extract_svg_doc( stream + end_index * 4 );
        FT_TRACE6(( "LEFT\n" ));
      }
      else
      {
        found = TRUE;
        FT_TRACE5(( "FOUND\n" ));
        break;
      }
    }

    FT_TRACE5(( "--- binary search end ---\n" ));
    /* search algo end */
    if ( found != TRUE )
    {
      FT_TRACE5(( "NOT FOUND\n" ));
      error = FT_THROW( Invalid_Glyph_Index );
    }
    else
    {
      *doc_offset = mid_doc.offset;
      *doc_length = mid_doc.length;
      *start_glyph = mid_doc.start_glyph_id;
      *end_glyph   = mid_doc.end_glyph_id;
      error = FT_Err_Ok;
    }
    return error;
  }

  FT_LOCAL_DEF( FT_Error )
  tt_face_load_svg_doc( FT_GlyphSlot  glyph,
                        FT_UInt       glyph_index )
  {

    /* TODO: (OT-SVG) properly clean stuff here on errors */

    FT_Byte*   doc_list;             /* Pointer to the Svg Document List */
    FT_UShort  num_entries;          /* Total no of entires in doc list  */

    FT_ULong   doc_offset;
    FT_ULong   doc_length;
    FT_UShort  start_glyph_id;
    FT_UShort  end_glyph_id;

    FT_ULong   uncomp_size;
    FT_Byte*   uncomp_buffer;

    FT_Error   error  = FT_Err_Ok;
    TT_Face    face   = (TT_Face)glyph->face;
    FT_Memory  memory = face->root.memory;
    Svg*       svg    = face->svg;

    FT_SVG_Document  svg_document = glyph->other;

    /* handle svg being 0x0 situation here */
    doc_list     = svg->svg_doc_list;
    num_entries  = FT_NEXT_USHORT( doc_list );

    error = find_doc( doc_list, num_entries, glyph_index,
                                &doc_offset, &doc_length,
                                &start_glyph_id, &end_glyph_id );
    if ( error != FT_Err_Ok )
      return error;

    doc_list = svg->svg_doc_list;   /* Reset to so we can use it again */
    doc_list = (FT_Byte*)( doc_list + doc_offset );

    if( ( doc_list[0] == 0x1F ) && ( doc_list[1] == 0x8B )
                                && ( doc_list[2] == 0x08 ) )
    {
      /* get the size of the orignal document. This helps in alotting the
       * buffer to accomodate the uncompressed version. The last 4 bytes
       * of the compressed document are equal to orignal_size modulo 2^32.
       * Since SVG docs will be lesser in size then 2^32, we can use this
       * accurately. The four bytes are stored in little-endian format.
       */
      FT_TRACE4(( "SVG document found is GZIP compressed\n" ));
      uncomp_size = (FT_ULong)doc_list[doc_length - 1] << 24 |
                    (FT_ULong)doc_list[doc_length - 2] << 16 |
                    (FT_ULong)doc_list[doc_length - 3] << 8  |
                    (FT_ULong)doc_list[doc_length - 4];

      uncomp_buffer = (FT_Byte*) memory->alloc(memory, uncomp_size);
      glyph->internal->flags |= FT_GLYPH_OWN_GZIP_SVG;
      error = FT_Gzip_Uncompress( memory, uncomp_buffer, &uncomp_size,
                                          doc_list,      doc_length  );
      if ( error != FT_Err_Ok )
      {
        error = FT_THROW( Invalid_Table );
        return error;
      }

      doc_list   = uncomp_buffer;
      doc_length = uncomp_size;
    }

    svg_document->svg_document        = doc_list;
    svg_document->svg_document_length = doc_length;
    svg_document->metrics             = glyph->face->size->metrics;
    svg_document->units_per_EM        = glyph->face->units_per_EM;
    svg_document->start_glyph_id      = start_glyph_id;
    svg_document->end_glyph_id        = end_glyph_id;

    FT_TRACE5(( "start_glyph_id: %d\n", start_glyph_id ));
    FT_TRACE5(( "end_glyph_id:   %d\n", end_glyph_id ));
    FT_TRACE5(( "svg_document:\n%.*s\n", doc_length, doc_list ));

    glyph->other = svg_document;

    return FT_Err_Ok;
  }

#else /* !FT_CONFIG_OPTION_SVG */

  /* ANSI C doesn't like empty source files */
  typedef int  _tt_cpal_dummy;

#endif /* !FT_CONFIG_OPTION_SVG */
