/***************************************************************************/
/*                                                                         */
/*  ttpload.h                                                              */
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


#include <ftdebug.h>
#include <ftobjs.h>
#include <ftstream.h>

#include <ttpload.h>
#include <tttags.h>
#include <tterrors.h>

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ttload

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Locations                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the locations table.                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_Locations( TT_Face    face,
                               FT_Stream  stream )
  {
    TT_Error   error;
    FT_Memory  memory = stream->memory;
    TT_Short   LongOffsets;
    TT_ULong   table_len;

    FT_TRACE2(( "Locations " ));
    LongOffsets = face->header.Index_To_Loc_Format;

    error = face->goto_table( face, TTAG_loca, stream, &table_len );
    if (error)
    {
      error = TT_Err_Locations_Missing;
      goto Exit;
    }

    if ( LongOffsets != 0 )
    {
      face->num_locations = (TT_UShort)(table_len >> 2);

      FT_TRACE2(( "(32 bits offsets): %12d ", face->num_locations ));

      if ( ALLOC_ARRAY( face->glyph_locations,
                        face->num_locations,
                        TT_Long ) )
        goto Exit;

      if ( ACCESS_Frame( face->num_locations * 4L ) )
        goto Exit;

      {
        TT_Long*  loc   = face->glyph_locations;
        TT_Long*  limit = loc + face->num_locations;

        for ( ; loc < limit; loc++ )
          *loc = GET_Long();
      }

      FORGET_Frame();
    }
    else
    {
      face->num_locations = (TT_UShort)(table_len >> 1);

      FT_TRACE2(( "(16 bits offsets): %12d ",
                   face->num_locations ));

      if ( ALLOC_ARRAY( face->glyph_locations,
                        face->num_locations,
                        TT_Long ) )
        goto Exit;

      if ( ACCESS_Frame( face->num_locations * 2L ) )
        goto Exit;
      {
        TT_Long*  loc   = face->glyph_locations;
        TT_Long*  limit = loc + face->num_locations;

        for ( ; loc < limit; loc++ )
          *loc = (TT_Long)((TT_ULong)GET_UShort() * 2);
      }
      FORGET_Frame();
    }

    FT_TRACE2(( "loaded\n" ));

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_CVT                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the control value table into a face object.                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: A handle to the input stream.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_CVT( TT_Face    face,
                         FT_Stream  stream )
  {
    TT_Error   error;
    FT_Memory  memory = stream->memory;
    TT_ULong   table_len;

    FT_TRACE2(( "CVT " ));

    error = face->goto_table( face, TTAG_cvt, stream, &table_len );
    if (error)
    {
      FT_TRACE2(( "is missing!\n" ));

      face->cvt_size = 0;
      face->cvt      = NULL;
      error          = TT_Err_Ok;

      goto Exit;
    }

    face->cvt_size = table_len / 2;

    if ( ALLOC_ARRAY( face->cvt,
                      face->cvt_size,
                      TT_Short ) )
      goto Exit;

    if ( ACCESS_Frame( face->cvt_size * 2L ) )
      goto Exit;

    {
      TT_Short*  cur   = face->cvt;
      TT_Short*  limit = cur + face->cvt_size;

      for ( ; cur <  limit; cur++ )
        *cur = GET_Short();
    }

    FORGET_Frame();
    FT_TRACE2(( "loaded\n" ));

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Progams                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the font program and the cvt program.                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: A handle to the input stream.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_Programs( TT_Face    face,
                              FT_Stream  stream )
  {
    TT_Error   error;
    FT_Memory  memory = stream->memory;
    TT_ULong   table_len;


    FT_TRACE2(( "Font program " ));

    /* The font program is optional */
    error = face->goto_table( face, TTAG_fpgm, stream, &table_len );
    if ( error )
    {
      face->font_program      = NULL;
      face->font_program_size = 0;
      FT_TRACE2(( "is missing!\n" ));
    }
    else
    {
      face->font_program_size = table_len;

      if ( ALLOC( face->font_program,
                  face->font_program_size ) ||

           FILE_Read( (void*)face->font_program,
                      face->font_program_size )   )
        goto Exit;

      FT_TRACE2(( "loaded, %12d bytes\n", face->font_program_size ));
    }

    FT_TRACE2(( "Prep program " ));

    error = face->goto_table( face, TTAG_prep, stream, &table_len );
    if ( error )
    {
      face->cvt_program      = NULL;
      face->cvt_program_size = 0;
      error                  = TT_Err_Ok;

      FT_TRACE2(( "is missing!\n" ));
    }
    else
    {
      face->cvt_program_size = table_len;

      if ( ALLOC( face->cvt_program,
                  face->cvt_program_size )           ||

           FILE_Read( (void*)face->cvt_program,
                      face->cvt_program_size ) )
        return error;

      FT_TRACE2(( "loaded, %12d bytes\n", face->cvt_program_size ));
    }

  Exit:
    return error;
  }


/* END */
