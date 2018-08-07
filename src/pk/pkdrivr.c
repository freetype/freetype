/****************************************************************************
 *
 * pkdrivr.c
 *
 *   FreeType font driver for TeX's PK FONT files.
 *
 * Copyright 1996-2018 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

#include <ft2build.h>

#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_OBJECTS_H
#include FT_TRUETYPE_IDS_H
#include FT_INTERNAL_TFM_H

#include FT_SERVICE_PK_H
#include FT_SERVICE_FONT_FORMAT_H

#include "pk.h"
#include "pkdrivr.h"
#include "pkerror.h"


  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_pkdriver


  typedef struct  PK_CMapRec_
  {
    FT_CMapRec        cmap;
    FT_UInt32         bc;       /* Beginning Character */
    FT_UInt32         ec;       /* End Character */
  } PK_CMapRec, *PK_CMap;


  FT_CALLBACK_DEF( FT_Error )
  pk_cmap_init(  FT_CMap     pkcmap,
                 FT_Pointer  init_data )
  {
    PK_CMap  cmap = (PK_CMap)pkcmap;
    PK_Face  face = (PK_Face)FT_CMAP_FACE( cmap );
    FT_UNUSED( init_data );

    cmap->bc     = face->pk_glyph->code_min;
    cmap->ec     = face->pk_glyph->code_max;

    return FT_Err_Ok;
  }


  FT_CALLBACK_DEF( void )
  pk_cmap_done( FT_CMap  pkcmap )
  {
    PK_CMap  cmap = (PK_CMap)pkcmap;

    cmap->bc     =  0;
    cmap->ec     = -1;
  }


  FT_CALLBACK_DEF( FT_UInt )
  pk_cmap_char_index(  FT_CMap    pkcmap,
                       FT_UInt32  char_code )
  {
    FT_UInt  gindex = 0;
    PK_CMap  cmap   = (PK_CMap)pkcmap;

    char_code -= cmap->bc;

    if ( char_code < cmap->ec - cmap->bc + 1 )
      gindex = (FT_UInt)( char_code );

    return gindex;
  }

  FT_CALLBACK_DEF( FT_UInt )
  pk_cmap_char_next(  FT_CMap    pkcmap,
                       FT_UInt32  *achar_code )
  {
    PK_CMap    cmap   = (PK_CMap)pkcmap;
    FT_UInt    gindex = 0;
    FT_UInt32  result = 0;
    FT_UInt32  char_code = *achar_code + 1;


    if ( char_code <= cmap->bc )
    {
      result = cmap->bc;
      gindex = 1;
    }
    else
    {
      char_code -= cmap->bc;
      if ( char_code < cmap->ec - cmap->bc + 1 )
      {
        result = char_code;
        gindex = (FT_UInt)( char_code );
      }
    }

    *achar_code = result;
    return gindex;
  }


  static
  const FT_CMap_ClassRec  pk_cmap_class =
  {
    sizeof ( PK_CMapRec ),
    pk_cmap_init,
    pk_cmap_done,
    pk_cmap_char_index,
    pk_cmap_char_next,

    NULL, NULL, NULL, NULL, NULL
  };


  FT_CALLBACK_DEF( void )
  PK_Face_Done( FT_Face        pkface )         /* PK_Face */
  {
    PK_Face    face   = (PK_Face)pkface;
    FT_Memory  memory;


    if ( !face )
      return;

    memory = FT_FACE_MEMORY( face );

    pk_free_font( face );

    FT_FREE( pkface->available_sizes );
  }


  FT_CALLBACK_DEF( FT_Error )
  PK_Face_Init(   FT_Stream      stream,
                  FT_Face        pkface,         /* PK_Face */
                  FT_Int         face_index,
                  FT_Int         num_params,
                  FT_Parameter*  params )
  {
    PK_Face     face   = (PK_Face)pkface;
    FT_Error    error  = FT_Err_Ok;
    FT_Memory   memory = FT_FACE_MEMORY( face );
    PK_Glyph    go=NULL;
    FT_UInt16   i,count;

    TFM_Service tfm;

    FT_UNUSED( num_params );
    FT_UNUSED( params );

    face->tfm = FT_Get_Module_Interface( FT_FACE_LIBRARY( face ),
                                           "tfm" );
    tfm = (TFM_Service)face->tfm;
    if ( !tfm )
    {
      FT_ERROR(( "GF_Face_Init: cannot access `tfm' module\n" ));
      error = FT_THROW( Missing_Module );
      goto Exit;
    }

    FT_TRACE2(( "PK driver\n" ));

    /* load font */
    error = pk_load_font( stream, memory, &go );
    if ( FT_ERR_EQ( error, Unknown_File_Format ) )
    {
      FT_TRACE2(( "  not a PK file\n" ));
      goto Fail;
    }
    else if ( error )
      goto Exit;

    /* we have a pk font: let's construct the face object */
    face->pk_glyph = go ;

    /* sanity check */
    if ( !face->pk_glyph->bm_table )
    {
      FT_TRACE2(( "glyph bitmaps not allocated\n" ));
      error = FT_THROW( Invalid_File_Format );
      goto Exit;
    }

    /* PK cannot have multiple faces in a single font file.
     * XXX: non-zero face_index is already invalid argument, but
     *      Type1, Type42 driver has a convention to return
     *      an invalid argument error when the font could be
     *      opened by the specified driver.
     */
    if ( face_index > 0 && ( face_index & 0xFFFF ) > 0 )
    {
      FT_ERROR(( "PK_Face_Init: invalid face index\n" ));
      PK_Face_Done( pkface );
      return FT_THROW( Invalid_Argument );
    }

    /* we now need to fill the root FT_Face fields */
    /* with relevant information                   */

    pkface->num_faces       = 1;
    pkface->face_index      = 0;
    pkface->face_flags     |= FT_FACE_FLAG_FIXED_SIZES |
                             FT_FACE_FLAG_HORIZONTAL ;
    /*
     * XXX: TO-DO: pkface->face_flags |= FT_FACE_FLAG_FIXED_WIDTH;
     * XXX: I have to check for this.
     */

    pkface->family_name     = NULL;
    count=0;
    for (i = 0; i < 256; i++)
    {
      if(go->bm_table[i].bitmap != NULL)
        count++;
    }
    pkface->num_glyphs      = (FT_Long)count;

    FT_TRACE4(( "  number of glyphs: allocated %d\n",pkface->num_glyphs ));

    if ( pkface->num_glyphs <= 0 )
    {
      FT_ERROR(( "PK_Face_Init: glyphs not allocated\n" ));
      error = FT_THROW( Invalid_File_Format );
      goto Exit;
    }

    pkface->num_fixed_sizes = 1;
    if ( FT_NEW_ARRAY( pkface->available_sizes, 1 ) )
      goto Exit;

    {
      FT_Bitmap_Size*  bsize = pkface->available_sizes;
      FT_UShort        x_res, y_res;

      bsize->height = (FT_Short) face->pk_glyph->font_bbx_h ;
      bsize->width  = (FT_Short) face->pk_glyph->font_bbx_w ;
      bsize->size   = (FT_Pos)   FT_MulDiv( FT_ABS( face->pk_glyph->ds ),
                                     64 * 7200,
                                     72270L );

      x_res = toint( face->pk_glyph->hppp * 72.27 );
      y_res = toint( face->pk_glyph->vppp * 72.27 );

      bsize->y_ppem = (FT_Pos) toint((face->pk_glyph->ds * y_res)/ 72.27) << 6 ;
      bsize->x_ppem = (FT_Pos)FT_MulDiv( bsize->y_ppem,
                                         x_res,
                                         y_res ); ;
    }

    /* Charmaps */
    {
      FT_CharMapRec  charmap;

      /* Unicode Charmap */
      charmap.encoding    = FT_ENCODING_UNICODE;
      charmap.platform_id = TT_PLATFORM_MICROSOFT;
      charmap.encoding_id = TT_MS_ID_UNICODE_CS;
      charmap.face        = FT_FACE( face );

      error = FT_CMap_New( &pk_cmap_class, NULL, &charmap, NULL );

      if ( error )
        goto Exit;
    }

    if ( go->code_max < go->code_min )
    {
      FT_TRACE2(( "invalid number of glyphs\n" ));
      error = FT_THROW( Invalid_File_Format );
      goto Exit;
    }

  Exit:
    return error;

  Fail:
    PK_Face_Done( pkface );
    return FT_THROW( Unknown_File_Format );
  }

  FT_CALLBACK_DEF( FT_Error )
  PK_Size_Select(  FT_Size   size,
                   FT_ULong  strike_index )
  {
    PK_Face     face  = (PK_Face)size->face;
    PK_Glyph    go    = face->pk_glyph;
    FT_UNUSED( strike_index );

    FT_Select_Metrics( size->face, 0 );

    size->metrics.ascender    = (go->font_bbx_h - go->font_bbx_yoff) * 64;
    size->metrics.descender   = -go->font_bbx_yoff * 64;
    size->metrics.max_advance = go->font_bbx_w * 64;

    return FT_Err_Ok;
  }

  FT_CALLBACK_DEF( FT_Error )
  PK_Size_Request( FT_Size          size,
                   FT_Size_Request  req )
  {
    PK_Face           face    = (PK_Face)size->face;
    FT_Bitmap_Size*   bsize   = size->face->available_sizes;
    FT_Error          error   = FT_ERR( Invalid_Pixel_Size );
    FT_Long           height;


    height = FT_REQUEST_HEIGHT( req );
    height = ( height + 32 ) >> 6;

    switch ( req->type )
    {
    case FT_SIZE_REQUEST_TYPE_NOMINAL:
      if ( height == ( ( bsize->y_ppem + 32 ) >> 6 ) )
        error = FT_Err_Ok;
      break;

    case FT_SIZE_REQUEST_TYPE_REAL_DIM:
      if ( height == face->pk_glyph->font_bbx_h )
        error = FT_Err_Ok;
      break;

    default:
      error = FT_THROW( Unimplemented_Feature );
      break;
    }

    if ( error )
      return error;
    else
      return PK_Size_Select( size, 0 );
  }



  FT_CALLBACK_DEF( FT_Error )
  PK_Glyph_Load(   FT_GlyphSlot  slot,
                   FT_Size       size,
                   FT_UInt       glyph_index,
                   FT_Int32      load_flags )
  {
    PK_Face      pk     = (PK_Face)FT_SIZE_FACE( size );
    FT_Face      face   = FT_FACE( pk );
    FT_Error     error  = FT_Err_Ok;
    FT_Bitmap*   bitmap = &slot->bitmap;
    PK_BitmapRec bm;
    PK_Glyph     go;

    go = pk->pk_glyph;

    FT_UNUSED( load_flags );

    if ( !face )
    {
      error = FT_THROW( Invalid_Face_Handle );
      goto Exit;
    }

    if ( !go                                         ||
         glyph_index >= (FT_UInt)( face->num_glyphs ) )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    FT_TRACE1(( "PK_Glyph_Load: glyph index %d\n", glyph_index ));

    if ( glyph_index < 0 )
      glyph_index = 0;

    if ((glyph_index < go->code_min) || (go->code_max < glyph_index))
    {
      FT_TRACE2(( "invalid glyph index\n" ));
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    if ( !go->bm_table )
    {
      FT_TRACE2(( "invalid bitmap table\n" ));
      error = FT_THROW( Invalid_File_Format );
      goto Exit;
    }

    /* slot, bitmap => freetype, bm => pklib */
    bm = pk->pk_glyph->bm_table[glyph_index];

    bitmap->rows       = bm.bbx_height;
    bitmap->width      = bm.bbx_width;
    bitmap->pixel_mode = FT_PIXEL_MODE_MONO;

    if ( !bm.raster )
    {
      FT_TRACE2(( "invalid bitmap width\n" ));
      error = FT_THROW( Invalid_File_Format );
      goto Exit;
    }

    bitmap->pitch = (int)bm.raster ;

    /* note: we don't allocate a new array to hold the bitmap; */
    /*       we can simply point to it                         */
    ft_glyphslot_set_bitmap( slot, bm.bitmap );

    slot->format      = FT_GLYPH_FORMAT_BITMAP;
    slot->bitmap_left = bm.off_x ;
    slot->bitmap_top  = bm.off_y ;

    slot->metrics.horiAdvance  = (FT_Pos) (bm.mv_x ) * 64;
    slot->metrics.horiBearingX = (FT_Pos) (bm.off_x ) * 64;
    slot->metrics.horiBearingY = (FT_Pos) (bm.bbx_height) * 64;
    slot->metrics.width        = (FT_Pos) ( bitmap->width * 64 );
    slot->metrics.height       = (FT_Pos) ( bitmap->rows * 64 );

    ft_synthesize_vertical_metrics( &slot->metrics, bm.bbx_height * 64 );

  Exit:
    return error;
  }

  FT_LOCAL_DEF( void )
  TFM_Done_Metrics( FT_Memory     memory,
                    TFM_FontInfo  fi )
  {
    FT_FREE(fi->width);
    FT_FREE(fi->height);
    FT_FREE(fi->depth);
    FT_FREE( fi );
  }

  /* parse a TFM metrics file */
  FT_LOCAL_DEF( FT_Error )
  TFM_Read_Metrics( FT_Face    pk_face,
                    FT_Stream  stream )
  {
    TFM_Service    tfm;
    FT_Memory      memory  = stream->memory;
    TFM_ParserRec  parser;
    TFM_FontInfo   fi      = NULL;
    FT_Error       error   = FT_ERR( Unknown_File_Format );
    PK_Face        face    = (PK_Face)pk_face;
    PK_Glyph       pk_glyph= face->pk_glyph;


    if ( face->tfm_data )
    {
      FT_TRACE1(( "TFM_Read_Metrics:"
                  " Freeing previously attached metrics data.\n" ));
      TFM_Done_Metrics( memory, (TFM_FontInfo)face->tfm_data );

      face->tfm_data = NULL;
    }

    if ( FT_NEW( fi ) )
      goto Exit;

    FT_TRACE4(( "TFM_Read_Metrics: Invoking TFM_Service.\n" ));

    tfm = (TFM_Service)face->tfm;
    if ( tfm->tfm_parser_funcs )
    {
      /* Initialise TFM Service */
      error = tfm->tfm_parser_funcs->init( &parser,
                       memory,
                       stream );

      if ( !error )
      {
        FT_TRACE4(( "TFM_Read_Metrics: Initialised tfm metric data.\n" ));
        parser.FontInfo  = fi;
        parser.user_data = pk_glyph;

        error = tfm->tfm_parser_funcs->parse_metrics( &parser );
        if( !error )
          FT_TRACE4(( "TFM_Read_Metrics: parsing TFM metric information done.\n" ));

        FT_TRACE6(( "TFM_Read_Metrics: TFM Metric Information:\n"
                    "                  Check Sum  : %ld\n"
                    "                  Design Size: %ld\n"
                    "                  Begin Char : %d\n"
                    "                  End Char   : %d\n"
                    "                  font_bbx_w : %d\n"
                    "                  font_bbx_h : %d\n"
                    "                  slant      : %d\n", parser.FontInfo->cs, parser.FontInfo->design_size, parser.FontInfo->begin_char,
                                                           parser.FontInfo->end_char, parser.FontInfo->font_bbx_w,
                                                           parser.FontInfo->font_bbx_h, parser.FontInfo->slant ));
        tfm->tfm_parser_funcs->done( &parser );
      }
    }

    if ( !error )
    {
      /* Modify PK_Glyph data according to TFM metric values */

      /*face->pk_glyph->font_bbx_w = fi->font_bbx_w;
      face->pk_glyph->font_bbx_h = fi->font_bbx_h;
      */

      face->tfm_data       = fi;
      fi                   = NULL;
    }

  Exit:
    if ( fi )
      TFM_Done_Metrics( memory, fi );

    return error;
  }

 /*
  *
  * SERVICES LIST
  *
  */

  static const FT_ServiceDescRec  pk_services[] =
  {
    { FT_SERVICE_ID_PK,          NULL },
    { FT_SERVICE_ID_FONT_FORMAT, FT_FONT_FORMAT_PK },
    { NULL, NULL }
  };


  FT_CALLBACK_DEF( FT_Module_Interface )
  pk_driver_requester( FT_Module    module,
                        const char*  name )
  {
    FT_UNUSED( module );

    return ft_service_list_lookup( pk_services, name );
  }


   FT_CALLBACK_TABLE_DEF
  const FT_Driver_ClassRec  pk_driver_class =
  {
    {
      FT_MODULE_FONT_DRIVER         |
      FT_MODULE_DRIVER_NO_OUTLINES,
      sizeof ( FT_DriverRec ),

      "pk",
      0x10000L,
      0x20000L,

      NULL,    									/* module-specific interface */

      NULL,                     /* FT_Module_Constructor  module_init   */
      NULL,                     /* FT_Module_Destructor   module_done   */
      pk_driver_requester       /* FT_Module_Requester    get_interface */
    },

    sizeof ( PK_FaceRec ),
    sizeof ( FT_SizeRec ),
    sizeof ( FT_GlyphSlotRec ),

    PK_Face_Init,               /* FT_Face_InitFunc  init_face */
    PK_Face_Done,               /* FT_Face_DoneFunc  done_face */
    NULL,                       /* FT_Size_InitFunc  init_size */
    NULL,                       /* FT_Size_DoneFunc  done_size */
    NULL,                       /* FT_Slot_InitFunc  init_slot */
    NULL,                       /* FT_Slot_DoneFunc  done_slot */

    PK_Glyph_Load,              /* FT_Slot_LoadFunc  load_glyph */

    NULL,                       /* FT_Face_GetKerningFunc   get_kerning  */
    TFM_Read_Metrics,           /* FT_Face_AttachFunc       attach_file  */
    NULL,                       /* FT_Face_GetAdvancesFunc  get_advances */

    PK_Size_Request,           /* FT_Size_RequestFunc  request_size */
    PK_Size_Select             /* FT_Size_SelectFunc   select_size  */
  };


/* END */
