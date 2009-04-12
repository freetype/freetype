/***************************************************************************/
/*                                                                         */
/*  ttpic.c                                                                */
/*                                                                         */
/*    The FreeType position independent code services for truetype module. */
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


#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_INTERNAL_OBJECTS_H
#include "ttpic.h"

#ifdef FT_CONFIG_OPTION_PIC

  /* forward declaration of PIC init functions from ttdriver.c */
  FT_Error ft_library_pic_alloc_tt_services( FT_Library, FT_ServiceDescRec**);
  void ft_library_pic_free_tt_services( FT_Library, FT_ServiceDescRec*);
  void ft_pic_init_tt_service_gx_multi_masters(FT_Service_MultiMastersRec*);
  void ft_pic_init_tt_service_truetype_glyf(FT_Service_TTGlyfRec*);

  static void
  pic_tt_driver_done( void*  data, FT_PicTable  pic )
  {
    TTModulePIC*  container = (TTModulePIC*) data;

    if (container->tt_services)
    {
      ft_library_pic_free_tt_services(pic->library, container->tt_services);
      container->tt_services = NULL;
    }
  }

  static FT_Error
  pic_tt_driver_init( void*  data, FT_PicTable  pic )
  {
    TTModulePIC*  container = (TTModulePIC*) data;
    FT_Error      error;

    error = ft_library_pic_alloc_tt_services(pic->library, &container->tt_services);
    if (error)
      goto Exit;

#ifdef TT_CONFIG_OPTION_GX_VAR_SUPPORT
    ft_pic_init_tt_service_gx_multi_masters(&container->tt_service_gx_multi_masters);
#endif
    ft_pic_init_tt_service_truetype_glyf(&container->tt_service_truetype_glyf);

  Exit:
    return error;
  }

  FT_Error
  tt_driver_class_pic_init(  FT_Library library )
  {
    FT_PicTable   pic_table = &library->pic_table;

    return ft_pic_table_init_data( pic_table,
                                   pic_table->truetype,
                                   sizeof(TTModulePIC),
                                   pic_tt_driver_init,
                                   pic_tt_driver_done );
  }

#endif /* FT_CONFIG_OPTION_PIC */


/* END */
