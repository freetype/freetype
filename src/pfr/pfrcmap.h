#ifndef __PFR_CMAP_H__
#define __PFR_CMAP_H__

#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include "pfrtypes.h"

FT_BEGIN_HEADER

  typedef struct PFR_CMapRec_
  {
    FT_CMapRec  cmap;
    FT_UInt     num_chars;
    PFR_Char    chars;
  
  } PFR_CMapRec, *PFR_CMap;


  FT_CALLBACK_TABLE const FT_CMap_ClassRec    pfr_cmap_class_rec;

FT_END_HEADER

#endif /* __PFR_CMAP_H__ */
