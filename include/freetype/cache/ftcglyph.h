/***************************************************************************/
/*                                                                         */
/*  ftcglyph.h                                                             */
/*                                                                         */
/*    FreeType abstract glyph cache (specification).                       */
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


  /*
   *
   *  FTC_GCache is an _abstract_ cache object optimized to store glyph
   *  data.  It works as follows:
   *
   *   - It manages FTC_GNode objects. Each one of them can hold one or more
   *     glyph `items'.  Item types are not specified in the FTC_GCache but
   *     in classes that extend it.
   *
   *   - Glyph attributes, like face ID, character size, render mode, etc.,
   *     can be grouped into abstract `glyph families'.  This avoids storing
   *     the attributes within the FTC_GCache, since it is likely that many
   *     FTC_GNodes will belong to the same family in typical uses.
   *
   *   - Each FTC_GNode is thus an FTC_Node with two additional fields:
   *
   *       * gindex: A glyph index, or the first index in a glyph range.
   *       * family: A pointer to a glyph `family'.
   *
   *   - Family types are not fully specific in the FTC_Family type, but
   *     by classes that extend it.
   *
   *  Note that both FTC_ImageCache and FTC_SBitCache extend FTC_GCache.
   *  They share an FTC_Family sub-class called FTC_BasicFamily which is
   *  used to store the following data: face ID, pixel/point sizes, load
   *  flags.  For more details see the file `src/cache/ftcbasic.c'.
   *
   *  Client applications can extend FTC_GNode with their own FTC_GNode
   *  and FTC_Family sub-classes to implement more complex caches (e.g.,
   *  handling automatic synthesis, like obliquing & emboldening, colored
   *  glyphs, etc.).
   *
   *  See also the FTC_ICache & FTC_SCache classes in `ftcimage.h' and
   *  `ftcsbits.h', which both extend FTC_GCache with additional
   *  optimizations.
   *
   *  A typical FTC_GCache implementation must provide at least the
   *  following:
   *
   *  - FTC_GNode sub-class, e.g. MyNode, with relevant methods:
   *        my_node_new            (must call FTC_GNode_Init)
   *        my_node_free           (must call FTC_GNode_Done)
   *        my_node_equal          (must call FTC_GNode_Compare)
   *        my_node_remove_faceid  (must call ftc_gnode_unselect in case
   *                                of match)
   *
   *  - FTC_Family sub-class, e.g. MyFamily, with relevant methods:
   *        my_family_compare
   *        my_family_init
   *        my_family_reset (optional)
   *        my_family_done
   *
   *  - Constant structures for a FTC_GNodeClass.
   *
   *  - MyCacheNew() can be implemented easily as a call to the convenience
   *    function FTC_GCache_New.
   *
   *  - MyCacheLookup with a call to FTC_GCache_Lookup.  This function will
   *    automatically:
   *
   *    - Search for the corresponding family in the cache, or create
   *      a new one if necessary.  Put it in FTC_GQUERY(myquery).family
   *
   *    - Call FTC_Cache_Lookup.
   *
   *    If it returns NULL, you should create a new node, then call
   *    ftc_cache_add as usual.
   */


  /*************************************************************************/
  /*                                                                       */
  /* Important: The functions defined in this file are only used to        */
  /*            implement an abstract glyph cache class.  You need to      */
  /*            provide additional logic to implement a complete cache.    */
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
#include FT_CACHE_INTERNAL_MANAGER_H


FT_BEGIN_HEADER


  typedef struct FTC_GCacheRec_*   FTC_GCache;


 /*
  *  We can group glyphs into `families'.  Each family correspond to a
  *  given face ID, character size, transform, etc.
  *
  *  Families are implemented as MRU list nodes.  They are
  *  reference-counted.
  */

  typedef const struct FTC_FamilyClassRec_*   FTC_FamilyClass;

  typedef struct  FTC_FamilyRec_
  {
    FTC_Family       link;      /* used for hashing too                   */
    FT_UInt32        hash;      /* hash value for this family             */
    FT_Int           num_nodes; /* current number of nodes in this family */
    FTC_GCache       cache;     /* cache the family belongs to            */

  } FTC_FamilyRec;

#define  FTC_FAMILY(x)    ( (FTC_Family)(x) )
#define  FTC_FAMILY_P(x)  ( (FTC_Family*)(x) )

