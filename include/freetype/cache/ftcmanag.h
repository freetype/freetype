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
/*                                                                         */
/*  A cache manager is in charge of the following:                         */
/*                                                                         */
/*  - maintain a mapping between generic FTC_FaceIDs and live              */
/*    FT_Face objects. The mapping itself is performed through a           */
/*    user-provided callback. However, the manager maintains a small       */
/*    cache of FT_Face & FT_Size objects in order to speed things          */
/*    considerably..                                                       */
/*                                                                         */
/*  - manage one or more cache object. Each cache is in charge of          */
/*    holding a varying number of "cache nodes". Each cache node           */
/*    represents a minimal amount of individually-accessible cached        */
/*    data. For example, a cache node can be a FT_Glyph image containing   */
/*    a vector outline, or some glyph metrics, or anything else..          */
/*                                                                         */
/*    each cache node has a certain size in bytes that is added to the     */
/*    total amount of "cache memory" within the manager.                   */
/*                                                                         */
/*    all cache nodes are located in a global LRU list, where the          */
/*    oldest node is at the tail of the list                               */
/*                                                                         */
/*    each node belongs to a single cache, and includes a reference        */
/*    count to avoid destroying it (due to caching)                        */
/*                                                                         */
/***************************************************************************/

 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/
 /*********                                                       **********/
 /*********                                                       **********/
 /*********        WARNING, THIS IS ALPHA CODE, THIS API          **********/
 /*********    IS DUE TO CHANGE UNTIL STRICTLY NOTIFIED BY THE    **********/
 /*********            FREETYPE DEVELOPMENT TEAM                  **********/
 /*********                                                       **********/
 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/


#ifndef FTCMANAG_H
#define FTCMANAG_H

#include <freetype/ftcache.h>
#include <freetype/cache/ftlru.h>


#ifdef __cplusplus
  extern "C" {
#endif

/* default values */
#define  FTC_MAX_FACES_DEFAULT  4
#define  FTC_MAX_SIZES_DEFAULT  8
#define  FTC_MAX_BYTES_DEFAULT  65536  /* 64 Kb by default */

/* maximum number of caches registered in a single manager */
#define  FTC_MAX_CACHES         16


  typedef struct  FTC_ManagerRec_
  {
    FT_Library          library;
    FT_Lru              faces_lru;
    FT_Lru              sizes_lru;

    FT_ULong            max_bytes;
    FT_ULong            num_bytes;
    FT_ListRec          global_lru;
    FTC_Cache           caches[ FTC_MAX_CACHES ];
    
    FT_Pointer          request_data;
    FTC_Face_Requester  request_face;
    
  } FTC_ManagerRec;


 /*********************************************************************/
 /*********************************************************************/
 /*****                                                           *****/
 /*****               CACHE NODE DEFINITIONS                      *****/
 /*****                                                           *****/
 /*********************************************************************/
 /*********************************************************************/
 
 

  /* each cache controls one or more cache nodes. Each node    */
  /* is part of the global_lru list of the manager. Its "data" */
  /* field however is used as a reference count for now..      */
  /*                                                           */
  /* a node can anything, depending on the type of information */
  /* held by the cache. It can be an individual glyph image,   */
  /* a set of bitmaps glyphs for a given size, some metrics,   */
  /* etc..                                                     */
  /*                                                           */
  
  typedef FT_ListNodeRec     FTC_CacheNodeRec;
  typedef FTC_CacheNodeRec*  FTC_CacheNode;


  /* the fields "cachenode.data" is typecasted to this type */
  typedef struct FTC_CacheNode_Data_
  {
    FT_UShort   cache_index;
    FT_Short    ref_count;
  
  } FTC_CacheNode_Data;
  
/* returns a pointer to the FTC_CacheNode_Data contained in a */
/* CacheNode's "data" field..                                 */
#define  FTC_CACHENODE_TO_DATA_P(n)  \
            ((FTC_CacheNode_Data*)&(n)->data)

#define  FTC_LIST_TO_CACHENODE(n)     ((FTC_CacheNode)(n))

  /* returns the size in bytes of a given cache node */
  typedef FT_ULong   (*FTC_CacheNode_SizeFunc)( FTC_CacheNode  node,
                                                FT_Pointer     user );

  /* finalise a given cache node */
  typedef void       (*FTC_CacheNode_DestroyFunc)( FTC_CacheNode  node,
                                                   FT_Pointer     user );

  /* this structure is used to provide functions to the cache manager   */
  /* It will use them to size and destroy cache nodes.. Note that there */
  /* is no "init_node" there because cache objects are entirely         */
  /* responsible for the creation of new cache nodes                    */
  /*                                                                    */
  typedef struct FTC_CacheNode_Class_
  {
    FTC_CacheNode_SizeFunc      size_node;
    FTC_CacheNode_DestroyFunc   destroy_node;
    
  } FTC_CacheNode_Class;
                                   

 /*********************************************************************/
 /*********************************************************************/
 /*****                                                           *****/
 /*****                       CACHE DEFINITIONS                   *****/
 /*****                                                           *****/
 /*********************************************************************/
 /*********************************************************************/
 

  typedef FT_Error   (*FTC_Cache_InitFunc)( FTC_Cache    cache );
  
  typedef void       (*FTC_Cache_DoneFunc)( FTC_Cache    cache );
  

  struct FTC_Cache_Class_
  {
    FT_UInt                  cache_byte_size;
    FTC_Cache_InitFunc       init_cache;
    FTC_Cache_DoneFunc       done_cache;
  };



  typedef struct  FTC_CacheRec_
  {
    FTC_Manager             manager;
    FT_Memory               memory;
    FTC_Cache_Class*        clazz;
    FTC_CacheNode_Class*    node_clazz;
    
    FT_UInt                 cache_index;  /* in manager's table          */
    FT_Pointer              cache_user;   /* passed to cache node methods*/
    
  } FTC_CacheRec;


  /* "compress" the manager's data, i.e. get rids of old cache nodes */
  /* that are not referenced anymore in order to limit the total     */
  /* memory used by the cache..                                      */
  FT_EXPORT_DEF(void)  FTC_Manager_Compress( FTC_Manager  manager );


#ifdef __cplusplus
  }
#endif


#endif /* FTCMANAG_H */


/* END */
