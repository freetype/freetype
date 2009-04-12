/***************************************************************************/
/*                                                                         */
/*  ftpic.c                                                                */
/*                                                                         */
/*    The FreeType position independent code services (body).              */
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
#include "basepic.h"

#ifdef FT_CONFIG_OPTION_PIC

  /* documentation is in ftpic.h */

  FT_BASE_DEF( FT_Error )
  ft_pic_table_init_data( FT_PicTable         pic,
                          FT_PicData          data,
                          FT_UInt             data_size,
                          FT_PicDataInitFunc  data_init,
                          FT_PicDataDoneFunc  data_done )
  {
    void*      pic_data = data->data;
    FT_Error   error    = 0;
    FT_Memory  memory   = pic->memory;

    /* if the PIC structure already exist, just increment its
     * reference count
     */
    if (pic_data != NULL)
    {
      data->ref_count += 1;
      return 0;
    }

    if ( FT_ALLOC( pic_data, data_size ) )
      goto Exit;

    error = data_init( pic_data, pic );
    if (error)
    {
        if (data_done)
          data_done( pic_data, pic );

        FT_FREE( pic_data );
        goto Exit;
    }

    data->data      = pic_data;
    data->ref_count = 1;
    data->done      = data_done;

  Exit:
    return error;
  }


  FT_BASE_DEF( void )
  ft_pic_table_done_data( FT_PicTable  pic,
                          FT_PicData   data )
  {
    FT_Memory  memory = pic->memory;

    if ( --data->ref_count != 0 )
        return;

    if (data->done)
        data->done( data->data, pic );

    FT_FREE(data->data);
    data->done = NULL;
  }


  FT_BASE_DEF( FT_Error )
  ft_library_pic_init( FT_Library library )
  {
    FT_PicTable  pic   = &library->pic_table;
    FT_Error     error = FT_Err_Ok;

    FT_ZERO(pic);

    pic->library = library;
    pic->memory  = library->memory;

    error = ft_base_pic_init( library );
    if(error)
      return error;

    return FT_Err_Ok;
  }


  /* Destroy the contents of the container. */
  FT_BASE_DEF( void )
  ft_library_pic_done( FT_Library library )
  {
    ft_base_pic_free( library );
  }

#endif /* FT_CONFIG_OPTION_PIC */


/* END */
