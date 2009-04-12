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

#define  COUNT__(prefix_)  prefix_##__LINE__
#define  COUNT_(prefix_)   COUNT__(prefix_)

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
    FT_ServiceDescRec*       cff_services;
    FT_Service_PsInfoRec     cff_service_ps_info;
    FT_Service_GlyphDictRec  cff_service_glyph_dict;
    FT_Service_PsFontNameRec cff_service_ps_name;
    FT_Service_TTCMapsRec    cff_service_get_cmap_info;
    FT_Service_CIDRec        cff_service_cid_info;
    FT_CMap_ClassRec         cff_cmap_encoding_class_rec;
    FT_CMap_ClassRec         cff_cmap_unicode_class_rec;
    CFF_Field_Handler        cff_field_handlers[CFF_FIELD_HANDLER_COUNT+1];
  } CffModulePIC;

#define CFF_GET_PIC(lib)                   ((CffModulePIC*)((lib)->pic_table.cff))
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
