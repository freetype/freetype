/***************************************************************************/
/*                                                                         */
/*  sfnt.h                                                                 */
/*                                                                         */
/*    Defines the function interface used to access SFNT files, i.e.,      */
/*    TrueType, OpenType-TT, and OpenType-T2 files (specification only).   */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef SFNT_H
#define SFNT_H

#include <tttypes.h>


  typedef TT_Long  (*SFNT_LookUp_Table)( TT_Face   face,
                                         TT_ULong  tag );


  typedef TT_Error  (*SFNT_Load_Table)( TT_Face    face,
                                        TT_ULong   tag,
                                        TT_Long    offset,
                                        void*      buffer,
                                        TT_Long*   length );

  typedef TT_Error  (*SFNT_Get_PS_Name)( TT_Face      face,
                                         TT_UShort    index,
                                         TT_String*  *ps_name );

  typedef struct  SFNT_Interface_
  {
    SFNT_LookUp_Table   lookup_table;
    SFNT_Load_Table     load_table;

  } SFNT_Interface;


#endif /* SFNT_H */


/* END */
