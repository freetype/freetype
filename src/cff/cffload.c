/***************************************************************************/
/*                                                                         */
/*  cffload.c                                                              */
/*                                                                         */
/*    OpenType and CFF data/program tables loader (body)                   */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
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
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_POSTSCRIPT_NAMES_H
#include FT_INTERNAL_CFF_ERRORS_H
#include FT_TRUETYPE_TAGS_H

#include FT_SOURCE_FILE( cff, cffload.h )
#include FT_SOURCE_FILE( cff, cffparse.h )


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t2load


  /* read a CFF offset from memory */
  static
  FT_ULong  cff_get_offset( FT_Byte*  p,
                            FT_Byte   off_size )
  {
    FT_ULong  result;


    for ( result = 0; off_size > 0; off_size-- )
    {
      result <<= 8;
      result  |= *p++;
    }

    return result;
  }


  static
  FT_Error  cff_new_index( CFF_Index*  index,
                           FT_Stream   stream,
                           FT_Bool     load )
  {
    FT_Error   error;
    FT_Memory  memory = stream->memory;
    FT_UShort  count;


    MEM_Set( index, 0, sizeof ( *index ) );

    index->stream = stream;
    if ( !READ_UShort( count ) &&
         count > 0             )
    {
      FT_Byte*   p;
      FT_Byte    offsize;
      FT_ULong   data_size;
      FT_ULong*  poff;


      /* there is at least one element; read the offset size,           */
      /* then access the offset table to compute the index's total size */
      if ( READ_Byte( offsize ) )
        goto Exit;

      index->stream   = stream;
      index->count    = count;
      index->off_size = offsize;
      data_size       = (FT_ULong)( count + 1 ) * offsize;

      if ( ALLOC_ARRAY( index->offsets, count + 1, FT_ULong ) ||
           ACCESS_Frame( data_size )                          )
        goto Exit;

      poff = index->offsets;
      p    = (FT_Byte*)stream->cursor;

      for ( ; (FT_Short)count >= 0; count-- )
      {
        poff[0] = cff_get_offset( p, offsize );
        poff++;
        p += offsize;
      }

      FORGET_Frame();

      index->data_offset = FILE_Pos();
      data_size          = poff[-1] - 1;

      if ( load )
      {
        /* load the data */
        if ( EXTRACT_Frame( data_size, index->bytes ) )
          goto Exit;
      }
      else
      {
        /* skip the data */
        if ( FILE_Skip( data_size ) )
          goto Exit;
      }
    }

  Exit:
    if ( error )
      FREE( index->offsets );

    return error;
  }


  static
  void  cff_done_index( CFF_Index*  index )
  {
    if ( index->stream )
    {
      FT_Stream  stream = index->stream;
      FT_Memory  memory = stream->memory;


      if ( index->bytes )
        RELEASE_Frame( index->bytes );

      FREE( index->offsets );
      MEM_Set( index, 0, sizeof ( *index ) );
    }
  }


  static
  FT_Error  cff_explicit_index( CFF_Index*  index,
                                FT_Byte***  table )
  {
    FT_Error   error  = 0;
    FT_Memory  memory = index->stream->memory;
    FT_UInt    n, offset, old_offset;
    FT_Byte**  t;


    *table = 0;

    if ( index->count > 0 && !ALLOC_ARRAY( t, index->count + 1, FT_Byte* ) )
    {
      old_offset = 1;
      for ( n = 0; n <= index->count; n++ )
      {
        offset = index->offsets[n];
        if ( !offset )
          offset = old_offset;

        t[n] = index->bytes + offset - 1;

        old_offset = offset;
      }
      *table = t;
    }

    return error;
  }


  FT_LOCAL_DEF
  FT_Error  CFF_Access_Element( CFF_Index*  index,
                                FT_UInt     element,
                                FT_Byte**   pbytes,
                                FT_ULong*   pbyte_len )
  {
    FT_Error  error = 0;


    if ( index && index->count > element )
    {
      /* compute start and end offsets */
      FT_ULong  off1, off2 = 0;


      off1 = index->offsets[element];
      if ( off1 )
      {
        do
        {
          element++;
          off2 = index->offsets[element];

        } while ( off2 == 0 && element < index->count );

        if ( !off2 )
          off1 = 0;
      }

      /* access element */
      if ( off1 )
      {
        *pbyte_len = off2 - off1;

        if ( index->bytes )
        {
          /* this index was completely loaded in memory, that's easy */
          *pbytes = index->bytes + off1 - 1;
        }
        else
        {
          /* this index is still on disk/file, access it through a frame */
          FT_Stream  stream = index->stream;


          if ( FILE_Seek( index->data_offset + off1 - 1 ) ||
               EXTRACT_Frame( off2 - off1, *pbytes )      )
            goto Exit;
        }
      }
      else
      {
        /* empty index element */
        *pbytes    = 0;
        *pbyte_len = 0;
      }
    }
    else
      error = T2_Err_Invalid_Argument;

  Exit:
    return error;
  }


  FT_LOCAL_DEF
  void  CFF_Forget_Element( CFF_Index*  index,
                            FT_Byte**   pbytes )
  {
    if ( index->bytes == 0 )
    {
      FT_Stream  stream = index->stream;


      RELEASE_Frame( *pbytes );
    }
  }


  FT_LOCAL_DEF
  FT_String*  CFF_Get_Name( CFF_Index*  index,
                            FT_UInt     element )
  {
    FT_Memory   memory = index->stream->memory;
    FT_Byte*    bytes;
    FT_ULong    byte_len;
    FT_Error    error;
    FT_String*  name = 0;


    error = CFF_Access_Element( index, element, &bytes, &byte_len );
    if ( error )
      goto Exit;

    if ( !ALLOC( name, byte_len + 1 ) )
    {
      MEM_Copy( name, bytes, byte_len );
      name[byte_len] = 0;
    }
    CFF_Forget_Element( index, &bytes );

  Exit:
    return name;
  }


  FT_LOCAL_DEF
  FT_String*  CFF_Get_String( CFF_Index*          index,
                              FT_UInt             sid,
                              PSNames_Interface*  interface )
  {
    /* if it is not a standard string, return it */
    if ( sid > 390 )
      return CFF_Get_Name( index, sid - 391 );

    /* that's a standard string, fetch a copy from the PSName module */
    {
      FT_String*   name       = 0;
      const char*  adobe_name = interface->adobe_std_strings( sid );
      FT_UInt      len;


      if ( adobe_name )
      {
        FT_Memory memory = index->stream->memory;
        FT_Error  error;


        len = (FT_UInt)strlen( adobe_name );
        if ( !ALLOC( name, len + 1 ) )
        {
          MEM_Copy( name, adobe_name, len );
          name[len] = 0;
        }
      }

      return name;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***   FD Select table support                                         ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/


  static
  void  CFF_Done_FD_Select( CFF_FD_Select*  select,
                            FT_Stream       stream )
  {
    if ( select->data )
      RELEASE_Frame( select->data );

    select->data_size   = 0;
    select->format      = 0;
    select->range_count = 0;
  }


  static
  FT_Error CFF_Load_FD_Select( CFF_FD_Select*  select,
                               FT_UInt         num_glyphs,
                               FT_Stream       stream,
                               FT_ULong        offset )
  {
    FT_Error  error;
    FT_Byte   format;
    FT_UInt   num_ranges;


    /* read format */
    if ( FILE_Seek( offset ) || READ_Byte( format ) )
      goto Exit;

    select->format      = format;
    select->cache_count = 0;   /* clear cache */

    switch ( format )
    {
    case 0:     /* format 0, that's simple */
      select->data_size = num_glyphs;
      goto Load_Data;

    case 3:     /* format 3, a tad more complex */
      if ( READ_UShort( num_ranges ) )
        goto Exit;

      select->data_size = num_ranges * 3 + 2;

    Load_Data:
      if ( EXTRACT_Frame( select->data_size, select->data ) )
        goto Exit;
      break;

    default:    /* hmm... that's wrong */
      error = T2_Err_Invalid_File_Format;
    }

  Exit:
    return error;
  }


  FT_LOCAL_DEF
  FT_Byte  CFF_Get_FD( CFF_FD_Select*  select,
                       FT_UInt         glyph_index )
  {
    FT_Byte  fd = 0;


    switch ( select->format )
    {
    case 0:
      fd = select->data[glyph_index];
      break;

    case 3:
      /* first, compare to cache */
      if ( (FT_UInt)(glyph_index-select->cache_first) < select->cache_count )
      {
        fd = select->cache_fd;
        break;
      }

      /* then, lookup the ranges array */
      {
        FT_Byte*  p       = select->data;
        FT_Byte*  p_limit = p + select->data_size;
        FT_Byte   fd2;
        FT_UInt   first, limit;


        first = NEXT_UShort( p );
        do
        {
          if ( glyph_index < first )
            break;

          fd2   = *p++;
          limit = NEXT_UShort( p );

          if ( glyph_index < limit )
          {
            fd = fd2;

            /* update cache */
            select->cache_first = first;
            select->cache_count = limit-first;
            select->cache_fd    = fd2;
            break;
          }
          first = limit;

        } while ( p < p_limit );
      }
      break;

    default:
      ;
    }

    return fd;
  }


  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***   CFF font support                                                ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/

  static
  void  CFF_Done_Encoding( CFF_Encoding*  encoding,
                           FT_Stream      stream )
  {
    FT_Memory  memory = stream->memory;


    FREE( encoding->bytes );
    encoding->format = 0;
    encoding->size   = 0;
    encoding->offset = 0;
  }


  static
  void  CFF_Done_Charset( CFF_Charset*  charset,
                          FT_Stream     stream )
  {
    FT_Memory  memory = stream->memory;


    FREE( charset->bytes );
    charset->format = 0;
    charset->offset = 0;
    charset->size   = 0;
  }


  static
  FT_Error  CFF_Load_Charset( CFF_Charset*   charset,
                              FT_UInt        num_glyphs,
                              FT_Stream      stream,
                              FT_ULong       base_offset,
                              FT_ULong       offset )
  {
    FT_Error   error  = 0;
    FT_Memory  memory = stream->memory;


    if ( offset > 2 )
    {
      FT_ULong  size = 1;


      charset->offset = base_offset + offset;

      /* we need to parse the table to determine its size */
      if ( FILE_Seek( charset->offset ) ||
           READ_Byte( charset->format ) )
        goto Exit;

      switch ( charset->format )
      {
      case 0:
        size += 2 * ( num_glyphs - 1 );
        break;

      case 1:
        {
          FT_UInt  glyph, nleft;


          num_glyphs--;
          while ( num_glyphs > 0 )
          {
            if ( READ_UShort( glyph ) ||
                 READ_Byte( nleft )   )
              goto Exit;

            size += 3;
            num_glyphs -= 1 + nleft;
          }
          break;
        }

      case 2:
        {
          FT_UInt  glyph, nleft;


          num_glyphs--;
          while ( num_glyphs > 0 )
          {
            if ( READ_UShort( glyph ) ||
                 READ_UShort( nleft ) )
              goto Exit;

            size += 4;
            num_glyphs -= 1 + nleft;
          }
          break;
        }

      default:
        FT_ERROR(( "CFF_Load_Charset: invalid table format!\n" ));
        error = FT_Err_Invalid_File_Format;
        goto Exit;
      }

      charset->size = size;

      if ( ALLOC( charset->bytes, size )     ||
           FILE_Seek( charset->offset  )     ||
           FILE_Read( charset->bytes, size ) )
        goto Exit;
    }
    else
      charset->offset = offset;

  Exit:
    return error;
  }


  static
  FT_Error  CFF_Load_Encoding( CFF_Encoding*  encoding,
                               FT_UInt        num_glyphs,
                               FT_Stream      stream,
                               FT_ULong       base_offset,
                               FT_ULong       offset )
  {
    FT_Error  error  = 0;
    FT_Memory memory = stream->memory;
    FT_UInt   count;

    FT_UNUSED( num_glyphs );


    if ( offset > 2 )
    {
      FT_ULong  size = 1;


      encoding->offset = base_offset + offset;

      /* we need to parse the table to determine its size */
      if ( FILE_Seek( encoding->offset ) ||
           READ_Byte( encoding->format ) ||
           READ_Byte( count )            )
        goto Exit;

      switch ( encoding->format & 0x7F )
      {
      case 0:
        size = 2 + count;
        break;

      case 1:
        size = 2 + count * 2;
        break;

      case 2:
        size = 2 + count * 3;
        break;

      default:
        FT_ERROR(( "CFF.Load_Encoding: invalid table format!\n" ));
        error = FT_Err_Invalid_File_Format;
        goto Exit;
      }

      if ( encoding->format & 0x80 )
      {
        /* count supplements */
        if ( FILE_Seek( encoding->offset + size ) ||
             READ_Byte( count )                   )
          goto Exit;
          
        size += 1 + 3 * count;
      }
      
      encoding->size = size;

      if ( ALLOC( encoding->bytes, size )     ||
           FILE_Seek( encoding->offset  )     ||
           FILE_Read( encoding->bytes, size ) )
        goto Exit;
    }
    else
      encoding->offset = offset;

  Exit:
    return error;
  }


  static
  FT_Error  CFF_Load_SubFont( CFF_SubFont*  font,
                              CFF_Index*    index,
                              FT_UInt       font_index,
                              FT_Stream     stream,
                              FT_ULong      base_offset )
  {
    FT_Error        error;
    CFF_Parser      parser;
    FT_Byte*        dict;
    FT_ULong        dict_len;
    CFF_Font_Dict*  top  = &font->font_dict;
    CFF_Private*    priv = &font->private_dict;


    CFF_Parser_Init( &parser, CFF_CODE_TOPDICT, &font->font_dict );

    /* set defaults */
    MEM_Set( top, 0, sizeof ( *top ) );

    top->underline_position  = -100;
    top->underline_thickness = 50;
    top->charstring_type     = 2;
    top->font_matrix.xx      = 0x10000L;
    top->font_matrix.yy      = 0x10000L;
    top->cid_count           = 8720;

    error = CFF_Access_Element( index, font_index, &dict, &dict_len ) ||
            CFF_Parser_Run( &parser, dict, dict + dict_len );

    CFF_Forget_Element( index, &dict );

    if ( error )
      goto Exit;

    /* if it is a CID font, we stop there */
    if ( top->cid_registry )
      goto Exit;

    /* parse the private dictionary, if any */
    if ( top->private_offset && top->private_size )
    {
      /* set defaults */
      MEM_Set( priv, 0, sizeof ( *priv ) );

      priv->blue_shift       = 7;
      priv->blue_fuzz        = 1;
      priv->lenIV            = -1;
      priv->expansion_factor = (FT_Fixed)0.06 * 0x10000L;
      priv->blue_scale       = (FT_Fixed)0.039625 * 0x10000L;

      CFF_Parser_Init( &parser, CFF_CODE_PRIVATE, priv );

      if ( FILE_Seek( base_offset + font->font_dict.private_offset ) ||
           ACCESS_Frame( font->font_dict.private_size )              )
        goto Exit;

      error = CFF_Parser_Run( &parser,
                             (FT_Byte*)stream->cursor,
                             (FT_Byte*)stream->limit );
      FORGET_Frame();
      if ( error )
        goto Exit;
    }

    /* read the local subrs, if any */
    if ( priv->local_subrs_offset )
    {
      if ( FILE_Seek( base_offset + top->private_offset +
                      priv->local_subrs_offset ) )
        goto Exit;

      error = cff_new_index( &font->local_subrs_index, stream, 1 );
      if ( error )
        goto Exit;

      font->num_local_subrs = font->local_subrs_index.count;
      error = cff_explicit_index( &font->local_subrs_index,
                                     &font->local_subrs );
      if ( error )
        goto Exit;
    }

  Exit:
    return error;
  }


  static
  void  CFF_Done_SubFont( FT_Memory     memory,
                          CFF_SubFont*  subfont )
  {
    if ( subfont )
    {
      cff_done_index( &subfont->local_subrs_index );
      FREE( subfont->local_subrs );
    }
  }


  FT_LOCAL_DEF
  FT_Error  CFF_Load_Font( FT_Stream  stream,
                           FT_Int     face_index,
                           CFF_Font*  font )
  {
    static const FT_Frame_Field  cff_header_fields[] =
    {
#undef  FT_STRUCTURE
#define FT_STRUCTURE  CFF_Font

      FT_FRAME_START( 4 ),
        FT_FRAME_BYTE( version_major ),
        FT_FRAME_BYTE( version_minor ),
        FT_FRAME_BYTE( header_size ),
        FT_FRAME_BYTE( absolute_offsize ),
      FT_FRAME_END
    };

    FT_Error        error;
    FT_Memory       memory = stream->memory;
    FT_ULong        base_offset;
    CFF_Font_Dict*  dict;


    MEM_Set( font, 0, sizeof ( *font ) );

    font->stream = stream;
    font->memory = memory;
    dict         = &font->top_font.font_dict;
    base_offset  = FILE_Pos();

    /* read CFF font header */
    if ( READ_Fields( cff_header_fields, font ) )
      goto Exit;

    /* check format */
    if ( font->version_major   != 1 ||
         font->header_size      < 4 ||
         font->absolute_offsize > 4 )
    {
      FT_TRACE2(( "[not a CFF font header!]\n" ));
      error = FT_Err_Unknown_File_Format;
      goto Exit;
    }

    /* skip the rest of the header */
    if ( FILE_Skip( font->header_size - 4 ) )
      goto Exit;

    /* read the name, top dict, string and global subrs index */
    if ( FT_SET_ERROR( cff_new_index( &font->name_index,         stream, 0 )) ||
         FT_SET_ERROR( cff_new_index( &font->font_dict_index,    stream, 0 )) ||
         FT_SET_ERROR( cff_new_index( &font->string_index,       stream, 0 )) ||
         FT_SET_ERROR( cff_new_index( &font->global_subrs_index, stream, 1 )) )
      goto Exit;

    /* well, we don't really forget the `disabled' fonts... */
    font->num_faces = font->name_index.count;
    if ( face_index >= (FT_Int)font->num_faces )
    {
      FT_ERROR(( "CFF_Load_Font: incorrect face index = %d\n",
                 face_index ));
      error = T2_Err_Invalid_Argument;
    }

    /* in case of a font format check, simply exit now */
    if ( face_index < 0 )
      goto Exit;

    /* now, parse the top-level font dictionary */
    error = CFF_Load_SubFont( &font->top_font,
                              &font->font_dict_index,
                              face_index,
                              stream,
                              base_offset );
    if ( error )
      goto Exit;

    /* now, check for a CID font */
    if ( dict->cid_registry )
    {
      CFF_Index     fd_index;
      CFF_SubFont*  sub;
      FT_UInt       index;


      /* this is a CID-keyed font, we must now allocate a table of */
      /* sub-fonts, then load each of them separately              */
      if ( FILE_Seek( base_offset + dict->cid_fd_array_offset ) )
        goto Exit;

      error = cff_new_index( &fd_index, stream, 0 );
      if ( error )
        goto Exit;

      if ( fd_index.count > CFF_MAX_CID_FONTS )
      {
        FT_ERROR(( "CFF_Load_Font: FD array too large in CID font\n" ));
        goto Fail_CID;
      }

      /* allocate & read each font dict independently */
      font->num_subfonts = fd_index.count;
      if ( ALLOC_ARRAY( sub, fd_index.count, CFF_SubFont ) )
        goto Fail_CID;

      /* setup pointer table */
      for ( index = 0; index < fd_index.count; index++ )
        font->subfonts[index] = sub + index;

      /* now load each sub font independently */
      for ( index = 0; index < fd_index.count; index++ )
      {
        sub = font->subfonts[index];
        error = CFF_Load_SubFont( sub, &fd_index, index,
                                  stream, base_offset );
        if ( error )
          goto Fail_CID;
      }

      /* now load the FD Select array */
      error = CFF_Load_FD_Select( &font->fd_select,
                                  dict->cid_count,
                                  stream,
                                  base_offset + dict->cid_fd_select_offset );

    Fail_CID:
      cff_done_index( &fd_index );

      if ( error )
        goto Exit;
    }
    else
      font->num_subfonts = 0;

    /* read the charstrings index now */
    if ( dict->charstrings_offset == 0 )
    {
      FT_ERROR(( "CFF_Load_Font: no charstrings offset!\n" ));
      error = FT_Err_Unknown_File_Format;
      goto Exit;
    }

    if ( FILE_Seek( base_offset + dict->charstrings_offset ) )
      goto Exit;

    error = cff_new_index( &font->charstrings_index, stream, 0 );
    if ( error )
      goto Exit;

    /* explicit the global subrs */
    font->num_global_subrs = font->global_subrs_index.count;
    font->num_glyphs       = font->charstrings_index.count;

    error = cff_explicit_index( &font->global_subrs_index,
                                &font->global_subrs ) ;

    if ( error )
      goto Exit;

    /* read the Charset and Encoding tables when available */
    error = CFF_Load_Charset( &font->charset, font->num_glyphs, stream,
                              base_offset, dict->charset_offset );
    if ( error )
      goto Exit;
      
    error = CFF_Load_Encoding( &font->encoding, font->num_glyphs, stream,
                               base_offset, dict->encoding_offset );
    if ( error )
      goto Exit;
      
    /* get the font name */
    font->font_name = CFF_Get_Name( &font->name_index, face_index );

  Exit:
    return error;
  }


  FT_LOCAL_DEF
  void  CFF_Done_Font( CFF_Font*  font )
  {
    FT_Memory  memory = font->memory;
    FT_UInt    index;


    cff_done_index( &font->global_subrs_index );
    cff_done_index( &font->string_index );
    cff_done_index( &font->font_dict_index );
    cff_done_index( &font->name_index );
    cff_done_index( &font->charstrings_index );

    /* release font dictionaries */
    for ( index = 0; index < font->num_subfonts; index++ )
      CFF_Done_SubFont( memory, font->subfonts[index] );

    CFF_Done_Encoding( &font->encoding, font->stream );
    CFF_Done_Charset( &font->charset, font->stream );

    CFF_Done_SubFont( memory, &font->top_font );

    CFF_Done_FD_Select( &font->fd_select, font->stream );

    FREE( font->global_subrs );
    FREE( font->font_name );
  }


/* END */
