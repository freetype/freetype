/***************************************************************************/
/*                                                                         */
/*  ftcglyph.c                                                             */
/*                                                                         */
/*    FreeType Glyph Image (FT_Glyph) cache (body).                        */
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
#include FT_CACHE_INTERNAL_GLYPH_H
#include FT_ERRORS_H
#include FT_LIST_H
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_DEBUG_H

#include "ftcerror.h"


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      GLYPH NODES                              *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

#define  FTC_GSET_HASH(gset,gindex)  \
             ((FT_UFast)(((gset)->hash << 16) | ((gindex) & 0xFFFF)))


  /* create a new glyph node, setting its cache index and ref count */
  FT_EXPORT_DEF( void )
  ftc_glyph_node_init( FTC_GlyphNode  gnode,
                       FT_UInt        gindex,
                       FTC_GlyphSet   gset )
  {
    gnode->gset      = gset;
    gnode->node.hash = FTC_GSET_HASH( gset, gindex );
    gset->num_glyphs++;
  }


  /* Important: This function is called from the cache manager to */
  /* destroy a given cache node during `cache compression'.  The  */
  /* second argument is always `cache.cache_data'.  Thus be       */
  /* certain that the function FTC_Glyph_Cache_New() does indeed  */
  /* set its `cache_data' field correctly, otherwise bad things   */
  /* will happen!                                                 */

  FT_EXPORT_DEF( void )
  ftc_glyph_node_done( FTC_GlyphNode   gnode )
  {
    FTC_GlyphSet  gset = gnode->gset;
    
    if ( --gset->num_glyphs <= 0 )
      FT_LruList_Remove( gset->gcache->gset_lru, (FT_LruNode) gset );
  }



  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      GLYPH SETS                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_EXPORT_DEF( FT_Error )
  ftc_glyph_set_init( FTC_GlyphSet  gset,
                       FT_LruList   lru )
  {
    FTC_GlyphCache  gcache = lru->user_data;
    
    gset->gcache     = gcache;
    gset->num_glyphs = 0;
    
    return 0;
  }                      


  FT_EXPORT_DEF( void )
  ftc_glyph_set_done( FTC_GlyphSet  gset )
  {
    /* for now, nothing to be done here */
    FT_UNUSED(gset);
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      GLYPH CACHES                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_EXPORT_DEF( void )
  ftc_glyph_cache_done(  FTC_GlyphCache  gcache )
  {
    /* remove all nodes in the cache */
    ftc_cache_done( &gcache->cache );
  
    /* simply delete all remaining glyph sets */
    if ( gcache->gset_lru )
    {
      FT_LruList_Destroy( gcache->gset_lru );
      gcache->gset_lru = NULL;
    }
  }


  FT_EXPORT_DEF( FT_Error )
  ftc_glyph_cache_init( FTC_GlyphCache    gcache,
                        FT_LruList_Class  gset_class )
  {
    FT_Error  error;

    error = ftc_cache_init( FTC_CACHE(gcache) );
    if (error) goto Exit;
        
    error = FT_LruList_New( gset_class, 0, gcache,
                            gcache->cache.memory,
                            &gcache->gset_lru );
  Exit:
    return error;
  }


  FT_EXPORT_DEF( FT_Error )
  ftc_glyph_cache_lookup( FTC_GlyphCache  gcache,
                          FTC_GlyphQuery  query,
                          FTC_GlyphNode  *anode )
  {
    FT_LruNode    node;
    FT_Error      error;
    
    error = FT_LruList_Lookup( gcache->gset_lru, query, &node );
    if ( !error )
    {
      FTC_GlyphSet  gset = (FTC_GlyphSet) node;
      FT_UFast      hash = FTC_GSET_HASH( gset, query->gindex );
      
      error = ftc_cache_lookup_node( FTC_CACHE(gcache), hash, query,
                                     FTC_NODE_P(anode) );
    }
    return error;
  }



/* END */
