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

#include <freetype/fterrors.h>
#include <freetype/tttags.h>
#include <t2load.h>
#include <t2parse.h>
#include <t2errors.h>

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ttload

 /* read a CFF offset from memory */
  LOCAL_FUNC
  FT_ULong  T2_Get_Offset( FT_Byte*  p,
                           FT_Byte   off_size )
  {
    FT_ULong  result;
    for ( result = 0; off_size > 0; off_size-- )
      result = (result <<= 8) | *p++;
    return result;
  }


#if 0
 /* read a CFF offset from a stream */
  LOCAL_FUNC
  FT_ULong  T2_Read_Offset( FT_Byte    off_size,
                            FT_Stream  stream )
  {
    FT_Byte   bytes[4];
    FT_Byte*  p;
    FT_ULong  result;

    if (off_size > 4)
      off_size = 4;

    /* first of all, read or access the bytes - this should really go     */
    /* in "src/base/ftstream.c", but there are great chances that it will */
    /* never be used elsewhere, so..                                      */
    if (stream->read)
    {
      p = bytes;
      if ( stream->read( stream, stream->pos, (char*)bytes, off_size ) != off_size )
        goto Fail;
    }
    else
    {
      p = (FT_Byte*)stream->base + stream->pos;
      if (p+off_size-1 >= (FT_Byte*)stream->limit)
        goto Fail;
    }

    result = 0;
    while (off_size > 0)
    {
      result = (result <<= 8) | *p++;
      off_size--;
    }
    stream->pos += off_size;
    return result;

  Fail:
    FT_ERROR(( "T2_Read_Offset:" ));
    FT_ERROR(( " invalid i/o, pos = 0x%lx, size = 0x%lx",
               stream->pos, stream->size ));
    return 0;
  }
#endif

 /* return the memory address of a CFF index's element, when the index */
 /* is already loaded in memory..                                      */

  LOCAL_FUNC
  FT_Error  T2_Access_Element( CFF_Index*   cff_index,
                               FT_UInt      element,
                               FT_Byte*    *pbytes,
                               FT_ULong    *pbyte_len )
  {
    FT_Error  error;

    if (cff_index && cff_index->bytes && element < (FT_UInt)cff_index->count)
    {
      FT_ULong  off1, off2;
      FT_Byte   offsize = cff_index->off_size;
      FT_Byte*  p       = cff_index->bytes + 3 + element*offsize;
      FT_Byte*  limit   = cff_index->bytes + cff_index->data_offset;

      /* read element offset */
      off1 = T2_Get_Offset(p,offsize);

      /* a value of 0 indicates no object !! */
      if (off1)
      {
        /* compute offset of next element - skip empty elements */
        do
        {
          p   += offsize;
          off2 = T2_Get_Offset(p,offsize);
        }
        while (off2 == 0 && p < limit);

        if (p >= limit)
          off1 = 0;
      }

      *pbytes    = 0;
      *pbyte_len = 0;
      if (off1)
      {
        *pbytes    = cff_index->bytes + cff_index->data_offset + off1 - 1;
        *pbyte_len = off2 - off1;
      }
      error = 0;
    }
    else
      error = FT_Err_Invalid_Argument;

    return error;
  }


  LOCAL_FUNC

  LOCAL_FUNC
  FT_Error  T2_Read_CFF_Index( CFF_Index*  index,
                               FT_Stream   stream )
  {
    FT_Error  error;
    FT_ULong  data_size;

    MEM_Set( index, 0, sizeof(*index) );
    index->file_offset = FILE_Pos();
    if ( !READ_UShort( index->count ) &&
         index->count > 0             )
    {
      FT_Byte*  p;
      FT_Byte   offsize;

      /* there is at least one element, read the offset size            */
      /* then access the offset table to compute the index's total size */
      if ( READ_Byte( offsize ) )
        goto Exit;

      index->off_size    = offsize;
      index->data_offset = ((FT_Long)index->count + 1)*offsize;

      if (ACCESS_Frame( index->data_offset ))
        goto Exit;

      /* now read element offset limit */
      p         = (FT_Byte*)stream->cursor + index->data_offset - offsize;
      data_size = T2_Get_Offset( p, offsize );

      FORGET_Frame();

      index->data_offset += 3;
      index->total_size   = index->data_offset + data_size;

      /* skip the data */
      (void)FILE_Skip( data_size );
    }
  Exit:
    return error;
  }


  LOCAL_FUNC
  FT_Error  T2_Load_CFF_Index( CFF_Index*  index,
                               FT_Stream   stream )
  {
    FT_Error   error;

    /* we begin by reading the index's data */
    error = T2_Read_CFF_Index( index, stream );
    if (!error && index->total_size > 0)
    {
      /* OK, read it from the file */
      if ( FILE_Seek( index->file_offset )                  ||
           EXTRACT_Frame( index->total_size, index->bytes ) )
        goto Exit;

      /* done !! */
    }
  Exit:
    return error;
  }


  LOCAL_FUNC
  void  T2_Done_CFF_Index( CFF_Index*  index,
                           FT_Stream   stream )
  {
    if (index->bytes)
      RELEASE_Frame( index->bytes );

    MEM_Set( index, 0, sizeof(*index) );
  }


  LOCAL_FUNC
  FT_Error  T2_Load_CFF_Font( FT_Stream   stream,
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

    MEM_Set( font, 0, sizeof(*font) );
    font->stream = stream;
    font->memory = stream->memory;

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

    /* read the name, top dict, strong and global subrs index */
    error = T2_Load_CFF_Index( &font->name_index, stream )     ||
            T2_Load_CFF_Index( &font->top_dict_index, stream ) ||
            T2_Read_CFF_Index( &font->string_index, stream )   ||
            T2_Load_CFF_Index( &font->global_subrs_index, stream );
    if (error) goto Exit;

    /* well, we don't really forget the "disable" fonts.. */
    font->num_faces = font->name_index.count;

  Exit:
    return error;
  }

  LOCAL_FUNC
  void  T2_Done_CFF_Font( CFF_Font*  font )
  {
    FT_Stream  stream = font->stream;

    T2_Done_CFF_Index( &font->global_subrs_index, stream );
    T2_Done_CFF_Index( &font->string_index, stream );
    T2_Done_CFF_Index( &font->top_dict_index, stream );
    T2_Done_CFF_Index( &font->name_index, stream );
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
