#ifndef __FT_OBJECT_H__
#define __FT_OBJECT_H__

#include <ft2build.h>
#include FT_FREETYPE_H

FT_BEGIN_HEADER

  typedef struct FT_ObjectRec_*        FT_Object;
  
  typedef const struct FT_ClassRec_*   FT_Class;
  
  typedef const struct FT_TypeRec_*    FT_Type;

  typedef struct FT_ObjectRec_
  {
    FT_Class  clazz;
    FT_Int    ref_count;
    
  } FT_ObjectRec;

#define  FT_OBJECT(x)    ((FT_Object)(x))
#define  FT_OBJECT_P(x)  ((FT_Object*)(x))

#define  FT_OBJECT__CLASS(x)      FT_OBJECT(x)->clazz
#define  FT_OBJECT__REF_COUNT(x)  FT_OBJECT(x)->ref_count
#define  FT_OBJECT__MEMORY(x)     FT_CLASS__MEMORY(FT_OBJECT(x)->clazz)
#define  FT_OBJECT__LIBRARY(x)    FT_CLASS__LIBRARY(FT_OBJECT(x)->clazz)

  typedef void  (*FT_Object_InitFunc)( FT_Object   object,
                                       FT_Pointer  init_data );

  typedef void  (*FT_Object_DoneFunc)( FT_Object   object );

  typedef void  (*FT_Object_CopyFunc)( FT_Object   target,
                                       FT_Object   source );

  typedef struct FT_ClassRec_
  {
    FT_ObjectRec        object;
    FT_UInt32           magic;
    FT_Type             type;
    FT_Memory           memory;
    FT_Library          library;
    FT_Pointer          info;
    FT_Pointer          data;    

    FT_UInt             obj_size;
    FT_Object_InitFunc  obj_init;
    FT_Object_DoneFunc  obj_done;
    FT_Object_CopyFunc  obj_copy;
  
  } FT_ClassRec;

#define  FT_CLASS(x)    ((FT_Class)(x))
#define  FT_CLASS_P(x)  ((FT_Class*)(x))

#define  FT_CLASS__MEMORY(x)   FT_CLASS(x)->memory
#define  FT_CLASS__LIBRARY(x)  FT_CLASS(x)->library
#define  FT_CLASS__MAGIC(x)    FT_CLASS(x)->magic
#define  FT_CLASS__TYPE(x)     FT_CLASS(x)->type
#define  FT_CLASS__DATA(x)     FT_CLASS(x)->data
#define  FT_CLASS__INFO(x)     FT_CLASS(x)->info

  typedef struct FT_TypeRec_
  {
    const char*         name;
    FT_Type             super;
    
    FT_UInt             class_size;
    FT_Object_InitFunc  class_init;
    FT_Object_DoneFunc  class_done;
    
    FT_UInt             obj_size;
    FT_Object_InitFunc  obj_init;
    FT_Object_DoneFunc  obj_done;
    FT_Object_CopyFunc  obj_copy;
  
  } FT_TypeRec;

#define  FT_TYPE(x)  ((FT_Type)(x))


 /* check that a handle points to a valid object */
  FT_BASE_DEF( FT_Int )
  ft_object_check( FT_Pointer  obj );


 /* check that an object is of a given class */
  FT_BASE_DEF( FT_Int )
  ft_object_is_a( FT_Pointer  obj,
                  FT_Class    clazz );


  FT_BASE_DEF( FT_Object )
  ft_object_new( FT_Class    clazz,
                 FT_Pointer  init_data );


FT_END_HEADER

#endif /* __FT_OBJECT_H__ */
