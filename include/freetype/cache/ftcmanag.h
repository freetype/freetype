/***************************************************************************/
/*                                                                         */
/*  ftcmanag.h                                                             */
/*                                                                         */
/*    FreeType Cache Manager (specification).                              */
/*                                                                         */
/*  Copyright 2000-2001 by                                                 */
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
  /*    & FT_Size objects in order to speed up things considerably.        */
  /*                                                                       */
  /*  - Manage one or more cache objects.  Each cache is in charge of      */
  /*    holding a varying number of `cache nodes'.  Each cache node        */
  /*    represents a minimal amount of individually accessible cached      */
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
  /*********             WARNING, THIS IS BETA CODE.               *********/
  /*********                                                       *********/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


#ifndef __FTCMANAG_H__
#define __FTCMANAG_H__


#include <ft2build.h>
#include FT_CACHE_H
#include FT_CACHE_INTERNAL_LRU_H


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    cache_subsystem                                                    */
  /*                                                                       */
  /*************************************************************************/


#define FTC_MAX_FACES_DEFAULT  2
#define FTC_MAX_SIZES_DEFAULT  4
#define FTC_MAX_BYTES_DEFAULT  100000L  /* 200kByte by default! */

  /* maximum number of caches registered in a single manager */
#define FTC_MAX_CACHES         16


 /* handle to cache object */
  typedef struct FTC_CacheRec_*  FTC_Cache;

 /* handle to cache class */
  typedef const struct FTC_Cache_ClassRec_*  FTC_Cache_Class;

 /* handle to cache node */
  typedef struct FTC_NodeRec_*   FTC_Node;



  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FTC_ManagerRec                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The cache manager structure.                                       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    library      :: A handle to a FreeType library instance.           */
  /*                                                                       */
  /*    faces_list   :: The lru list of FT_Face objects in the cache.      */
  /*                                                                       */
  /*    sizes_list   :: The lru list of FT_Size objects in the cache.      */
  /*                                                                       */
  /*    max_weight   :: The maximum cache pool weight..                    */
  /*                                                                       */
  /*    cur_weight   :: The current cache pool weight.                     */
  /*                                                                       */
  /*    num_nodes    :: The current number of nodes in the manager.        */
  /*                                                                       */
  /*    nodes_list   :: The global lru list of all cache nodes.            */
  /*                                                                       */
  /*    caches       :: A table of installed/registered cache objects.     */
  /*                                                                       */
  /*    request_data :: User-provided data passed to the requester.        */
  /*                                                                       */
  /*    request_face :: User-provided function used to implement a mapping */
  /*                    between abstract FTC_FaceIDs and real FT_Face      */
  /*                    objects.                                           */
  /*                                                                       */
  typedef struct  FTC_ManagerRec_
  {
    FT_Library          library;
    FT_LruList          faces_list;
    FT_LruList          sizes_list;

    FT_ULong            max_weight;
    FT_ULong            cur_weight;
    
    FT_UInt             num_nodes;
    FTC_Node            nodes_list;
    
    FTC_Cache           caches[FTC_MAX_CACHES];

    FT_Pointer          request_data;
    FTC_Face_Requester  request_face;

  } FTC_ManagerRec;


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_Manager_Compress                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used to check the state of the cache manager if   */
  /*    its `num_bytes' field is greater than its `max_bytes' field.  It   */
  /*    will flush as many old cache nodes as possible (ignoring cache     */
  /*    nodes with a non-zero reference count).                            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    manager :: A handle to the cache manager.                          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Client applications should not call this function directly.  It is */
  /*    normally invoked by specific cache implementations.                */
  /*                                                                       */
  /*    The reason this function is exported is to allow client-specific   */
  /*    cache classes.                                                     */
  /*                                                                       */
  FT_EXPORT( void )
  FTC_Manager_Compress( FTC_Manager  manager );


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


 /* structure size should be 20 bytes on 32-bits machines */  
  typedef struct FTC_NodeRec_
  {
    FTC_Node   mru_next;     /* circular mru list pointer           */
    FTC_Node   mru_prev;     /* circular mru list pointer           */
    FTC_Node   link;         /* used for hashing..                  */
    FT_UInt32  hash;         /* used for hashing too..              */
    FT_UShort  cache_index;  /* index of cache the node belongs to  */
    FT_Short   ref_count;    /* reference count for this node..     */
  
  } FTC_NodeRec;

