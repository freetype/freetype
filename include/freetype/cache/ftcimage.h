/***************************************************************************/
/*                                                                         */
/*  ftcimage.h                                                             */
/*                                                                         */
/*    FreeType Generic Image cache (specification)                         */
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


 /*
  *  FTC_ICache is an _abstract_ cache used to store a single FT_Glyph
  *  image per cache node.
  *
  *  FTC_ICache extends FTC_GCache.  For an implementation example,
  *  see FTC_ImageCache in `src/cache/ftbasic.c'.
  */


  /*************************************************************************/
  /*                                                                       */
  /* Each image cache really manages FT_Glyph objects.                     */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FTCIMAGE_H__
#define __FTCIMAGE_H__


#include <ft2build.h>
#include FT_CACHE_H
#include FT_CACHE_INTERNAL_GLYPH_H

FT_BEGIN_HEADER


  /* the FT_Glyph image node type - we store only 1 glyph per node */
  typedef struct  FTC_INodeRec_
  {
    FTC_GNodeRec  gnode;
    FT_Glyph      glyph;

  } FTC_INodeRec, *FTC_INode;

#define FTC_INODE( x )          ( (FTC_INode)( x ) )
#define FTC_INODE__GLYPH(x)     ( FTC_INODE(x)->glyph )


  typedef FT_Error
  (*FTC_IFamily_LoadGlyphFunc)( FTC_Family   family,
                                FT_UInt      gindex,
                                FTC_Manager  manager,
                                FT_Glyph    *aglyph );

  typedef struct
  {
    FTC_GCacheClassRec          clazz;
    FTC_IFamily_LoadGlyphFunc   fam_load_glyph;

  } FTC_ICacheClassRec;

  typedef const FTC_ICacheClassRec*   FTC_ICacheClass;

#define  FTC_ICACHE_CLASS(x)  ((FTC_ICacheClass)(x))

#define  FTC_ICACHE__CLASS(x)       FTC_ICACHE_CLASS(FTC_GCACHE__CLASS(x))
#define  FTC_ICACHE__LOAD_GLYPH(x)  (FTC_ICACHE__CLASS(x)->fam_load_glyph)

#define  FTC_DEFINE_ICACHE_CLASS(_gcache_class,_family_load_glyph) \
  {                                                  \
    _gcache_class,                                   \
    (FTC_IFamily_LoadGlyphFunc) _family_load_glyph   \
  }


  /* can be used as a @FTC_Node_FreeFunc */
  FT_EXPORT( void )
  FTC_INode_Free( FTC_INode   inode,
                  FTC_GCache  cache );

  /* Can be used as @FTC_Node_NewFunc.  `gquery.index' and `gquery.family'
   * must be set correctly.  This function will call the `family_load_glyph'
   * method to load the FT_Glyph into the cache node.
   */
  FT_EXPORT( FT_Error )
  FTC_INode_New( FTC_INode   *pinode,
                 FTC_GNode    gquery,
                 FTC_GCache   cache );

  /* can be used as @FTC_Node_WeightFunc */
  FT_EXPORT( FT_ULong )
  FTC_INode_Weight( FTC_INode  inode );

 /* */

FT_END_HEADER

#endif /* __FTCIMAGE_H__ */


/* END */
