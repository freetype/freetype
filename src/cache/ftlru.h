/***************************************************************************/
/*                                                                         */
/*  ftlru.h                                                                */
/*                                                                         */
/*    XXX                                                                  */
/*                                                                         */
/*  Copyright 2000 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef FTLRU_H
#define FTLRU_H

#include <freetype/freetype.h>


#ifdef __cplusplus
  extern "C" {
#endif


  typedef FT_Pointer  FT_LruKey;


  typedef struct  FT_LruNodeRec_
  {
    FT_ListNodeRec  root;
    FT_LruKey       key;
  
  } FT_LruNodeRec, *FT_LruNode;


  typedef struct FT_LruRec_*  FT_Lru;



  typedef struct  FT_Lru_Class_
  {
    FT_UInt   lru_size;      /* object size in bytes */
    
    FT_Error  (*init_element)( FT_Lru      lru,
                               FT_LruNode  node );
                                  
    void      (*done_element)( FT_Lru      lru,
                               FT_LruNode  node );
    
    FT_Error  (*flush_element)( FT_Lru      lru,
                                FT_LruNode  node,
                                FT_LruKey   new_key );  
                                   
    FT_Bool   (*compare_element)( FT_LruNode  node,
                                  FT_LruKey   key );

  } FT_Lru_Class;



  typedef FT_Bool  (*FT_Lru_Selector)( FT_Lru      lru,
                                       FT_LruNode  node,
                                       FT_Pointer  data );


  typedef struct  FT_LruRec_
  {
    FT_Lru_Class*   clazz;
    FT_UInt         max_elements;
    FT_UInt         num_elements;
    FT_ListRec      elements;
    FT_Memory       memory;
    FT_Pointer      user_data;
    
    /* the following fields are only meaningful for static lru containers */
    FT_ListRec      free_nodes;
    FT_LruNode      nodes;
    
  } FT_LruRec;


  FT_EXPORT_DEF( FT_Error )  FT_Lru_New( const FT_Lru_Class*  clazz,
                                         FT_UInt              max_elements,
                                         FT_Pointer           user_data,
                                         FT_Memory            memory,
                                         FT_Bool              pre_alloc,
                                         FT_Lru*              alru );
                                          
  FT_EXPORT_DEF( void )  FT_Lru_Reset( FT_Lru  lru ); 
                                       
  FT_EXPORT_DEF( void )  FT_Lru_Done( FT_Lru  lru ); 

  FT_EXPORT_DEF( FT_Error )  FT_Lru_Lookup_Node( FT_Lru        lru,
                                                 FT_LruKey     key,
                                                 FT_LruNode*  anode );

  FT_EXPORT_DEF( FT_Error )  FT_Lru_Lookup( FT_Lru       lru,
                                            FT_LruKey    key,
                                            FT_Pointer*  aobject );
 
  FT_EXPORT_DEF( void )  FT_Lru_Remove_Node( FT_Lru      lru,
                                             FT_LruNode  node );  

  FT_EXPORT_DEF( void )  FT_Lru_Remove_Selection( FT_Lru           lru,
                                                  FT_Lru_Selector  selector,
                                                  FT_Pointer       data );


#ifdef __cplusplus
  }
#endif


#endif /* FTLRU_H */


/* END */
