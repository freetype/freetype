/***************************************************************************/
/*                                                                         */
/*  ftcchunk.c                                                             */
/*                                                                         */
/*    FreeType chunk cache cache (body).                                   */
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
#include FT_CACHE_INTERNAL_CHUNK_H
#include FT_LIST_H
#include FT_ERRORS_H
#include FT_INTERNAL_OBJECTS_H

#include "ftcerror.h"


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      GLYPH NODES                              *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

#define  FTC_CSET_CHUNK_INDEX(cset,gindex)   \
                  ( (gindex) / (cset)->item_count )
                  
#define  FTC_CSET_START(cset,gindex)    \
                  ( FTC_CSET_CHUNK_INDEX(cset,gindex) * (cset)->item_count )

#define  FTC_CSET_HASH(cset,gindex)  \
             ((FT_UFast)( ((cset)->hash << 16) | \
                          (FTC_CSET_CHUNK_INDEX(cset,gindex) & 0xFFFF) ))

  /* create a new chunk node, setting its cache index and ref count */
  FT_EXPORT_DEF( FT_Error )
  ftc_chunk_node_init( FTC_ChunkNode  cnode,
                       FTC_ChunkSet   cset,
                       FT_UInt        gindex,
                       FT_Bool        alloc )
  {
    FTC_ChunkCache  ccache = cset->ccache;
    FT_Error        error  = 0;
    FT_UInt         len;
    FT_UInt         start  = FTC_CSET_START(cset,gindex);

    cnode->cset       = cset;
    cnode->node.hash  = FTC_CSET_HASH(cset,gindex);
    cnode->item_start = start;

    len = cset->item_total - start;
    if ( len > cset->item_count )
      len = cset->item_count;

    cnode->item_count = len;
    
    if ( alloc )
    {
      FT_Memory  memory = ccache->cache.memory;
     
      error = MEM_Alloc( cnode->items, cset->item_size * cnode->item_count ); 
    }

    if   (!error )
      cset->num_chunks++;
      
    return error;
  }


  FT_EXPORT_DEF( void )
  ftc_chunk_node_done( FTC_ChunkNode  cnode )
  {
    FTC_ChunkSet  cset  = cnode->cset;
    FT_Memory     memory = cset->ccache->cache.memory;

    /* destroy the node */
    FREE( cnode->items );
    cnode->item_count = 0;
    cnode->item_start = 0;

    /* remove from parent set table - eventually destroy the set */
    if ( --cset->num_chunks <= 0 )
      FT_LruList_Remove( cset->ccache->cset_lru, (FT_LruNode) cset );
  }



  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      CHUNK SETS                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_EXPORT_DEF( FT_Error )
  ftc_chunk_set_init( FTC_ChunkSet    cset,
                      FT_UInt         item_size,
                      FT_UInt         item_count,
                      FT_UInt         item_total,
                      FTC_ChunkCache  cache )
  {
    cset->ccache     = cache;
    cset->num_chunks = 0;

    cset->item_total = item_total;
    cset->item_size  = item_size;
    cset->item_count = item_count;

    return 0;
  }


  FT_EXPORT_DEF( void )
  ftc_chunk_set_done( FTC_ChunkSet  cset )
  {
    /* nothing for now */
    FT_UNUSED( cset );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      CHUNK CACHES                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_EXPORT_DEF( void )
  ftc_chunk_cache_done(  FTC_ChunkCache  ccache )
  {
    ftc_cache_done( FTC_CACHE(ccache) );

    /* simply delete all remaining glyph sets */
    if ( ccache->cset_lru )
    {
      FT_LruList_Destroy( ccache->cset_lru );
      ccache->cset_lru = NULL;
    }
  }



  FT_EXPORT_DEF( FT_Error )
  ftc_chunk_cache_init( FTC_ChunkCache    ccache,
                        FT_LruList_Class  cset_class )
  {
    FT_Error  error;

    error = ftc_cache_init( FTC_CACHE(ccache) );
    if (error) goto Exit;

    error = FT_LruList_New( cset_class, 0, ccache,
                            ccache->cache.memory,
                            &ccache->cset_lru );
  Exit:
    return error;
  }



  FT_EXPORT_DEF( FT_Error )
  ftc_chunk_cache_lookup( FTC_ChunkCache   ccache,
                          FTC_ChunkQuery   query,
                          FTC_ChunkNode   *anode )
  {
    FT_LruNode    node;
    FT_Error      error;
    
    error = FT_LruList_Lookup( ccache->cset_lru, query, &node );
    if ( !error )
    {
      FTC_ChunkSet  cset  = FTC_CHUNK_SET(node);
      FT_UFast      hash  = FTC_CSET_HASH( cset, query->gindex );

      error = ftc_cache_lookup_node( FTC_CACHE(ccache), hash, query,
                                     FTC_NODE_P(anode) );
    }
    return error;
  }


/* END */
