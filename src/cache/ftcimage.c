/***************************************************************************/
/*                                                                         */
/*  ftcimage.c                                                             */
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


#include <cache/ftcimage.h>
#include <freetype/fterrors.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftlist.h>

 /***********************************************************************/
 /***********************************************************************/
 /*****                                                             *****/
 /*****                      IMAGE NODE MANAGEMENT                  *****/
 /*****                                                             *****/
 /*****  For now, we simply ALLOC/FREE the FTC_ImageNode. However,  *****/
 /*****  it certainly is a good idea to use a chunk manager in the  *****/
 /*****  future in order to reduce memory waste/fragmentation       *****/
 /*****                                                             *****/
 /***********************************************************************/
 /***********************************************************************/
 

  static
  FT_Error  FTC_ImageNode_New( FTC_Image_Cache   cache,
                               FTC_ImageNode    *anode )
  {
    FT_Error       error;
    FT_Memory      memory = cache->memory;
    FTC_ImageNode  node;
    
    *anode = 0;
    if (!ALLOC( node, sizeof(*node) ))
      *anode = node;
      
    return error;
  }                                      


  static
  void    FTC_ImageNode_Done( FTC_Image_Cache   cache,
                              FTC_ImageNode     node )
  {
    /* for now, we simply discard the node. We may later add a chunk */
    /* manager to the image cache..                                  */
    FT_Memory  memory = cache->memory;
    FREE( node );
  }                                     



 /***********************************************************************/
 /***********************************************************************/
 /*****                                                             *****/
 /*****                      GLYPH IMAGE QUEUES                     *****/
 /*****                                                             *****/
 /***********************************************************************/
 /***********************************************************************/
 

  static
  void  ftc_done_glyph_image( FTC_Image_Queue  queue,
                              FTC_ImageNode    node )
  {
    FT_UNUSED( queue );

    FT_Done_Glyph( FTC_IMAGENODE_GET_GLYPH( node ) );
  }


  static
  FT_ULong  ftc_size_bitmap_image( FTC_Image_Queue  queue,
                                   FTC_ImageNode    node )
  {
    FT_Long         pitch;
    FT_BitmapGlyph  glyph;
    
    FT_UNUSED( queue );


    glyph = (FT_BitmapGlyph)FTC_IMAGENODE_GET_GLYPH(node);
    pitch = glyph->bitmap.pitch;
    if ( pitch < 0 )
      pitch = -pitch;
      
    return (FT_ULong)(pitch * glyph->bitmap.rows + sizeof ( *glyph ) );
  }


  static
  FT_ULong  ftc_size_outline_image( FTC_Image_Queue  queue,
                                    FTC_ImageNode    node )
  {
    FT_OutlineGlyph  glyph;
    FT_Outline*      outline;
    
    FT_UNUSED( queue );


    glyph   = (FT_OutlineGlyph)FTC_IMAGENODE_GET_GLYPH( node );
    outline = &glyph->outline;
    
    return (FT_ULong)( 
      outline->n_points *  ( sizeof ( FT_Vector ) + sizeof ( FT_Byte ) ) +
      outline->n_contours * sizeof ( FT_Short )                          +
      sizeof( *glyph ) );
  }


  static
  FT_Error  ftc_init_mono_image( FTC_Image_Queue  queue,
                                 FTC_ImageNode    node )
  {  
    FT_Face   face;
    FT_Size   size;
    FT_Error  error;

    error = FTC_Manager_Lookup_Size( queue->manager,
                                     &queue->descriptor.size,
                                     &face, &size );
    if ( !error )
    {
      FT_UInt  glyph_index = FTC_IMAGENODE_GET_GINDEX( node );

      
      error = FT_Load_Glyph( face, glyph_index,
                             FT_LOAD_RENDER | FT_LOAD_MONOCHROME );
      if ( !error )
      {
        if ( face->glyph->format            != ft_glyph_format_bitmap ||
             face->glyph->bitmap.pixel_mode != ft_pixel_mode_mono     )
        {
          /* there is no monochrome glyph for this font! */
          error = FT_Err_Invalid_Glyph_Index;
        }
        else
        {
          /* ok, copy it */
          FT_Glyph  glyph;
          
          
          error = FT_Get_Glyph( face->glyph, &glyph );
          if ( !error )
            FTC_IMAGENODE_SET_GLYPH( node, glyph );
        }
      }
    }
    return error;
  }


  static
  FT_Error  ftc_init_gray_image( FTC_Image_Queue  queue,
                                 FTC_ImageNode    node )
  {  
    FT_Face   face;
    FT_Size   size;
    FT_Error  error;
    

    error = FTC_Manager_Lookup_Size( queue->manager,
                                     &queue->descriptor.size,
                                     &face, &size );
    if ( !error )
    {
      FT_UInt  glyph_index = FTC_IMAGENODE_GET_GINDEX( node );
      

      error = FT_Load_Glyph( face, glyph_index,
                             FT_LOAD_RENDER );
      if ( !error )
      {
        if ( face->glyph->format            != ft_glyph_format_bitmap ||
             face->glyph->bitmap.pixel_mode != ft_pixel_mode_grays )
        {
          /* there is no anti-aliased glyph for this font! */
          error = FT_Err_Invalid_Glyph_Index;
        }
        else
        {
          /* ok, copy it */
          FT_Glyph  glyph;
          
          
          error = FT_Get_Glyph( face->glyph, &glyph );
          if ( !error )
            FTC_IMAGENODE_SET_GLYPH( node, glyph );
        }
      }
    }
    return error;
  }


  static
  FT_Error  ftc_init_outline_image( FTC_Image_Queue  queue,
                                    FTC_ImageNode    node )
  {  
    FT_Face   face;
    FT_Size   size;
    FT_Error  error;
    

    error = FTC_Manager_Lookup_Size( queue->manager,
                                     &queue->descriptor.size,
                                     &face, &size );
    if ( !error )
    {
      FT_UInt  glyph_index = FTC_IMAGENODE_GET_GINDEX( node );
      

      error = FT_Load_Glyph( face, glyph_index,
                             FT_LOAD_NO_BITMAP );
      if ( !error )
      {
        if ( face->glyph->format != ft_glyph_format_outline )
        {
          /* there is no outline glyph for this font! */
          error = FT_Err_Invalid_Glyph_Index;
        }
        else
        {
          /* ok, copy it */
          FT_Glyph  glyph;
          
          
          error = FT_Get_Glyph( face->glyph, &glyph );
          if ( !error )
            FTC_IMAGENODE_SET_GLYPH( node, glyph );
        }
      }
    }
    return error;
  }



  static
  FT_Error  ftc_init_master_outline_image( FTC_Image_Queue  queue,
                                           FTC_ImageNode    node )
  {  
    FT_Face   face;
    FT_Size   size;
    FT_Error  error;
    

    error = FTC_Manager_Lookup_Size( queue->manager,
                                     &queue->descriptor.size,
                                     &face, &size );
    if ( !error )
    {
      FT_UInt  glyph_index = FTC_IMAGENODE_GET_GINDEX( node );
      

      error = FT_Load_Glyph( face, glyph_index,
                             FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP );
      if ( !error )
      {
        if ( face->glyph->format != ft_glyph_format_outline )
        {
          /* there is no outline glyph for this font! */
          error = FT_Err_Invalid_Glyph_Index;
        }
        else
        {
          /* ok, copy it */
          FT_Glyph  glyph;
          
          
          error = FT_Get_Glyph( face->glyph, &glyph );
          if ( !error )
            FTC_IMAGENODE_SET_GLYPH( node, glyph );
        }
      }
    }
    return error;
  }


  static
  const FTC_Image_Class   ftc_mono_image_class =
  {
    ftc_init_mono_image,
    ftc_done_glyph_image,
    ftc_size_bitmap_image
  };
  
  static
  const FTC_Image_Class   ftc_gray_image_class =
  {
    ftc_init_gray_image,
    ftc_done_glyph_image,
    ftc_size_bitmap_image
  };
  
  static
  const FTC_Image_Class   ftc_outline_image_class =
  {
    ftc_init_outline_image,
    ftc_done_glyph_image,
    ftc_size_outline_image
  };
  
  static
  const FTC_Image_Class   ftc_master_outline_image_class =
  {
    ftc_init_master_outline_image,
    ftc_done_glyph_image,
    ftc_size_outline_image
  };


  static
  FT_Error   FTC_Image_Queue_New( FTC_Image_Cache   cache,
                                  FTC_Image_Desc*   desc,
                                  FTC_Image_Queue  *aqueue )
  {
    FT_Error         error;
    FT_Memory        memory = cache->memory;
    FTC_Manager      manager = cache->manager;
    FTC_Image_Queue  queue = 0;
    
    const FTC_Image_Class* clazz;
    
    *aqueue = 0;
    if ( ALLOC( queue, sizeof(*queue) ) )
      goto Exit;
    
    queue->cache      = cache;
    queue->manager    = manager;
    queue->memory     = memory;
    queue->descriptor = *desc;
    queue->hash_size  = 32;
    
    if ( ALLOC_ARRAY( queue->buckets, queue->hash_size, FT_ListRec ) )
      goto Exit;

    switch (desc->image_type)
    {
      case ftc_image_mono:
        clazz = &ftc_mono_image_class;
        break;
        
      case ftc_image_grays:
        clazz = &ftc_gray_image_class;
        break;
        
      case ftc_image_outline:
        clazz = &ftc_outline_image_class;
        break;
        
      case ftc_image_master_outline:
        clazz = &ftc_master_outline_image_class;
        break;
        
      default:
        /* invalid image type !! */
        error = FT_Err_Invalid_Argument;
        goto Exit;
    }

    queue->clazz = (FTC_Image_Class*)clazz;
    *aqueue = queue;
  Exit:
    if (error)
      FREE( queue );

    return error;
  }                                  


  static
  void   FTC_Image_Queue_Done( FTC_Image_Queue  queue )
  {
    FTC_Image_Cache  cache        = queue->cache;
    FT_List          glyphs_lru   = &cache->glyphs_lru;
    FT_List          bucket       = queue->buckets;
    FT_List          bucket_limit = bucket + queue->hash_size;
    FT_Memory        memory       = cache->memory;
    
    /* for each bucket, free the list of image nodes */
    for ( ; bucket < bucket_limit; bucket++ )
    {
      FT_ListNode    node = bucket->head;
      FT_ListNode    next = 0;
      FT_ListNode    lrunode;
      FTC_ImageNode  inode;
      
      for ( ; node; node = next )
      {
        next    = node->next;
        inode   = (FTC_ImageNode)node;
        lrunode = FTC_IMAGENODE_TO_LISTNODE(inode);
        
        queue->clazz->done_image( queue, inode );
        FT_List_Remove( glyphs_lru, lrunode );
        
        FTC_ImageNode_Done( cache, inode );
      }
      
      bucket->head = bucket->tail = 0;
    }
    FREE( queue->buckets );
    FREE( queue );
  }


  static
  FT_Error  FTC_Image_Queue_Lookup_Node( FTC_Image_Queue  queue,
                                         FT_UInt          glyph_index,
                                         FTC_ImageNode   *anode )
  {
    FTC_Image_Cache  cache      = queue->cache;
    FT_UInt          hash_index = glyph_index % queue->hash_size;
    FT_List          bucket     = queue->buckets + hash_index;
    FT_ListNode      node, next;
    FT_Error         error;
    FTC_ImageNode    inode;
    
    *anode = 0;
    for ( node = bucket->head; node; node = next )
    {
      inode = (FTC_ImageNode)node;
      
      if ( FTC_IMAGENODE_GET_GINDEX(inode) == glyph_index )
      {
        /* we found it !! - move glyph to start of the list */
        FT_List_Up( bucket, node );
        *anode = inode;
        return 0;
      }
    }
    
    /* we didn't found the glyph image, we will now create a new one */
    error = FTC_ImageNode_New( queue->cache, &inode );
    if (error)
      goto Exit;
    
    error = queue->clazz->init_image( queue, inode );
    if (error)
    {
      FTC_ImageNode_Done( queue->cache, inode );
      goto Exit;
    }
    
    /* set the glyph and queue indices in the image node */
    FTC_IMAGENODE_SET_INDICES( inode, glyph_index, queue->index );
    
    /* insert the node at the start of our bucket list */
    FT_List_Insert( bucket, (FT_ListNode)inode );
    
    /* insert the node at the start the global LRU glyph list */
    FT_List_Insert( &cache->glyphs_lru, FTC_IMAGENODE_TO_LISTNODE(inode) );
    
    cache->num_bytes += queue->clazz->size_image(queue,inode);

    *anode = inode;

  Exit:
    return error;
  }


  
 /***********************************************************************/
 /***********************************************************************/
 /*****                                                             *****/
 /*****                         IMAGE CACHE CALLBACKS               *****/
 /*****                                                             *****/
 /***********************************************************************/
 /***********************************************************************/
 
