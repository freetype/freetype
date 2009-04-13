/***************************************************************************/
/*                                                                         */
/*  services/listdecl.h                                                    */
/*                                                                         */
/*    Service list declaration template                                    */
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
 * to declare a list of services in a given module. It relies on
 * FT_SERVICE_LIST_NAME and FT_SERVICE_LIST_FIELDS being defined
 *
 * FT_SERVICE_NAME is the name of the service list
 * FT_SERVICE_ITEMS is a list of _FT_SERVICE_ITEM(id,value)
 * statements that will be used to define the corresponding service
 * item in the list
 */

#ifndef FT_SERVICE_ITEMS
#error  FT_SERVICE_ITEMS must be defined
#endif

#ifndef FT_SERVICE_NAME
#error  FT_SERVICE_NAME must be defined
#endif

#ifdef FT_CONFIG_OPTION_PIC

/* For PIC mode, count the number of items in the list
 * and define the appropriate array type.
 */
  /* compute number of active items in array */
  enum {
    FT_GLUE(_ft_service_count_,FT_SERVICE_NAME) = 0
    #define _FT_SERVICE_ITEM(x,y) +1
    FT_SERVICE_ITEMS
    #undef _FT_SERVICE_ITEM
  };

  typedef FT_ServiceDescRec  FT_GLUE(FT_ServiceItems_,FT_SERVICE_NAME)
                             [ FT_GLUE(_ft_service_count_,FT_SERVICE_NAME)+1 ];

  void
  FT_GLUE(ft_pic_services_init_,FT_SERVICE_NAME)
  ( FT_ServiceDescRec*  items, FT_Library  library );

#endif /* FT_CONFIG_OPTION_PIC */

#undef FT_SERVICE_NAME
#undef FT_SERVICE_ITEMS
