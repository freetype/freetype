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

  /* A FT_PicDataRec object models a heap-allocated structure
   * used to hold data that, in non-PIC builds of the engine,
   * would be constant and contain pointers.
   *
   * The FT_PicDataRec contains book-keeping information about
   * the structure, whose exact fields depend on which module
   * is defining/using it. The structure itself can be
   * accessed as the 'data' field of a FT_PicDataRec object.
   */
  typedef struct FT_PicDataRec_*  FT_PicData;

  /* A FT_PicTableRec object contains the FT_PicDataRec of all
   * PIC structures needed by the library at runtime.
   */
  typedef struct FT_PicTableRec_*  FT_PicTable;

  /* A special callback function call to finalize the structure
   * backed by a FT_PicDataRec object. It is called by
   * ft_pic_table_done_data and its first parameter is the 'data'
   * field of the corresponding FT_PicDataRec. Note that the
   * PIC structure itself is allocated by ft_pic_table_init_data
   * and freed by ft_pic_table_done_data.
   */
  typedef void (*FT_PicDataDoneFunc)( void*  data, FT_PicTable  pic );

  /* A special initialization function called by ft_pic_table_init_data
   * to initialize a freshly allocated PIC structure.
   */
  typedef FT_Error (*FT_PicDataInitFunc)( void*  data, FT_PicTable  pic );

  /* The FT_PicDataRec itself holds a pointer to heap-allocated
   * PIC data, a reference count and an optional finalizer function.
   *
   * One should call ft_pic_table_init_data() to register a new
   * heap-allocated PIC structure, which address will be placed
   * into the 'data' field.
   */
  typedef struct FT_PicDataRec_
  {
    void*               data;
    FT_Int              ref_count;
    FT_PicDataDoneFunc  done;

  } FT_PicDataRec;

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
    FT_Library  library;
    FT_Memory   memory;

/* this macro is used to define the list of FT_PicDataRec
 * stored in the PIC table. Add new entries here.
 */
#define  FT_PIC_ENTRY_LIST  \
  _FT_PICDATA( base ) \
  _FT_PICDATA( autofit ) \

/* now define the entries in the PIC table itself */
#define _FT_PICDATA(name)  FT_PicDataRec  name [1];
    FT_PIC_ENTRY_LIST
#undef _FT_PICDATA

    void*  cff;
    void*  pshinter;
    void*  psnames;
    void*  raster;
    void*  sfnt;
    void*  smooth;
    void*  truetype;

  } FT_PicTableRec;

#define  FT_LIBRARY_GET_PIC_DATA(library_,name_) \
  library_->pic_table.name_->data

  /* allocate and initialize a PIC structure in the heap.
   * This should be called at module initialization time
   * before the PIC data can be used.
   *
   * if the structure already exists, this simply increments
   * its reference count.
   */
  FT_BASE( FT_Error )
  ft_pic_table_init_data( FT_PicTable         table,
                          FT_PicData          data,
                          FT_UInt             data_size,
                          FT_PicDataInitFunc  data_init,
                          FT_PicDataDoneFunc  data_done );

  /* finalize and free a given PIC structure. This really
   * decrements the reference count and performs destruction
   * if it reaches 0.
   *
   * this should be called a module exit time.
   */
  FT_BASE( void )
  ft_pic_table_done_data( FT_PicTable  table,
                          FT_PicData   data );

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
