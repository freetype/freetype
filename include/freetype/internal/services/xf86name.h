#ifndef __FT_SERVICE_XF86_NAME_H__
#define __FT_SERVICE_XF86_NAME_H__

#include FT_INTERNAL_SERVICE_H

FT_BEGIN_HEADER

 /*
  *  a trivial service used to return the name of a face's font driver,
  *  according to the XFree86 nomenclature. Note that the service data
  *  is a simple constant string pointer
  *
  */

#define  FT_SERVICE_ID_XF86_NAME  "xf86-driver-name"

#define  FT_XF86_FORMAT_TRUETYPE  "TrueType"
#define  FT_XF86_FORMAT_TYPE_1    "Type 1"
#define  FT_XF86_FORMAT_BDF       "BDF"
#define  FT_XF86_FORMAT_PCF       "PCF"
#define  FT_XF86_FORMAT_TYPE_42   "Type 42"
#define  FT_XF86_FORMAT_CID       "CID Type 1"
#define  FT_XF86_FORMAT_CFF       "CFF"
#define  FT_XF86_FORMAT_PFR       "PFR"
#define  FT_XF86_FORMAT_WINFNT    "Windows FNT"

 /* */
 
FT_END_HEADER

#endif /* __FT_SERVICE_XF86_NAME_H__ */
