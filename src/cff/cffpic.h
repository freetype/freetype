/***************************************************************************/
/*                                                                         */
/*  cffpic.h                                                               */
/*                                                                         */
/*    The FreeType position independent code services for cff module.      */
/*                                                                         */
/*  Copyright 2009 by                                                      */
/*  Oran Agra and Mickey Gabel.                                            */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __CFFPIC_H__
#define __CFFPIC_H__

FT_BEGIN_HEADER

#include FT_INTERNAL_OBJECTS_H

/* CFF services list */
#ifndef FT_CONFIG_OPTION_NO_GLYPH_NAMES
#  define IF_GLYPH_NAMES(x)  x
#else
#  define IF_GLYPH_NAMES(x)  /* nothing */
#endif

#define  CFF_SERVICE_ITEMS \
  _FT_SERVICE_ITEM( XF86_NAME,            FT_XF86_FORMAT_CFF ) \
  _FT_SERVICE_ITEM( POSTSCRIPT_INFO,      &FT_CFF_SERVICE_PS_INFO_GET ) \
  _FT_SERVICE_ITEM( POSTSCRIPT_FONT_NAME, &FT_CFF_SERVICE_PS_NAME_GET ) \
  IF_GLYPH_NAMES( \
    _FT_SERVICE_ITEM( GLYPH_DICT,         &FT_CFF_SERVICE_GLYPH_DICT_GET ) \
  ) \
  _FT_SERVICE_ITEM( TT_CMAP, &FT_CFF_SERVICE_GET_CMAP_INFO_GET ) \
  _FT_SERVICE_ITEM( CID,     &FT_CFF_SERVICE_CID_INFO_GET ) \

#define  FT_SERVICE_NAME   Cff
#define  FT_SERVICE_ITEMS  CFF_SERVICE_ITEMS

#include FT_INTERNAL_SERVICE_LIST_DECLARE_H



#include FT_INTERNAL_PIC_H

#ifndef FT_CONFIG_OPTION_PIC
#define FT_CFF_CONST_(name_)    cff_##name_

#else /* FT_CONFIG_OPTION_PIC */

#include FT_SERVICE_GLYPH_DICT_H
#include "cffparse.h"
#include FT_SERVICE_POSTSCRIPT_INFO_H
#include FT_SERVICE_POSTSCRIPT_NAME_H
#include FT_SERVICE_TT_CMAP_H
#include FT_SERVICE_CID_H

/* count the number of items declared by cfftoken.h */
enum {
    CFF_FIELD_HANDLER_COUNT = 0
#define  CFF_FIELD(x,y,z)         +1
#define  CFF_FIELD_CALLBACK(x,y)  +1
#define  CFF_FIELD_DELTA(x,y,z)   +1
#include "cfftoken.h"
};

  typedef struct CffModulePIC_
  {
    FT_Service_PsInfoRec     cff_service_ps_info;
    FT_Service_GlyphDictRec  cff_service_glyph_dict;
    FT_Service_PsFontNameRec cff_service_ps_name;
    FT_Service_TTCMapsRec    cff_service_get_cmap_info;
    FT_Service_CIDRec        cff_service_cid_info;
    FT_CMap_ClassRec         cff_cmap_encoding_class_rec;
    FT_CMap_ClassRec         cff_cmap_unicode_class_rec;
    CFF_Field_Handler        cff_field_handlers[CFF_FIELD_HANDLER_COUNT+1];
    FT_ServiceItems_Cff      cff_services;
  } CffModulePIC;

#define CFF_GET_PIC(lib)                   ((CffModulePIC*)FT_LIBRARY_GET_PIC_DATA(lib,cff))
#define FT_CFF_CONST_(name_)               (CFF_GET_PIC(library)->cff_##name_)

#endif /* FT_CONFIG_OPTION_PIC */

#define FT_CFF_SERVICE_PS_INFO_GET         FT_CFF_CONST_(service_ps_info)
#define FT_CFF_SERVICE_GLYPH_DICT_GET      FT_CFF_CONST_(service_glyph_dict)
#define FT_CFF_SERVICE_PS_NAME_GET         FT_CFF_CONST_(service_ps_name)
#define FT_CFF_SERVICE_GET_CMAP_INFO_GET   FT_CFF_CONST_(service_get_cmap_info)
#define FT_CFF_SERVICE_CID_INFO_GET        FT_CFF_CONST_(service_cid_info)
#define FT_CFF_SERVICES_GET                FT_CFF_CONST_(services)
#define FT_CFF_CMAP_ENCODING_CLASS_REC_GET FT_CFF_CONST_(cmap_encoding_class_rec)
#define FT_CFF_CMAP_UNICODE_CLASS_REC_GET  FT_CFF_CONST_(cmap_unicode_class_rec)
#define FT_CFF_FIELD_HANDLERS_GET          FT_CFF_CONST_(field_handlers)


 /* */

FT_END_HEADER

#endif /* __CFFPIC_H__ */


/* END */