#define  FTC_QUEUE_LRU_GET_CACHE(lru)    ((FTC_Image_Cache)(lru)->user_data)
#define  FTC_QUEUE_LRU_GET_MANAGER(lru)  FTC_QUEUE_LRU_GET_CACHE(lru)->manager
#define  FTC_LRUNODE_QUEUE(node)         ((FTC_Image_Queue)(node)->root.data)

  static
  FT_Error   ftc_image_cache_init_queue( FT_Lru      lru,
                                         FT_LruNode  node )
  {
    FTC_Image_Cache  cache   = FTC_QUEUE_LRU_GET_CACHE(lru);
    FTC_Image_Desc*  desc    = (FTC_Image_Desc*)node->key;
    FT_Error         error;
    FTC_Image_Queue  queue;
    
    error = FTC_Image_Queue_New( cache, desc, &queue );
    if (!error)
    {
      /* good, now set the queue index within the queue object */
      queue->index    = node - lru->nodes;
      node->root.data = queue;
    }
    
    return error;
  }


  static
  void       ftc_image_cache_done_queue( FT_Lru      lru,
                                         FT_LruNode  node )
  {
    FTC_Image_Queue  queue = FTC_LRUNODE_QUEUE(node);
    
    FT_UNUSED(lru);
    FTC_Image_Queue_Done(queue);
  }


  static
  FT_Bool    ftc_image_cache_compare_queue( FT_LruNode  node,
                                            FT_LruKey   key )
  {
    FTC_Image_Queue  queue = FTC_LRUNODE_QUEUE(node);
    FTC_Image_Desc*  desc2 = (FTC_Image_Desc*)key;
    FTC_Image_Desc*  desc1 = &queue->descriptor;
    
    return  ( desc1->size.face_id    == desc2->size.face_id    &&
              desc1->size.pix_width  == desc2->size.pix_width  &&
              desc1->size.pix_height == desc2->size.pix_height &&
              desc1->image_type      == desc2->image_type      );
  }                                            



  static
  const FT_Lru_Class   ftc_image_queue_lru_class =
  {
    sizeof( FT_LruRec ),
    ftc_image_cache_init_queue,
    ftc_image_cache_done_queue,
    0,  /* no flush */
    ftc_image_cache_compare_queue
  };





 /* compress image cache if it needs it.. i.e. discards all old glyph images    */
 /* until "cache.num_bytes" is under "cache.max_bytes". Note that this function */
 /* will avoid to remove "new_node"..                                           */
  static
  void    FTC_Image_Cache_Compress( FTC_Image_Cache  cache,
                                    FTC_ImageNode    new_node )
  {
    while (cache->num_bytes > cache->max_bytes)
    {
      FT_ListNode      cur = cache->glyphs_lru.tail;
      FTC_Image_Queue  queue;
      FT_UInt          glyph_index;
      FT_UInt          hash_index;
      FT_UInt          queue_index;
      FT_ULong         size;
      FTC_ImageNode    inode;
      
      /* exit our loop if there isn't any glyph image left, or if  */
      /* we reached the newly created node (which is always at the */
      /* start of the list..)                                      */
      
      inode = FTC_LISTNODE_TO_IMAGENODE(cur);
      if ( !cur || inode == new_node )
        break;
        
      glyph_index = FTC_IMAGENODE_GET_GINDEX(inode);
      queue_index = FTC_IMAGENODE_GET_QINDEX(inode);
      queue       = (FTC_Image_Queue)cache->queues_lru->nodes[queue_index].root.data;
      hash_index  = glyph_index % queue->hash_size;
      size        = queue->clazz->size_image( queue, inode );

      FT_List_Remove( &cache->glyphs_lru, cur );
      FT_List_Remove( queue->buckets + hash_index, (FT_ListNode)inode );
      queue->clazz->done_image( queue, inode );
      FTC_ImageNode_Done( cache, inode );
      
      cache->num_bytes -= size;
    }
  }                                    
  

  FT_EXPORT_DEF( FT_Error )  FTC_Image_Cache_New( FTC_Manager       manager,
                                                  FT_ULong          max_bytes,
                                                  FTC_Image_Cache*  acache )
  {
    FT_Error         error;
    FT_Memory        memory;
    FTC_Image_Cache  cache;
    
    
    *acache = 0;
    memory  = manager->library->memory;
    
    if ( ALLOC( cache, sizeof(*cache) ) )
      goto Exit;
    
    cache->manager   = manager;
    cache->max_bytes = max_bytes;

    error = FT_Lru_New( &ftc_image_queue_lru_class,
                        FTC_MAX_IMAGE_QUEUES,
                        cache,
                        memory,
                        1, /* pre_alloc == TRUE */
                        &cache->queues_lru );
    if (error)
      goto Exit;                        
    
    *acache = cache;

  Exit:
    if ( error )
      FREE( cache );
      
    return error;
  }                                                  



  FT_EXPORT_DEF( void )  FTC_Image_Cache_Done( FTC_Image_Cache  cache )
  {
    FT_Memory  memory = cache->memory;
    
    
    /* discard image queues */
    FT_Lru_Done( cache->queues_lru );
    
    /* discard cache */
    FREE( cache );
  }



  FT_EXPORT_DEF( FT_Error )  FTC_Image_Cache_Lookup(
                               FTC_Image_Cache  cache,
                               FTC_Image_Desc*  desc,
                               FT_UInt          gindex,
                               FT_Glyph*        aglyph )
  {
    FT_Error         error;
    FTC_Image_Queue  queue;
    FTC_ImageNode    inode;

    *aglyph = 0;    
    error   = FT_Lru_Lookup( cache->queues_lru,
                             (FT_LruKey)desc,
                             (FT_Pointer*)&queue );
    if (error)
      goto Exit;

    error = FTC_Image_Queue_Lookup_Node( queue, gindex, &inode );
    if (error)
      goto Exit;

    FTC_Image_Cache_Compress( cache, inode );
    *aglyph = FTC_IMAGENODE_GET_GLYPH(inode);

  Exit:
    return error;
  }


/* END */