#define  FTC_FAMILY__CACHE(f)  (FTC_FAMILY(f)->cache)
#define  FTC_FAMILY__CLASS(f)  FTC_GCACHE__FAMILY_CLASS(FTC_FAMILY__CACHE(f))

 /* note that the content of 'key' has already been copied to 'family'
  * when this method is called. This callback is only necessary when you
  * need to perform non-trivial initialization (e.g. duplicating
  * heap-allocated memory, like strings, owned by the family)
  *
  * if this method returns an error, the corresponding FTC_Family_DoneFunc,
  * will be called, if is not defined to NULL
  */
  typedef FT_Error  (*FTC_Family_InitFunc)( FTC_Family  family,
                                            FTC_Family  key );

 /* finalize the content of a given family object
  */
  typedef void      (*FTC_Family_DoneFunc)( FTC_Family  family );

 /* test wether a family matches a given key. Note that this method
  * is only called when (family.hash == key.hash), so there is no
  * need to test it again
  */
  typedef FT_Bool   (*FTC_Family_EqualFunc)( FTC_Family  family,
                                             FTC_Family  key );

 /* test wether a family matches a given FTC_FaceID
  */
  typedef FT_Bool   (*FTC_Family_EqualFaceIDFunc)( FTC_Family  family,
                                                   FTC_FaceID  face_id );

  typedef struct FTC_FamilyClassRec_
  {
    FT_UInt                     fam_size;
    FTC_Family_InitFunc         fam_init;
    FTC_Family_DoneFunc         fam_done;
    FTC_Family_EqualFunc        fam_equal;
    FTC_Family_EqualFaceIDFunc  fam_equal_faceid;

  } FTC_FamilyClassRec;


#define  FTC_FAMILY_CLASS(x)  ((FTC_FamilyClass)(x))

#define  FTC_DEFINE_FAMILY_CLASS(_type,_init,_done,_equal,_equal_faceid) \
  {                                              \
    sizeof(_type),                               \
    (FTC_Family_InitFunc)       (_init),         \
    (FTC_Family_DoneFunc)       (_done),         \
    (FTC_Family_EqualFunc)      (_equal),        \
    (FTC_Family_EqualFaceIDFunc)(_equal_faceid)  \
  }


  typedef struct  FTC_GNodeRec_
  {
    FTC_NodeRec      node;
    FTC_Family       family;
    FT_UInt          gindex;

  } FTC_GNodeRec, *FTC_GNode;

#define FTC_GNODE( x )    ( (FTC_GNode)(x) )
#define FTC_GNODE_P( x )  ( (FTC_GNode*)(x) )

  /*************************************************************************/
  /*                                                                       */
  /* These functions are exported so that they can be called from          */
  /* user-provided cache classes; otherwise, they are really part of the   */
  /* cache sub-system internals.                                           */
  /*                                                                       */

  /* must be called by derived FTC_Node_InitFunc routines */
  FT_EXPORT( void )
  FTC_GNode_Init( FTC_GNode   node,
                  FT_UInt     gindex,  /* glyph index for node */
                  FTC_Family  family );

  /* this macro can be used to test wether a glyph node matches a given
   * glyph query
   */
#define  FTC_GNODE_EQUAL(node,key,cache)                             \
             ( FTC_GNODE(node)->family == FTC_GNODE(key)->family &&  \
               FTC_GNODE(node)->gindex == FTC_GNODE(key)->gindex )


  /* same as FTC_GNODE_EQUAL, but can be used as a callback */
  FT_EXPORT( FT_Bool )
  FTC_GNode_Equal( FTC_GNode   gnode,
                   FTC_GNode   gquery,
                   FTC_GCache  cache );

  /* call this function to clear a node's family -- this is necessary */
  /* to implement the `node_remove_faceid' cache method correctly     */
  FT_EXPORT( FT_Bool )
  FTC_GNode_EqualFaceID( FTC_GNode   gnode,
                         FTC_FaceID  face_id,
                         FTC_GCache  cache );

  /* must be called by derived FTC_Node_DoneFunc routines */
  FT_EXPORT( void )
  FTC_GNode_Done( FTC_GNode   node );


  FT_EXPORT( void )
  FTC_Family_Init( FTC_Family  family,
                   FT_UInt32   hash,
                   FTC_GCache  cache );


  typedef struct FTC_GCacheRec_
  {
    FTC_CacheRec                cache;
    FTC_Family_EqualFunc        fam_equal;
    FTC_Family_EqualFaceIDFunc  fam_equal_faceid;
    FTC_Family                  families;

  } FTC_GCacheRec;

