/***************************************************************************/
/*                                                                         */
/*  services/define.h                                                      */
/*                                                                         */
/*    The FreeType PostScript info service (specification).                */
/*                                                                         */
/*  Copyright 2009 by David Turner                                         */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/* this header file can be included multipled times and is used
 * to declare the types corresponding to a given service described
 * by FT_SERVICE_NAME and FT_SERVICE_FIELDS
 *
 * FT_SERVICE_NAME is the name of the service
 * FT_SERVICE_FIELDS is a list of _FT_SERVICE_FIELD(type,varname)
 * statements that will be used to define the corresponding service
 * structure and pointer types.
 */

#ifndef FT_SERVICE_FIELDS
#error  FT_SERVICE_FIELDS must be defined
#endif

#ifndef FT_SERVICE_PREFIX
#error  FT_SERVICE_PREFIX must be defined
#endif

#ifndef FT_SERVICE_NAME
#error  FT_SERVICE_NAME must be defined
#endif

#ifndef FT_CONFIG_OPTION_PIC

  static const FT_GLUE3(FT_Service_,FT_SERVICE_NAME,Rec)
  FT_GLUE(ft_service_,FT_SERVICE_NAME) =
  {
#define  _FT_SERVICE_FIELD(type_,name_) \
    FT_GLUE(FT_SERVICE_PREFIX,name_),
    FT_SERVICE_FIELDS
#undef  _FT_SERVICE_FIELD
  };

#else /* FT_CONFIG_OPTION_PIC */

  void FT_GLUE(ft_pic_service_init_,FT_SERVICE_NAME)(
         FT_GLUE3(FT_Service_,FT_SERVICE_NAME,Rec)*  clazz )
  {
#define  _FT_SERVICE_FIELD(type_,name_) \
         clazz->name_ = FT_GLUE(FT_SERVICE_PREFIX,name_);
    FT_SERVICE_FIELDS
#undef   _FT_SERVICE_FIELD
  }

#endif /* FT_CONFIG_OPTION_PIC */

#undef FT_SERVICE_NAME
#undef FT_SERVICE_FIELDS
#undef FT_SERVICE_PREFIX