#define  FTC_NODE(x)    ((FTC_Node)(x))
#define  FTC_NODE_P(x)  ((FTC_Node*)(x))


 /* each cache really implements a dynamic hash table to manage its nodes */
  typedef struct  FTC_CacheRec_
  {
    FTC_Manager      manager;
    FT_Memory        memory;
    FTC_Cache_Class  clazz;

    FT_UInt          cache_index;  /* in manager's table         */
    FT_Pointer       cache_data;   /* used by cache node methods */

    FT_UFast         nodes;
    FT_UFast         size;
    FTC_Node*        buckets;

  } FTC_CacheRec;


#define  FTC_CACHE(x)    ((FTC_Cache)(x))
#define  FTC_CACHE_P(x)  ((FTC_Cache*)(x))


 /* initialize a given cache */
  typedef FT_Error
  (*FTC_Cache_InitFunc)( FTC_Cache   cache );
  
 /* finalize a given cache */
  typedef void
  (*FTC_Cache_DoneFunc)( FTC_Cache   cache );

 /* initialize a new cache node */
  typedef FT_Error
  (*FTC_Node_InitFunc)( FTC_Node    node,
                        FT_Pointer  type,
                        FTC_Cache   cache );

 /* compute the weight of a given cache node */
  typedef FT_ULong
  (*FTC_Node_WeightFunc)( FTC_Node    node,
                          FTC_Cache   cache );

 /* compare a node to a given key pair */
  typedef FT_Bool
  (*FTC_Node_CompareFunc)( FTC_Node   node,
                           FT_Pointer key,
                           FTC_Cache  cache );

 /* finalize a given cache node */
  typedef void
  (*FTC_Node_DoneFunc)( FTC_Node    node,
                        FTC_Cache   cache );

  typedef struct FTC_Cache_ClassRec_
  {
    FT_UInt               cache_size;
    FTC_Cache_InitFunc    cache_init;
    FTC_Cache_DoneFunc    cache_done;
    
    FT_UInt               node_size;
    FTC_Node_InitFunc     node_init;
    FTC_Node_WeightFunc   node_weight;
    FTC_Node_CompareFunc  node_compare;
    FTC_Node_DoneFunc     node_done;
  
  } FTC_Cache_ClassRec;

  /* */

#define  FTC_CACHE_RESIZE_TEST(c)            \
            ( (c)->nodes*3 < (c)->size  ||   \
              (c)->size*3  < (c)->nodes )      


  /* this must be used internally for the moment */
  FT_EXPORT( FT_Error )
  FTC_Manager_Register_Cache( FTC_Manager      manager,
                              FTC_Cache_Class  clazz,
                              FTC_Cache       *acache );


 /* can be used directory as FTC_Cache_DoneFunc, or called by custom */
 /* cache finalizers..                                               */
  FT_EXPORT( void )
  ftc_cache_done( FTC_Cache  cache );

 /* initalize the hash table within the cache */
  FT_EXPORT( FT_Error )
  ftc_cache_init( FTC_Cache  cache );

 /* can be used when FTC_CACHE_RESIZE_TEST returns TRUE after a node */
 /* insertion..                                                      */
  FT_EXPORT(void)
  ftc_cache_resize( FTC_Cache  cache );


 /* can be called when the key's hash value has been computed */
  FT_EXPORT(FT_Error)
  ftc_cache_lookup_node( FTC_Cache    cache,
                         FT_UFast     key_hash,
                         FT_Pointer   key,
                         FTC_Node    *anode );

 /* can be called to increment a node's reference count */
  FT_EXPORT(void)
  ftc_node_ref( FTC_Node   node,
                FTC_Cache  cache );

 /* can be called to decrement a node's reference count */
  FT_EXPORT(void)
  ftc_node_unref( FTC_Node   node,
                  FTC_Cache  cache );

 /* */

FT_END_HEADER

#endif /* __FTCMANAG_H__ */


/* END */
