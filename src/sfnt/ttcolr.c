/****************************************************************************
 *
 * ttcolr.c
 *
 *   TrueType and OpenType color outline support.
 *
 * Copyright 2018 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * Written by Shao Yu Zhang <shaozhang@fb.com>.
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
   * `COLR' and `CPAL' table specification:
   *
   *   https://www.microsoft.com/typography/otspec/colr.htm
   *   https://www.microsoft.com/typography/otspec/cpal.htm
   *
   */


#include <ft2build.h>
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_TRUETYPE_TAGS_H
#include FT_COLOR_H


#ifdef TT_CONFIG_OPTION_COLOR_LAYERS

#include "ttcolr.h"


  /* NOTE: These are the table sizes calculated through the specs. */
#define BASE_GLYPH_SIZE            6
#define LAYER_SIZE                 4
#define COLR_HEADER_SIZE          14
#define CPAL_V0_HEADER_BASE_SIZE  12
#define COLOR_SIZE                 4


  typedef struct BaseGlyphRecord_
  {
    FT_UShort  gid;
    FT_UShort  first_layer_index;
    FT_UShort  num_layers;

  } BaseGlyphRecord;


  typedef struct Colr_
  {
    FT_UShort  version;
    FT_UShort  num_base_glyphs;
    FT_UShort  num_layers;

    FT_Byte*  base_glyphs;
    FT_Byte*  layers;

  } Colr;


  /* all data from `CPAL' not covered in FT_Palette */
  typedef struct Cpal_
  {
    FT_UShort  version;        /* Table version number (0 or 1 supported). */
    FT_UShort  num_colors;               /* Total number of color records, */
                                         /* combined for all palettes.     */
    FT_Byte*  colors;                              /* RGBA array of colors */
    FT_Byte*  color_indices; /* Index of each palette's first color record */
                             /* in the combined color record array.        */

  } Cpal;


  typedef struct ColrCpal_
  {
    /* Accessors into the colr/cpal tables. */
    Colr  colr;
    Cpal  cpal;

    /* The memory which backs up colr/cpal tables. */
    void*  colr_table;
    void*  cpal_table;

  } ColrCpal;


  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ttcolrcpal


  FT_LOCAL_DEF( FT_Error )
  tt_face_load_colr( TT_Face    face,
                     FT_Stream  stream )
  {
    FT_Error   error;
    FT_Memory  memory = face->root.memory;

    FT_Byte*  colr_table = NULL;
    FT_Byte*  cpal_table = NULL;
    FT_Byte*  p          = NULL;

    Colr       colr;
    Cpal       cpal;
    ColrCpal*  cc = NULL;

    FT_ULong  base_glyph_offset, layer_offset, colors_offset;
    FT_ULong  table_size;


    /*
     * COLR
     */

    error = face->goto_table( face, TTAG_COLR, stream, &table_size );
    if ( error )
      goto NoColor;

    if ( table_size < COLR_HEADER_SIZE )
      goto InvalidTable;

    if ( FT_FRAME_EXTRACT( table_size, colr_table ) )
      goto NoColor;

    p = colr_table;

    FT_ZERO( &colr );
    colr.version = FT_NEXT_USHORT( p );
    if ( colr.version != 0 )
      goto InvalidTable;

    colr.num_base_glyphs = FT_NEXT_USHORT( p );
    base_glyph_offset    = FT_NEXT_ULONG( p );

    if ( base_glyph_offset >= table_size )
      goto InvalidTable;
    if ( colr.num_base_glyphs * BASE_GLYPH_SIZE >
           table_size - base_glyph_offset )
      goto InvalidTable;

    layer_offset    = FT_NEXT_ULONG( p );
    colr.num_layers = FT_NEXT_USHORT( p );

    if ( layer_offset >= table_size )
      goto InvalidTable;
    if ( colr.num_layers * LAYER_SIZE > table_size - layer_offset )
      goto InvalidTable;

    colr.base_glyphs = (FT_Byte*)( colr_table + base_glyph_offset );
    colr.layers      = (FT_Byte*)( colr_table + layer_offset      );

    /*
     * CPAL
     */

    error = face->goto_table( face, TTAG_CPAL, stream, &table_size );
    if ( error )
      goto NoColor;

    if ( table_size < CPAL_V0_HEADER_BASE_SIZE )
      goto InvalidTable;

    if ( FT_FRAME_EXTRACT( table_size, cpal_table ) )
      goto NoColor;

    p = cpal_table;

    FT_ZERO( &cpal );
    cpal.version = FT_NEXT_USHORT( p );
    if ( cpal.version > 1 )
      goto InvalidTable;

    face->palette.num_palette_entries = FT_NEXT_USHORT( p );
    face->palette.num_palettes        = FT_NEXT_USHORT( p );

    cpal.num_colors = FT_NEXT_USHORT( p );
    colors_offset   = FT_NEXT_ULONG( p );

    if ( colors_offset >= table_size )
      goto InvalidTable;
    if ( cpal.num_colors * COLOR_SIZE > table_size - colors_offset )
      goto InvalidTable;

    cpal.color_indices = p;
    cpal.colors        = (FT_Byte*)( cpal_table + colors_offset );

    if ( cpal.version == 1 )
    {
      FT_ULong    type_offset, label_offset, entry_label_offset;
      FT_UShort*  array;
      FT_UShort*  limit;
      FT_UShort*  q;


      p += face->palette.num_palettes * 2;

      type_offset        = FT_NEXT_ULONG( p );
      label_offset       = FT_NEXT_ULONG( p );
      entry_label_offset = FT_NEXT_ULONG( p );

      if ( type_offset )
      {
        if ( type_offset >= table_size )
          goto InvalidTable;
        if ( face->palette.num_palettes * 2 > table_size - type_offset )
          goto InvalidTable;

        if ( FT_QNEW_ARRAY( array, face->palette.num_palettes ) )
          goto NoColor;

        p     = cpal_table + type_offset;
        q     = array;
        limit = q + face->palette.num_palettes * sizeof ( FT_UShort );

        while ( q < limit )
          *q++ = FT_NEXT_USHORT( p );

        face->palette.palette_types = array;
      }

      if ( label_offset )
      {
        if ( label_offset >= table_size )
          goto InvalidTable;
        if ( face->palette.num_palettes * 2 > table_size - label_offset )
          goto InvalidTable;

        if ( FT_QNEW_ARRAY( array, face->palette.num_palettes ) )
          goto NoColor;

        p     = cpal_table + label_offset;
        q     = array;
        limit = q + face->palette.num_palettes * sizeof ( FT_UShort );

        while ( q < limit )
          *q++ = FT_NEXT_USHORT( p );

        face->palette.palette_name_ids = array;
      }

      if ( entry_label_offset )
      {
        if ( entry_label_offset >= table_size )
          goto InvalidTable;
        if ( face->palette.num_palette_entries * 2 >
               table_size - entry_label_offset )
          goto InvalidTable;

        if ( FT_QNEW_ARRAY( array, face->palette.num_palette_entries ) )
          goto NoColor;

        p     = cpal_table + entry_label_offset;
        q     = array;
        limit = q + face->palette.num_palette_entries * sizeof ( FT_UShort );

        while ( q < limit )
          *q++ = FT_NEXT_USHORT( p );

        face->palette.palette_entry_name_ids = array;
      }
    }

    if ( FT_NEW( cc ) )
      goto NoColor;

    cc->colr       = colr;
    cc->cpal       = cpal;
    cc->colr_table = colr_table;
    cc->cpal_table = cpal_table;

    face->colr_and_cpal = cc;

    return FT_Err_Ok;

  InvalidTable:
    error = FT_THROW( Invalid_File_Format );

  NoColor:
    FT_FRAME_RELEASE( colr_table );
    FT_FRAME_RELEASE( cpal_table );

    /* arrays in `face->palette' are freed in `sfnt_face_done' */

    return error;
  }


  FT_LOCAL_DEF( void )
  tt_face_free_colr( TT_Face  face )
  {
    FT_Stream  stream = face->root.stream;
    FT_Memory  memory = face->root.memory;

    ColrCpal*  colr_and_cpal = (ColrCpal*)face->colr_and_cpal;


    if ( colr_and_cpal )
    {
      FT_FRAME_RELEASE( colr_and_cpal->colr_table );
      FT_FRAME_RELEASE( colr_and_cpal->cpal_table );

      FT_FREE( face->colr_and_cpal );
    }
  }


  static FT_Bool
  find_base_glyph_record( FT_Byte*          base_glyph_begin,
                          FT_Int            num_base_glyph,
                          FT_UInt           glyph_id,
                          BaseGlyphRecord*  record )
  {
    FT_Int  min = 0;
    FT_Int  max = num_base_glyph - 1;


    while ( min <= max )
    {
      FT_Int    mid = min + ( max - min ) / 2;
      FT_Byte*  p   = base_glyph_begin + mid * BASE_GLYPH_SIZE;

      FT_UShort  gid = FT_NEXT_USHORT( p );


      if ( gid < glyph_id )
        min = mid + 1;
      else if (gid > glyph_id )
        max = mid - 1;
      else
      {
        record->gid               = gid;
        record->first_layer_index = FT_NEXT_USHORT( p );
        record->num_layers        = FT_NEXT_USHORT( p );

        return 1;
      }
    }

    return 0;
  }


  FT_LOCAL_DEF( FT_Error )
  tt_face_load_colr_layers( TT_Face          face,
                            FT_UInt          glyph_id,
                            FT_Glyph_Layer  *ret_layers,
                            FT_UShort*       ret_num_layers )
  {
    FT_Error   error;
    FT_Memory  memory = face->root.memory;

    ColrCpal*  colr_and_cpal = (ColrCpal *)face->colr_and_cpal;
    Colr*      colr          = &colr_and_cpal->colr;

    BaseGlyphRecord  glyph_record;
    FT_Glyph_Layer   layers;
    int              layer_idx;
    FT_Byte*         layer_record_ptr;


    if ( !ret_layers || !ret_num_layers )
      return FT_THROW( Invalid_Argument );

    if ( !find_base_glyph_record( colr->base_glyphs,
                                  colr->num_base_glyphs,
                                  glyph_id,
                                  &glyph_record ) )
    {
      *ret_layers     = NULL;
      *ret_num_layers = 0;

      return FT_Err_Ok;
    }

    /* Load all colors for the glyphs; this would be stored in the slot. */
    layer_record_ptr = colr->layers +
                       glyph_record.first_layer_index * LAYER_SIZE;

    if ( FT_NEW_ARRAY( layers, glyph_record.num_layers ) )
      goto Error;

    for ( layer_idx = 0; layer_idx < glyph_record.num_layers; layer_idx++ )
    {
      FT_UShort  gid           = FT_NEXT_USHORT( layer_record_ptr );
      FT_UShort  palette_index = FT_NEXT_USHORT( layer_record_ptr );


      if ( palette_index != 0xFFFF                            &&
           palette_index >= face->palette.num_palette_entries )
      {
        error = FT_THROW( Invalid_File_Format );
        goto Error;
      }

      layers[layer_idx].color_index = palette_index;
      layers[layer_idx].glyph_index = gid;
    }

    *ret_layers     = layers;
    *ret_num_layers = glyph_record.num_layers;

    return FT_Err_Ok;

  Error:
    if ( layers )
      FT_FREE( layers );

    return error;
  }


  static FT_Bool
  tt_face_find_color( TT_Face   face,
                      FT_UInt   color_index,
                      FT_Byte*  blue,
                      FT_Byte*  green,
                      FT_Byte*  red,
                      FT_Byte*  alpha )
  {
    ColrCpal*  colr_and_cpal = (ColrCpal *)face->colr_and_cpal;
    Cpal*      cpal          = &colr_and_cpal->cpal;

    FT_Int    palette_index = 0;
    FT_Byte*  p;
    FT_Int    color_offset;


    if ( color_index >= face->palette.num_palette_entries )
      return 0;

    p = cpal->color_indices + palette_index * (int)sizeof ( FT_UShort );

    color_offset = FT_NEXT_USHORT( p );

    p = cpal->colors + color_offset + COLOR_SIZE * color_index;

    *blue  = FT_NEXT_BYTE( p );
    *green = FT_NEXT_BYTE( p );
    *red   = FT_NEXT_BYTE( p );
    *alpha = FT_NEXT_BYTE( p );

    return 1;
  }


  FT_LOCAL_DEF( FT_Error )
  tt_face_colr_blend_layer( TT_Face       face,
                            FT_UInt       color_index,
                            FT_GlyphSlot  dstSlot,
                            FT_GlyphSlot  srcSlot )
  {
    FT_Error  error;

    FT_UInt  x, y;
    FT_Byte  b, g, r, alpha;

    FT_ULong  size;
    FT_Byte*  src;
    FT_Byte*  dst;


    if ( !dstSlot->bitmap.buffer )
    {
      /* Initialize destination of color bitmap */
      /* with the size of first component.      */
      dstSlot->bitmap_left = srcSlot->bitmap_left;
      dstSlot->bitmap_top  = srcSlot->bitmap_top;

      dstSlot->bitmap.width      = srcSlot->bitmap.width;
      dstSlot->bitmap.rows       = srcSlot->bitmap.rows;
      dstSlot->bitmap.pixel_mode = FT_PIXEL_MODE_BGRA;
      dstSlot->bitmap.pitch      = (int)dstSlot->bitmap.width * 4;
      dstSlot->bitmap.num_grays  = 256;

      size = dstSlot->bitmap.rows * (unsigned int)dstSlot->bitmap.pitch;

      error = ft_glyphslot_alloc_bitmap( dstSlot, size );
      if ( error )
        return error;

      FT_MEM_ZERO( dstSlot->bitmap.buffer, size );
    }
    else
    {
      /* Resize destination if needed such that new component fits. */
      FT_Int  x_min, x_max, y_min, y_max;


      x_min = FT_MIN( dstSlot->bitmap_left, srcSlot->bitmap_left );
      x_max = FT_MAX( dstSlot->bitmap_left + (FT_Int)dstSlot->bitmap.width,
                      srcSlot->bitmap_left + (FT_Int)srcSlot->bitmap.width );

      y_min = FT_MIN( dstSlot->bitmap_top - (FT_Int)dstSlot->bitmap.rows,
                      srcSlot->bitmap_top - (FT_Int)srcSlot->bitmap.rows );
      y_max = FT_MAX( dstSlot->bitmap_top, srcSlot->bitmap_top );

      if ( x_min != dstSlot->bitmap_left                                 ||
           x_max != dstSlot->bitmap_left + (FT_Int)dstSlot->bitmap.width ||
           y_min != dstSlot->bitmap_top - (FT_Int)dstSlot->bitmap.rows   ||
           y_max != dstSlot->bitmap_top                                  )
      {
        FT_Memory  memory = face->root.memory;

        FT_UInt  width = (FT_UInt)( x_max - x_min );
        FT_UInt  rows  = (FT_UInt)( y_max - y_min );
        FT_UInt  pitch = width * 4;

        FT_Byte*  buf;
        FT_Byte*  p;
        FT_Byte*  q;


        size  = rows * pitch;
        if ( FT_ALLOC( buf, size ) )
          return error;

        p = dstSlot->bitmap.buffer;
        q = buf +
            (int)pitch * ( y_max - dstSlot->bitmap_top ) +
            4 * ( dstSlot->bitmap_left - x_min );

        for ( y = 0; y < dstSlot->bitmap.rows; y++ )
        {
          FT_MEM_COPY( q, p, dstSlot->bitmap.width * 4 );

          p += dstSlot->bitmap.pitch;
          q += pitch;
        }

        ft_glyphslot_set_bitmap( dstSlot, buf );

        dstSlot->bitmap_top  = y_max;
        dstSlot->bitmap_left = x_min;

        dstSlot->bitmap.width = width;
        dstSlot->bitmap.rows  = rows;
        dstSlot->bitmap.pitch = (int)pitch;

        dstSlot->internal->flags |= FT_GLYPH_OWN_BITMAP;
        dstSlot->format           = FT_GLYPH_FORMAT_BITMAP;
      }
    }

    /* Default assignments to pacify compiler. */
    r = g = b = 0;
    alpha = 255;

    if ( color_index != 0xFFFF )
      tt_face_find_color( face, color_index, &b, &g, &r, &alpha );
    else
    {
      /* TODO. foreground color from argument?                */
      /* Add public FT_Render_Glyph_Color() with color value? */
    }

    /* XXX Convert if srcSlot.bitmap is not grey? */
    src = srcSlot->bitmap.buffer;
    dst = dstSlot->bitmap.buffer +
          dstSlot->bitmap.pitch * ( dstSlot->bitmap_top - srcSlot->bitmap_top ) +
          4 * ( srcSlot->bitmap_left - dstSlot->bitmap_left );

    for ( y = 0; y < srcSlot->bitmap.rows; y++ )
    {
      for ( x = 0; x < srcSlot->bitmap.width; x++ )
      {
        int  aa = src[x];
        int  fa = alpha * aa / 255;

        int  fb = b * fa / 255;
        int  fg = g * fa / 255;
        int  fr = r * fa / 255;

        int  ba2 = 255 - fa;

        int  bb = dst[4 * x + 0];
        int  bg = dst[4 * x + 1];
        int  br = dst[4 * x + 2];
        int  ba = dst[4 * x + 3];


        dst[4 * x + 0] = (FT_Byte)( bb * ba2 / 255 + fb );
        dst[4 * x + 1] = (FT_Byte)( bg * ba2 / 255 + fg );
        dst[4 * x + 2] = (FT_Byte)( br * ba2 / 255 + fr );
        dst[4 * x + 3] = (FT_Byte)( ba * ba2 / 255 + fa );
      }

      src += srcSlot->bitmap.pitch;
      dst += dstSlot->bitmap.pitch;
    }

    return FT_Err_Ok;
  }

#else /* !TT_CONFIG_OPTION_COLOR_LAYERS */

  /* ANSI C doesn't like empty source files */
  typedef int  _tt_colr_dummy;

#endif /* !TT_CONFIG_OPTION_COLOR_LAYERS */

/* EOF */
