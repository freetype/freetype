/***************************************************************************/
/*                                                                         */
/*  pshpic.c                                                               */
/*                                                                         */
/*    The FreeType position independent code services for pshinter module. */
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
#include "pshpic.h"

#ifdef FT_CONFIG_OPTION_PIC

  /* forward declaration of PIC init functions from pshmod.c */
  void ft_pic_init_pshinter_interface( FT_Library, PSHinter_Interface*);

  void
  pshinter_module_class_pic_free( FT_Library library )
  {
    FT_PicTable  pic_table = &library->pic_table;
    FT_Memory    memory    = library->memory;

    if ( pic_table->pshinter )
    {
      FT_FREE( pic_table->pshinter );
      pic_table->pshinter = NULL;
    }
  }


  FT_Error
  pshinter_module_class_pic_init( FT_Library library )
  {
    FT_PicTable   pic_table = &library->pic_table;
    FT_Error      error     = FT_Err_Ok;
    FT_Memory     memory    = library->memory;
    PSHinterPIC*  container;

    /* allocate pointer, clear and set global container pointer */
    if ( FT_NEW ( container ) )
      return error;

    pic_table->pshinter = container;

    /* add call to initialization function when you add new scripts */
    ft_pic_init_pshinter_interface(library, &container->pshinter_interface);

/*Exit:*/
    if(error)
      pshinter_module_class_pic_free(library);

    return error;
  }

#endif /* FT_CONFIG_OPTION_PIC */

/* END */
