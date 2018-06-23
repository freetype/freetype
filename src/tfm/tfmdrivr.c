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
    /* TO-DO */
  } TFM_CMapRec, *TFM_CMap;


  FT_CALLBACK_DEF( FT_Error )
  tfm_cmap_init(  FT_CMap     tfmcmap,
                  FT_Pointer  init_data )
  {
    /* TO-DO */
  }


  FT_CALLBACK_DEF( void )
  tfm_cmap_done( FT_CMap  tfmcmap )
  {
    /* TO-DO */
  }


  FT_CALLBACK_DEF( FT_UInt )
  tfm_cmap_char_index(  FT_CMap    tfmcmap,
                        FT_UInt32  char_code )
  {
    /* TO-DO */
  }

  FT_CALLBACK_DEF( FT_UInt )
  tfm_cmap_char_next(  FT_CMap     tfmcmap,
                       FT_UInt32  *achar_code )
  {
    /* TO-DO */
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
    /* TO-DO */
  }


  FT_CALLBACK_DEF( FT_Error )
  TFM_Face_Init(  FT_Stream      stream,
                  FT_Face        tfmface,         /* TFM_Face */
                  FT_Int         face_index,
                  FT_Int         num_params,
                  FT_Parameter*  params )
  {
    /* TO-DO */
  }

  FT_CALLBACK_DEF( FT_Error )
  TFM_Size_Select(  FT_Size   size,
                    FT_ULong  strike_index )
  {
    /* TO-DO */
  }

  FT_CALLBACK_DEF( FT_Error )
  TFM_Size_Request(  FT_Size          size,
                     FT_Size_Request  req )
  {
    /* TO-DO */
  }



  FT_CALLBACK_DEF( FT_Error )
  TFM_Glyph_Load(  FT_GlyphSlot  slot,
                   FT_Size       size,
                   FT_UInt       glyph_index,
                   FT_Int32      load_flags )
  {
    /* TO-DO */
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
