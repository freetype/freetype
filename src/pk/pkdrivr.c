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
    /* TO-DO */
  }


  FT_CALLBACK_DEF( FT_Error )
  PK_Face_Init(   FT_Stream      stream,
                  FT_Face        pkface,         /* PK_Face */
                  FT_Int         face_index,
                  FT_Int         num_params,
                  FT_Parameter*  params )
  {
    /* TO-DO */
  }

  FT_CALLBACK_DEF( FT_Error )
  PK_Size_Select(  FT_Size   size,
                   FT_ULong  strike_index )
  {
    /* TO-DO */
  }

  FT_CALLBACK_DEF( FT_Error )
  PK_Size_Request( FT_Size          size,
                   FT_Size_Request  req )
  {
    /* TO-DO */
  }



  FT_CALLBACK_DEF( FT_Error )
  PK_Glyph_Load(   FT_GlyphSlot  slot,
                   FT_Size       size,
                   FT_UInt       glyph_index,
                   FT_Int32      load_flags )
  {
    /* TO-DO */
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
      NULL      								/* FT_Module_Requester    get_interface */
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
    NULL,                       /* FT_Face_AttachFunc       attach_file  */
    NULL,                       /* FT_Face_GetAdvancesFunc  get_advances */

    PK_Size_Request,           /* FT_Size_RequestFunc  request_size */
    PK_Size_Select             /* FT_Size_SelectFunc   select_size  */
  };


/* END */
