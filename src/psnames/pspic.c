/***************************************************************************/
/*                                                                         */
/*  pspic.c                                                                */
/*                                                                         */
/*    The FreeType position independent code services for psnames module.  */
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
#include "pspic.h"

#ifdef FT_CONFIG_OPTION_PIC

  /* forward declaration of PIC init functions from psmodule.c */
  FT_Error ft_library_pic_alloc_pscmaps_services ( FT_Library, FT_ServiceDescRec** );
  void     ft_library_pic_free_pscmaps_services( FT_Library, FT_ServiceDescRec* );
  void     ft_pic_init_pscmaps_interface  ( FT_Library, FT_Service_PsCMapsRec* );

  void
  psnames_module_class_pic_free(  FT_Library library )
  {
    FT_PicTable  pic_table = &library->pic_table;
    FT_Memory    memory    = library->memory;


    if ( pic_table->psnames )
    {
      PSModulePIC* container = (PSModulePIC*)pic_table->psnames;


      if(container->pscmaps_services)
        ft_library_pic_free_pscmaps_services(library, container->pscmaps_services);

      container->pscmaps_services = NULL;
      FT_FREE( container );

      pic_table->psnames = NULL;
    }
  }


  FT_Error
  psnames_module_class_pic_init(  FT_Library library )
  {
    FT_PicTable   pic_table = &library->pic_table;
    FT_Error      error     = FT_Err_Ok;
    FT_Memory     memory    = library->memory;
    PSModulePIC*  container;

    /* allocate pointer, clear and set global container pointer */
    if ( FT_NEW ( container ) )
      return error;

    pic_table->psnames = container;

    /* initialize pointer table - this is how the module usually expects this data */
    error = ft_library_pic_alloc_pscmaps_services(library, &container->pscmaps_services);
    if(error) 
      goto Exit;

    ft_pic_init_pscmaps_interface(library, &container->pscmaps_interface);

Exit:
    if(error)
      psnames_module_class_pic_free(library);

    return error;
  }


#endif /* FT_CONFIG_OPTION_PIC */


/* END */
