/***************************************************************************/
/*                                                                         */
/*  ftbase.h                                                               */
/*                                                                         */
/*    The FreeType private functions used in base module (specification).  */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2005, 2006, 2008 by             */
/*  David Turner, Robert Wilhelm, Werner Lemberg, and suzuki toshiya.      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTBASE_H__
#define __FTBASE_H__


#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H


FT_BEGIN_HEADER


  /* Check whether the sfnt image in the buffer is sfnt-wrapped PS Type1 */
  /* or sfnt-wrapped CID-keyed font. */
  FT_LOCAL_DEF( FT_Error )
  ft_lookup_PS_in_sfnt( FT_Byte*   sfnt,
                        FT_ULong*  offset,
                        FT_ULong*  length,
                        FT_Bool*   is_sfnt_cid );
 
  /* Create a new FT_Face given a buffer and a driver name. */
  /* from ftmac.c */
  FT_LOCAL_DEF( FT_Error )
  open_face_from_buffer( FT_Library   library,
                         FT_Byte*     base,
                         FT_ULong     size,
                         FT_Long      face_index,
                         const char*  driver_name,
                         FT_Face     *aface );


FT_END_HEADER

#endif /* __FTBASE_H__ */


/* END */
