#ifndef FTCIMAGE_H
#define FTCIMAGE_H

#include <cache/ftcmanag.h>
#include <freetype/ftglyph.h>

  typedef struct FTC_Image_QueueRec_*     FTC_Image_Queue;
  typedef struct FTC_Image_CacheRec_*     FTC_Image_Cache; 
  typedef struct FTC_ImageNodeRec_*       FTC_ImageNode;
  
  /* types of glyph images */
  typedef enum FTC_Image_Type_
  {
    ftc_image_mono = 0,         /* monochrome bitmap   */
    ftc_image_grays,            /* anti-aliased bitmap */
    ftc_image_outline,          /* scaled outline      */
    ftc_image_master_outline,   /* original outline    */
  
  } FTC_Image_Type;

 
  /* a descriptor used to describe all glyphs in a given queue */
  typedef struct FTC_Image_Desc_
  {
    FTC_FaceID   face_id;
    FT_UInt      pix_width;
    FT_UInt      pix_height;
    FT_UInt      image_type;
  
  } FTC_Image_Desc;

/* macros used to pack a glyph index and a queue index in a single ptr */  
#define  FTC_PTR_TO_GINDEX(p)   ((FT_UInt)((FT_ULong)(p) >> 16))
#define  FTC_PTR_TO_QINDEX(p)   ((FT_UInt)((FT_ULong)(p) & 0xFFFF)) 
#define  FTC_INDICES_TO_PTR(g,q)  ((FT_Pointer)(((FT_ULong)(g) << 16) |  \
                                               ((FT_ULong)(q) & 0xFFFF)))
    
  typedef struct FTC_ImageNodeRec_
  {
    FT_ListNodeRec  root1;  /* root1.data contains a FT_Glyph handle           */
    FT_ListNodeRec  root2;  /* root2.data contains a glyph index + queue index */
 
  } FTC_ImageNodeRec;

/* macros to read/set the glyph & queue index in a FTC_ImageNode */
#define  FTC_IMAGENODE_GET_GINDEX(n)  FTC_PTR_TO_GINDEX((n)->root2.data)
#define  FTC_IMAGENODE_GET_QINDEX(n)  FTC_PTR_TO_QINDEX((n)->root2.data)
#define  FTC_IMAGENODE_SET_INDICES(g,q)   \
            do { (n)->root2.data = FTC_INDICES_TO_PTR(g,q); } while (0)


  typedef struct FTC_Image_CacheRec_
  {
    FTC_Manager  manager;
    FT_Memory    memory;
    
    FT_ULong     max_bytes;   /* maximum size of cache in bytes */
    FT_ULong     num_bytes;   /* current size of cache in bytes */
    
    FT_Lru       queues_lru;  /* static queues lru list */

  } FTC_Image_Cache;


 /* a table of functions used to generate/manager glyph images */
  typedef struct FTC_Image_Class_
  {
    FT_Error     (*init_image)( FTC_Image_Queue  queue,
                                FTC_Image_Node   node );
                                
    void         (*done_image)( FTC_Image_Queue  queue,
                                FTC_Image_Node   node );
                                
    FT_ULong     (*size_image)( FTC_Image_Queue  queue,
                                FTC_Image_Node   node );

  } FTC_Image_Class;


  typedef struct FTC_Image_QueueRec_
  {
    FTC_Image_Cache   cache;
    FTC_Manager       manager;
    FT_Memory         memory;
    FTC_Image_Class*  clazz;
    FTC_Image_Desc    descriptor;
    FT_UInt           hash_size;
    FT_List           buckets;

  } FTC_Image_SubCacheRec;


  FT_EXPORT_DEF(FT_Error) FTC_Image_Cache_New( FTC_Manager       manager,
                                               FT_ULong          max_bytes,
                                               FTC_Image_Cache  *acache );
                                                
  FT_EXPORT_DEF(void)     FTC_Image_Cache_Done( FTC_Image_Cache  cache );
  
  FT_EXPORT_DEF(FT_Error) FTC_Image_Cache_Lookup( FTC_Image_Cache  cache,
                                                  FTC_Image_Desc*  desc,
                                                  FT_UInt          gindex,
                                                  FT_Glyph        *aglyph );

#endif /* FTCIMAGE_H */
