/***************************************************************************/
/*                                                                         */
/*  ftcsbits.h                                                             */
/*                                                                         */
/*    A small-bitmap cache (specification).                                */
/*                                                                         */
/*  Copyright 2000-2001, 2002, 2003 by                                     */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTCSBITS_H__
#define __FTCSBITS_H__


#include <ft2build.h>
#include FT_CACHE_H
#include FT_CACHE_INTERNAL_GLYPH_H


FT_BEGIN_HEADER

#define FTC_SBIT_ITEMS_PER_NODE  16

  typedef struct  FTC_SNodeRec_
  {
    FTC_GNodeRec  gnode;
    FT_UInt       count;
    FTC_SBitRec   sbits[FTC_SBIT_ITEMS_PER_NODE];

  } FTC_SNodeRec, *FTC_SNode;


#define FTC_SNODE( x )         ( (FTC_SNode)( x ) )
#define FTC_SNODE__COUNT(x)    ( FTC_SNODE(x)->count )


  typedef FT_UInt
  (*FTC_SFamily_GetCountFunc)( FTC_Family   family,
                               FTC_Manager  manager );

  typedef FT_Error
  (*FTC_SFamily_LoadGlyphFunc)( FTC_Family   family,
                                FT_UInt      gindex,
                                FTC_Manager  manager,
                                FT_Face     *aface );

  typedef struct
  {
    FTC_GCacheClassRec         clazz;
    FTC_SFamily_GetCountFunc   fam_get_count;
    FTC_SFamily_LoadGlyphFunc  fam_load_glyph;

  } FTC_SCacheClassRec;

  typedef const FTC_SCacheClassRec*   FTC_SCacheClass;

#define  FTC_SCACHE_CLASS(x)   ((FTC_SCacheClass)(x))
#define  FTC_SCACHE__CLASS(c)  FTC_SCACHE_CLASS(FTC_CACHE__CLASS(c))

#define  FTC_DEFINE_SCACHE_CLASS(_gcache_class,_get_count,_get_glyph) \
  {                                               \
    _gcache_class,                                \
    (FTC_SFamily_GetCountFunc) (_get_count),      \
    (FTC_SFamily_LoadGlyphFunc)(_get_glyph)       \
  }

#define  FTC_SFAMILY__CLASS(f)  FTC_SCACHE__CLASS(FTC_FAMILY__CACHE(f))


#define FTC_CACHE__SFAMILY_CLASS( x )  \
          FTC_SFAMILY_CLASS( FTC_CACHE__GCACHE_CLASS( x )->family_class )


  FT_EXPORT( void )
  FTC_SNode_Free( FTC_SNode   snode,
                  FTC_GCache  cache );

  FT_EXPORT( FT_Error )
  FTC_SNode_New( FTC_SNode   *psnode,
                 FTC_GNode    gquery,
                 FTC_GCache   cache );

#define  FTC_SNODE_EQUAL(node,query,cache)  \
  FTC_SNode_Equal( FTC_SNODE(node), FTC_GNODE(query), FTC_CACHE(cache) )

  FT_EXPORT( FT_Bool )
  FTC_SNode_Equal( FTC_SNode  snode,
                   FTC_GNode  gquery,
                   FTC_Cache  cache );

  FT_EXPORT( FT_ULong )
  FTC_SNode_Weight( FTC_SNode  inode );


  /* */

FT_END_HEADER

#endif /* __FTCSBITS_H__ */


/* END */
