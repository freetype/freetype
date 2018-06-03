/***************************************************************************/
/*                                                                         */
/*  gfdrivr.h                                                              */
/*                                                                         */
/*    FreeType font driver for TeX's GF FONT files                         */
/*                                                                         */
/*  Copyright 1996-2018 by                                                 */
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
#include FT_INTERNAL_OBJECTS_H


#include "gf.h"
#include "gfdrivr.h"
#include "gferror.h"


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
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
    FT_UNUSED( init_data );

    cmap->bc     = 0;
    cmap->ec     = 255;

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
    //TO-DO
  }


  FT_CALLBACK_DEF( FT_Error )
  GF_Face_Init(  FT_Stream      stream,
                 FT_Face        gfface,         /* GF_Face */
                 FT_Int         face_index,
                 FT_Int         num_params,
                 FT_Parameter*  params )
  {
    //TO-DO
  }

  FT_CALLBACK_DEF( FT_Error )
  GF_Size_Select(  FT_Size   size,
                   FT_ULong  strike_index )
  {
    GF_Face        face   = (GF_Face)size->face;

    FT_UNUSED( strike_index );


    FT_Select_Metrics( size->face, 0 );

    size->metrics.ascender    =  /*  */  ;
    size->metrics.descender   =  /*  */  ;
    size->metrics.max_advance =  /*  */  ;

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
      if ( height == /*  */ )
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
    //TO-DO
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
