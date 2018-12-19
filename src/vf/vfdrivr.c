/****************************************************************************
 *
 * vfdrivr.c
 *
 *   FreeType font driver for TeX's VF FONT files.
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

#include FT_SERVICE_VF_H
#include FT_SERVICE_FONT_FORMAT_H


#include "vf.h"
#include "vfdrivr.h"
#include "vferror.h"


  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_vfdriver


  typedef struct  VF_CMapRec_
  {
    FT_CMapRec        cmap;
    FT_UInt32         bc;       /* Beginning Character */
    FT_UInt32         ec;       /* End Character */
  } VF_CMapRec, *VF_CMap;


  FT_CALLBACK_DEF( FT_Error )
  vf_cmap_init(  FT_CMap     vfcmap,
                 FT_Pointer  init_data )
  {
    VF_CMap  cmap = (VF_CMap)vfcmap;
    VF_Face  face = (VF_Face)FT_CMAP_FACE( cmap );
    FT_UNUSED( init_data );

    cmap->bc     = 0;
    cmap->ec     = 255;/* TO-DO */

    return FT_Err_Ok;
  }


  FT_CALLBACK_DEF( void )
  vf_cmap_done( FT_CMap  vfcmap )
  {
    VF_CMap  cmap = (VF_CMap)vfcmap;

    cmap->bc     =  0;
    cmap->ec     = -1;
  }


  FT_CALLBACK_DEF( FT_UInt )
  vf_cmap_char_index(  FT_CMap    vfcmap,
                       FT_UInt32  char_code )
  {
    FT_UInt  gindex = 0;
    VF_CMap  cmap   = (VF_CMap)vfcmap;

    char_code -= cmap->bc;

    if ( char_code < cmap->ec - cmap->bc + 1 )
      gindex = (FT_UInt)( char_code );

    return gindex;
  }

  FT_CALLBACK_DEF( FT_UInt )
  vf_cmap_char_next(  FT_CMap    vfcmap,
                       FT_UInt32  *achar_code )
  {
    VF_CMap    cmap   = (VF_CMap)vfcmap;
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
  const FT_CMap_ClassRec  vf_cmap_class =
  {
    sizeof ( VF_CMapRec ),
    vf_cmap_init,
    vf_cmap_done,
    vf_cmap_char_index,
    vf_cmap_char_next,

    NULL, NULL, NULL, NULL, NULL
  };


  FT_CALLBACK_DEF( void )
  VF_Face_Done( FT_Face        vfface )         /* VF_Face */
  {
    VF_Face    face = (VF_Face) vfface;
    FT_Memory  memory;

    if ( !face )
      return;

    memory = FT_FACE_MEMORY( face );

    FT_FREE( vfface->available_sizes );

    vf_free_font( face );
  }


  FT_CALLBACK_DEF( FT_Error )
  VF_Face_Init(   FT_Stream      stream,
                  FT_Face        vfface,         /* VF_Face */
                  FT_Int         face_index,
                  FT_Int         num_params,
                  FT_Parameter*  params )
  {
    VF_Face     face   = (VF_Face) vfface;
    FT_Error    error  = FT_Err_Ok;
    FT_Memory   memory = FT_FACE_MEMORY( face );

    TFM_Service tfm;

    FT_UNUSED( num_params );
    FT_UNUSED( params );


    face->tfm = FT_Get_Module_Interface( FT_FACE_LIBRARY( face ), "tfm" );

    tfm = (TFM_Service) face->tfm;
    if ( !tfm )
    {
      FT_ERROR(( "vf_Face_Init: cannot access `tfm' module\n" ));
      error = FT_THROW( Missing_Module );
      goto Exit;
    }

    FT_TRACE2(( "VF driver\n" ));

    /* load font */
    error = vf_read_info( stream, memory, &go );

    if ( FT_ERR_EQ( error, Unknown_File_Format ) )
    {
      FT_TRACE2(( "  not a vf file\n" ));
      goto Fail;
    }
    else if ( error )
      goto Exit;

    /* we have a vf font: let's construct the face object */

    /* sanity check */

    /* we now need to fill the root FT_Face fields */
    /* with relevant information                   */

    vfface->num_faces       = 1;
    vfface->face_index      = 0;
    vfface->face_flags     |= FT_FACE_FLAG_FIXED_SIZES |
                             FT_FACE_FLAG_HORIZONTAL ;
    /*
     * XXX: TO-DO: vfface->face_flags |= FT_FACE_FLAG_FIXED_WIDTH;
     */

    vfface->family_name     = NULL;
    vfface->num_glyphs      = (FT_Long) 999; /* TO-DO*/

    FT_TRACE4(( "  number of glyphs: allocated %d\n", vfface->num_glyphs ));

    if ( vfface->num_glyphs <= 0 )
    {
      FT_ERROR(( "vf_Face_Init: glyphs not allocated\n" ));
      error = FT_THROW( Invalid_File_Format );
      goto Exit;
    }

    vfface->num_fixed_sizes = 1;
    if ( FT_NEW_ARRAY( vfface->available_sizes, 1 ) )
      goto Exit;

    {
      FT_Bitmap_Size*  bsize = vfface->available_sizes;
      FT_UShort        x_res, y_res;

      /* Add Dummy values */
      /* To be modified   */
      bsize->height = (FT_Short) 999 ;
      bsize->width  = (FT_Short) 999 ;
      bsize->size   = (FT_Pos)   999 ;


      bsize->y_ppem = (FT_Pos) 999 ;
      bsize->x_ppem = (FT_Pos) 999 ;
    }

    /* set up charmap */
    {
      /* FT_Bool     unicode_charmap ; */

      /*
       * XXX: TO-DO
       * Currently the unicode_charmap is set to `0'
       * The functionality of extracting coding scheme
       * will be added.
      */
    }

    /* Charmaps */
    {
      FT_CharMapRec  charmap;
      FT_Bool        unicode_charmap = 0;

      charmap.face        = FT_FACE( face );
      charmap.encoding    = FT_ENCODING_NONE;
      charmap.platform_id = TT_PLATFORM_APPLE_UNICODE;
      charmap.encoding_id = TT_APPLE_ID_DEFAULT;

      if( unicode_charmap )
      {
        /* Unicode Charmap */
        charmap.encoding    = FT_ENCODING_UNICODE;
        charmap.platform_id = TT_PLATFORM_MICROSOFT;
        charmap.encoding_id = TT_MS_ID_UNICODE_CS;
      }

      error = FT_CMap_New( &vf_cmap_class, NULL, &charmap, NULL );

      if ( error )
        goto Exit;
    }

  Exit:
    return error;

  Fail:
    vf_Face_Done( vfface );
    return FT_THROW( Unknown_File_Format );
  }


  FT_CALLBACK_DEF( FT_Error )
  VF_Size_Select(  FT_Size   size,
                   FT_ULong  strike_index )
  {
    VF_Face     face  = (VF_Face)size->face;

    FT_UNUSED( strike_index );

    FT_Select_Metrics( size->face, 0 );

    /* Add Dummy values */
    /* To be modified   */

    size->metrics.ascender    = 999 * 64;
    size->metrics.descender   = 999 * 64;
    size->metrics.max_advance = 999 * 64;

    return FT_Err_Ok;
  }

  FT_CALLBACK_DEF( FT_Error )
  VF_Size_Request( FT_Size          size,
                   FT_Size_Request  req )
  {
    VF_Face           face    = (VF_Face) size->face;
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
      if ( height == 999 ) /* TO-DO */
        error = FT_Err_Ok;
      break;

    default:
      error = FT_THROW( Unimplemented_Feature );
      break;
    }

    if ( error )
      return error;
    else
      return VF_Size_Select( size, 0 );
  }



  FT_CALLBACK_DEF( FT_Error )
  VF_Glyph_Load(   FT_GlyphSlot  slot,
                   FT_Size       size,
                   FT_UInt       glyph_index,
                   FT_Int32      load_flags )
  {
    VF_Face      vf     = (VF_Face) FT_SIZE_FACE( size );
    FT_Face      face   = FT_FACE ( vf );
    FT_Error     error  = FT_Err_Ok;
    FT_Bitmap*   bitmap = &slot->bitmap;

    FT_UNUSED( load_flags );

    if ( !face )
    {
      error = FT_THROW( Invalid_Face_Handle );
      goto Exit;
    }


    /* slot, bitmap => freetype, bm => gflib */

    bitmap->rows       = 999; /* TO-DO */
    bitmap->width      = 999; /* TO-DO */
    bitmap->pixel_mode = FT_PIXEL_MODE_MONO;

    bitmap->pitch = (FT_Int) 999 ; /* TO-DO */

    /* note: we don't allocate a new array to hold the bitmap; */
    /*       we can simply point to it                         */
    /* ft_glyphslot_set_bitmap( slot, bm->bitmap );            */ /* TO-DO */

    slot->format      = FT_GLYPH_FORMAT_BITMAP;
    slot->bitmap_left = 999 ; /* TO-DO */
    slot->bitmap_top  = 999 ; /* TO-DO */

    slot->metrics.horiAdvance  = (FT_Pos) 999 * 64; /* TO-DO */
    slot->metrics.horiBearingX = (FT_Pos) 999 * 64; /* TO-DO */
    slot->metrics.horiBearingY = (FT_Pos) 999 * 64; /* TO-DO */
    slot->metrics.width        = (FT_Pos) ( 999 * 64 ); /* TO-DO */
    slot->metrics.height       = (FT_Pos) ( 999 * 64 ); /* TO-DO */

    ft_synthesize_vertical_metrics( &slot->metrics, 999 * 64 ); /* TO-DO */

  Exit:
    return error;
  }


 /*
  *
  * SERVICES LIST
  *
  */

  static const FT_ServiceDescRec  vf_services[] =
  {
    { FT_SERVICE_ID_VF,          NULL },
    { FT_SERVICE_ID_FONT_FORMAT, FT_FONT_FORMAT_VF },
    { NULL, NULL }
  };


  FT_CALLBACK_DEF( FT_Module_Interface )
  vf_driver_requester( FT_Module    module,
                       const char*  name )
  {
    FT_UNUSED( module );

    return ft_service_list_lookup( vf_services, name );
  }


  FT_CALLBACK_TABLE_DEF
  const FT_Driver_ClassRec  vf_driver_class =
  {
    {
      FT_MODULE_FONT_DRIVER         |
      FT_MODULE_DRIVER_NO_OUTLINES,
      sizeof ( FT_DriverRec ),

      "vf",
      0x10000L,
      0x20000L,

      NULL,                     /* module-specific interface */

      NULL,                     /* FT_Module_Constructor  module_init   */
      NULL,                     /* FT_Module_Destructor   module_done   */
      vf_driver_requester       /* FT_Module_Requester    get_interface */
    },

    sizeof ( VF_FaceRec ),
    sizeof ( FT_SizeRec ),
    sizeof ( FT_GlyphSlotRec ),

    VF_Face_Init,               /* FT_Face_InitFunc  init_face */
    VF_Face_Done,               /* FT_Face_DoneFunc  done_face */
    NULL,                       /* FT_Size_InitFunc  init_size */
    NULL,                       /* FT_Size_DoneFunc  done_size */
    NULL,                       /* FT_Slot_InitFunc  init_slot */
    NULL,                       /* FT_Slot_DoneFunc  done_slot */

    VF_Glyph_Load,              /* FT_Slot_LoadFunc  load_glyph */

    NULL,                       /* FT_Face_GetKerningFunc   get_kerning  */
    NULL,                       /* FT_Face_AttachFunc       attach_file  */
    NULL,                       /* FT_Face_GetAdvancesFunc  get_advances */

    VF_Size_Request,           /* FT_Size_RequestFunc  request_size */
    VF_Size_Select             /* FT_Size_SelectFunc   select_size  */
  };


/* END */
