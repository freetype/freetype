/***************************************************************************/
/*                                                                         */
/*  ftcglyph.c                                                             */
/*                                                                         */
/*    FreeType Glyph Image (FT_Glyph) cache (body).                        */
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
  /*  Note: The implementation of glyph queues is rather generic in this   */
  /*        code.  This will allow other glyph node/cache types to be      */
  /*        easily included in the future.  For now, we only cache glyph   */
  /*        images.                                                        */
  /*                                                                       */
  /*************************************************************************/


#include <freetype/cache/ftcglyph.h>
#include <freetype/fterrors.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftlist.h>
#include <freetype/fterrors.h>


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      GLYPH NODES                              *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  /* In the future, we might provide a better scheme for managing glyph  */
  /* node elements.  For the moment, we simply use FT_Alloc()/FT_Free(). */


  /* create a new glyph node, setting its cache index and ref count */
  FT_EXPORT_FUNC( void )  FTC_GlyphNode_Init( FTC_GlyphNode    node,
                                              FTC_Glyph_Queue  queue,
                                              FT_UInt          gindex )
  {
    FTC_Glyph_Cache      cache = queue->cache;
    FTC_CacheNode_Data*  data  = FTC_CACHENODE_TO_DATA_P( &node->root );


    data->cache_index = (FT_UShort)cache->root.cache_index;
    data->ref_count   = (FT_Short) 0;
    node->queue_index = (FT_UShort)queue->queue_index;
    node->glyph_index = (FT_UShort)gindex;
  }


  /* Important: This function is called from the cache manager to */
  /* destroy a given cache node during `cache compression'.  The  */
  /* second argument is always `cache.user_data'.  Thus be        */
  /* certain that the function FTC_Glyph_Cache_New() does indeed  */
  /* set its `user_data' field correctly, otherwise bad things    */
  /* will happen!                                                 */

  FT_EXPORT_FUNC( void )  FTC_GlyphNode_Destroy( FTC_GlyphNode    node,
                                                 FTC_Glyph_Cache  cache )
  {
    FT_LruNode       queue_lru = cache->queues_lru->nodes + node->queue_index;
    FTC_Glyph_Queue  queue     = (FTC_Glyph_Queue)queue_lru->root.data;
    FT_UInt          hash      = node->glyph_index % queue->hash_size;
    
    /* remove the node from its queue's bucket list */
    {
      FTC_GlyphNode*  pnode = queue->buckets + hash;
      FTC_GlyphNode   cur;
      
      for (;;)
      {
        cur = *pnode;
        if (!cur)
        {
          /* that's very strange, this should not happen !! */
          FT_ERROR(( "FTC_GlyphNode_Destroy:"
                     " trying to delete an unlisted node !!!!" ));
          return;
        }
          
        if (cur == node)
        {
          *pnode = cur->queue_next;
          break;
        }
        pnode = &cur->queue_next;
      }
    }

    /* destroy the node */
    queue->clazz->destroy_node( node, queue );
  }


  /* Important: This function is called from the cache manager to */
  /* size a given cache node during `cache compression'.  The     */
  /* second argument is always `cache.user_data'.  Thus be        */
  /* certain that the function FTC_Glyph_Cache_New() does indeed  */
  /* set its `user_data' field correctly, otherwise bad things    */
  /* will happen!                                                 */

  FT_EXPORT_FUNC( FT_ULong )  FTC_GlyphNode_Size( FTC_GlyphNode    node,
                                                  FTC_Glyph_Cache  cache )
  {
    FT_LruNode       queue_lru = cache->queues_lru->nodes + node->queue_index;
    FTC_Glyph_Queue  queue     = (FTC_Glyph_Queue)queue_lru->root.data;


    return queue->clazz->size_node( node, queue );
  }


  FT_CPLUSPLUS( const FTC_CacheNode_Class )  ftc_glyph_cache_node_class =
  {
    (FTC_CacheNode_SizeFunc)   FTC_GlyphNode_Size,
    (FTC_CacheNode_DestroyFunc)FTC_GlyphNode_Destroy
  };


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      GLYPH QUEUES                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_EXPORT_FUNC( FT_Error )  FTC_Glyph_Queue_New( FTC_Glyph_Cache   cache,
                                                   FT_Pointer        type,
                                                   FTC_Glyph_Queue*  aqueue )
  {
    FT_Error         error;
    FT_Memory        memory  = cache->root.memory;
    FTC_Manager      manager = cache->root.manager;
    FTC_Glyph_Queue  queue   = 0;

    FTC_Glyph_Cache_Class*  gcache_class;
    FTC_Glyph_Queue_Class*  clazz;


    gcache_class = (FTC_Glyph_Cache_Class*)cache->root.clazz;
    clazz        = gcache_class->queue_class;

    *aqueue = 0;

    if ( ALLOC( queue, clazz->queue_byte_size ) )
      goto Exit;

    queue->cache     = cache;
    queue->manager   = manager;
    queue->memory    = memory;
    queue->hash_size = FTC_QUEUE_HASH_SIZE_DEFAULT;
    queue->clazz     = clazz;

    /* allocate buckets table */
    if ( ALLOC_ARRAY( queue->buckets, queue->hash_size, FTC_GlyphNode ) )
      goto Exit;

    /* initialize queue by type if needed */
    if ( clazz->init )
    {
      error = clazz->init( queue, type );
      if ( error )
        goto Exit;
    }

    *aqueue = queue;

  Exit:
    if ( error && queue )
    {
      FREE( queue->buckets );
      FREE( queue );
    }

    return error;
  }


  FT_EXPORT_FUNC( void )  FTC_Glyph_Queue_Done( FTC_Glyph_Queue  queue )
  {
    FTC_Glyph_Cache         cache        = queue->cache;
    FTC_Manager             manager      = cache->root.manager;
    FT_List                 glyphs_lru   = &manager->global_lru;
    FTC_GlyphNode*          bucket       = queue->buckets;
    FTC_GlyphNode*          bucket_limit = bucket + queue->hash_size;
    FT_Memory               memory       = cache->root.memory;

    FTC_Glyph_Queue_Class*  clazz = queue->clazz;


    /* for each bucket, free the list of glyph nodes */
    for ( ; bucket < bucket_limit; bucket++ )
    {
      FTC_GlyphNode   node = bucket[0];
      FTC_GlyphNode   next = 0;
      FT_ListNode     lrunode;


      for ( ; node; node = next )
      {
        next    = node->queue_next;
        lrunode = FTC_GLYPHNODE_TO_LRUNODE( node );

        manager->num_bytes -= clazz->size_node( node, queue );

        FT_List_Remove( glyphs_lru, lrunode );

        clazz->destroy_node( node, queue );
      }

      bucket[0] = 0;
    }

    if ( clazz->done )
      clazz->done( queue );

    FREE( queue->buckets );
    FREE( queue );
  }


  FT_EXPORT_FUNC( FT_Error )
  FTC_Glyph_Queue_Lookup_Node( FTC_Glyph_Queue  queue,
                               FT_UInt          glyph_index,
                               FTC_GlyphNode*   anode )
  {
    FTC_Glyph_Cache         cache      = queue->cache;
    FTC_Manager             manager    = cache->root.manager;
    FT_UInt                 hash_index = glyph_index % queue->hash_size;
    FTC_GlyphNode*          bucket     = queue->buckets + hash_index;
    FTC_GlyphNode*          pnode      = bucket;
    FTC_GlyphNode           node;
    FT_Error                error;

    FTC_Glyph_Queue_Class*  clazz = queue->clazz;


    *anode = 0;
    
    for ( ;; )
    {
      node = *pnode;
      if (!node)
        break;
        
      if ( node->glyph_index == glyph_index )
      {
        /* we found it! -- move glyph to start of the lists */
        *pnode           = node->queue_next;
        node->queue_next = bucket[0];
        bucket[0]        = node;
        
        FT_List_Up( &manager->global_lru, FTC_GLYPHNODE_TO_LRUNODE( node ) );
        *anode = node;
        return 0;
      }
      /* go to next node in bucket */
      pnode = &node->queue_next;
    }

    /* we didn't found the glyph image, we will now create a new one */
    error = clazz->new_node( queue, glyph_index, &node );
    if ( error )
      goto Exit;

    /* insert the node at the start of our bucket list */
    node->queue_next = bucket[0];
    bucket[0]        = node;

    /* insert the node at the start the global LRU glyph list */
    FT_List_Insert( &manager->global_lru, FTC_GLYPHNODE_TO_LRUNODE( node ) );

    manager->num_bytes += clazz->size_node( node, queue );

    if (manager->num_bytes > manager->max_bytes)
      FTC_Manager_Compress( manager );

    *anode = node;

  Exit:
    return error;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                   GLYPH QUEUES LRU CALLBACKS                  *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


#define FTC_QUEUE_LRU_GET_CACHE( lru )   \
          ( (FTC_Glyph_Cache)(lru)->user_data )

#define FTC_QUEUE_LRU_GET_MANAGER( lru ) \
          FTC_QUEUE_LRU_GET_CACHE( lru )->manager

#define FTC_LRUNODE_QUEUE( node )        \
          ( (FTC_Glyph_Queue)(node)->root.data )


  LOCAL_FUNC_X
  FT_Error  ftc_glyph_queue_lru_init( FT_Lru      lru,
                                      FT_LruNode  node )
  {
    FTC_Glyph_Cache  cache = FTC_QUEUE_LRU_GET_CACHE( lru );
    FT_Error         error;
    FTC_Glyph_Queue  queue;


    error = FTC_Glyph_Queue_New( cache,
                                 (FT_Pointer)node->key,
                                 &queue );
    if ( !error )
    {
      /* good, now set the queue index within the queue object */
      queue->queue_index = node - lru->nodes;
      node->root.data    = queue;
    }

    return error;
  }


  LOCAL_FUNC_X
  void  ftc_glyph_queue_lru_done( FT_Lru      lru,
                                  FT_LruNode  node )
  {
    FTC_Glyph_Queue  queue = FTC_LRUNODE_QUEUE( node );

    FT_UNUSED( lru );


    FTC_Glyph_Queue_Done( queue );
  }


  LOCAL_FUNC_X
  FT_Bool  ftc_glyph_queue_lru_compare( FT_LruNode  node,
                                        FT_LruKey   key )
  {
    FTC_Glyph_Queue  queue = FTC_LRUNODE_QUEUE( node );


    return queue->clazz->compare( queue, (FT_Pointer)key );
  }


  FT_CPLUSPLUS( const FT_Lru_Class )  ftc_glyph_queue_lru_class =
  {
    sizeof( FT_LruRec ),
    ftc_glyph_queue_lru_init,
    ftc_glyph_queue_lru_done,
    0,  /* no flush */
    ftc_glyph_queue_lru_compare
  };


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                GLYPH IMAGE CACHE OBJECTS                      *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_EXPORT_FUNC( FT_Error )  FTC_Glyph_Cache_Init( FTC_Glyph_Cache  cache )
  {
    FT_Memory  memory = cache->root.memory;
    FT_Error   error;


    /* set up root node_class to be used by manager */
    cache->root.node_clazz =
      (FTC_CacheNode_Class*)&ftc_glyph_cache_node_class;

    /* The following is extremely important for ftc_destroy_glyph_image() */
    /* to work properly, as the second parameter that is sent to it       */
    /* through the cache manager is `user_data' and must be set to        */
    /* `cache' here.                                                      */
    /*                                                                    */
    cache->root.cache_user = cache;

    error = FT_Lru_New( &ftc_glyph_queue_lru_class,
                        FTC_MAX_GLYPH_QUEUES,
                        cache,
                        memory,
                        1, /* pre_alloc == TRUE */
                        &cache->queues_lru );
    return error;
  }


  FT_EXPORT_FUNC( void )  FTC_Glyph_Cache_Done( FTC_Glyph_Cache  cache )
  {
    /* discard glyph queues */
    FT_Lru_Done( cache->queues_lru );
  }


/* END */
