/***************************************************************************/
/*                                                                         */
/*  ftcmanag.h                                                             */
/*                                                                         */
/*    FreeType Cache Manager (specification).                              */
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


#ifndef FTCMANAG_H
#define FTCMANAG_H

#include <freetype/ftcache.h>
#include <freetype/cache/ftlru.h>


#ifdef __cplusplus
  extern "C" {
#endif


#define  FTC_MAX_FACES_DEFAULT   4
#define  FTC_MAX_SIZES_DEFAULT   8
#define  FTC_MAX_BYTES_DEFAULT   65536

#define  FTC_MAX_CACHES  8

  /* opaque pointer to a cache object */
  typedef struct FTC_CacheRec_*  FTC_Cache;



  /* a ftc node is used to 
  typedef FT_ListNode   FTC_Node;

  /* macros to read/set the glyph & queue index in a FTC_Node */
#define FTC_IMAGENODE_GET_GINDEX( n )  FTC_PTR_TO_GINDEX( (n)->data )
#define FTC_IMAGENODE_GET_QINDEX( n )  FTC_PTR_TO_QINDEX( (n)->data )
          
#define FTC_IMAGENODE_SET_INDICES( n, g, q )              \
          do {                                            \
            (n)->data = FTC_INDICES_TO_PTR( g, q ); \
          } while ( 0 )




  /* a function used to initialize a cache */
  typedef FT_Error  (FTC_Cache_Init_Func) ( FTC_Cache  cache );
  
  /* a function used to finalize a cache */
  typedef void      (FTC_Cache_Done_Func) ( FTC_Cache  cache );
  
  /* a function used to return the size in bytes of a given cache node */
  typedef FT_ULong  (FTC_Cache_Size_Func) ( FTC_Cache  cache,
                                            FT_Pointer object );
                                            
  /* a function used to purge a given cache node */
  typedef void      (FTC_Cache_Purge_Func)( FTC_Cache  cache,
                                            FT_Pointer object );


  /* cache class */
  typedef struct FTC_Cache_Class_
  {
    FT_UInt                cache_size;  /* size of cache object in bytes */
    FTC_Cache_Init_Func    init;
    FTC_Cache_Done_Func    done;
    FTC_Cache_Size_Func    size;
    FTC_Cache_Purge_Func   purge;
  
  } FTC_Cache_Class;


  typedef struct FTC_CacheRec_
  {
    FTC_Manager       manager;   /* cache manager..                   */
    FTC_Cache_Class*  clazz;     /* cache clazz                       */
    FT_Memory         memory;    /* memory allocator                  */
    FT_UInt           cache_id;
    
  } FTC_CacheRec;


  typedef struct  FTC_ManagerRec_
  {
    FT_Library          library;
    FT_Lru              faces_lru;
    FT_Lru              sizes_lru;
    
    FT_Pointer          request_data;
    FTC_Face_Requester  request_face;
    
    FT_ULong            num_bytes;  /* current number of bytes in the caches */
    FT_ULong            max_bytes;  /* maximum number of bytes in the caches */
    FT_ListRec          global_lru; /* the global LRU list of nodes          */
    
    FT_UInt             num_caches;
    FT_UInt             last_id;
    FTC_Cache           caches[ FTC_MAX_CACHES ];
    
  } FTC_ManagerRec;



#ifdef __cplusplus
  }
#endif


#endif /* FTCMANAG_H */


/* END */