#define FTC_GCACHE( x )  ((FTC_GCache)(x))


  /* can be used as @FTC_Cache_InitFunc */
  FT_EXPORT( FT_Error )
  FTC_GCache_Init( FTC_GCache  cache );


  /* can be used as @FTC_Cache_DoneFunc */
  FT_EXPORT( void )
  FTC_GCache_Done( FTC_GCache  cache );


  /* the glyph cache class adds fields for the family implementation */
  typedef struct  FTC_GCacheClassRec_
  {
    FTC_CacheClassRec   clazz;
    FTC_FamilyClassRec  family_class;

  } FTC_GCacheClassRec;

  typedef const FTC_GCacheClassRec*   FTC_GCacheClass;


#define  FTC_DEFINE_GCACHE_CLASS(_cache_class,_family_class)  \
  {                                                           \
    _cache_class,                                             \
    _family_class                                             \
  }


#define FTC_GCACHE_CLASS( x )  ((FTC_GCacheClass)(x))

#define FTC_GCACHE__CLASS( x ) \
          FTC_GCACHE_CLASS( FTC_CACHE__CLASS(x) )

#define FTC_GCACHE__FAMILY_CLASS(c) \
          (&FTC_GCACHE__CLASS(c)->family_class)


  FT_EXPORT( FT_Error )
  FTC_GCache_New( FTC_Manager       manager,
                  FTC_GCacheClass   clazz,
                  FTC_GCache       *acache );


  /* used by FTC_GCACHE_GET_FAMILY, don't call directly */
  FT_EXPORT( FT_Error )
  FTC_GCache_NewFamily( FTC_GCache   cache,
                        FT_UInt32    hash,
                        FTC_Family   query,
                        FTC_Family  *afamily );

  FT_EXPORT( void )
  FTC_GCache_FreeFamily( FTC_GCache  cache,
                         FTC_Family  family );

#define  FTC_FAMILY_FREE(f)  FTC_GCache_FreeFamily( (f)->cache, (f) )

  /* query.hash must be set correctly !! */
  FT_EXPORT( FT_Error )
  FTC_GCache_GetFamily( FTC_GCache   cache,
                        FT_UInt32    hash,
                        FTC_Family   query,
                        FTC_Family  *afamily );

 /* query.family and query.gindex must be set correctly !!
  */
  FT_EXPORT( FT_Error )
  FTC_GCache_GetNode( FTC_GCache  cache,
                      FT_UInt32   hash,
                      FTC_GNode   query,
                      FTC_Node   *anode );

  /* */


#ifdef FTC_INLINE

#define  FTC_GCACHE_GET_FAMILY( cache, famcmp, hash, key, family, error )  \
  FT_BEGIN_STMNT                                                       \
    FTC_GCache             _gcache = FTC_GCACHE( cache );              \
    FTC_Family             _key    = FTC_FAMILY( key );                \
    FTC_Family_EqualFunc   _fequal = (FTC_Family_EqualFunc)(famcmp);   \
    FTC_Family*            _pfamily = &_gcache->families;              \
    FTC_Family             _family;                                    \
                                                                       \
    error = 0;                                                         \
                                                                       \
    for (;;)                                                           \
    {                                                                  \
      _family = *_pfamily;                                             \
      if ( _family == NULL )                                           \
        goto _NewFamily;                                               \
                                                                       \
      if ( _family->hash == (hash) && _fequal( _family, _key ) )       \
        break;                                                         \
                                                                       \
      _pfamily = &_family->link;                                       \
    }                                                                  \
                                                                       \
    if ( _family != _gcache->families )                                \
    {                                                                  \
      *_pfamily         = _family->link;                               \
      _family->link     = _gcache->families;                           \
      _gcache->families = _family;                                     \
    }                                                                  \
    goto _FoundIt;                                                     \
                                                                       \
  _NewFamily:                                                          \
    error = FTC_GCache_NewFamily( _gcache, hash, _key, &_family );     \
  _FoundIt:                                                            \
    if ( !error )                                                      \
      _family->num_nodes++;                                            \
                                                                       \
    *(FTC_Family*)(void*)(family) = _family;                           \
  FT_END_STMNT

#else /* !FTC_INLINE */

#define FTC_GCACHE_GET_FAMILY( cache, famcmp, key, family, error )  \
  error = FTC_GCache_GetFamily( (cache), (key), &(family) )

#endif /* !FTC_INLINE */


FT_END_HEADER


#endif /* __FTCGLYPH_H__ */


/* END */
