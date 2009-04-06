/***************************************************************************/
/*                                                                         */
/*  sfntpic.c                                                              */
/*                                                                         */
/*    The FreeType position independent code services for sfnt module.     */
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
#include "sfntpic.h"

#ifdef FT_CONFIG_OPTION_PIC

  /* forward declaration of PIC init functions from sfdriver.c */
  FT_Error ft_library_pic_alloc_sfnt_services( FT_Library, FT_ServiceDescRec**);
  void ft_library_pic_free_sfnt_services( FT_Library, FT_ServiceDescRec*);
  void ft_pic_init_sfnt_service_bdf( FT_Service_BDFRec*);
  void ft_pic_init_sfnt_interface( FT_Library, SFNT_Interface*);
  void ft_pic_init_sfnt_service_glyph_dict( FT_Library, FT_Service_GlyphDictRec*);
  void ft_pic_init_sfnt_service_ps_name( FT_Library, FT_Service_PsFontNameRec*);
  void ft_pic_init_tt_service_get_cmap_info( FT_Library, FT_Service_TTCMapsRec*);
  void ft_pic_init_sfnt_service_sfnt_table( FT_Service_SFNT_TableRec*);

  /* forward declaration of PIC init functions from ttcmap.c */
  FT_Error ft_library_pic_alloc_tt_cmap_classes( FT_Library, TT_CMap_Class**);
  void ft_library_pic_free_tt_cmap_classes( FT_Library, TT_CMap_Class*);

  void
  sfnt_module_class_pic_free(  FT_Library library )
  {
    FT_PicTable  pic_table = &library->pic_table;
    FT_Memory    memory    = library->memory;

    if ( pic_table->sfnt )
    {
      sfntModulePIC* container = (sfntModulePIC*)pic_table->sfnt;

      if(container->sfnt_services)
        ft_library_pic_free_sfnt_services(library, container->sfnt_services);

      container->sfnt_services = NULL;
      if(container->tt_cmap_classes)
      {
        ft_library_pic_free_tt_cmap_classes(library, container->tt_cmap_classes);
        container->tt_cmap_classes = NULL;
      }
      FT_FREE( container );
      pic_table->sfnt = NULL;
    }
  }


  FT_Error
  sfnt_module_class_pic_init(  FT_Library library )
  {
    FT_PicTable     pic_table = &library->pic_table;
    FT_Error        error     = FT_Err_Ok;
    FT_Memory       memory    = library->memory;
    sfntModulePIC*  container;

    /* allocate pointer, clear and set global container pointer */
    if ( FT_NEW ( container ) )
      return error;

    pic_table->sfnt = container;

    /* initialize pointer table - this is how the module usually expects this data */
    error = ft_library_pic_alloc_sfnt_services(library, &container->sfnt_services);
    if(error)
      goto Exit;

    error = ft_library_pic_alloc_tt_cmap_classes(library, &container->tt_cmap_classes);
    if(error)
      goto Exit;

    ft_pic_init_sfnt_service_glyph_dict (library, &container->sfnt_service_glyph_dict);
    ft_pic_init_sfnt_service_ps_name    (library, &container->sfnt_service_ps_name);
    ft_pic_init_tt_service_get_cmap_info(library, &container->tt_service_get_cmap_info);

    ft_pic_init_sfnt_service_sfnt_table(&container->sfnt_service_sfnt_table);
#ifdef TT_CONFIG_OPTION_BDF
    ft_pic_init_sfnt_service_bdf       (&container->sfnt_service_bdf);
#endif
    ft_pic_init_sfnt_interface(library, &container->sfnt_interface);

Exit:
    if(error)
      sfnt_module_class_pic_free(library);

    return error;
  }



#endif /* FT_CONFIG_OPTION_PIC */


/* END */
