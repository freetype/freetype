#ifndef FTCIMAGE_H
#define FTCIMAGE_H

#include <freetype/cache/ftcglyph.h>

 /* the glyph image queue type */
  typedef struct FTC_Image_QueueRec_
  {
    FTC_Glyph_QueueRec  root;
    FTC_Image_Desc      description;
  
  } FTC_Image_QueueRec, *FTC_Image_Queue;

  typedef struct FTC_Image_CacheRec_
  {
    FTC_Glyph_CacheRec  root;
    
  } FTC_Image_CacheRec;

#endif /* FTCIMAGE_H */

