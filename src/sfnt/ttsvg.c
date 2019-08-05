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

#ifdef FT_CONFIG_OPTION_SVG

#include "ttsvg.h"

  typedef struct Svg_
  {
    FT_UShort  version;        /* Table version (starting at 0)  */
    FT_UShort  num_entries;    /* Number of SVG document records */
    /* Pointer to the starting of SVG Document List */
    FT_Byte*   svg_doc_list;
    void*     table;           /* Memory that backs up SVG */
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

#else /* !FT_CONFIG_OPTION_SVG */

  /* ANSI C doesn't like empty source files */
  typedef int  _tt_svg_dummy;

#endif /* !FT_CONFIG_OPTION_SVG */
