/***************************************************************************/
/*                                                                         */
/*  cffpic.c                                                               */
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


#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_INTERNAL_OBJECTS_H
#include "cffpic.h"

#ifdef FT_CONFIG_OPTION_PIC

  /* forward declaration of PIC init functions from cffdrivr.c */
  FT_Error ft_library_pic_alloc_cff_services( FT_Library, FT_ServiceDescRec**);
  void ft_library_pic_free_cff_services( FT_Library, FT_ServiceDescRec*);
  void ft_pic_init_cff_service_ps_info( FT_Library, FT_Service_PsInfoRec*);
  void ft_pic_init_cff_service_glyph_dict( FT_Library, FT_Service_GlyphDictRec*);
  void ft_pic_init_cff_service_ps_name( FT_Library, FT_Service_PsFontNameRec*);
  void ft_pic_init_cff_service_get_cmap_info( FT_Library, FT_Service_TTCMapsRec*);
  void ft_pic_init_cff_service_cid_info( FT_Library, FT_Service_CIDRec*);

  /* forward declaration of PIC init functions from cffparse.c */
  void cff_pic_field_handlers_init( CFF_Field_Handler* );

#if 0  /* defined by cffcmap.h */
  /* forward declaration of PIC init functions from cffcmap.c */
  void ft_pic_init_cff_cmap_encoding_class_rec( FT_Library, FT_CMap_ClassRec*);
  void ft_pic_init_cff_cmap_unicode_class_rec( FT_Library, FT_CMap_ClassRec*);
#endif

  static void
  pic_cff_done( void*  _cff, FT_PicTable  pic )
  {
    CffModulePIC* container = (CffModulePIC*) _cff;


    if (container->cff_services)
    {
      ft_library_pic_free_cff_services(pic->library, container->cff_services);
      container->cff_services = NULL;
    }
  }


  static FT_Error
  pic_cff_init( void*  _cff, FT_PicTable  pic )
  {
    FT_Library     library   = pic->library;
    FT_Error       error     = FT_Err_Ok;
    CffModulePIC*  container = (CffModulePIC*) _cff;

    /* initialize pointer table - this is how the module usually expects this data */
    error = ft_library_pic_alloc_cff_services(library, &container->cff_services);
    if(error) 
      goto Exit;

    cff_pic_field_handlers_init(container->cff_field_handlers);

    ft_pic_init_cff_service_ps_info    (library, &container->cff_service_ps_info);
    ft_pic_init_cff_service_glyph_dict (library, &container->cff_service_glyph_dict);
    ft_pic_init_cff_service_ps_name    (library, &container->cff_service_ps_name);

    ft_pic_init_cff_service_get_cmap_info(library, &container->cff_service_get_cmap_info);
    ft_pic_init_cff_service_cid_info     (library, &container->cff_service_cid_info);

    ft_pic_init_cff_cmap_encoding_class_rec (library,
                                               &container->cff_cmap_encoding_class_rec);
    ft_pic_init_cff_cmap_unicode_class_rec  (library, 
                                               &container->cff_cmap_unicode_class_rec);
Exit:
    return error;
  }


  FT_Error
  cff_driver_class_pic_init(  FT_Library library )
  {
    FT_PicTable  pic = &library->pic_table;

    return ft_pic_table_init_data( pic, pic->cff,
                                   sizeof(CffModulePIC),
                                   pic_cff_init,
                                   pic_cff_done );
  }

#endif /* FT_CONFIG_OPTION_PIC */


/* END */
