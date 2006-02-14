/***************************************************************************/
/*                                                                         */
/*  ttmtx.c                                                                */
/*                                                                         */
/*    Load the metrics tables common to TTF and OTF fonts (body).          */
/*                                                                         */
/*  Copyright 2006 by                                                      */
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
#include FT_INTERNAL_STREAM_H
#include FT_TRUETYPE_TAGS_H
#include "ttmtx.h"

#include "sferrors.h"


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ttmtx


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    tt_face_load_hmtx                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Load the `hmtx' or `vmtx' table into a face object.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face     :: A handle to the target face object.                    */
  /*                                                                       */
  /*    stream   :: The input stream.                                      */
  /*                                                                       */
  /*    vertical :: A boolean flag.  If set, load `vmtx'.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
#ifdef FT_OPTIMIZE_MEMORY

  static FT_Error
  tt_face_load_hmtx( TT_Face    face,
                     FT_Stream  stream,
                     FT_Bool    vertical )
  {
    FT_Error   error;
    FT_ULong   table_size;
    FT_Byte**  ptable;
    FT_ULong*  ptable_size;
    
    
    FT_TRACE2(( "TT_Load_%s_Metrics: %08p\n", vertical ? "Vertical"
                                                       : "Horizontal",
                                              face ));

    if ( vertical )
    {
      ptable      = &face->vert_metrics;
      ptable_size = &face->vert_metrics_size;
      
      /* The table is optional, quit silently if it wasn't found.      */
      /*                                                               */
      /* XXX: Some fonts have a valid vertical header with a non-null  */
      /*      `number_of_VMetrics' fields, but no corresponding `vmtx' */
      /*      table to get the metrics from (e.g. mingliu).            */
      /*                                                               */
      /*      For safety, we set the field to 0!                       */
      /*                                                               */
      error = face->goto_table( face, TTAG_vmtx, stream, &table_size );
      if ( error )
      {
        /* Set number_Of_VMetrics to 0! */
        FT_TRACE2(( "  no vertical header in file.\n" ));
        error = SFNT_Err_Ok;
        goto Exit;
      }
    }
    else
    {
      ptable      = &face->horz_metrics;
      ptable_size = &face->horz_metrics_size;

      error = face->goto_table( face, TTAG_hmtx, stream, &table_size );
      if ( error )
      {
#ifdef FT_CONFIG_OPTION_INCREMENTAL
        /* If this is an incrementally loaded font and there are */
        /* overriding metrics, tolerate a missing `hmtx' table.  */
        if ( face->root.internal->incremental_interface          &&
             face->root.internal->incremental_interface->funcs->
               get_glyph_metrics                                 )
        {
          face->horizontal.number_Of_HMetrics = 0;
          error = SFNT_Err_Ok;
          goto Exit;
        }
#endif

        FT_ERROR(( " no horizontal metrics in file!\n" ));
        error = SFNT_Err_Hmtx_Table_Missing;
        goto Exit;
      }
    }
    
    if ( FT_FRAME_EXTRACT( table_size, *ptable ) )
      goto Exit;
      
    *ptable_size = table_size;
    
  Exit:
    return error;
  }

