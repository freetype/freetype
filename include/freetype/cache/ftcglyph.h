/***************************************************************************/
/*                                                                         */
/*  ftcglyph.h                                                             */
/*                                                                         */
/*    FreeType glyph image (FT_Glyph) cache (specification).               */
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


  /*************************************************************************/
  /*                                                                       */
  /* Important: The functions defined in this file are only used to        */
  /*            implement an abstract glyph cache class.  You need to      */
  /*            provide additional logic to implement a complete cache.    */
  /*            For example, see `ftcimage.h' and `ftcimage.c' which       */
  /*            implement a FT_Glyph cache based on this code.             */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*********                                                       *********/
  /*********        WARNING, THIS IS ALPHA CODE, THIS API          *********/
  /*********    IS DUE TO CHANGE UNTIL STRICTLY NOTIFIED BY THE    *********/
  /*********            FREETYPE DEVELOPMENT TEAM                  *********/
  /*********                                                       *********/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


#ifndef FTCGLYPH_H
#define FTCGLYPH_H


#include <freetype/cache/ftcmanag.h>
#include <stddef.h>


#ifdef __cplusplus
  extern "C" {
#endif


  /* maximum number of queues per glyph cache; must be < 256 */
#define FTC_MAX_GLYPH_QUEUES  16

#define FTC_QUEUE_HASH_SIZE_DEFAULT  64

  typedef struct FTC_Glyph_QueueRec_*  FTC_Glyph_Queue;
  typedef struct FTC_GlyphNodeRec_*    FTC_GlyphNode;
  typedef struct FTC_Glyph_CacheRec_*  FTC_Glyph_Cache;

  typedef struct  FTC_GlyphNodeRec_
  {
    FTC_CacheNodeRec  root;
    FTC_GlyphNode     queue_next;   /* next in queue's bucket list */
    FT_UShort         glyph_index;
    FT_UShort         queue_index;

  } FTC_GlyphNodeRec;


#define FTC_GLYPHNODE(x)               ( (FTC_GlyphNode)(x) )
#define FTC_GLYPHNODE_TO_LRUNODE( n )  ( (FT_ListNode)(n) )
#define FTC_LRUNODE_TO_GLYPHNODE( n )  ( (FTC_GlyphNode)(n) )


  /*************************************************************************/
  /*                                                                       */
  /* Glyph queue methods.                                                  */
  /*                                                                       */

  typedef FT_Error  (*FTC_Glyph_Queue_InitFunc)( FTC_Glyph_Queue  queue,
                                                 FT_Pointer       type );

  typedef void  (*FTC_Glyph_Queue_DoneFunc)( FTC_Glyph_Queue  queue );

  typedef FT_Bool  (*FTC_Glyph_Queue_CompareFunc)( FTC_Glyph_Queue  queue,
                                                   FT_Pointer       type );

  typedef FT_Error  (*FTC_Glyph_Queue_NewNodeFunc)( FTC_Glyph_Queue  queue,
                                                    FT_UInt          gindex,
                                                    FTC_GlyphNode*   anode );

  typedef void  (*FTC_Glyph_Queue_DestroyNodeFunc)( FTC_GlyphNode    node,
                                                    FTC_Glyph_Queue  queue );

  typedef FT_ULong (*FTC_Glyph_Queue_SizeNodeFunc)( FTC_GlyphNode    node,
                                                    FTC_Glyph_Queue  queue );


  typedef struct  FTC_Glyph_Queue_Class_
  {
    FT_UInt                          queue_byte_size;

    FTC_Glyph_Queue_InitFunc         init;
    FTC_Glyph_Queue_DoneFunc         done;
    FTC_Glyph_Queue_CompareFunc      compare;

    FTC_Glyph_Queue_NewNodeFunc      new_node;
    FTC_Glyph_Queue_SizeNodeFunc     size_node;
    FTC_Glyph_Queue_DestroyNodeFunc  destroy_node;

  } FTC_Glyph_Queue_Class;


  typedef struct  FTC_Glyph_QueueRec_
  {
    FTC_Glyph_Cache         cache;
    FTC_Manager             manager;
    FT_Memory               memory;
    FTC_Glyph_Queue_Class*  clazz;
    FTC_Image_Desc          descriptor;
    FT_UInt                 hash_size;
    FTC_GlyphNode*          buckets;
    FT_UInt                 queue_index;  /* index in parent cache    */

  } FTC_Glyph_QueueRec;


  /* the abstract glyph cache class */
  typedef struct  FTC_Glyph_Cache_Class_
  {
    FTC_Cache_Class         root;
    FTC_Glyph_Queue_Class*  queue_class;

  } FTC_Glyph_Cache_Class;


  /* the abstract glyph cache object */
  typedef struct  FTC_Glyph_CacheRec_
  {
    FTC_CacheRec     root;
    FT_Lru           queues_lru;  /* static queues lru list */
    FTC_Glyph_Queue  last_queue;  /* small cache :-)        */

  } FTC_Glyph_CacheRec;


  /*************************************************************************/
  /*                                                                       */
  /* These functions are exported so that they can be called from          */
  /* user-provided cache classes; otherwise, they are really parts of the  */
  /* cache sub-system internals.                                           */
  /*                                                                       */

  FT_EXPORT_FUNC( void )
  FTC_GlyphNode_Init( FTC_GlyphNode    node,
                      FTC_Glyph_Queue  queue,
                      FT_UInt          gindex );

#define FTC_GlyphNode_Ref( n ) \
          FTC_CACHENODE_TO_DATA_P( &(n)->root )->ref_count++

#define FTC_GlyphNode_Unref( n ) \
          FTC_CACHENODE_TO_DATA_P( &(n)->root )->ref_count--


  FT_EXPORT_DEF( void )
  FTC_Destroy_Glyph_Node( FTC_GlyphNode    node,
                          FTC_Glyph_Cache  cache );



  FT_EXPORT_DEF( FT_Error )
  FTC_Glyph_Cache_Init( FTC_Glyph_Cache  cache );


  FT_EXPORT_DEF( void )
  FTC_Glyph_Cache_Done( FTC_Glyph_Cache  cache );




  FT_EXPORT_DEF( FT_Error )
  FTC_Glyph_Queue_New( FTC_Glyph_Cache   cache,
                       FT_Pointer        type,
                       FTC_Glyph_Queue*  aqueue );


  FT_EXPORT_DEF( FT_Error )
  FTC_Glyph_Queue_Lookup_Node( FTC_Glyph_Queue  queue,
                               FT_UInt          glyph_index,
                               FTC_GlyphNode*   anode );


#ifdef __cplusplus
  }
#endif


#endif /* FTCIMAGE_H */

/* END */
