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


  /*************************************************************************/
  /*                                                                       */
  /* A cache manager is in charge of the following:                        */
  /*                                                                       */
  /*  - Maintain a mapping between generic FTC_FaceIDs and live FT_Face    */
  /*    objects.  The mapping itself is performed through a user-provided  */
  /*    callback.  However, the manager maintains a small cache of FT_Face */
  /*    & FT_Size objects in order to speed things considerably.           */
  /*                                                                       */
  /*  - Manage one or more cache objects.  Each cache is in charge of      */
  /*    holding a varying number of `cache nodes'.  Each cache node        */
  /*    represents a minimal amount of individually-accessible cached      */
  /*    data.  For example, a cache node can be an FT_Glyph image          */
  /*    containing a vector outline, or some glyph metrics, or anything    */
  /*    else.                                                              */
  /*                                                                       */
  /*    Each cache node has a certain size in bytes that is added to the   */
  /*    total amount of `cache memory' within the manager.                 */
  /*                                                                       */
  /*    All cache nodes are located in a global LRU list, where the oldest */
  /*    node is at the tail of the list.                                   */
  /*                                                                       */
  /*    Each node belongs to a single cache, and includes a reference      */
  /*    count to avoid destroying it (due to caching).                     */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*********                                                       *********/
  /*********        WARNING, THIS IS ALPHA CODE, THIS API          *********/
  /*********    IS DUE TO CHANGE UNTIL STRICTLY NOTIFIED BY THE    *********/
  /*********            FREETYPE DEVELOPMENT TEAM                  *********/
  /*********                                                       *********/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


#ifndef FTCMANAG_H
#define FTCMANAG_H

#include <freetype/ftcache.h>
#include <freetype/cache/ftlru.h>


