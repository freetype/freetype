/***************************************************************************/
/*                                                                         */
/*  ftcglyph.h                                                             */
/*                                                                         */
/*    FreeType abstract glyph cache (specification).                       */
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
  /*            implement an abstract glyph cache class.  You need to      */
  /*            provide additional logic to implement a complete cache.    */
  /*            For example, see `ftcimage.h' and `ftcimage.c' which       */
  /*            implement a FT_Glyph cache based on this code.             */
  /*                                                                       */
  /*  NOTE: For now, each glyph set is implemented as a static hash table. */
  /*        It would be interesting to experiment with dynamic hashes to   */
  /*        see whether this improves performance or not (I don't know why */
  /*        but something tells me it won't).                              */
  /*                                                                       */
  /*        In all cases, this change should not affect any derived glyph  */
  /*        cache class.                                                   */
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


#ifndef __FTCGLYPH_H__
#define __FTCGLYPH_H__


#include <ft2build.h>
#include FT_CACHE_H
#include FT_CACHE_MANAGER_H

#include <stddef.h>


FT_BEGIN_HEADER


 /* each glyph set is caracterized by a "glyph set type" which must be */
 /* defined by sub-classes..                                           */
  typedef struct FTC_GlyphSetRec_*     FTC_GlyphSet;

 /* handle to a glyph cache node */
  typedef struct FTC_GlyphNodeRec_*    FTC_GlyphNode;

 /* a glyph cache, its nodes are all glyph-specific */
  typedef struct FTC_GlyphCacheRec_*   FTC_GlyphCache;

 /* glyph sets class handle */
  typedef const struct FTC_GlyphSet_ClassRec_*   FTC_GlyphSet_Class;


 /* size should be 24 bytes on 32-bit machines                      */
 /* note that the node's hash is ((gset->hash << 16) | glyph_index) */
 /* this _must_ be set properly by the glyph node initializer       */
 /*                                                                 */
  typedef struct FTC_GlyphNodeRec_
  {
    FTC_NodeRec     node;
    FTC_GlyphSet    gset;

  } FTC_GlyphNodeRec;

#define  FTC_GLYPH_NODE(x)    ((FTC_GlyphNode)(x))
#define  FTC_GLYPH_NODE_P(x)  ((FTC_GlyphNode*)(x))


 /* the glyph set structure. each glyph set is used to model a set of     */
 /* glyphs of the same "type". The type itself is defined in sub-classes  */
 /*                                                                       */
 /* for example, the "image cache" uses face_id + character_pixel_sizes + */
 /* image_format to characterize glyph sets..                             */
 /*                                                                       */
 /* a pure "master outlines" cache would only use face_id, etc..          */
 /*                                                                       */
  typedef struct  FTC_GlyphSetRec_
  {
    FT_LruNodeRec   lru;         /* glyph sets are LRU nodes within */
    FTC_GlyphCache  gcache;      /* parent cache..                  */
    FT_UFast        hash;        /* must be set by initializer !!   */
    FT_Fast         num_glyphs;  /* destroyed when 0..              */

  } FTC_GlyphSetRec;

#define  FTC_GLYPH_SET(x)     ((FTC_GlyphSet)(x))
#define  FTC_GLYPH_SET_P(x)   ((FTC_GlyphSet*)(x))

#define  FTC_GLYPH_SET_MEMORY(x)  ((x)->gcache->cache.memory)


/* retrieve glyph index of glyph node */
#define  FTC_GLYPH_NODE_GINDEX(x)  \
             ((FT_UInt)(FTC_GLYPH_NODE(x)->node.hash & 0xFFFF))

  /* the abstract glyph cache object */
  typedef struct  FTC_GlyphCacheRec_
  {
    FTC_CacheRec  cache;
    FT_LruList    gset_lru;   /* LRU list of glyph sets */

  } FTC_GlyphCacheRec;

#define  FTC_GLYPH_CACHE(x)    ((FTC_GlyphCache)(x))
#define  FTC_GLYPH_CACHE_P(x)  ((FTC_GlyphCache*)(x))


  typedef struct FTC_GlyphQueryRec_
  {
   /* input */
    FT_UInt       gindex;
   
   /* output */
    FTC_GlyphSet  gset;

  } FTC_GlyphQueryRec, *FTC_GlyphQuery;


  /*************************************************************************/
  /*                                                                       */
  /* These functions are exported so that they can be called from          */
  /* user-provided cache classes; otherwise, they are really part of the   */
  /* cache sub-system internals.                                           */
  /*                                                                       */

 /* must be called by derived FTC_Node_InitFunc routines */
  FT_EXPORT( void )
  ftc_glyph_node_init( FTC_GlyphNode  node,
                       FT_UInt        gindex,  /* glyph index for node */
                       FTC_GlyphSet   gset );

 /* must be called by derived FTC_Node_DoneFunc routines */
  FT_EXPORT( void )
  ftc_glyph_node_done( FTC_GlyphNode  node );


 /* can be used as a FTC_LruNode_InitFunc or called by sub-classes */
  FT_EXPORT( FT_Error )
  ftc_glyph_set_init( FTC_GlyphSet  gset,
                      FT_LruList    list );

 /* can be used as a FTC_LruNode_DoneFunc or called by sub-classes */
  FT_EXPORT( void )
  ftc_glyph_set_done( FTC_GlyphSet  gset );


 /* can be used as a FTC_Cache_DoneFunc or called by sub-classes */
  FT_EXPORT( void )
  ftc_glyph_cache_done(  FTC_GlyphCache  cache );

 /* must be called in a FTC_Cache_InitFunc !! */
  FT_EXPORT( FT_Error )
  ftc_glyph_cache_init( FTC_GlyphCache    cache,
                        FT_LruList_Class  gset_class );

 /* can be called directly or from sub-classes */
  FT_EXPORT( FT_Error )
  ftc_glyph_cache_lookup( FTC_GlyphCache  cache,
                          FTC_GlyphQuery  query,
                          FTC_GlyphNode  *anode );


FT_END_HEADER

#endif /* __FTCGLYPH_H__ */


/* END */
