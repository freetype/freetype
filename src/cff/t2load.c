/***************************************************************************/
/*                                                                         */
/*  t2load.h                                                               */
/*                                                                         */
/*    TrueType glyph data/program tables loader (body).                    */
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
/***************************************************************************/


#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftstream.h>
#include <freetype/internal/psnames.h>

#include <freetype/fterrors.h>
#include <freetype/tttags.h>
#include <t2load.h>
#include <t2parse.h>
#include <freetype/internal/t2errors.h>

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ttload

 /* read a CFF offset from memory */
  static
  FT_ULong  t2_get_offset( FT_Byte*  p,
                           FT_Byte   off_size )
  {
    FT_ULong  result;
    for ( result = 0; off_size > 0; off_size-- )
      result = (result <<= 8) | *p++;
    return result;
  }



  static
  FT_Error  t2_new_cff_index( CFF_Index*  index,
                              FT_Stream   stream,
                              FT_Bool     load )
  {
    FT_Error  error;
    FT_Memory memory = stream->memory;
    FT_UShort count;

    MEM_Set( index, 0, sizeof(*index) );
    if ( !READ_UShort( count ) &&
          count > 0            )
    {
      FT_Byte*  p;
      FT_Byte   offsize;
      FT_ULong  data_size;
      FT_ULong* poff;

      /* there is at least one element, read the offset size            */
      /* then access the offset table to compute the index's total size */
      if ( READ_Byte( offsize ) )
        goto Exit;

      index->stream   = stream;
      index->count    = count;
      index->off_size = offsize;
      data_size       = (FT_ULong)(count+1) * offsize;

      if ( ALLOC_ARRAY( index->offsets, count+1, FT_ULong ) ||
           ACCESS_Frame( data_size ))
        goto Exit;

      poff = index->offsets;
      p    = (FT_Byte*)stream->cursor;
      for ( ; (FT_Short)count >= 0; count-- )
      {
        poff[0] = t2_get_offset( p, offsize );
        poff++;
        p += offsize;
      }

      FORGET_Frame();

      index->data_offset = FILE_Pos();
      data_size          = poff[-1]-1;

      if (load)
      {
        /* load the data */
        if ( EXTRACT_Frame( data_size, index->bytes ) )
          goto Exit;
      }
      else
      {
        /* skip the data */
        (void)FILE_Skip( data_size );
      }
    }
  Exit:
    if (error)
      FREE( index->offsets );

    return error;
  }


  static
  void  t2_done_cff_index( CFF_Index*  index )
  {
    if ( index->stream )
    {
      FT_Stream  stream = index->stream;
      FT_Memory  memory = stream->memory;
    
      if (index->bytes)
        RELEASE_Frame( index->bytes );

      FREE( index->offsets );
      MEM_Set( index, 0, sizeof(*index) );
    }
  }


  static
  FT_Error  t2_access_element( CFF_Index*   index,
                               FT_UInt      element,
                               FT_Byte*    *pbytes,
                               FT_ULong    *pbyte_len )
  {
    FT_Error  error = 0;

    if ( index && index->count > element )
    {
      /* compute start and end offsets */
      FT_ULong  off1, off2;
      
      off1 = index->offsets[element];
      if (off1)
      {
        do
        {
          element++;
          off2 = index->offsets[element];
        }
        while (off2 == 0 && element < index->count);
        if (!off2)
          off1 = 0;
      }
    
      /* access element */
      if (off1)
      {
        *pbyte_len = off2 - off1;
        
        if (index->bytes)
        {
          /* this index was completely loaded in memory, that's easy */
          *pbytes   = index->bytes + off1 - 1;
        }
        else
        {
          /* this index is still on disk/file, access it through a frame */
          FT_Stream  stream = index->stream;
          
          if ( FILE_Seek( index->data_offset + off1 - 1 ) ||
               EXTRACT_Frame( off2-off1, *pbytes )        )
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
      error = FT_Err_Invalid_Argument;
      
  Exit:
    return error;
  }


  static
  void  t2_forget_element( CFF_Index*  index,
                           FT_Byte*   *pbytes )
  {
    if (index->bytes == 0)
    {
      FT_Stream  stream = index->stream;
      RELEASE_Frame( *pbytes );
    }
  }                           


  static
  FT_String*  t2_get_name( CFF_Index*  index,
                           FT_UInt     element )
  {
    FT_Memory  memory = index->stream->memory;
    FT_Byte*   bytes;
    FT_ULong   byte_len;
    FT_Error   error;
    FT_String* name = 0;
    
    error = t2_access_element( index, element, &bytes, &byte_len );
    if (error) goto Exit;
    
    if ( !ALLOC( name, byte_len+1 ) )
    {
      MEM_Copy( name, bytes, byte_len );
      name[byte_len] = 0;
    }
    t2_forget_element( index, &bytes );
    
  Exit:
    return name;
  }                           


#if 0
  LOCAL_FUNC
  FT_String*  T2_Get_String( CFF_Index*          index,
                             FT_UInt             sid,
                             PSNames_Interface*  interface )
  {
    /* if it's not a standard string, return it */
    if ( sid > 390 )
      return t2_get_name( index, sid - 390 );
      
    /* that's a standard string, fetch a copy from the psnamed module */
    {
      FT_String*   name       = 0;
      const char*  adobe_name = interface->adobe_std_strings( sid );
      FT_UInt      len;
      
      if (adobe_name)
      {
        FT_Memory memory = index->stream->memory;
        FT_Error  error;
        
        len = (FT_UInt)strlen(adobe_name);
        if ( !ALLOC( name, len+1 ) )
        {
          MEM_Copy( name, adobe_name, len );
          name[len] = 0;
        }
      }
      return name;
    }
  }                             
#endif

  LOCAL_FUNC
  FT_Error  T2_Load_CFF_Font( FT_Stream   stream,
                              FT_Int      face_index,
                              CFF_Font*   font )
  {
    static const FT_Frame_Field  cff_header_fields[] = {
                     FT_FRAME_START(4),
                       FT_FRAME_BYTE( CFF_Font,  version_major ),
                       FT_FRAME_BYTE( CFF_Font,  version_minor ),
                       FT_FRAME_BYTE( CFF_Font,  header_size   ),
                       FT_FRAME_BYTE( CFF_Font,  absolute_offsize ),
                     FT_FRAME_END };

    FT_Error  error;
    FT_Memory memory = stream->memory;
    FT_ULong  base_offset;

    MEM_Set( font, 0, sizeof(*font) );
    font->stream = stream;
    font->memory = memory;
    base_offset  = FILE_Pos();

    /* read CFF font header */
    if ( READ_Fields( cff_header_fields, font ) )
      goto Exit;

    /* check format */
    if ( font->version_major   != 1 ||
         font->header_size      < 4 ||
         font->absolute_offsize > 4 )
    {
      FT_ERROR(( "incorrect CFF font header !!\n" ));
      error = FT_Err_Unknown_File_Format;
      goto Exit;
    }

    /* skip the rest of the header */
    (void)FILE_Skip( font->header_size - 4 );

    /* read the name, top dict, string and global subrs index */
    error = t2_new_cff_index( &font->name_index, stream, 0 )       ||
            t2_new_cff_index( &font->top_dict_index, stream, 0 )   ||
            t2_new_cff_index( &font->string_index, stream, 0 )     ||
            t2_new_cff_index( &font->global_subrs_index, stream, 1 );
    if (error) goto Exit;

    /* well, we don't really forget the "disabled" fonts.. */
    font->num_faces = font->name_index.count;
    if (face_index >= font->num_faces)
    {
      FT_ERROR(( "T2.Load_Font: incorrect face index = %d\n", face_index ));
      error = FT_Err_Invalid_Argument;
    }

    /* in case of a font format check, simply exit now */
    if (face_index >= 0)
    {
      T2_Parser  parser;
      FT_Byte*   dict;
      FT_ULong   dict_len;
      CFF_Index* index = &font->top_dict_index;

      /* parse the top-level font dictionary */      
      T2_Parser_Init( &parser, T2CODE_TOPDICT, &font->top_dict );
      
      error = t2_access_element( index, face_index, &dict, &dict_len ) ||
              T2_Parser_Run( &parser, dict, dict + dict_len );

      t2_forget_element( &font->top_dict_index, &dict );
      if (error) goto Exit;
      
      /* parse the private dictionary, if any */
      if (font->top_dict.private_offset && font->top_dict.private_size)
      {
        T2_Parser_Init( &parser, T2CODE_PRIVATE, &font->private_dict );
        
        if ( FILE_Seek( base_offset + font->top_dict.private_offset ) ||
             ACCESS_Frame( font->top_dict.private_size )               )
          goto Exit;

        error = T2_Parser_Run( &parser,
                               (FT_Byte*)stream->cursor,
                               (FT_Byte*)stream->limit );        
        FORGET_Frame();             
        if (error) goto Exit;
      }
      
      /* read the charstrings index now */
      if ( font->top_dict.charstrings_offset == 0 )
      {
        FT_ERROR(( "T2.New_CFF_Font: no charstrings offset !!\n" ));
        error = FT_Err_Unknown_File_Format;
        goto Exit;
      }
      
      if ( FILE_Seek( base_offset + font->top_dict.charstrings_offset ) )
        goto Exit;
        
      error = t2_new_cff_index( &font->charstrings_index, stream, 0 );
      if (error) goto Exit;
    }

    /* get the font name */      
    font->font_name = t2_get_name( &font->name_index, face_index );

  Exit:
    return error;
  }

  LOCAL_FUNC
  void  T2_Done_CFF_Font( CFF_Font*  font )
  {
    t2_done_cff_index( &font->global_subrs_index );
    t2_done_cff_index( &font->string_index );
    t2_done_cff_index( &font->top_dict_index );
    t2_done_cff_index( &font->name_index );
    t2_done_cff_index( &font->charstrings_index );
  }



 /***********************************************************************/
 /***********************************************************************/
 /***********************************************************************/
 /*****                                                             *****/
 /*****      TYPE 2 TABLES DECODING..                               *****/
 /*****                                                             *****/
 /***********************************************************************/
 /***********************************************************************/
 /***********************************************************************/

/* END */
