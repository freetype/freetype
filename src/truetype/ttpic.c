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

  void
  tt_driver_class_pic_free(  FT_Library library )
  {
    FT_PicTable  pic_table = &library->pic_table;
    FT_Memory    memory    = library->memory;

    if ( pic_table->truetype )
    {
      TTModulePIC* container = (TTModulePIC*)pic_table->truetype;

      if(container->tt_services)
      {
        ft_library_pic_free_tt_services(library, container->tt_services);
        container->tt_services = NULL;
      }
      FT_FREE( container );
      pic_table->truetype = NULL;
    }
  }


  FT_Error
  tt_driver_class_pic_init(  FT_Library library )
  {
    FT_PicTable   pic_table = &library->pic_table;
    FT_Error      error     = FT_Err_Ok;
    FT_Memory     memory    = library->memory;
    TTModulePIC*  container;

    /* allocate pointer, clear and set global container pointer */
    if ( FT_NEW ( container ) )
      return error;

    pic_table->truetype = container;

    /* initialize pointer table - this is how the module usually expects this data */
    error = ft_library_pic_alloc_tt_services(library, &container->tt_services);
    if(error) 
      goto Exit;
#ifdef TT_CONFIG_OPTION_GX_VAR_SUPPORT
    ft_pic_init_tt_service_gx_multi_masters(&container->tt_service_gx_multi_masters);
#endif
    ft_pic_init_tt_service_truetype_glyf(&container->tt_service_truetype_glyf);

Exit:
    if(error)
      tt_driver_class_pic_free(library);

    return error;
  }

#endif /* FT_CONFIG_OPTION_PIC */


/* END */