#else /* !OPTIMIZE_MEMORY */

  static FT_Error
  tt_face_load_hmtx( TT_Face    face,
                     FT_Stream  stream,
                     FT_Bool    vertical )
  {
    FT_Error   error;
    FT_Memory  memory = stream->memory;

    FT_ULong   table_len;
    FT_Long    num_shorts, num_longs, num_shorts_checked;

    TT_LongMetrics *   longs;
    TT_ShortMetrics**  shorts;


    FT_TRACE2(( "TT_Load_%s_Metrics: %08p\n", vertical ? "Vertical"
                                                       : "Horizontal",
                                              face ));

    if ( vertical )
    {
      /* The table is optional, quit silently if it wasn't found.      */
      /*                                                               */
      /* XXX: Some fonts have a valid vertical header with a non-null  */
      /*      `number_of_VMetrics' fields, but no corresponding `vmtx' */
      /*      table to get the metrics from (e.g. mingliu).            */
      /*                                                               */
      /*      For safety, we set the field to 0!                       */
      /*                                                               */
      error = face->goto_table( face, TTAG_vmtx, stream, &table_len );
      if ( error )
      {
        /* Set number_Of_VMetrics to 0! */
        FT_TRACE2(( "  no vertical header in file.\n" ));
        face->vertical.number_Of_VMetrics = 0;
        error = SFNT_Err_Ok;
        goto Exit;
      }

      num_longs = face->vertical.number_Of_VMetrics;
      longs     = (TT_LongMetrics *)&face->vertical.long_metrics;
      shorts    = (TT_ShortMetrics**)&face->vertical.short_metrics;
    }
    else
    {
      error = face->goto_table( face, TTAG_hmtx, stream, &table_len );
      if ( error )
      {

#ifdef FT_CONFIG_OPTION_INCREMENTAL
        /* If this is an incrementally loaded font and there are */
        /* overriding metrics, tolerate a missing `hmtx' table.  */
        if ( face->root.internal->incremental_interface          &&
             face->root.internal->incremental_interface->funcs->
               get_glyph_metrics                                 )
        {
          face->horizontal.number_Of_HMetrics = 0;
          error = SFNT_Err_Ok;
          goto Exit;
        }
#endif

        FT_ERROR(( " no horizontal metrics in file!\n" ));
        error = SFNT_Err_Hmtx_Table_Missing;
        goto Exit;
      }

      num_longs = face->horizontal.number_Of_HMetrics;
      longs     = (TT_LongMetrics *)&face->horizontal.long_metrics;
      shorts    = (TT_ShortMetrics**)&face->horizontal.short_metrics;
    }

    /* never trust derived values */

    num_shorts         = face->max_profile.numGlyphs - num_longs;
    num_shorts_checked = ( table_len - num_longs * 4L ) / 2;

    if ( num_shorts < 0 )
    {
      FT_ERROR(( "TT_Load_%s_Metrics: more metrics than glyphs!\n",
                 vertical ? "Vertical"
                          : "Horizontal" ));

      /* Adobe simply ignores this problem.  So we shall do the same. */
#if 0
      error = vertical ? SFNT_Err_Invalid_Vert_Metrics
                       : SFNT_Err_Invalid_Horiz_Metrics;
      goto Exit;
#else
      num_shorts = 0;
#endif
    }

    if ( FT_QNEW_ARRAY( *longs,  num_longs  ) ||
         FT_QNEW_ARRAY( *shorts, num_shorts ) )
      goto Exit;

    if ( FT_FRAME_ENTER( table_len ) )
      goto Exit;

    {
      TT_LongMetrics  cur   = *longs;
      TT_LongMetrics  limit = cur + num_longs;


      for ( ; cur < limit; cur++ )
      {
        cur->advance = FT_GET_USHORT();
        cur->bearing = FT_GET_SHORT();
      }
    }

    /* do we have an inconsistent number of metric values? */
    {
      TT_ShortMetrics*  cur   = *shorts;
      TT_ShortMetrics*  limit = cur +
                                FT_MIN( num_shorts, num_shorts_checked );


      for ( ; cur < limit; cur++ )
        *cur = FT_GET_SHORT();

      /* We fill up the missing left side bearings with the     */
      /* last valid value.  Since this will occur for buggy CJK */
      /* fonts usually only, nothing serious will happen.       */
      if ( num_shorts > num_shorts_checked && num_shorts_checked > 0 )
      {
        FT_Short  val = (*shorts)[num_shorts_checked - 1];


        limit = *shorts + num_shorts;
        for ( ; cur < limit; cur++ )
          *cur = val;
      }
    }

    FT_FRAME_EXIT();

    FT_TRACE2(( "loaded\n" ));

  Exit:
    return error;
  }

