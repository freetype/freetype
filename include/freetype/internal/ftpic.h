/***************************************************************************/
/*                                                                         */
/*  ftpic.h                                                                */
/*                                                                         */
/*    The FreeType position independent code services (declaration).       */
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

  /*************************************************************************/
  /*                                                                       */
  /*  Modules that ordinarily have const global data that need address     */
  /*  can instead define pointers here.                                    */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FTPIC_H__
#define __FTPIC_H__


FT_BEGIN_HEADER

#ifdef FT_CONFIG_OPTION_PIC

  /* When FT_CONFIG_OPTION_PIC is defined, the FT_PicTable
   * is used to hold all constant structures that normally
   * contain pointers in the normal build of FreeType.
   *
   * Note that the table itself only contains un-typed pointers
   * to base or module-specific data, which will be allocated
   * on the heap when the corresponding code is initialized.
   */
  typedef struct FT_PicTableRec_
  {
    /* pic containers for base */
    void*  base;

    /* pic containers for modules */
    void*  autofit;
    void*  cff;
    void*  pshinter;
    void*  psnames;
    void*  raster;
    void*  sfnt;
    void*  smooth;
    void*  truetype;

  } FT_PicTableRec, *FT_PicTable;

  /* Initialize the PIC table's base */
  FT_BASE( FT_Error )
  ft_library_pic_init( FT_Library library );


  /* Destroy the contents of the PIC table. */
  FT_BASE( void )
  ft_library_pic_done( FT_Library library );

#endif /* FT_CONFIG_OPTION_PIC */

 /* */

FT_END_HEADER

#endif /* __FTPIC_H__ */


/* END */
