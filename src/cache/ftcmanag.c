/***************************************************************************/
/*                                                                         */
/*  ftcmanag.c                                                             */
/*                                                                         */
/*    FreeType Cache Manager (body).                                       */
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


#include <ft2build.h>
#include FT_CACHE_H
#include FT_CACHE_MANAGER_H
#include FT_CACHE_INTERNAL_LRU_H
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_DEBUG_H
#include FT_LIST_H
#include FT_SIZES_H

#include "ftcerror.h"


#undef  FT_COMPONENT
#define FT_COMPONENT  trace_cache

#define FTC_LRU_GET_MANAGER( lru )  ( (FTC_Manager)(lru)->user_data )


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    FACE LRU IMPLEMENTATION                    *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef struct FTC_FaceNodeRec_*  FTC_FaceNode;
  typedef struct FTC_SizeNodeRec_*  FTC_SizeNode;


  typedef struct  FTC_FaceNodeRec_
  {
    FT_LruNodeRec  lru;
    FT_Face        face;

  } FTC_FaceNodeRec;


  typedef struct  FTC_SizeNodeRec_
  {
    FT_LruNodeRec  lru;
    FT_Size        size;

  } FTC_SizeNodeRec;


  FT_CALLBACK_DEF( FT_Error )
  ftc_face_node_init( FTC_FaceNode  node,
                      FTC_FaceID    face_id,
                      FT_LruList    list )
  {
    FTC_Manager  manager = FTC_LRU_GET_MANAGER( list );
    FT_Error     error;


    error = manager->request_face( face_id,
                                   manager->library,
                                   manager->request_data,
                                   &node->face );
    if ( !error )
    {
      /* destroy initial size object; it will be re-created later */
      if ( node->face->size )
        FT_Done_Size( node->face->size );
    }

    return error;
  }


  /* helper function for ftc_manager_done_face() */
  FT_CALLBACK_DEF( FT_Bool )
  ftc_size_node_select( FTC_SizeNode  node,
                        FT_Face       face )
  {
    return FT_BOOL( node->size->face == face );
  }


  FT_CALLBACK_DEF( void )
  ftc_face_node_done( FTC_FaceNode  node,
                      FT_LruList    list )
  {
    FTC_Manager  manager = FTC_LRU_GET_MANAGER( list );
    FT_Face      face    = node->face;


    /* we must begin by removing all sizes for the target face */
    /* from the manager's list                                 */
    FT_LruList_Remove_Selection( manager->sizes_list,
                                 (FT_LruNode_SelectFunc)ftc_size_node_select,
                                 face );

    /* all right, we can discard the face now */
    FT_Done_Face( face );
    node->face = NULL;
  }


  FT_CALLBACK_TABLE_DEF
  const FT_LruList_ClassRec  ftc_face_list_class =
  {
    sizeof ( FT_LruListRec ),
    (FT_LruList_InitFunc)   0,
    (FT_LruList_DoneFunc)   0,

    sizeof ( FTC_FaceNodeRec ),
    (FT_LruNode_InitFunc)   ftc_face_node_init,
    (FT_LruNode_DoneFunc)   ftc_face_node_done,
    (FT_LruNode_FlushFunc)  0,  /* no flushing needed                      */
    (FT_LruNode_CompareFunc)0,  /* direct comparison of FTC_FaceID handles */
  };


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      SIZES LRU IMPLEMENTATION                 *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  typedef struct  FTC_SizeQueryRec_
  {
    FT_Face  face;
    FT_UInt  width;
    FT_UInt  height;

  } FTC_SizeQueryRec, *FTC_SizeQuery;


  FT_CALLBACK_DEF( FT_Error )
  ftc_size_node_init( FTC_SizeNode   node,
                      FTC_SizeQuery  query )
  {
    FT_Face   face = query->face;
    FT_Size   size;
    FT_Error  error;


    node->size = NULL;
    error = FT_New_Size( face, &size );
    if ( !error )
    {
      FT_Activate_Size( size );
      error = FT_Set_Pixel_Sizes( query->face,
                                  query->width,
                                  query->height );
      if ( error )
        FT_Done_Size( size );
      else
        node->size = size;
    }
    return error;
  }


  FT_CALLBACK_DEF( void )
  ftc_size_node_done( FTC_SizeNode  node )
  {
    if ( node->size )
    {
      FT_Done_Size( node->size );
      node->size = NULL;
    }
  }


  FT_CALLBACK_DEF( FT_Error )
  ftc_size_node_flush( FTC_SizeNode   node,
                       FTC_SizeQuery  query )
  {
    FT_Size   size = node->size;
    FT_Error  error;


    if ( size->face == query->face )
    {
      FT_Activate_Size( size );
      error = FT_Set_Pixel_Sizes( query->face, query->width, query->height );
      if ( error )
      {
        FT_Done_Size( size );
        node->size = NULL;
      }
    }
    else
    {
      FT_Done_Size( size );
      node->size = NULL;

      error = ftc_size_node_init( node, query );
    }
    return error;
  }


  FT_CALLBACK_DEF( FT_Bool )
  ftc_size_node_compare( FTC_SizeNode   node,
                         FTC_SizeQuery  query )
  {
    FT_Size  size = node->size;


    return FT_BOOL( size->face                    == query->face   &&
                    (FT_UInt)size->metrics.x_ppem == query->width  &&
                    (FT_UInt)size->metrics.y_ppem == query->height );
  }


  FT_CALLBACK_TABLE_DEF
  const FT_LruList_ClassRec  ftc_size_list_class =
  {
    sizeof ( FT_LruListRec ),
    (FT_LruList_InitFunc)   0,
    (FT_LruList_DoneFunc)   0,

    sizeof ( FTC_SizeNodeRec ),
    (FT_LruNode_InitFunc)   ftc_size_node_init,
    (FT_LruNode_DoneFunc)   ftc_size_node_done,
    (FT_LruNode_FlushFunc)  ftc_size_node_flush,
    (FT_LruNode_CompareFunc)ftc_size_node_compare
  };


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    CACHE MANAGER ROUTINES                     *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  /* documentation is in ftcache.h */

  FT_EXPORT_DEF( FT_Error )
  FTC_Manager_New( FT_Library          library,
                   FT_UInt             max_faces,
                   FT_UInt             max_sizes,
                   FT_ULong            max_bytes,
                   FTC_Face_Requester  requester,
                   FT_Pointer          req_data,
                   FTC_Manager        *amanager )
  {
    FT_Error     error;
    FT_Memory    memory;
    FTC_Manager  manager = 0;


    if ( !library )
      return FTC_Err_Invalid_Library_Handle;

    memory = library->memory;

    if ( ALLOC( manager, sizeof ( *manager ) ) )
      goto Exit;

    if ( max_faces == 0 )
      max_faces = FTC_MAX_FACES_DEFAULT;

    if ( max_sizes == 0 )
      max_sizes = FTC_MAX_SIZES_DEFAULT;

    if ( max_bytes == 0 )
      max_bytes = FTC_MAX_BYTES_DEFAULT;

    error = FT_LruList_New( &ftc_face_list_class,
                            max_faces,
                            manager,
                            memory,
                            &manager->faces_list );
    if ( error )
      goto Exit;

    error = FT_LruList_New( &ftc_size_list_class,
                            max_sizes,
                            manager,
                            memory,
                            &manager->sizes_list );
    if ( error )
      goto Exit;

    manager->library      = library;
    manager->max_weight   = max_bytes;
    manager->cur_weight   = 0;

    manager->request_face = requester;
    manager->request_data = req_data;

    *amanager = manager;

  Exit:
    if ( error && manager )
    {
      FT_LruList_Destroy( manager->faces_list );
      FT_LruList_Destroy( manager->sizes_list );
      FREE( manager );
    }

    return error;
  }


  /* documentation is in ftcache.h */

  FT_EXPORT_DEF( void )
  FTC_Manager_Done( FTC_Manager  manager )
  {
    FT_Memory  memory;
    FT_UInt    index;


    if ( !manager || !manager->library )
      return;

    memory = manager->library->memory;

    /* now discard all caches */
    for (index = 0; index < FTC_MAX_CACHES; index++ )
    {
      FTC_Cache  cache = manager->caches[index];


      if ( cache )
      {
        cache->clazz->cache_done( cache );
        FREE( cache );
        manager->caches[index] = 0;
      }
    }
    
    /* discard faces and sizes */
    FT_LruList_Destroy( manager->faces_list );
    manager->faces_list = 0;

    FT_LruList_Destroy( manager->sizes_list );
    manager->sizes_list = 0;

    FREE( manager );
  }


  /* documentation is in ftcache.h */

  FT_EXPORT_DEF( void )
  FTC_Manager_Reset( FTC_Manager  manager )
  {
    if ( manager )
    {
      FT_LruList_Reset( manager->sizes_list );
      FT_LruList_Reset( manager->faces_list );
    }
    /* XXX: FIXME: flush the caches? */
  }


  /* documentation is in ftcache.h */

  FT_EXPORT_DEF( FT_Error )
  FTC_Manager_Lookup_Face( FTC_Manager  manager,
                           FTC_FaceID   face_id,
                           FT_Face     *aface )
  {
    FT_Error      error;
    FTC_FaceNode  node;


    if ( aface == NULL )
      return FTC_Err_Bad_Argument;

    *aface = NULL;

    if ( !manager )
      return FTC_Err_Invalid_Cache_Handle;

    error = FT_LruList_Lookup( manager->faces_list,
                               (FT_LruKey)face_id,
                               (FT_LruNode*)&node );
    if ( !error )
      *aface = node->face;
    
    return error;
  }


  /* documentation is in ftcache.h */

  FT_EXPORT_DEF( FT_Error )
  FTC_Manager_Lookup_Size( FTC_Manager  manager,
                           FTC_Font     font,
                           FT_Face     *aface,
                           FT_Size     *asize )
  {
    FT_Error  error;


    /* check for valid `manager' delayed to FTC_Manager_Lookup_Face() */
    if ( aface )
      *aface = 0;

    if ( asize )
      *asize = 0;

    error = FTC_Manager_Lookup_Face( manager, font->face_id, aface );
    if ( !error )
    {
      FTC_SizeQueryRec  query;
      FTC_SizeNode      node;


      query.face   = *aface;
      query.width  = font->pix_width;
      query.height = font->pix_height;

      error = FT_LruList_Lookup( manager->sizes_list,
                                 (FT_LruKey)&query,
                                 (FT_LruNode*)&node );
      if ( !error )
      {
        /* select the size as the current one for this face */
        FT_Activate_Size( node->size );

        if ( asize )
          *asize = node->size;
      }
    }

    return error;
  }


  /* add a new node to the head of the manager's circular MRU list */
  static void
  ftc_node_mru_link( FTC_Node     node,
                     FTC_Manager  manager )
  {
    FTC_Node  first = manager->nodes_list;


    if ( first )
    {
      node->mru_prev = first->mru_prev;
      node->mru_next = first;

      first->mru_prev->mru_next = node;
      first->mru_prev           = node;
    }
    else
    {
      node->mru_next = node;
      node->mru_prev = node;
    }

    manager->nodes_list = node;
    manager->num_nodes++;
  }


  /* remove a node from the manager's MRU list */
  static void
  ftc_node_mru_unlink( FTC_Node     node,
                       FTC_Manager  manager )
  {
    FTC_Node  prev  = node->mru_prev;
    FTC_Node  next  = node->mru_next;
    FTC_Node  first = manager->nodes_list;


    prev->mru_next = next;
    next->mru_prev = prev;

    if ( node->mru_next == first )
    {
      /* this is the last node in the list; update its head pointer */
      if ( node == first )
        manager->nodes_list = NULL;
      else
        first->mru_prev = prev;
    }

    node->mru_next = NULL;
    node->mru_prev = NULL;
    manager->num_nodes--;
  }


  /* move a node to the head of the manager's MRU list */
  static void
  ftc_node_mru_up( FTC_Node     node,
                   FTC_Manager  manager )
  {
    FTC_Node  first = manager->nodes_list;


    if ( node != first )
    {
      ftc_node_mru_unlink( node, manager );
      ftc_node_mru_link( node, manager );
    }
  }


  /* remove a node from its cache's hash table */
  static void
  ftc_node_hash_unlink( FTC_Node   node,
                        FTC_Cache  cache )
  {
    FTC_Node  *pnode = cache->buckets + ( node->hash % cache->size );


    for (;;)
    {
      if ( *pnode == NULL )
      {
        FT_ERROR(( "FreeType.cache.hash_unlink: unknown node!\n" ));
        return;
      }

      if ( *pnode == node )
      {
        *pnode     = node->link;
        node->link = NULL;

        cache->nodes--;
        return;
      }

      pnode = &(*pnode)->link;
    }
  }


  /* add a node to the "top" of its cache's hash table */
  static void
  ftc_node_hash_link( FTC_Node   node,
                      FTC_Cache  cache )
  {
    FTC_Node  *pnode = cache->buckets + ( node->hash % cache->size );


    node->link = *pnode;
    *pnode     = node;

    cache->nodes++;
  }


  /* remove a node from the cache manager */
  static void
  ftc_node_destroy( FTC_Node     node,
                    FTC_Manager  manager )
  {
    FT_Memory        memory  = manager->library->memory;
    FTC_Cache        cache;
    FTC_Cache_Class  clazz;


#ifdef FT_DEBUG_ERROR
    /* find node's cache */
    if ( node->cache_index >= FTC_MAX_CACHES )
    {
      FT_ERROR(( "FreeType.cache.node_destroy: invalid node handle\n" ));
      return;
    }
#endif

    cache = manager->caches[node->cache_index];

#ifdef FT_DEBUG_ERROR
    if ( cache == NULL )
    {
      FT_ERROR(( "FreeType.cache.node_destroy: invalid node handle\n" ));
      return;
    }
#endif

    clazz = cache->clazz;

    manager->cur_weight -= clazz->node_weight( node, cache );

    /* remove node from mru list */
    ftc_node_mru_unlink( node, manager );

    /* remove node from cache's hash table */
    ftc_node_hash_unlink( node, cache );

    /* now finalize it */

    if ( clazz->node_done )
      clazz->node_done( node, cache );

    FREE( node );

    /* check, just in case of general corruption :-) */
    if ( manager->num_nodes <= 0 )
      FT_ERROR(( "FTC_Manager_Compress: Invalid cache node count! = %d\n",
                  manager->num_nodes ));
  }


  FT_EXPORT_DEF( void )
  FTC_Manager_Check( FTC_Manager  manager )
  {
    FTC_Node  node, first;
    

    first = manager->nodes_list;

    /* check node weights */
    if ( first )
    {
      FT_ULong  weight = 0;
      

      node = first;

      do
      {
        FTC_Cache  cache = manager->caches[node->cache_index];
        

        weight += cache->clazz->node_weight( node, cache );
        node    = node->mru_next;

      } while ( node != first );
      
      if ( weight != manager->cur_weight )
        FT_ERROR((
          "FTC_Manager_Compress: invalid weight %ld instead of %ld\n",
          manager->cur_weight, weight ));
    }

    /* check circular list */
    if ( first )
    {
      FT_UFast  count = 0;
      

      node = first;
      do
      {
        count++;
        node = node->mru_next;

      } while ( node != first );
      
      if ( count != manager->num_nodes )
        FT_ERROR((
          "FTC_Manager_Compress: invalid cache node count %d instead of %d\n",
          manager->num_nodes, count ));
    }
  }


  /* `Compress' the manager's data, i.e., get rid of old cache nodes */
  /* that are not referenced anymore in order to limit the total     */
  /* memory used by the cache.                                       */

  /* documentation is in ftcmanag.h */

  FT_EXPORT_DEF( void )
  FTC_Manager_Compress( FTC_Manager  manager )
  {
    FTC_Node   node, first;


    if ( !manager )
      return;

    first = manager->nodes_list;

#if 0
    FTC_Manager_Check( manager );

    FT_ERROR(( "compressing, weight = %ld, max = %ld, nodes = %d\n",
               manager->cur_weight, manager->max_weight,
               manager->num_nodes ));
#endif

    if ( manager->cur_weight < manager->max_weight || first == NULL )
      return;

    /* go to last node - it's a circular list */
    node = first->mru_prev;
    do
    {
      FTC_Node  prev = node->mru_prev;


      prev = ( node == first ) ? NULL : node->mru_prev;

      if ( node->ref_count <= 0 )
        ftc_node_destroy( node, manager );

      node = prev;

    } while ( node && manager->cur_weight > manager->max_weight );
  }


  FT_EXPORT_DEF( FT_Error )
  FTC_Manager_Register_Cache( FTC_Manager      manager,
                              FTC_Cache_Class  clazz,
                              FTC_Cache       *acache )
  {
    FT_Error   error = FTC_Err_Invalid_Argument;
    FTC_Cache  cache = NULL;


    if ( manager && clazz && acache )
    {
      FT_Memory  memory = manager->library->memory;
      FT_UInt    index  = 0;


      /* check for an empty cache slot in the manager's table */
      for ( index = 0; index < FTC_MAX_CACHES; index++ )
      {
        if ( manager->caches[index] == 0 )
          break;
      }

      /* return an error if there are too many registered caches */
      if ( index >= FTC_MAX_CACHES )
      {
        error = FTC_Err_Too_Many_Caches;
        FT_ERROR(( "FTC_Manager_Register_Cache:" ));
        FT_ERROR(( " too many registered caches\n" ));
        goto Exit;
      }

      if ( !ALLOC( cache, clazz->cache_size ) )
      {
        cache->manager = manager;
        cache->memory  = memory;
        cache->clazz   = clazz;

        /* THIS IS VERY IMPORTANT!  IT WILL WRETCH THE MANAGER */
        /* IF IT IS NOT SET CORRECTLY                          */
        cache->cache_index = index;

        if ( clazz->cache_init )
        {
          error = clazz->cache_init( cache );
          if ( error )
          {
            if ( clazz->cache_done )
              clazz->cache_done( cache );

            FREE( cache );
            goto Exit;
          }
        }

        manager->caches[index] = cache;
      }
    }

  Exit:
    *acache = cache;
    return error;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    ABSTRACT CACHE CLASS                       *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

#define FTC_PRIMES_MIN  7
#define FTC_PRIMES_MAX  13845163

  static const FT_UInt  ftc_primes[] =
  {
    7,
    11,
    19,
    37,
    73,
    109,
    163,
    251,
    367,
    557,
    823,
    1237,
    1861,
    2777,
    4177,
    6247,
    9371,
    14057,
    21089,
    31627,
    47431,
    71143,
    106721,
    160073,
    240101,
    360163,
    540217,
    810343,
    1215497,
    1823231,
    2734867,
    4102283,
    6153409,
    9230113,
    13845163,
  };


  static FT_UFast
  ftc_prime_closest( FT_UFast  num )
  {
    FT_UInt  i;


    for ( i = 0; i < sizeof ( ftc_primes ) / sizeof ( ftc_primes[0] ); i++ )
      if ( ftc_primes[i] > num )
        return ftc_primes[i];

    return FTC_PRIMES_MAX;
  }


  FT_EXPORT_DEF( void )
  ftc_cache_resize( FTC_Cache  cache )
  {
    FT_UFast  new_size;


    new_size = ftc_prime_closest( cache->nodes );
    if ( new_size != cache->size )
    {
      FT_Memory  memory = cache->memory;
      FT_Error   error;
      FTC_Node*  new_buckets ;
      FT_ULong   i;


      if ( ALLOC_ARRAY( new_buckets, new_size, FTC_Node ) )
        return;

      for ( i = 0; i < cache->size; i++ )
      {
        FTC_Node   node, next, *pnode;
        FT_UFast   hash;


        node = cache->buckets[i];
        while ( node )
        {
          next  = node->link;
          hash  = node->hash % new_size;
          pnode = new_buckets + hash;

          node->link = pnode[0];
          pnode[0]   = node;

          node = next;
        }
      }

      if ( cache->buckets )
        FREE( cache->buckets );

      cache->buckets = new_buckets;
      cache->size    = new_size;
    }
  }


  FT_EXPORT_DEF( FT_Error )
  ftc_cache_init( FTC_Cache  cache )
  {
    FT_Memory  memory = cache->memory;
    FT_Error   error;
    

    cache->nodes = 0;
    cache->size  = FTC_PRIMES_MIN;
    
    if ( ALLOC_ARRAY( cache->buckets, cache->size, FTC_Node ) )
      goto Exit;
      
  Exit:
    return error;
  }


  FT_EXPORT_DEF( void )
  ftc_cache_done( FTC_Cache  cache )
  {
    if ( cache )
    {
      FT_Memory        memory  = cache->memory;
      FTC_Cache_Class  clazz   = cache->clazz;
      FTC_Manager      manager = cache->manager;
      FT_UFast         i;


      for ( i = 0; i < cache->size; i++ )
      {
        FTC_Node  *pnode = cache->buckets + i, next, node = *pnode;

        while ( node )
        {
          next        = node->link;
          node->link  = NULL;

          /* remove node from mru list */
          ftc_node_mru_unlink( node, manager );

          /* now finalize it */
          manager->cur_weight -= clazz->node_weight( node, cache );

          if ( clazz->node_done )
            clazz->node_done( node, cache );

          FREE( node );
          node = next;
        }
        cache->buckets[i] = NULL;
      }

      FREE( cache->buckets );

      cache->nodes = 0;
      cache->size  = 0;
    }
  }


  /* Look up a node in "top" of its cache's hash table. */
  /* If not found, create a new node.                   */
  /*                                                    */
  FT_EXPORT_DEF( FT_Error )
  ftc_cache_lookup_node( FTC_Cache   cache,
                         FT_UFast    key_hash,
                         FT_Pointer  key,
                         FTC_Node   *anode )
  {
    FT_Error   error  = 0;
    FTC_Node   result = NULL;
    FTC_Node*  bucket = cache->buckets + ( key_hash % cache->size );


    if ( *bucket )
    {
      FTC_Node*             pnode   = bucket;
      FTC_Node_CompareFunc  compare = cache->clazz->node_compare;


      for (;;)
      {
        FTC_Node  node;


        node = *pnode;
        if ( node == NULL )
          break;

        if ( compare( node, key, cache ) )
        {
          /* move to head of bucket list */
          if ( pnode != bucket )
          {
            *pnode     = node->link;
            node->link = *bucket;
            *bucket    = node;
          }
          
          /* move to head of MRU list */
          if ( node != cache->manager->nodes_list )
            ftc_node_mru_up( node, cache->manager );

          result = node;
          goto Exit;
        }

        pnode = &(*pnode)->link;
      }
    }

    /* didn't find a node, create a new one */
    {
      FTC_Cache_Class  clazz   = cache->clazz;
      FTC_Manager      manager = cache->manager;
      FT_Memory        memory  = cache->memory;
      FTC_Node         node;


      if ( ALLOC( node, clazz->node_size ) )
        goto Exit;

      /* node initializer must set 'hash' field */

      node->cache_index = cache->cache_index;
      node->ref_count   = 0;

      error = clazz->node_init( node, key, cache );
      if ( error )
      {
        FREE( node );
        goto Exit;
      }

      ftc_node_hash_link( node, cache );
      ftc_node_mru_link( node, cache->manager );

      cache->manager->cur_weight += clazz->node_weight( node, cache );

      /* now try to compress the node pool when necessary */
      if ( manager->cur_weight >= manager->max_weight )
      {
        node->ref_count++;
        FTC_Manager_Compress( manager );
        node->ref_count--;
      }

      /* try to resize the hash table when appropriate */
      if ( FTC_CACHE_RESIZE_TEST( cache ) )
        ftc_cache_resize( cache );

      result = node;
    }

  Exit:
   *anode = result;
    return error;
  }


  /* maybe the next two functions will disappear eventually */

  FT_EXPORT_DEF( void )
  ftc_node_ref( FTC_Node   node,
                FTC_Cache  cache )
  {
    if ( node && cache && (FT_UInt)node->cache_index == cache->cache_index )
      node->ref_count++;
  }


  FT_EXPORT_DEF( void )
  ftc_node_unref( FTC_Node   node,
                  FTC_Cache  cache )
  {
    if ( node && cache && (FT_UInt)node->cache_index == cache->cache_index )
      node->ref_count--;
  }


/* END */