#endif /* !FT_OPTIMIZE_METRICS */


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    tt_face_load_hhea                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Load the `hhea' or 'vhea' table into a face object.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face     :: A handle to the target face object.                    */
  /*                                                                       */
  /*    stream   :: The input stream.                                      */
  /*                                                                       */
  /*    vertical :: A boolean flag.  If set, load `vhea'.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_LOCAL_DEF( FT_Error )
  tt_face_load_hhea( TT_Face    face,
                     FT_Stream  stream,
                     FT_Bool    vertical )
  {
    FT_Error        error;
    TT_HoriHeader*  header;

    const FT_Frame_Field  metrics_header_fields[] =
    {
#undef  FT_STRUCTURE
#define FT_STRUCTURE  TT_HoriHeader

      FT_FRAME_START( 36 ),
        FT_FRAME_ULONG ( Version ),
        FT_FRAME_SHORT ( Ascender ),
        FT_FRAME_SHORT ( Descender ),
        FT_FRAME_SHORT ( Line_Gap ),
        FT_FRAME_USHORT( advance_Width_Max ),
        FT_FRAME_SHORT ( min_Left_Side_Bearing ),
        FT_FRAME_SHORT ( min_Right_Side_Bearing ),
        FT_FRAME_SHORT ( xMax_Extent ),
        FT_FRAME_SHORT ( caret_Slope_Rise ),
        FT_FRAME_SHORT ( caret_Slope_Run ),
        FT_FRAME_SHORT ( caret_Offset ),
        FT_FRAME_SHORT ( Reserved[0] ),
        FT_FRAME_SHORT ( Reserved[1] ),
        FT_FRAME_SHORT ( Reserved[2] ),
        FT_FRAME_SHORT ( Reserved[3] ),
        FT_FRAME_SHORT ( metric_Data_Format ),
        FT_FRAME_USHORT( number_Of_HMetrics ),
      FT_FRAME_END
    };


    FT_TRACE2(( vertical ? "Vertical header " : "Horizontal header " ));

    if ( vertical )
    {
      face->vertical_info = 0;

      /* The vertical header table is optional, so return quietly if */
      /* we don't find it.                                           */
      error = face->goto_table( face, TTAG_vhea, stream, 0 );
      if ( error )
      {
        error = SFNT_Err_Ok;
        goto Exit;
      }

      face->vertical_info = 1;
      header = (TT_HoriHeader*)&face->vertical;
    }
    else
    {
      /* The horizontal header is mandatory for most fonts; return */
      /* an error if we don't find it.                             */
      error = face->goto_table( face, TTAG_hhea, stream, 0 );
      if ( error )
      {
        error = SFNT_Err_Horiz_Header_Missing;

        /* No `hhea' table necessary for SFNT Mac fonts. */
        if ( face->format_tag == TTAG_true )
        {
          FT_TRACE2(( "missing.  This is an SFNT Mac font.\n"));
          error = SFNT_Err_Ok;
        }

        goto Exit;
      }

      header = &face->horizontal;
    }

    if ( FT_STREAM_READ_FIELDS( metrics_header_fields, header ) )
      goto Exit;

    header->long_metrics  = NULL;
    header->short_metrics = NULL;

    FT_TRACE2(( "loaded\n" ));

    /* Now try to load the corresponding metrics */

    error = tt_face_load_hmtx( face, stream, vertical );

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    tt_face_get_metrics                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the horizontal or vertical metrics in font units for a     */
  /*    given glyph.  The metrics are the left side bearing (resp. top     */
  /*    side bearing) and advance width (resp. advance height).            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    header  :: A pointer to either the horizontal or vertical metrics  */
  /*               structure.                                              */
  /*                                                                       */
  /*    idx     :: The glyph index.                                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    bearing :: The bearing, either left side or top side.              */
  /*                                                                       */
  /*    advance :: The advance width resp. advance height.                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function will much probably move to another component in the  */
  /*    near future, but I haven't decided which yet.                      */
  /*                                                                       */
#ifdef FT_OPTIMIZE_MEMORY

  FT_LOCAL_DEF( FT_Error )
  tt_face_get_metrics( TT_Face     face,
                       FT_Bool     vertical,
                       FT_UInt     gindex,
                       FT_Short   *abearing,
                       FT_UShort  *aadvance )
  {
    TT_HoriHeader*  header;
    FT_Byte*        p;
    FT_Byte*        limit;
    FT_UShort       k;


    if ( vertical )
    {
      header = (TT_HoriHeader*)&face->vertical;
      p      = face->vert_metrics;
      limit  = p + face->vert_metrics_size;
    }
    else
    {
      header = &face->horizontal;
      p      = face->horz_metrics;
      limit  = p + face->horz_metrics_size;
    }

    k = header->number_Of_HMetrics;

    if ( k > 0 )
    {
      if ( gindex < (FT_UInt)k )
      {
        p += 4 * gindex;
        if ( p + 4 > limit )
          goto NoData;

        *aadvance = FT_NEXT_USHORT( p );
        *abearing = FT_NEXT_SHORT( p );
      }
      else
      {
        p += 4 * ( k - 1 );
        if ( p + 4 > limit )
          goto NoData;

        *aadvance = FT_NEXT_USHORT( p );
        p += 2 + 2 * ( gindex - k );
        if ( p + 2 > limit )
          *abearing = 0;
        else
          *abearing = FT_PEEK_SHORT( p );
      }
    }
    else
    {
    NoData:
      *abearing = 0;
      *aadvance = 0;
    }

    return SFNT_Err_Ok;
  }

#else /* !FT_OPTIMIZE_MEMORY */

  FT_LOCAL_DEF( FT_Error )
  tt_face_get_metrics( TT_Face     face,
                       FT_Bool     vertical,
                       FT_UInt     gindex,
                       FT_Short*   abearing,
                       FT_UShort*  aadvance )
  {
    TT_HoriHeader*  header = vertical ? (TT_HoriHeader*)&face->vertical
                                      :                 &face->horizontal;
    TT_LongMetrics  longs_m;
    FT_UShort       k      = header->number_Of_HMetrics;


    if ( k == 0 )
    {
      *abearing = *aadvance = 0;
      return SFNT_Err_Ok;
    }

    if ( gindex < (FT_UInt)k )
    {
      longs_m   = (TT_LongMetrics)header->long_metrics + gindex;
      *abearing = longs_m->bearing;
      *aadvance = longs_m->advance;
    }
    else
    {
      *abearing = ((TT_ShortMetrics*)header->short_metrics)[gindex - k];
      *aadvance = ((TT_LongMetrics)header->long_metrics)[k - 1].advance;
    }

    return SFNT_Err_Ok;
  }

#endif /* !FT_OPTIMIZE_MEMORY */


/* END */
/***************************************************************************/
/*                                                                         */
/*  ttmtx.c                                                                */
/*                                                                         */
/*    Load the metrics tables common to TTF and OTF fonts (body).          */
/*                                                                         */
/*  Copyright 2006 by                                                      */
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
#include FT_INTERNAL_STREAM_H
#include FT_TRUETYPE_TAGS_H
#include "ttmtx.h"

#include "sferrors.h"


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ttmtx


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    tt_face_load_hmtx                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Load the `hmtx' or `vmtx' table into a face object.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face     :: A handle to the target face object.                    */
  /*                                                                       */
  /*    stream   :: The input stream.                                      */
  /*                                                                       */
  /*    vertical :: A boolean flag.  If set, load `vmtx'.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
#ifdef FT_OPTIMIZE_MEMORY

  static FT_Error
  tt_face_load_hmtx( TT_Face    face,
                     FT_Stream  stream,
                     FT_Bool    vertical )
  {
    FT_Error   error;
    FT_ULong   table_size;
    FT_Byte**  ptable;
    FT_ULong*  ptable_size;
    
    
    FT_TRACE2(( "TT_Load_%s_Metrics: %08p\n", vertical ? "Vertical"
                                                       : "Horizontal",
                                              face ));

    if ( vertical )
    {
      ptable      = &face->vert_metrics;
      ptable_size = &face->vert_metrics_size;
      
      /* The table is optional, quit silently if it wasn't found.      */
      /*                                                               */
      /* XXX: Some fonts have a valid vertical header with a non-null  */
      /*      `number_of_VMetrics' fields, but no corresponding `vmtx' */
      /*      table to get the metrics from (e.g. mingliu).            */
      /*                                                               */
      /*      For safety, we set the field to 0!                       */
      /*                                                               */
      error = face->goto_table( face, TTAG_vmtx, stream, &table_size );
      if ( error )
      {
        /* Set number_Of_VMetrics to 0! */
        FT_TRACE2(( "  no vertical header in file.\n" ));
        error = SFNT_Err_Ok;
        goto Exit;
      }
    }
    else
    {
      ptable      = &face->horz_metrics;
      ptable_size = &face->horz_metrics_size;

      error = face->goto_table( face, TTAG_hmtx, stream, &table_size );
      if ( error )
      {
#ifdef FT_CONFIG_OPTION_INCREMENTAL
        /* If this is an incrementally loaded font and there are */
        /* overriding metrics, tolerate a missing `hmtx' table.  */
        if ( face->root.internal->incremental_interface          &&
             face->root.internal->incremental_interface->funcs->
               get_glyph_metrics                                 )
        {
          face->horizontal.number_Of_HMetrics = 0;
          error = SFNT_Err_Ok;
          goto Exit;
        }
#endif

        FT_ERROR(( " no horizontal metrics in file!\n" ));
        error = SFNT_Err_Hmtx_Table_Missing;
        goto Exit;
      }
    }
    
    if ( FT_FRAME_EXTRACT( table_size, *ptable ) )
      goto Exit;
      
    *ptable_size = table_size;
    
  Exit:
    return error;
  }

