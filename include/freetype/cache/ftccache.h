/***************************************************************************/
/*                                                                         */
/*  ftccache.h                                                             */
/*                                                                         */
/*    FreeType internal cache interface (specification).                   */
/*                                                                         */
/*  Copyright 2000-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTCCACHE_H__
#define __FTCCACHE_H__



FT_BEGIN_HEADER

  /* handle to cache object */
  typedef struct FTC_CacheRec_*  FTC_Cache;

  /* handle to cache class */
  typedef const struct FTC_CacheClassRec_*  FTC_CacheClass;

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
  /*************************************************************************/

  /* structure size should be 20 bytes on 32-bits machines */
  typedef struct  FTC_NodeRec_
  {
    FTC_Node   mru_next;     /* circular mru list pointer           */
    FTC_Node   mru_prev;     /* circular mru list pointer           */
    FTC_Node   link;         /* used for hashing                    */
    FT_UInt32  hash;         /* used for hashing too                */
    FT_UShort  cache_index;  /* index of cache the node belongs to  */
    FT_Short   ref_count;    /* reference count for this node       */

  } FTC_NodeRec;


#define FTC_NODE( x )    ( (FTC_Node)(x) )
#define FTC_NODE_P( x )  ( (FTC_Node*)(x) )


  /*************************************************************************/
  /*                                                                       */
  /* These functions are exported so that they can be called from          */
  /* user-provided cache classes; otherwise, they are really part of the   */
  /* cache sub-system internals.                                           */
  /*                                                                       */

  /* reserved for manager's use */
  FT_EXPORT( void )
  ftc_node_destroy( FTC_Node     node,
                    FTC_Manager  manager );



  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       CACHE DEFINITIONS                       *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* initialize a new cache node */
  typedef FT_Error    (*FTC_Node_NewFunc)( FTC_Node    *pnode,
                                           FT_Pointer   query,
                                           FTC_Cache    cache );

  typedef FT_ULong    (*FTC_Node_WeightFunc)( FTC_Node   node,
                                              FTC_Cache  cache );

  /* compare a node to a given key pair */
  typedef FT_Bool     (*FTC_Node_CompareFunc)( FTC_Node    node,
                                               FT_Pointer  key,
                                               FTC_Cache   cache );


  typedef void        (*FTC_Node_FreeFunc)( FTC_Node   node,
                                            FTC_Cache  cache );

  typedef FT_Error    (*FTC_Cache_InitFunc)( FTC_Cache   cache );

  typedef void        (*FTC_Cache_DoneFunc)( FTC_Cache   cache );


  typedef struct FTC_CacheClassRec_
  {
    FTC_Node_NewFunc        node_new;
    FTC_Node_WeightFunc     node_weight;
    FTC_Node_CompareFunc    node_compare;
    FTC_Node_CompareFunc    node_remove_faceid;
    FTC_Node_FreeFunc       node_free;

    FT_UInt                 cache_size;
    FTC_Cache_InitFunc      cache_init;
    FTC_Cache_DoneFunc      cache_done;

  } FTC_CacheClassRec;

  /* each cache really implements a dynamic hash table to manage its nodes */
  typedef struct  FTC_CacheRec_
  {
    FT_UFast             p;
    FT_UFast             mask;
    FT_Long              slack;
    FTC_Node*            buckets;

    FTC_CacheClassRec    clazz;      /* local copy, for speed */

    FTC_Manager          manager;
    FT_Memory            memory;
    FT_UInt              index;  /* in manager's table         */

    FTC_CacheClass       org_class;  /* original class pointer */

  } FTC_CacheRec;


#define FTC_CACHE( x )    ( (FTC_Cache)(x) )
#define FTC_CACHE_P( x )  ( (FTC_Cache*)(x) )


 /* default cache initialize */
  FT_EXPORT( FT_Error )
  FTC_Cache_Init( FTC_Cache       cache );

 /* default cache finalizer */
  FT_EXPORT( void )
  FTC_Cache_Done( FTC_Cache  cache );

 /* call this function to lookup the cache. if no corresponding
  * node is found, a new one is automatically created. This function
  * is capable of flushing the cache adequately to make room for the
  * new cache object.
  */
  FT_EXPORT( FT_Error )
  FTC_Cache_Lookup( FTC_Cache   cache,
                    FT_UInt32   hash,
                    FT_Pointer  query,
                    FTC_Node   *anode );

 /* remove all nodes that relate to a given face_id. This is useful
  * when un-installing fonts. Note that if a cache node relates to
  * the face_id, but is locked (i.e. has 'ref_count > 0'), the node
  * will _not_ be destroyed, but its internal face_id reference will
  * be modified.
  *
  * the end result will be that the node will never come back
  * in further lookup requests, and will be flushed on demand from
  * the cache normally when its reference count reaches 0
  */
  FT_EXPORT( void )
  FTC_Cache_RemoveFaceID( FTC_Cache    cache,
                          FTC_FaceID   face_id );

 /* */

FT_END_HEADER


#endif /* __FTCCACHE_H__ */


/* END */
