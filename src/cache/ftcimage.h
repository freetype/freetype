/***************************************************************************/
/*                                                                         */
/*  ftcimage.h                                                             */
/*                                                                         */
/*    FreeType Image Cache                                                 */
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


#ifndef FTCIMAGE_H
#define FTCIMAGE_H

#include <cache/ftcmanag.h>
#include <freetype/ftglyph.h>
#include <stddef.h>

#ifdef __cplusplus
  extern "C" {
#endif

#define FTC_MAX_IMAGE_QUEUES  16

  typedef struct FTC_Image_QueueRec_*  FTC_Image_Queue;
  typedef struct FTC_ImageNodeRec_*    FTC_ImageNode;
  

  /* macros used to pack a glyph index and a queue index in a single ptr */  
#define FTC_PTR_TO_GINDEX( p )  ( (FT_UInt)( (FT_ULong)(p) >> 16 ) )
#define FTC_PTR_TO_QINDEX( p )  ( (FT_UInt)( (FT_ULong)(p) & 0xFFFF ) ) 
#define FTC_INDICES_TO_PTR( g, q )                      \
          ( (FT_Pointer)( ( (FT_ULong)(g) << 16 )   |   \
                          ( (FT_ULong)(q) & 0xFFFF) ) )
    
  typedef struct  FTC_ImageNodeRec_
  {
    /* root1.data contains a FT_Glyph handle           */
    FT_ListNodeRec  root1;

    /* root2.data contains a glyph index + queue index */
    FT_ListNodeRec  root2;
 
  } FTC_ImageNodeRec;


  /* macros to read/set the glyph & queue index in a FTC_ImageNode */
#define FTC_IMAGENODE_GET_GINDEX( n )  FTC_PTR_TO_GINDEX( (n)->root2.data )
#define FTC_IMAGENODE_GET_QINDEX( n )  FTC_PTR_TO_QINDEX( (n)->root2.data )
#define FTC_IMAGENODE_GET_GLYPH(n)     ((FT_Glyph)(n)->root1.data)
#define FTC_IMAGENODE_SET_GLYPH(n,g)   do { (n)->root1.data = g; } while (0)
#define FTC_IMAGENODE_SET_INDICES( n, g, q )              \
          do                                              \
          {                                               \
            (n)->root2.data = FTC_INDICES_TO_PTR( g, q ); \
          } while ( 0 )


  /* this macro is used to extract a handle to the global LRU list node */
  /* corresponding to a given image node..                              */
#define FTC_IMAGENODE_TO_LISTNODE(n)  \
             ((FT_ListNode)&(n)->root2)

  /* this macro is used to extract a handle to a given image node from */
  /* the corresponding LRU glyph list node. That's a bit hackish..     */
#define FTC_LISTNODE_TO_IMAGENODE(p)  \
             ((FTC_ImageNode)((char*)(p) - offsetof(FTC_ImageNodeRec,root2)))


  typedef struct  FTC_Image_CacheRec_
  {
    FTC_Manager  manager;
    FT_Memory    memory;
    
    FT_ULong     max_bytes;   /* maximum size of cache in bytes */
    FT_ULong     num_bytes;   /* current size of cache in bytes */
    
    FT_Lru       queues_lru;   /* static queues lru list          */
    FT_ListRec   glyphs_lru;   /* global lru list of glyph images */
    
    FTC_Image_Queue  last_queue;  /* small cache */

  } FTC_Image_CacheRec;


  /* a table of functions used to generate/manager glyph images */
  typedef struct  FTC_Image_Class_
  {
    FT_Error  (*init_image)( FTC_Image_Queue  queue,
                             FTC_ImageNode    node );
                                
    void      (*done_image)( FTC_Image_Queue  queue,
                             FTC_ImageNode    node );
                                
    FT_ULong  (*size_image)( FTC_Image_Queue  queue,
                             FTC_ImageNode    node );

  } FTC_Image_Class;


  typedef struct  FTC_Image_QueueRec_
  {
    FTC_Image_Cache   cache;
    FTC_Manager       manager;
    FT_Memory         memory;
    FTC_Image_Class*  clazz;
    FTC_Image_Desc    descriptor;
    FT_UInt           hash_size;
    FT_List           buckets;
    FT_UInt           index;     /* index in parent cache  */

  } FTC_Image_QueueRec;



#ifdef __cplusplus
  }
#endif


#endif /* FTCIMAGE_H */


/* END */
