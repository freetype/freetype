/***************************************************************************/
/*                                                                         */
/*  ftcglyph.c                                                             */
/*                                                                         */
/*    FreeType Glyph Image (FT_Glyph) cache (body).                        */
/*                                                                         */
/*  Copyright 2000-2001, 2003, 2004 by                                     */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include "ftcint.h"
#include FT_CACHE_INTERNAL_GLYPH_H
#include FT_ERRORS_H
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_DEBUG_H

#include "ftcerror.h"


  FT_EXPORT_DEF( void )
  FTC_Family_Unref( FTC_Family  family )
  {
    if ( family && --family->num_nodes <= 0 )
      FTC_GCache_FreeFamily( family->cache, family );
  }


  /* create a new chunk node, setting its cache index and ref count */
  FT_EXPORT_DEF( void )
  FTC_GNode_Init( FTC_GNode   gnode,
                  FT_UInt     gindex,
                  FTC_Family  family )
  {
    gnode->family = family;
    gnode->gindex = gindex;
    family->num_nodes++;
  }


  FT_EXPORT_DEF( void )
  FTC_GNode_Done( FTC_GNode   gnode )
  {
    FTC_Family  family = gnode->family;

    /* finalize the node */
    gnode->gindex = 0;
    gnode->family = 0;
    if ( family && --family->num_nodes == 0 )
      FTC_GCache_FreeFamily( family->cache, family );
  }


  FT_EXPORT_DEF( FT_Bool )
  FTC_GNode_Equal( FTC_GNode   gnode,
                   FTC_GNode   query,
                   FTC_GCache  cache )
  {
    FT_UNUSED(cache);

    return FTC_GNODE_EQUAL(gnode,query,cache);
  }


  FT_EXPORT( FT_Bool )
  FTC_GNode_EqualFaceID( FTC_GNode   gnode,
                         FTC_FaceID  face_id,
                         FTC_GCache  cache )
  {
    FTC_Family  family = gnode->family;
    FT_Bool     result;

    result = cache->fam_equal_faceid( family, face_id );
    if ( result )
    {
      gnode->family = NULL;
      if ( family && --family->num_nodes == 0 )
        FTC_GCache_FreeFamily( cache, family );
    }
    return  result;
  }




  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      CHUNK SETS                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_EXPORT_DEF( FT_Error )
  FTC_GCache_Init( FTC_GCache  cache )
  {
    FT_Error    error;


    error = FTC_Cache_Init( FTC_CACHE( cache ) );
    if ( !error )
    {
      FTC_FamilyClass  clazz = FTC_GCACHE__FAMILY_CLASS(cache);

      cache->families         = NULL;

      /* create local copies for better performance */
      cache->fam_equal        = clazz->fam_equal;
      cache->fam_equal_faceid = clazz->fam_equal_faceid;
    }

    return error;
  }


  FT_EXPORT_DEF( void )
  FTC_GCache_Done( FTC_GCache  cache )
  {
    FT_Memory  memory = FTC_CACHE__MEMORY(cache);

    FTC_Cache_Done( (FTC_Cache)cache );

   /* destroy any families that the clients didn't
    * release properly !!
    */
    if ( cache->families )
    {
      FTC_Family           family, next;
      FTC_FamilyClass      clazz       = FTC_GCACHE__FAMILY_CLASS(cache);
      FTC_Family_DoneFunc  family_done = clazz->fam_done;

      for ( family = cache->families; family; family = next )
      {
        next = family->link;

        if ( family_done )
          family_done( family );

        FT_FREE( family );
      }
    }
  }


  FT_EXPORT_DEF( FT_Error )
  FTC_GCache_New( FTC_Manager       manager,
                  FTC_GCacheClass   clazz,
                  FTC_GCache       *acache )
  {
    return FTC_Manager_RegisterCache( manager,
                                      (FTC_CacheClass)clazz,
                                      (FTC_Cache*)acache );
  }


  FT_EXPORT_DEF( void )
  FTC_GCache_FreeFamily( FTC_GCache  cache,
                         FTC_Family  family )
  {
    FTC_Family*      pfamily = &cache->families;
    FTC_FamilyClass  clazz   = FTC_GCACHE__FAMILY_CLASS(cache);
    FT_Memory        memory  = FTC_CACHE__MEMORY(cache);

    for (;;)
    {
      if ( *pfamily == NULL )
        return;

      if ( *pfamily == family )
      {
        *pfamily = family->link;
        break;
      }

      pfamily = &(*pfamily)->link;
    }

    if ( clazz->fam_done )
      clazz->fam_done( family );

    FT_FREE( family );
  }

  FT_EXPORT_DEF( FT_Error )
  FTC_GCache_NewFamily( FTC_GCache   cache,
                        FT_UInt32    hash,
                        FTC_Family   query,
                        FTC_Family  *afamily )
  {
    FT_Memory        memory = FTC_CACHE__MEMORY(cache);
    FT_Error         error;
    FTC_Family       family;
    FTC_FamilyClass  clazz = FTC_GCACHE__FAMILY_CLASS(cache);

    if ( !FT_QALLOC( family, clazz->fam_size ) )
    {
      FT_MEM_COPY( family, query, clazz->fam_size );

      family->cache     = cache;
      family->link      = NULL;
      family->num_nodes = 0;
      family->hash      = hash;

      if ( clazz->fam_init )
      {
        error = clazz->fam_init( family, query );
        if ( error )
        {
          if ( clazz->fam_done )
            clazz->fam_done( family );

          FT_FREE( family );
          goto Exit;
        }
      }

      family->link    = cache->families;
      cache->families = family;
    }

  Exit:
    *afamily = family;
    return error;
  }


  FT_EXPORT_DEF( FT_Error )
  FTC_GCache_GetFamily( FTC_GCache   cache,
                        FT_UInt32    hash,
                        FTC_Family   query,
                        FTC_Family  *afamily )
  {
    FTC_Family*           pfamily = &cache->families;
    FTC_Family            family;
    FTC_Family_EqualFunc  fequal  = cache->fam_equal;
    FT_Error              error   = 0;

    for (;;)
    {
      family = *pfamily;
      if ( family == NULL )
        break;

      if ( family->hash == hash && fequal( family, query ) )
      {
        if ( family != cache->families )
        {
          *pfamily        = family->link;
          family->link    = cache->families;
          cache->families = family;
        }
        goto FoundIt;
      }

      pfamily = &family->link;
    }

    /* we didn't found it */
    error = FTC_GCache_NewFamily( cache, hash, query, &family );

  FoundIt:
    if ( !error )
      family->num_nodes++;

    *afamily = family;
    return error;
  }


  FT_EXPORT( FT_Error )
  FTC_GCache_GetNode( FTC_GCache  cache,
                      FT_UInt     hash,
                      FTC_GNode   query,
                      FTC_Node   *anode )
  {
    FTC_Node  node  = NULL;
    FT_Error  error = 0;

    FTC_CACHE_LOOKUP_CMP( cache, FTC_CACHE(cache)->node_equal, hash,
                          query, node, error );

    *anode = node;
    return error;
  }

#if 0
  FT_EXPORT_DEF( FT_Error )
  FTC_GCache_Lookup( FTC_GCache   cache,
                     FT_UInt32    hash,
                     FT_UInt      gindex,
                     FTC_GNode    query,
                     FTC_Node    *anode )
  {
    FT_Error  error;


    query->gindex = gindex;

    FTC_MRULIST_LOOKUP( &cache->families,
                        query->family,
                        query->family,
                        error );
    if ( !error )
    {
      FTC_Family  family = query->family;

      /* prevent the family from being destroyed too early when an        */
      /* out-of-memory condition occurs during glyph node initialization. */
      family->num_nodes++;

      error = FTC_Cache_Lookup( FTC_CACHE( cache ), hash, query, anode );

      if ( --family->num_nodes == 0 )
        FTC_FAMILY_FREE( family, cache );
    }
    return error;
  }
#endif


/* END */