#else /* !OPTIMIZE_MEMORY */

  static FT_Error
  tt_face_load_hmtx( TT_Face    face,
                     FT_Stream  stream,
                     FT_Bool    vertical )
  {
    FT_Error   error;
    FT_Memory  memory = stream->memory;

    FT_ULong   table_len;
    FT_Long    num_shorts, num_longs, num_shorts_checked;

    TT_LongMetrics *   longs;
    TT_ShortMetrics**  shorts;


    FT_TRACE2(( "TT_Load_%s_Metrics: %08p\n", vertical ? "Vertical"
                                                       : "Horizontal",
                                              face ));

    if ( vertical )
    {
      /* The table is optional, quit silently if it wasn't found.      */
      /*                                                               */
      /* XXX: Some fonts have a valid vertical header with a non-null  */
      /*      `number_of_VMetrics' fields, but no corresponding `vmtx' */
      /*      table to get the metrics from (e.g. mingliu).            */
      /*                                                               */
      /*      For safety, we set the field to 0!                       */
      /*                                                               */
      error = face->goto_table( face, TTAG_vmtx, stream, &table_len );
      if ( error )
      {
        /* Set number_Of_VMetrics to 0! */
        FT_TRACE2(( "  no vertical header in file.\n" ));
        face->vertical.number_Of_VMetrics = 0;
        error = SFNT_Err_Ok;
        goto Exit;
      }

      num_longs = face->vertical.number_Of_VMetrics;
      longs     = (TT_LongMetrics *)&face->vertical.long_metrics;
      shorts    = (TT_ShortMetrics**)&face->vertical.short_metrics;
    }
    else
    {
      error = face->goto_table( face, TTAG_hmtx, stream, &table_len );
      if ( error )
      {

#ifdef FT_CONFIG_OPTION_INCREMENTAL
        /* If this is an incrementally loaded font and there are */
        /* overriding metrics, tolerate a missing `hmtx' table.  */
        if ( face->root.internal->incremental_interface          &&
             face->root.internal->incremental_interface->funcs->
               get_glyph_metrics                                 )
        {
          face->horizontal.number_Of_HMetrics = 0;
          error = SFNT_Err_Ok;
          goto Exit;
        }
#endif

        FT_ERROR(( " no horizontal metrics in file!\n" ));
        error = SFNT_Err_Hmtx_Table_Missing;
        goto Exit;
      }

      num_longs = face->horizontal.number_Of_HMetrics;
      longs     = (TT_LongMetrics *)&face->horizontal.long_metrics;
      shorts    = (TT_ShortMetrics**)&face->horizontal.short_metrics;
    }

    /* never trust derived values */

    num_shorts         = face->max_profile.numGlyphs - num_longs;
    num_shorts_checked = ( table_len - num_longs * 4L ) / 2;

    if ( num_shorts < 0 )
    {
      FT_ERROR(( "TT_Load_%s_Metrics: more metrics than glyphs!\n",
                 vertical ? "Vertical"
                          : "Horizontal" ));

      /* Adobe simply ignores this problem.  So we shall do the same. */
#if 0
      error = vertical ? SFNT_Err_Invalid_Vert_Metrics
                       : SFNT_Err_Invalid_Horiz_Metrics;
      goto Exit;
#else
      num_shorts = 0;
#endif
    }

    if ( FT_QNEW_ARRAY( *longs,  num_longs  ) ||
         FT_QNEW_ARRAY( *shorts, num_shorts ) )
      goto Exit;

    if ( FT_FRAME_ENTER( table_len ) )
      goto Exit;

    {
      TT_LongMetrics  cur   = *longs;
      TT_LongMetrics  limit = cur + num_longs;


      for ( ; cur < limit; cur++ )
      {
        cur->advance = FT_GET_USHORT();
        cur->bearing = FT_GET_SHORT();
      }
    }

    /* do we have an inconsistent number of metric values? */
    {
      TT_ShortMetrics*  cur   = *shorts;
      TT_ShortMetrics*  limit = cur +
                                FT_MIN( num_shorts, num_shorts_checked );


      for ( ; cur < limit; cur++ )
        *cur = FT_GET_SHORT();

      /* We fill up the missing left side bearings with the     */
      /* last valid value.  Since this will occur for buggy CJK */
      /* fonts usually only, nothing serious will happen.       */
      if ( num_shorts > num_shorts_checked && num_shorts_checked > 0 )
      {
        FT_Short  val = (*shorts)[num_shorts_checked - 1];


        limit = *shorts + num_shorts;
        for ( ; cur < limit; cur++ )
          *cur = val;
      }
    }

    FT_FRAME_EXIT();

    FT_TRACE2(( "loaded\n" ));

  Exit:
    return error;
  }

