/****************************************************************************
 *
 * gfdrivr.c
 *
 *   FreeType font driver for TeX's GF FONT files
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
#include FT_SERVICE_FONT_FORMAT_H

#include "gf.h"
#include "gfdrivr.h"
#include "gferror.h"


  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gfdriver


  typedef struct  GF_CMapRec_
  {
    FT_CMapRec        cmap;
    FT_UInt32         bc;       /* Beginning Character */
    FT_UInt32         ec;       /* End Character */
  } GF_CMapRec, *GF_CMap;


  FT_CALLBACK_DEF( FT_Error )
  gf_cmap_init(  FT_CMap     gfcmap,
                 FT_Pointer  init_data )
  {
    GF_CMap  cmap = (GF_CMap)gfcmap;
    GF_Face  face = (GF_Face)FT_CMAP_FACE( cmap );
    FT_UNUSED( init_data );

    cmap->bc     = face->gf_glyph->code_min;
    cmap->ec     = face->gf_glyph->code_max;

    return FT_Err_Ok;
  }


  FT_CALLBACK_DEF( void )
  gf_cmap_done( FT_CMap  gfcmap )
  {
    GF_CMap  cmap = (GF_CMap)gfcmap;

    cmap->bc     =  0;
    cmap->ec     = -1;

  }


  FT_CALLBACK_DEF( FT_UInt )
  gf_cmap_char_index(  FT_CMap    gfcmap,
                       FT_UInt32  char_code )
  {
    FT_UInt  gindex = 0;
    GF_CMap  cmap   = (GF_CMap)gfcmap;

    char_code -= cmap->bc;

    if ( char_code < cmap->ec - cmap->bc + 1 )
      gindex = (FT_UInt)( char_code );

    return gindex;
  }

  FT_CALLBACK_DEF( FT_UInt )
  gf_cmap_char_next(  FT_CMap     gfcmap,
                      FT_UInt32  *achar_code )
  {
    GF_CMap    cmap   = (GF_CMap)gfcmap;
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
  const FT_CMap_ClassRec  gf_cmap_class =
  {
    sizeof ( GF_CMapRec ),
    gf_cmap_init,
    gf_cmap_done,
    gf_cmap_char_index,
    gf_cmap_char_next,

    NULL, NULL, NULL, NULL, NULL
  };


  FT_CALLBACK_DEF( void )
  GF_Face_Done( FT_Face        gfface )         /* GF_Face */
  {
    GF_Face    face = (GF_Face)gfface;
    FT_Memory  memory= FT_FACE_MEMORY( gfface );


    if ( !face )
      return;

    memory = FT_FACE_MEMORY( face );

    gf_free_font( gfface, memory );
    /* FT_FREE(  ); */
  }


  FT_CALLBACK_DEF( FT_Error )
  GF_Face_Init(  FT_Stream      stream,
                 FT_Face        gfface,         /* GF_Face */
                 FT_Int         face_index,
                 FT_Int         num_params,
                 FT_Parameter*  params )
  {
    GF_Face     face   = (GF_Face)gfface;
    FT_Error    error;
    FT_Memory   memory = FT_FACE_MEMORY( face );
    GF_Glyph    go;
    int i,count;

    FT_UNUSED( num_params );
    FT_UNUSED( params );
    go=NULL;
    FT_TRACE2(( "GF driver\n" ));

    /* load font */
    error = gf_load_font( stream, memory, &go );
    if ( error )
      goto Exit;

    face->gf_glyph = go ;
    /* we now need to fill the root FT_Face fields */
    /* with relevant information                   */

    gfface->num_faces       = 1;
    gfface->face_index      = 0;
    gfface->face_flags     |= FT_FACE_FLAG_FIXED_SIZES |
                             FT_FACE_FLAG_HORIZONTAL ;
    /*
     * XXX: TO-DO: gfface->face_flags |= FT_FACE_FLAG_FIXED_WIDTH;
     * XXX: I have to check for this.
     */
    gfface->family_name     = NULL;
    count=0;
    for (i = 0; i < 256; i++)
    {
      if(go->bm_table[i].bitmap != NULL)
        count++;
    }
    gfface->num_glyphs      = (FT_Long)count;printf("count is %d", count);


    if ( FT_NEW_ARRAY( gfface->available_sizes, 1 ) )
      goto Exit;

    {
      FT_Bitmap_Size*  bsize = gfface->available_sizes;

      bsize->width  = (FT_Short) face->gf_glyph->font_bbx_w ;
      bsize->height = (FT_Short) face->gf_glyph->font_bbx_h ;
      bsize->size   = (FT_Short) face->gf_glyph->ds         ; /* Preliminary to be checked for 26.6 fractional points*/

      /*x_res =  ;  To be Checked for x_resolution and y_resolution
      y_res =  ;*/

      bsize->y_ppem = face->gf_glyph->font_bbx_yoff ;
      bsize->x_ppem = face->gf_glyph->font_bbx_xoff ;
    }

      /* Charmaps */

      {
        FT_CharMapRec  charmap;


        charmap.encoding    = FT_ENCODING_NONE;
        /* initial platform/encoding should indicate unset status? */
        charmap.platform_id = TT_PLATFORM_APPLE_UNICODE;  /*Preliminary */
        charmap.encoding_id = TT_APPLE_ID_DEFAULT;
        charmap.face        = FT_FACE( face );

        error = FT_CMap_New( &gf_cmap_class, NULL, &charmap, NULL );

        if ( error )
          goto Fail;
      }

  Fail:
    GF_Face_Done( gfface );

  Exit:
    return error;
  }

  FT_CALLBACK_DEF( FT_Error )
  GF_Size_Select(  FT_Size   size,
                   FT_ULong  strike_index )
  {
    GF_Face        face   = (GF_Face)size->face;

    FT_UNUSED( strike_index );


    FT_Select_Metrics( size->face, 0 );

    size->metrics.ascender    = face->gf_glyph->font_bbx_xoff    * 64;
    size->metrics.descender   = face->gf_glyph->font_bbx_yoff    * 64;
    size->metrics.max_advance = face->gf_glyph->font_bbx_w       * 64;

    return FT_Err_Ok;

  }

  FT_CALLBACK_DEF( FT_Error )
  GF_Size_Request(  FT_Size          size,
                    FT_Size_Request  req )
  {
    GF_Face           face    = (GF_Face)size->face;
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
      if ( height == face->gf_glyph->font_bbx_h )  /* Preliminary */
        error = FT_Err_Ok;
      break;

    default:
      error = FT_THROW( Unimplemented_Feature );
      break;
    }

    if ( error )
      return error;
    else
      return GF_Size_Select( size, 0 );
  }



  FT_CALLBACK_DEF( FT_Error )
  GF_Glyph_Load(  FT_GlyphSlot  slot,
                  FT_Size       size,
                  FT_UInt       glyph_index,
                  FT_Int32      load_flags )
  {
    GF_Face      gf     = (GF_Face)FT_SIZE_FACE( size );
    FT_Face      face   = FT_FACE( gf );
    FT_Error     error  = FT_Err_Ok;
    FT_Bitmap*   bitmap = &slot->bitmap;
    GF_BitmapRec bm ;
    GF_Glyph    go;

    go = gf->gf_glyph;

    FT_UNUSED( load_flags );


    if ( !face )
    {
      error = FT_THROW( Invalid_Face_Handle );
      goto Exit;
    }

    if ( glyph_index >= (FT_UInt)face->num_glyphs )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    FT_TRACE1(( "GF_Glyph_Load: glyph index %d\n", glyph_index ));

    if ( glyph_index < 0 )
      glyph_index = 0;

    if ((glyph_index < go->code_min) || (go->code_max < glyph_index))
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* slot, bitmap => freetype, glyph => gflib */
    bm = gf->gf_glyph->bm_table[glyph_index];

    bitmap->rows  = bm.mv_y   ; /* Prelimiary */
    bitmap->width = bm.mv_x   ; /* Prelimiary */
    bitmap->pitch = bm.raster ; /* Prelimiary */

    /* note: we don't allocate a new array to hold the bitmap; */
    /*       we can simply point to it                         */
    ft_glyphslot_set_bitmap( slot, bm.bitmap ); /* TO CHECK for column and row? like winfont.*/

    slot->format      = FT_GLYPH_FORMAT_BITMAP;
    slot->bitmap_left = bm.off_x ; /* Prelimiary */
    slot->bitmap_top  = bm.off_y ; /* Prelimiary */

    slot->metrics.horiAdvance  = (FT_Pos) bm.bbx_width - bm.off_x       ; /* Prelimiary */
    slot->metrics.horiBearingX = (FT_Pos) bm.off_x                      ; /* Prelimiary */
    slot->metrics.horiBearingY = (FT_Pos) bm.off_y                      ; /* Prelimiary */
    slot->metrics.width        = (FT_Pos) ( bitmap->width * 64 )        ; /* Prelimiary */
    slot->metrics.height       = (FT_Pos) ( bitmap->rows * 64 )         ; /* Prelimiary */

    ft_synthesize_vertical_metrics( &slot->metrics, bm.bbx_height * 64 );

  Exit:
    return error;
  }


   FT_CALLBACK_TABLE_DEF
  const FT_Driver_ClassRec  gf_driver_class =
  {
    {
      FT_MODULE_FONT_DRIVER         |
      FT_MODULE_DRIVER_NO_OUTLINES,
      sizeof ( FT_DriverRec ),

      "gf",
      0x10000L,
      0x20000L,

      NULL,    									/* module-specific interface */

      NULL,                     /* FT_Module_Constructor  module_init   */
      NULL,                     /* FT_Module_Destructor   module_done   */
      NULL      								/* FT_Module_Requester    get_interface */
    },

    sizeof ( GF_FaceRec ),
    sizeof ( FT_SizeRec ),
    sizeof ( FT_GlyphSlotRec ),

    GF_Face_Init,               /* FT_Face_InitFunc  init_face */
    GF_Face_Done,               /* FT_Face_DoneFunc  done_face */
    NULL,                       /* FT_Size_InitFunc  init_size */
    NULL,                       /* FT_Size_DoneFunc  done_size */
    NULL,                       /* FT_Slot_InitFunc  init_slot */
    NULL,                       /* FT_Slot_DoneFunc  done_slot */

    GF_Glyph_Load,              /* FT_Slot_LoadFunc  load_glyph */

    NULL,                       /* FT_Face_GetKerningFunc   get_kerning  */
    NULL,                       /* FT_Face_AttachFunc       attach_file  */
    NULL,                       /* FT_Face_GetAdvancesFunc  get_advances */

    GF_Size_Request,           /* FT_Size_RequestFunc  request_size */
    GF_Size_Select             /* FT_Size_SelectFunc   select_size  */
  };


/* END */