#ifdef __cplusplus
  extern "C" {
#endif


#define FTC_MAX_FACES_DEFAULT  2
#define FTC_MAX_SIZES_DEFAULT  4
#define FTC_MAX_BYTES_DEFAULT  200000  /* 200kByte by default! */

  /* maximum number of caches registered in a single manager */
#define FTC_MAX_CACHES         16


 /****************************************************************
  *
  * <Struct> FTC_ManagerRec
  *
  * <Description>
  *    the cache manager structure. Each cache manager is in
  *    charge of performing two tasks:
  *
  * <Fields>
  *    library     :: handle to FreeType library instance
  *    faces_lru   :: lru list of FT_Face objects in cache
  *    sizes_lru   :: lru list of FT_Size objects in cache
  *
  *    max_bytes   :: maximum number of bytes to be allocated
  *                   in the cache. this is only related to
  *                   the byte size of the nodes cached by
  *                   the manager.
  * 
  *    num_bytes   :: current number of bytes allocated in
  *                   the cache. only related to the byte size
  *                   of cached nodes.
  *
  *    num_nodes   :: current number of nodes in the manager
  *
  *    global_lru  :: the global lru list of all cache nodes
  *
  *    caches      :: a table of installed/registered cache
  *                   objects
  *
  *    request_data :: user-provided data passed to the requester
  *    request_face :: user-provided function used to implement
  *                    a mapping between abstract FTC_FaceIDs
  *                    and real FT_Face objects..
  */
  typedef struct  FTC_ManagerRec_
  {
    FT_Library          library;
    FT_Lru              faces_lru;
    FT_Lru              sizes_lru;

    FT_ULong            max_bytes;
    FT_ULong            num_bytes;
    FT_UInt             num_nodes;
    FT_ListRec          global_lru;
    FTC_Cache           caches[FTC_MAX_CACHES];

    FT_Pointer          request_data;
    FTC_Face_Requester  request_face;

  } FTC_ManagerRec;


 /**********************************************************************
  *
  * <Function> FTC_Manager_Compress
  *
  * <Description>
  *    this function is used to check the state of the cache manager
  *    if its "num_bytes" field is greater than its "max_bytes"
  *    field, this function will flush as many old cache nodes as
  *    possible (ignoring cache nodes with a non-zero reference
  *    count).
  *
  * <input>
  *    manager ::  handle to cache manager
  *
  * <note>
  *    client applications should not call this function directly.
  *    it is normally invoked by specific cache implementations.
  *
  *    the reason this function is exported is to allow client-
  *    specific cache classes..
  *
  */
  FT_EXPORT_DEF( void )  FTC_Manager_Compress( FTC_Manager  manager );


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                   CACHE NODE DEFINITIONS                      *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* Each cache controls one or more cache nodes.  Each node is part of    */
  /* the global_lru list of the manager.  Its `data' field however is used */
  /* as a reference count for now.                                         */
  /*                                                                       */
  /* A node can be anything, depending on the type of information held by  */
  /* the cache.  It can be an individual glyph image, a set of bitmaps     */
  /* glyphs for a given size, some metrics, etc.                           */
  /*                                                                       */

  typedef FT_ListNodeRec     FTC_CacheNodeRec;
  typedef FTC_CacheNodeRec*  FTC_CacheNode;


  /* the fields `cachenode.data' is typecasted to this type */
  typedef struct  FTC_CacheNode_Data_
  {
    FT_UShort  cache_index;
    FT_Short   ref_count;

  } FTC_CacheNode_Data;

  /* return a pointer to the FTC_CacheNode_Data contained in a */
  /* CacheNode's `data' field                                  */
#define FTC_CACHENODE_TO_DATA_P( n ) \
          ( (FTC_CacheNode_Data*)&(n)->data )

#define FTC_LIST_TO_CACHENODE( n )  ( (FTC_CacheNode)(n) )

 /**********************************************************************
  *
  * <FuncType> FTC_CacheNode_SizeFunc
  *
  * <Description>
  *    a function used to compute the total size in bytes of a given
  *    cache node. It is used by the cache manager to compute the
  *    number of old nodes to flush when the cache is full..
  *
  * <Input>
  *    node       :: handle to target cache node
  *    cache_data :: a generic pointer passed to the destructor.
  */
  typedef FT_ULong  (*FTC_CacheNode_SizeFunc)( FTC_CacheNode  node,
                                               FT_Pointer     cache_data );

 /**********************************************************************
  *
  * <FuncType> FTC_CacheNode_DestroyFunc
  *
  * <Description>
  *    a function used to destroy a given cache node. It is called
  *    by the manager when the cache is full and old nodes need to
  *    be flushed out..
  *
  * <Input>
  *    node       :: handle to target cache node
  *    cache_data :: a generic pointer passed to the destructor.
  */
  typedef void  (*FTC_CacheNode_DestroyFunc)( FTC_CacheNode  node,
                                              FT_Pointer     cache_data );

 /**********************************************************************
  *
  * <Struct> FTC_CacheNode_Class
  *
  * <Description>
  *    a very simple structure used to describe a cache node's class
  *    to the cache manager
  *
  * <Fields>
  *    size_node    :: a function used to size the node
  *    destroy_node :: a function used to destroy the node
  *
  * <Note>
  *    the cache node class doesn't include a "new_node" function
  *    because the cache manager never allocates cache node directly,
  *    it delegates this task to its cache objects..
  *
  */
  typedef struct  FTC_CacheNode_Class_
  {
    FTC_CacheNode_SizeFunc     size_node;
    FTC_CacheNode_DestroyFunc  destroy_node;

  } FTC_CacheNode_Class;


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       CACHE DEFINITIONS                       *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


 /**********************************************************************
  *
  * <FuncType> FTC_Cache_InitFunc
  *
  * <Description>
  *    a function used to initialize a given cache object
  *
  * <Input>
  *    cache  :: handle to new cache
  */
  typedef FT_Error  (*FTC_Cache_InitFunc)( FTC_Cache  cache );


 /**********************************************************************
  *
  * <FuncType> FTC_Cache_DoneFunc
  *
  * <Description>
  *    a function used to finalize a given cache object
  *
  * <Input>
  *    cache  :: handle to target cache
  */
  typedef void      (*FTC_Cache_DoneFunc)( FTC_Cache  cache );


 /**********************************************************************
  *
  * <Struct> FTC_Cache_Class
  *
  * <Description>
  *    a structure used to describe a given cache object class to
  *    the cache manager.
  *
  * <Fields>
  *    cache_byte_size  :: size of cache object in bytes
  *    init_cache       :: cache object initializer
  *    done_cache       :: cache object finalizer
  */
  struct  FTC_Cache_Class_
  {
    FT_UInt             cache_byte_size;
    FTC_Cache_InitFunc  init_cache;
    FTC_Cache_DoneFunc  done_cache;
  };


 /**********************************************************************
  *
  * <Struct> FTC_CacheRec
  *
  * <Description>
  *    a structure used to describe an abstract cache object
  *
  * <Fields>
  *    manager     :: handle to parent cache manager
  *    memory      :: handle to memory manager
  *    clazz       :: pointer to cache clazz
  *    node_clazz  :: pointer to cache's node clazz
  *
  *    cache_index :: index of cache in manager's table
  *    cache_data  :: data passed to the cache node constructor/finalizer
  */
  typedef struct  FTC_CacheRec_
  {
    FTC_Manager           manager;
    FT_Memory             memory;
    FTC_Cache_Class*      clazz;
    FTC_CacheNode_Class*  node_clazz;

    FT_UInt               cache_index;  /* in manager's table           */
    FT_Pointer            cache_data;   /* passed to cache node methods */

  } FTC_CacheRec;



#ifdef __cplusplus
  }
#endif


#endif /* FTCMANAG_H */


/* END */
