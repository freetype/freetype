/***************************************************************************/
/*                                                                         */
/*  cidriver.c                                                             */
/*                                                                         */
/*    CID driver interface (body).                                         */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
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
#include "cidriver.h"
#include "cidgload.h"
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_POSTSCRIPT_NAMES_H

#include "ciderrs.h"


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ciddriver


  static const char*
  cid_get_postscript_name( CID_Face  face )
  {
    const char*  result = face->cid.cid_font_name;


    if ( result && result[0] == '/' )
      result++;

    return result;
  }


  static FT_Module_Interface
  CID_Get_Interface( FT_Driver         driver,
                     const FT_String*  cid_interface )
  {
    FT_UNUSED( driver );
    FT_UNUSED( cid_interface );

    if ( ft_strcmp( (const char*)cid_interface, "postscript_name" ) == 0 )
      return (FT_Module_Interface)cid_get_postscript_name;

    return 0;
  }



  FT_CALLBACK_TABLE_DEF
  const FT_Driver_ClassRec  t1cid_driver_class =
  {
    /* first of all, the FT_Module_Class fields */
    {
      ft_module_font_driver       |
      ft_module_driver_scalable   |
      ft_module_driver_has_hinter ,

      sizeof( FT_DriverRec ),
      "t1cid",   /* module name           */
      0x10000L,  /* version 1.0 of driver */
      0x20000L,  /* requires FreeType 2.0 */

      0,

      (FT_Module_Constructor)CID_Driver_Init,
      (FT_Module_Destructor) CID_Driver_Done,
      (FT_Module_Requester)  CID_Get_Interface
    },

    /* then the other font drivers fields */
    sizeof( CID_FaceRec ),
    sizeof( CID_SizeRec ),
    sizeof( CID_GlyphSlotRec ),

    (FT_Face_InitFunc)        CID_Face_Init,
    (FT_Face_DoneFunc)        CID_Face_Done,

    (FT_Size_InitFunc)        CID_Size_Init,
    (FT_Size_DoneFunc)        CID_Size_Done,
    (FT_Slot_InitFunc)        CID_GlyphSlot_Init,
    (FT_Slot_DoneFunc)        CID_GlyphSlot_Done,

    (FT_Size_ResetPointsFunc) CID_Size_Reset,
    (FT_Size_ResetPixelsFunc) CID_Size_Reset,

    (FT_Slot_LoadFunc)        CID_Load_Glyph,

    (FT_Face_GetKerningFunc)  0,
    (FT_Face_AttachFunc)      0,

    (FT_Face_GetAdvancesFunc) 0,
  };


/* END */
