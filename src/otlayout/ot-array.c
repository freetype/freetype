/* OTArray
 *
 * Copyright (C) 2003 Red Hat.
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

#include "ot-array.h"
#include FT_INTERNAL_MEMORY_H
#include FT_CONFIG_STANDARD_LIBRARY_H

#define OT_REALLOC_ARRAY( _pointer_, _old_, _new_, eltsize )	\
          FT_SET_ERROR( FT_MEM_REALLOC( _pointer_,		\
                                        (_old_) * eltsize,	\
                                        (_new_) * eltsize ) )

  FT_LOCAL_DEF( OTArray * ) 
  OT_Array_New ( FT_UInt element_size, FT_Memory memory )
  {
    FT_Error error;
    OTArray * array;
    
    if ( FT_NEW ( array ) )
      return NULL;

    array->data 	= NULL;
    array->length 	= 0;
    array->allocated    = 0;
    array->element_size = element_size;
    array->memory 	= memory;
    return OT_Array_Set_Size ( array, 4 );
  }

  FT_LOCAL_DEF( void )      
  OT_Array_Free ( OTArray * array )
  {
    FT_Memory memory = array->memory;
    FT_FREE( array->data );
    FT_FREE( array );
  }
  
  FT_LOCAL_DEF( OTArray * ) 
  OT_Array_Set_Size ( OTArray * array, FT_UInt length )
  {
    FT_Error error;
    FT_Memory memory = array->memory;

    if ( length > array->allocated )
      {
	if ( OT_REALLOC_ARRAY(array->data,
			      array->allocated,
			      length,
			      array->element_size) )
	  return NULL;
	array->allocated = length;
      }
    array->length = length;
    return array;
  }

  FT_LOCAL_DEF( void )
  OT_Array_Sort ( OTArray * array, OT_Array_Comapre_Func func )
  {
    ft_qsort ( array->data, array->length, array->element_size, func );
  }

  FT_LOCAL_DEF( OTArray * )
  OT_Array_Append_Val ( OTArray * array, FT_Pointer newval )
  {
    FT_UInt index;
    FT_Pointer dest;
    
    if ( !OT_Array_Set_Size( array, array->length + 1 ) )
      return NULL;

    index = array->length - 1;
    dest  = array->data + (  index * array->element_size );
    FT_MEM_COPY ( dest, newval, array->element_size );
    return array;
  }
