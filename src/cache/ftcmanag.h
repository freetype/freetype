#ifndef FTCMANAG_H
#define FTCMANAG_H

#include <cache/ftlru.h>

#define  FTC_MAX_FACES  4
#define  FTC_MAX_SIZES  8

  typedef FT_Pointer  FTC_FaceID;
  
  typedef struct FTC_SizeRec_
  {
    FTC_FaceID   face_id;
    FT_UShort    pix_width;
    FT_UShort    pix_height;
    
  } FTC_SizeRec, *FTC_SizeID;

 
  typedef FT_Error  (*FTC_Face_Requester)( FTC_FaceID  face_id,
                                           FT_Pointer  data, 
                                           FT_Face    *aface );


  typedef struct FTC_ManagerRec_*  FTC_Manager;

  typedef struct FTC_Manager_LruRec_
  {
    FT_LruRec    root;
    FTC_Manager  manager;
  
  } FTC_Manager_LruRec, *FTC_Manager_Lru;


  typedef struct FTC_ManagerRec_
  {
    FT_Library          library;
    FT_Lru              faces_lru;
    FT_Lru              sizes_lru;
    
    FT_Pointer          request_data;
    FTC_Face_Requester  request_face;
    
  } FTC_ManagerRec;


  FT_EXPORT_DEF(FT_Error)  FTC_Manager_New  ( FT_Library          library,
                                              FTC_Face_Requester  requester,
                                              FT_Pointer          req_data,
                                              FTC_Manager        *amanager );
                        
  FT_EXPORT_DEF(void)      FTC_Manager_Done ( FTC_Manager  manager );

  FT_EXPORT_DEF(void)      FTC_Manager_Reset( FTC_Manager  manager );

  FT_EXPORT_DEF(FT_Error)  FTC_Manager_Lookup_Face( FTC_Manager  manager,
                                                    FTC_FaceID   face_id,
                                                    FT_Face     *aface );
 
  FT_EXPORT_DEF(FT_Error)  FTC_Manager_Lookup_Size( FTC_Manager  manager,
                                                    FTC_SizeID   size_id,
                                                    FT_Face     *aface,
                                                    FT_Size     *asize );

#endif /* FTCMANAG_H */
