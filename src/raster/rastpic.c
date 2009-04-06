/***************************************************************************/
/*                                                                         */
/*  rastpic.c                                                              */
/*                                                                         */
/*    The FreeType position independent code services for raster module.   */
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
#include "rastpic.h"

#ifdef FT_CONFIG_OPTION_PIC

  /* forward declaration of PIC init functions from ftraster.c */
  void ft_pic_init_ft_standard_raster(FT_Raster_Funcs*);

  void
  ft_raster1_renderer_class_pic_free(  FT_Library library )
  {
    FT_PicTable  pic_table = &library->pic_table;
    FT_Memory    memory    = library->memory;

    if ( pic_table->raster )
    {
      RasterPIC* container = (RasterPIC*)pic_table->raster;

      if(--container->ref_count)
        return;

      FT_FREE( container );
      pic_table->raster = NULL;
    }
  }


  FT_Error
  ft_raster1_renderer_class_pic_init(  FT_Library library )
  {
    FT_PicTable  pic_table = &library->pic_table;
    FT_Error     error     = FT_Err_Ok;
    FT_Memory    memory    = library->memory;
    RasterPIC*   container;

    /* since this function also serve raster5 renderer, 
       it implements reference counting */
    if(pic_table->raster)
    {
      ((RasterPIC*)pic_table->raster)->ref_count++;
      return error;
    }

    /* allocate pointer, clear and set global container pointer */
    if ( FT_NEW ( container ) )
      return error;

    pic_table->raster    = container;
    container->ref_count = 1;

    /* initialize pointer table - this is how the module usually expects this data */
    ft_pic_init_ft_standard_raster(&container->ft_standard_raster);
/*Exit:*/
    if(error)
      ft_raster1_renderer_class_pic_free(library);

    return error;
  }

  /* re-route these init and free functions to the above functions */
  FT_Error ft_raster5_renderer_class_pic_init(FT_Library library)
  {
    return ft_raster1_renderer_class_pic_init(library);
  }

  void ft_raster5_renderer_class_pic_free(FT_Library library)
  {
    ft_raster1_renderer_class_pic_free(library);
  }

#endif /* FT_CONFIG_OPTION_PIC */


/* END */
