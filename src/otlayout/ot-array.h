/* OTArray
 *
 * Copyright (C) 2003 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __OT_ARRAY_H__
#define __OT_ARRAY_H__

#include <ft2build.h>
#include FT_FREETYPE_H

FT_BEGIN_HEADER

  typedef struct _OTArray OTArray;  
  typedef FT_Int (*OT_Array_Comapre_Func) (const void * a, const void * b );

  struct _OTArray
  {
    FT_Char * data;
    FT_UInt   length;
    FT_UInt   element_size;
    FT_UInt   allocated;
    FT_Memory memory;
  };
  
  FT_LOCAL( OTArray * ) OT_Array_New        ( FT_UInt element_size, FT_Memory memory );
  FT_LOCAL( void )      OT_Array_Free       ( OTArray * array );
  FT_LOCAL( OTArray * ) OT_Array_Set_Size   ( OTArray * array, FT_UInt length );
  FT_LOCAL( void )      OT_Array_Sort       ( OTArray * array, OT_Array_Comapre_Func func );
  FT_LOCAL( OTArray * ) OT_Array_Append_Val ( OTArray * array, FT_Pointer newval );
#define OT_Array_Index(array, type, index) (((type*)((array)->data))[(index)])

FT_END_HEADER

#endif /* Not def: __OT_ARRAY_H__ */
