#ifndef __FT_SERVICE_MULTIPLE_MASTERS_H__
#define __FT_SERVICE_MULTIPLE_MASTERS_H__

#include FT_INTERNAL_SERVICE_H

 /*
  *  a service used to manage multiple-masters data in a given face
  *
  *  see the related APIs in "ftmm.h" / FT_MULTIPLE_MASTERS_H
  *
  */


  typedef FT_Error
  (*FT_Get_MM_Func)( FT_Face           face,
                     FT_Multi_Master*  master );

  typedef FT_Error
  (*FT_Set_MM_Design_Func)( FT_Face   face,
                            FT_UInt   num_coords,
                            FT_Long*  coords );

  typedef FT_Error
  (*FT_Set_MM_Blend_Func)( FT_Face   face,
                           FT_UInt   num_coords,
                           FT_Long*  coords );

#define FT_SERVICE_ID_MULTI_MASTERS  "multi-masters"

  FT_DEFINE_SERVICE( MultiMasters )
  {
    FT_Get_MM_Func         get_mm;
    FT_Set_MM_Design_Func  set_mm_design;
    FT_Set_MM_Blend_Func   set_mm_blend;
  };

#endif /* __FT_SERVICE_MULTIPLE_MASTERS_H__ */
