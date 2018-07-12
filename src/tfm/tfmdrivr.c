/****************************************************************************
 *
 * tfmdrivr.c
 *
 *   FreeType font driver for TeX's TFM FONT files
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


#include "tfm.h"
#include "tfmdrivr.h"
#include "tfmerror.h"


  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_tfmdriver


  typedef struct  TFM_CMapRec_
  {
    FT_CMapRec        cmap;
    FT_UInt32         begin_char;       /* Beginning Character */
    FT_UInt32         end_char  ;       /* End Character */
  } TFM_CMapRec, *TFM_CMap;


  FT_CALLBACK_DEF( FT_Error )
  tfm_cmap_init(  FT_CMap     tfmcmap,
                  FT_Pointer  init_data )
  {
    TFM_CMap  cmap = (TFM_CMap)tfmcmap;
    TFM_Face  face = (TFM_Face)FT_CMAP_FACE( cmap );
    FT_UNUSED( init_data );

    /*cmap->begin_char     = ;
    cmap->end_char       = ;
    */
    return FT_Err_Ok;
  }


  FT_CALLBACK_DEF( void )
  tfm_cmap_done( FT_CMap  tfmcmap )
  {
    TFM_CMap  cmap = (TFM_CMap)tfmcmap;

    /*cmap->begin_char     = ;
    cmap->end_char       = ;
    */
  }


  FT_CALLBACK_DEF( FT_UInt )
  tfm_cmap_char_index(  FT_CMap    tfmcmap,
                        FT_UInt32  char_code )
  {
    FT_UInt  gindex = 0;
    TFM_CMap  cmap   = (TFM_CMap)tfmcmap;

    char_code -= cmap->begin_char;

    if ( char_code < cmap->end_char - cmap->begin_char + 1 )
      gindex = (FT_UInt)( char_code );

    return gindex;
  }

  FT_CALLBACK_DEF( FT_UInt )
  tfm_cmap_char_next(  FT_CMap     tfmcmap,
                       FT_UInt32  *achar_code )
  {
    TFM_CMap    cmap      = (TFM_CMap)tfmcmap;
    FT_UInt     gindex    = 0;
    FT_UInt32   result    = 0;
    FT_UInt32   char_code = *achar_code + 1;


    if ( char_code <= cmap->begin_char )
    {
      result = cmap->begin_char;
      gindex = 1;
    }
    else
    {
      char_code -= cmap->begin_char;
      if ( char_code < cmap->end_char - cmap->begin_char + 1 )
      {
        result = char_code;
        gindex = (FT_UInt)( char_code );
      }
    }

    *achar_code = result;
    return gindex;
  }


  static
  const FT_CMap_ClassRec  tfm_cmap_class =
  {
    sizeof ( TFM_CMapRec ),
    tfm_cmap_init,
    tfm_cmap_done,
    tfm_cmap_char_index,
    tfm_cmap_char_next,

    NULL, NULL, NULL, NULL, NULL
  };


  FT_CALLBACK_DEF( void )
  TFM_Face_Done( FT_Face        tfmface )         /* TFM_Face */
  {
    TFM_Face    face   = (TFM_Face)face;
    FT_Memory   memory;


    if ( !face )
      return;

    memory = FT_FACE_MEMORY( face );

    tfm_free_font( face );

    FT_FREE( tfmface->available_sizes );
  }


  FT_CALLBACK_DEF( FT_Error )
  TFM_Face_Init(  FT_Stream      stream,
                  FT_Face        tfmface,         /* TFM_Face */
                  FT_Int         face_index,
                  FT_Int         num_params,
                  FT_Parameter*  params )
  {
    TFM_Face    face   = (TFM_Face)tfmface;
    FT_Error    error  = FT_Err_Ok;
    FT_Memory   memory = FT_FACE_MEMORY( face );
    TFM_Glyph   tfm=NULL;
    FT_UInt16   i,count;

    FT_UNUSED( num_params );
    FT_UNUSED( params );


    FT_TRACE2(( "TFM driver\n" ));

    /* load font */
    error = tfm_load_font( stream, memory, &tfm );
    if ( FT_ERR_EQ( error, Unknown_File_Format ) )
    {
      FT_TRACE2(( "  not a TFM file\n" ));
      goto Fail;
    }
    else if ( error )
      goto Exit;

    /* we have a tfm font: let's construct the face object */
    face->tfm_glyph = tfm ;

    /* TFM cannot have multiple faces in a single font file.
     * XXX: non-zero face_index is already invalid argument, but
     *      Type1, Type42 driver has a convention to return
     *      an invalid argument error when the font could be
     *      opened by the specified driver.
     */
    if ( face_index > 0 && ( face_index & 0xFFFF ) > 0 )
    {
      FT_ERROR(( "TFM_Face_Init: invalid face index\n" ));
      TFM_Face_Done( tfmface );
      return FT_THROW( Invalid_Argument );
    }

    /* we now need to fill the root FT_Face fields */
    /* with relevant information                   */

    tfmface->num_faces       = 1;
    tfmface->face_index      = 0;
    tfmface->face_flags     |= FT_FACE_FLAG_FIXED_SIZES |
                             FT_FACE_FLAG_HORIZONTAL ;

    tfmface->family_name     = NULL;

    FT_TRACE4(( "  number of glyphs: allocated %d\n",tfmface->num_glyphs ));

    if ( tfmface->num_glyphs <= 0 )
    {
      FT_ERROR(( "TFM_Face_Init: glyphs not allocated\n" ));
      error = FT_THROW( Invalid_File_Format );
      goto Exit;
    }

    tfmface->num_fixed_sizes = 1;
    if ( FT_NEW_ARRAY( tfmface->available_sizes, 1 ) )
      goto Exit;

    {
      FT_Bitmap_Size*  bsize = tfmface->available_sizes;
      FT_UShort        x_res, y_res;

      bsize->height = (FT_Short)/* TO-DO */ ;
      bsize->width  = (FT_Short)/* TO-DO */ ;
      bsize->size   = (FT_Pos)  /* TO-DO */ ;

      x_res = /* TO-DO */;
      y_res = /* TO-DO */;

      bsize->y_ppem = (FT_Pos) /* TO-DO */;
      bsize->x_ppem = (FT_Pos) /* TO-DO */;
    }

    /* Charmaps */
    {
      FT_CharMapRec  charmap;

      /* Unicode Charmap */
      charmap.encoding    = FT_ENCODING_UNICODE;
      charmap.platform_id = TT_PLATFORM_MICROSOFT;
      charmap.encoding_id = TT_MS_ID_UNICODE_CS;
      charmap.face        = FT_FACE( face );

      error = FT_CMap_New( &tfm_cmap_class, NULL, &charmap, NULL );

      if ( error )
        goto Exit;
    }

  Exit:
    return error;

  Fail:
    TFM_Face_Done( tfmface );
    return FT_THROW( Unknown_File_Format );
  }

  FT_CALLBACK_DEF( FT_Error )
  TFM_Size_Select(  FT_Size   size,
                    FT_ULong  strike_index )
  {
    TFM_Face     face  = (TFM_Face)size->face;
    FT_UNUSED( strike_index );

    FT_Select_Metrics( size->face, 0 );

    size->metrics.ascender    = /* TO-DO */;
    size->metrics.descender   = /* TO-DO */;
    size->metrics.max_advance = /* TO-DO */;

    return FT_Err_Ok;
  }

  FT_CALLBACK_DEF( FT_Error )
  TFM_Size_Request(  FT_Size          size,
                     FT_Size_Request  req )
  {
    TFM_Face           face    = (TFM_Face)size->face;
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
      if ( height == /* TO-DO */ )
        error = FT_Err_Ok;
      break;

    default:
      error = FT_THROW( Unimplemented_Feature );
      break;
    }

    if ( error )
      return error;
    else
      return TFM_Size_Select( size, 0 );
  }



  FT_CALLBACK_DEF( FT_Error )
  TFM_Glyph_Load(  FT_GlyphSlot  slot,
                   FT_Size       size,
                   FT_UInt       glyph_index,
                   FT_Int32      load_flags )
  {
    TFM_Face      tfm     = (TFM_Face)FT_SIZE_FACE( size );
    FT_Face       face   = FT_FACE( tfm );
    FT_Error      error  = FT_Err_Ok;
    FT_Bitmap*    bitmap = &slot->bitmap;

    FT_UNUSED( load_flags );

    if ( !face )
    {
      error = FT_THROW( Invalid_Face_Handle );
      goto Exit;
    }

    if ( glyph_index >= (FT_UInt)( face->num_glyphs ) )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    FT_TRACE1(( "TFM_Glyph_Load: glyph index %d\n", glyph_index ));

    if ( glyph_index < 0 )
      glyph_index = 0;

    /* slot, bitmap => freetype, bm => tfmlib */

    bitmap->rows       = /* TO-DO */;
    bitmap->width      = /* TO-DO */;
    bitmap->pixel_mode = FT_PIXEL_MODE_MONO;

    bitmap->pitch = (int)/* TO-DO */;

    /* note: we don't allocate a new array to hold the bitmap; */
    /*       we can simply point to it                         */
    ft_glyphslot_set_bitmap( slot, /* TO-DO */);

    slot->format      = FT_GLYPH_FORMAT_BITMAP;
    slot->bitmap_left = /* TO-DO */ ;
    slot->bitmap_top  = /* TO-DO */ ;

    slot->metrics.horiAdvance  = (FT_Pos) /* TO-DO */ * 64;
    slot->metrics.horiBearingX = (FT_Pos) /* TO-DO */ * 64;
    slot->metrics.horiBearingY = (FT_Pos) /* TO-DO */ * 64;
    slot->metrics.width        = (FT_Pos) ( bitmap->width * 64 );
    slot->metrics.height       = (FT_Pos) ( bitmap->rows * 64 );

    ft_synthesize_vertical_metrics( &slot->metrics, /* TO-DO */ * 64 );

  Exit:
    return error;
  }


   FT_CALLBACK_TABLE_DEF
  const FT_Driver_ClassRec  tfm_driver_class =
  {
    {
      FT_MODULE_FONT_DRIVER         |
      FT_MODULE_DRIVER_NO_OUTLINES,
      sizeof ( FT_DriverRec ),

      "tfm",
      0x10000L,
      0x20000L,

      NULL,    									/* module-specific interface */

      NULL,                     /* FT_Module_Constructor  module_init   */
      NULL,                     /* FT_Module_Destructor   module_done   */
      NULL      								/* FT_Module_Requester    get_interface */
    },

    sizeof ( TFM_FaceRec ),
    sizeof ( FT_SizeRec ),
    sizeof ( FT_GlyphSlotRec ),

    TFM_Face_Init,               /* FT_Face_InitFunc  init_face */
    TFM_Face_Done,               /* FT_Face_DoneFunc  done_face */
    NULL,                       /* FT_Size_InitFunc  init_size */
    NULL,                       /* FT_Size_DoneFunc  done_size */
    NULL,                       /* FT_Slot_InitFunc  init_slot */
    NULL,                       /* FT_Slot_DoneFunc  done_slot */

    TFM_Glyph_Load,              /* FT_Slot_LoadFunc  load_glyph */

    NULL,                       /* FT_Face_GetKerningFunc   get_kerning  */
    NULL,                       /* FT_Face_AttachFunc       attach_file  */
    NULL,                       /* FT_Face_GetAdvancesFunc  get_advances */

    TFM_Size_Request,           /* FT_Size_RequestFunc  request_size */
    TFM_Size_Select             /* FT_Size_SelectFunc   select_size  */
  };


/* END */