#endif /* !FT_OPTIMIZE_METRICS */


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    tt_face_load_hhea                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Load the `hhea' or 'vhea' table into a face object.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face     :: A handle to the target face object.                    */
  /*                                                                       */
  /*    stream   :: The input stream.                                      */
  /*                                                                       */
  /*    vertical :: A boolean flag.  If set, load `vhea'.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_LOCAL_DEF( FT_Error )
  tt_face_load_hhea( TT_Face    face,
                     FT_Stream  stream,
                     FT_Bool    vertical )
  {
    FT_Error        error;
    TT_HoriHeader*  header;

    const FT_Frame_Field  metrics_header_fields[] =
    {
#undef  FT_STRUCTURE
#define FT_STRUCTURE  TT_HoriHeader

      FT_FRAME_START( 36 ),
        FT_FRAME_ULONG ( Version ),
        FT_FRAME_SHORT ( Ascender ),
        FT_FRAME_SHORT ( Descender ),
        FT_FRAME_SHORT ( Line_Gap ),
        FT_FRAME_USHORT( advance_Width_Max ),
        FT_FRAME_SHORT ( min_Left_Side_Bearing ),
        FT_FRAME_SHORT ( min_Right_Side_Bearing ),
        FT_FRAME_SHORT ( xMax_Extent ),
        FT_FRAME_SHORT ( caret_Slope_Rise ),
        FT_FRAME_SHORT ( caret_Slope_Run ),
        FT_FRAME_SHORT ( caret_Offset ),
        FT_FRAME_SHORT ( Reserved[0] ),
        FT_FRAME_SHORT ( Reserved[1] ),
        FT_FRAME_SHORT ( Reserved[2] ),
        FT_FRAME_SHORT ( Reserved[3] ),
        FT_FRAME_SHORT ( metric_Data_Format ),
        FT_FRAME_USHORT( number_Of_HMetrics ),
      FT_FRAME_END
    };


    FT_TRACE2(( vertical ? "Vertical header " : "Horizontal header " ));

    if ( vertical )
    {
      face->vertical_info = 0;

      /* The vertical header table is optional, so return quietly if */
      /* we don't find it.                                           */
      error = face->goto_table( face, TTAG_vhea, stream, 0 );
      if ( error )
      {
        error = SFNT_Err_Ok;
        goto Exit;
      }

      face->vertical_info = 1;
      header = (TT_HoriHeader*)&face->vertical;
    }
    else
    {
      /* The horizontal header is mandatory for most fonts; return */
      /* an error if we don't find it.                             */
      error = face->goto_table( face, TTAG_hhea, stream, 0 );
      if ( error )
      {
        error = SFNT_Err_Horiz_Header_Missing;

        /* No `hhea' table necessary for SFNT Mac fonts. */
        if ( face->format_tag == TTAG_true )
        {
          FT_TRACE2(( "missing.  This is an SFNT Mac font.\n"));
          error = SFNT_Err_Ok;
        }

        goto Exit;
      }

      header = &face->horizontal;
    }

    if ( FT_STREAM_READ_FIELDS( metrics_header_fields, header ) )
      goto Exit;

    header->long_metrics  = NULL;
    header->short_metrics = NULL;

    FT_TRACE2(( "loaded\n" ));

    /* Now try to load the corresponding metrics */

    error = tt_face_load_hmtx( face, stream, vertical );

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    tt_face_get_metrics                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the horizontal or vertical metrics in font units for a     */
  /*    given glyph.  The metrics are the left side bearing (resp. top     */
  /*    side bearing) and advance width (resp. advance height).            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    header  :: A pointer to either the horizontal or vertical metrics  */
  /*               structure.                                              */
  /*                                                                       */
  /*    idx     :: The glyph index.                                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    bearing :: The bearing, either left side or top side.              */
  /*                                                                       */
  /*    advance :: The advance width resp. advance height.                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function will much probably move to another component in the  */
  /*    near future, but I haven't decided which yet.                      */
  /*                                                                       */
#ifdef FT_OPTIMIZE_MEMORY

  FT_LOCAL_DEF( FT_Error )
  tt_face_get_metrics( TT_Face     face,
                       FT_Bool     vertical,
                       FT_UInt     gindex,
                       FT_Short   *abearing,
                       FT_UShort  *aadvance )
  {
    TT_HoriHeader*  header;
    FT_Byte*        p;
    FT_Byte*        limit;
    FT_UShort       k;


    if ( vertical )
    {
      header = (TT_HoriHeader*)&face->vertical;
      p      = face->vert_metrics;
      limit  = p + face->vert_metrics_size;
    }
    else
    {
      header = &face->horizontal;
      p      = face->horz_metrics;
      limit  = p + face->horz_metrics_size;
    }

    k = header->number_Of_HMetrics;

    if ( k > 0 )
    {
      if ( gindex < (FT_UInt)k )
      {
        p += 4 * gindex;
        if ( p + 4 > limit )
          goto NoData;

        *aadvance = FT_NEXT_USHORT( p );
        *abearing = FT_NEXT_SHORT( p );
      }
      else
      {
        p += 4 * ( k - 1 );
        if ( p + 4 > limit )
          goto NoData;

        *aadvance = FT_NEXT_USHORT( p );
        p += 2 + 2 * ( gindex - k );
        if ( p + 2 > limit )
          *abearing = 0;
        else
          *abearing = FT_PEEK_SHORT( p );
      }
    }
    else
    {
    NoData:
      *abearing = 0;
      *aadvance = 0;
    }

    return SFNT_Err_Ok;
  }

#else /* !FT_OPTIMIZE_MEMORY */

  FT_LOCAL_DEF( FT_Error )
  tt_face_get_metrics( TT_Face     face,
                       FT_Bool     vertical,
                       FT_UInt     gindex,
                       FT_Short*   abearing,
                       FT_UShort*  aadvance )
  {
    TT_HoriHeader*  header = vertical ? (TT_HoriHeader*)&face->vertical
                                      :                 &face->horizontal;
    TT_LongMetrics  longs_m;
    FT_UShort       k      = header->number_Of_HMetrics;


    if ( k == 0 )
    {
      *abearing = *aadvance = 0;
      return SFNT_Err_Ok;
    }

    if ( gindex < (FT_UInt)k )
    {
      longs_m   = (TT_LongMetrics)header->long_metrics + gindex;
      *abearing = longs_m->bearing;
      *aadvance = longs_m->advance;
    }
    else
    {
      *abearing = ((TT_ShortMetrics*)header->short_metrics)[gindex - k];
      *aadvance = ((TT_LongMetrics)header->long_metrics)[k - 1].advance;
    }

    return SFNT_Err_Ok;
  }

#endif /* !FT_OPTIMIZE_MEMORY */


/* END */
