/***************************************************************************/
/*                                                                         */
/*  ftcchunk.h                                                             */
/*                                                                         */
/*    FreeType chunk cache (specification).                                */
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
  /* Important: The functions defined in this file are only used to        */
  /*            implement an abstract chunk cache class.  You need to      */
  /*            provide additional logic to implement a complete cache.    */
  /*            For example, see `ftcmetrx.h' and `ftcmetrx.c' which       */
  /*            implement a glyph metrics cache based on this code.        */
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


#ifndef __FTCCHUNK_H__
#define __FTCCHUNK_H__


#include <ft2build.h>
#include FT_CACHE_H
#include FT_CACHE_MANAGER_H


FT_BEGIN_HEADER


  /* maximum number of chunk sets in a given chunk cache */
#define FTC_MAX_CHUNK_SETS  16


  typedef struct FTC_ChunkNodeRec_*   FTC_ChunkNode;
  typedef struct FTC_ChunkSetRec_*    FTC_ChunkSet;
  typedef struct FTC_ChunkCacheRec_*  FTC_ChunkCache;

  typedef struct  FTC_ChunkNodeRec_
  {
    FTC_NodeRec   node;
    FTC_ChunkSet  cset;
    FT_UShort     item_count;
    FT_UShort     item_start;
    FT_Byte*      items;

  } FTC_ChunkNodeRec;

#define FTC_CHUNK_NODE( x )  ((FTC_ChunkNode)( x ))

  /* a chunk set is used to categorize chunks of a given type */
  typedef struct  FTC_ChunkSetRec_
  {
    FT_LruNodeRec   lru;
    FT_UFast        hash;
    FTC_ChunkCache  ccache;
    FT_Fast         num_chunks;
    FT_UInt         item_total;   /* total number of glyphs in set   */
    FT_UInt         item_size;    /* size of each glyph item in set  */
    FT_UInt         item_count;   /* number of glyph items per chunk */
  
  } FTC_ChunkSetRec;

#define FTC_CHUNK_SET( x )  ((FTC_ChunkSet)( x ))

#define FTC_CHUNK_SET_MEMORY( x )  (( x )->ccache->cache.memory)

  /* the abstract chunk cache class */
  typedef struct  FTC_ChunkCacheRec_
  {
    FTC_CacheRec  cache;
    FT_LruList    cset_lru;  /* LRU list of chunk sets */
  
  } FTC_ChunkCacheRec;

#define FTC_CHUNK_CACHE( x )  ((FTC_ChunkCache)( x ))


  typedef struct  FTC_ChunkQueryRec_
  {
    /* input */
    FT_UInt       gindex;      /* glyph index */

    /* output */
    FTC_ChunkSet  cset;
  
  } FTC_ChunkQueryRec, *FTC_ChunkQuery;


  /*************************************************************************/
  /*                                                                       */
  /* These functions are exported so that they can be called from          */
  /* user-provided cache classes; otherwise, they are really part of the   */
  /* cache sub-system internals.                                           */
  /*                                                                       */

  FT_EXPORT( FT_Error )
  ftc_chunk_node_init( FTC_ChunkNode  node,
                       FTC_ChunkSet   cset,
                       FT_UInt        index,
                       FT_Bool        alloc );

  /* chunk set objects */

  FT_EXPORT( void )
  ftc_chunk_node_done( FTC_ChunkNode  node );


  FT_EXPORT( FT_Error )
  ftc_chunk_set_init( FTC_ChunkSet    cset,
                      FT_UInt         item_size,
                      FT_UInt         item_count,
                      FT_UInt         item_total,
                      FTC_ChunkCache  cache );

  FT_EXPORT( void )
  ftc_chunk_set_done( FTC_ChunkSet  cset );


  /* chunk cache objects */

  FT_EXPORT( FT_Error )
  ftc_chunk_cache_init( FTC_ChunkCache    cache,
                        FT_LruList_Class  cset_class );

  FT_EXPORT( void )
  ftc_chunk_cache_done( FTC_ChunkCache  cache );


  FT_EXPORT( FT_Error )
  ftc_chunk_cache_lookup( FTC_ChunkCache  cache,
                          FTC_ChunkQuery  query,
                          FTC_ChunkNode  *anode );

 /* */

FT_END_HEADER

#endif /* __FTCCHUNK_H__ */


/* END */
