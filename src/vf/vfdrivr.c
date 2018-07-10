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
    /* TO-DO */
  } VF_CMapRec, *VF_CMap;


  FT_CALLBACK_DEF( FT_Error )
  vf_cmap_init(  FT_CMap     vfcmap,
                 FT_Pointer  init_data )
  {
    /* TO-DO */
    return FT_Err_Ok;
  }


  FT_CALLBACK_DEF( void )
  vf_cmap_done( FT_CMap  vfcmap )
  {
    /* TO-DO */
  }


  FT_CALLBACK_DEF( FT_UInt )
  vf_cmap_char_index(  FT_CMap    vfcmap,
                       FT_UInt32  char_code )
  {
    /* TO-DO */
    return gindex;
  }

  FT_CALLBACK_DEF( FT_UInt )
  vf_cmap_char_next(  FT_CMap    vfcmap,
                       FT_UInt32  *achar_code )
  {
    /* To-DO */
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
    /* TO-DO */
  }


  FT_CALLBACK_DEF( FT_Error )
  VF_Face_Init(   FT_Stream      stream,
                  FT_Face        vfface,         /* VF_Face */
                  FT_Int         face_index,
                  FT_Int         num_params,
                  FT_Parameter*  params )
  {
    /* TO-DO */
  }

  FT_CALLBACK_DEF( FT_Error )
  VF_Size_Select(  FT_Size   size,
                   FT_ULong  strike_index )
  {
    /* TO-DO */
  }

  FT_CALLBACK_DEF( FT_Error )
  VF_Size_Request( FT_Size          size,
                   FT_Size_Request  req )
  {
    /* TO-DO */
  }



  FT_CALLBACK_DEF( FT_Error )
  VF_Glyph_Load(   FT_GlyphSlot  slot,
                   FT_Size       size,
                   FT_UInt       glyph_index,
                   FT_Int32      load_flags )
  {
    /* TO-DO */
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

      NULL,    									/* module-specific interface */

      NULL,                     /* FT_Module_Constructor  module_init   */
      NULL,                     /* FT_Module_Destructor   module_done   */
      NULL      								/* FT_Module_Requester    get_interface */
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
