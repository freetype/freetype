#ifndef __FT_SYSTEM_MEMORY_H__
#define __FT_SYSTEM_MEMORY_H__

#include <ft2build.h>

FT_BEGIN_HEADER

 /* handle to memory structure */
  typedef struct FT_MemoryRec_*   FT_Memory;

 /* a function used to allocate a new block of memory from a heap */
  typedef FT_Pointer  (*FT_Memory_AllocFunc)( FT_ULong   size,
                                              FT_Memory  memory );
  
 /* a function used to free a block of memory */
  typedef void        (*FT_Memory_FreeFunc) ( FT_Pointer  block,
                                              FT_Memory   memory );
  
 /* a function used to reallocate a given memory block to a new size */
  typedef FT_Pointer  (*FT_Memory_ReallocFunc)( FT_Pointer   block,
                                                FT_ULong     new_size,
                                                FT_ULong     cur_size,
                                                FT_Memory    memory );

 /* a function called to allocate a new structure of 'size' bytes that */
 /* will be used for a new FT_Memory object..                          */
 /*                                                                    */
  typedef FT_Pointer  (*FT_Memory_CreateFunc)( FT_UInt     size,
                                               FT_Pointer  init_data );

 /* a function used to destroy a FT_Memory object */  
  typedef void        (*FT_Memory_DestroyFunc)( FT_Memory  memory );

 /* a structure holding the functions used to describe a given FT_Memory */
 /* implementation..                                                     */
 /*                                                                      */
  typedef struct FT_Memory_FuncsRec_
  {
    FT_Memory_AllocFunc     mem_alloc;
    FT_Memory_FreeFunc      mem_free;
    FT_Memory_ReallocFunc   mem_realloc;
    FT_Memory_CreateFunc    mem_create;
    FT_Memory_DestroyFunc   mem_destroy;
  
  } FT_Memory_FuncsRec, *FT_Memory_Funcs;


 /* a function used to create a new custom FT_Memory object */
 /*                                                         */
  FT_BASE_DEF( FT_Memory )
  ft_memory_new( FT_Memory_Funcs  mem_funcs,
                 FT_Pointer       mem_init_data );

 /* a function used to destroy a custom FT_Memory object */
  FT_BASE_DEF( void )
  ft_memory_destroy( FT_Memory  memory );

 /* a pointer to the default memory functions used by FreeType in a */
 /* given build. By default, uses the ISO C malloc/free/realloc     */
 /*                                                                 */
  FT_APIVAR( const FT_MemoryFuncs )    ft_memory_funcs_default;

/* */

FT_END_HEADER

#endif /* __FT_SYSTEM_MEMORY_H__ */
