#ifndef __FT_SERVICE_H__
#define __FT_SERVICE_H__

 /*
  *  each module can export one or more 'services'. Each service is
  *  identified by a constant string, and modeled by a pointer, which
  *  generally corresponds to a structure containing function pointers.
  *
  *  note that a service's data cannot be a mere function
  *  pointer. that's because in C, function pointers might be implemented
  *  differently than data pointers (e.g. 48 bits instead of 32)
  */

 /* this macro is used to lookup a service from a face's driver module
  *
  *   ptr :: variable that receives the service pointer. will be NULL
  *          if not found
  *
  *   id  :: a string describing the service. the list of valid service
  *          identifiers is below
  *
  *   face :: the source face handle
  */
#define  FT_FACE_FIND_SERVICE(ptr,face,id)                         \
   FT_BEGIN_STMNT                                                  \
     FT_Module  module = FT_MODULE(FT_FACE(face)->driver);         \
                                                                   \
     (ptr) = NULL;                                                 \
     if ( module->clazz->get_interface )                           \
       (ptr) = module->clazz->get_interface( module, id );         \
   FT_END_STMNT


 /*************************************************************************/
 /*************************************************************************/
 /*****                                                               *****/
 /*****         S E R V I C E   D E S C R I P T O R S                 *****/
 /*****                                                               *****/
 /*************************************************************************/
 /*************************************************************************/
 
 /* the following structure is used to _describe_ a given service
  * to the library. this is useful to build simple static service lists..
  */  
  typedef struct FT_ServiceDescRec_
  {
    const char*  serv_id;     /* service name         */
    const void*  serv_data;   /* service pointer/data */
  
  } FT_ServiceDescRec;

  typedef const FT_ServiceDescRec*   FT_ServiceDesc;


 /* parse a list of FT_ServiceDescRec descriptors and look for
  * a specific service by id. Note that the last element in the
  * array must be { NULL, NULL }, and that the function should
  * return NULL if the service isn't available
  *
  * this function can be used by modules to implement their "get_service"
  * method
  */
  FT_BASE( FT_Pointer )
  ft_service_list_lookup( FT_ServiceDesc  service_descriptors,
                          const char*     service_id );


 /*************************************************************************/
 /*************************************************************************/
 /*****                                                               *****/
 /*****             S E R V I C E S   C A C H E                       *****/
 /*****                                                               *****/
 /*************************************************************************/
 /*************************************************************************/
 
 /*  this structure is used to store a cache for several often-used
  *  services. It is the type of 'face->internal->services'. You
  *  should only use FT_FACE_LOOKUP_SERVICE to access it
  *
  *  all fields should have the type FT_Pointer to relax compilation
  *  dependencies. We assume the developer isn't completely stupid
  *
  *
  */
  typedef struct FT_ServiceCacheRec_
  {
    FT_Pointer     postscript_name;
    FT_Pointer     multi_masters;
    FT_Pointer     glyph_dict;
    
  } FT_ServiceCacheRec, *FT_ServiceCache;

 /* a magic number used within the services cache
  */
#define  FT_SERVICE_UNAVAILABLE  ((FT_Pointer)-2)  /* magic number */

 /*  this macro is used to lookup a service from a face's driver module
  *  using its cache.
  *
  *  ptr   :: variable receiving the service data. NULL if not available
  *  face  :: source face handle containing the cache
  *  field :: field name in cache
  *  id    :: service id
  *
  */
#define  FT_FACE_LOOKUP_SERVICE(face,ptr,field,id)                       \
   FT_BEGIN_STMNT                                                        \
     (ptr) = FT_FACE(face)->internal->services. field ;                  \
     if ( (ptr) == FT_SERVICE_UNAVAILABLE )                              \
       (ptr) = NULL;                                                     \
     else if ( (ptr) == NULL )                                           \
     {                                                                   \
       FT_FACE_FIND_SERVICE( ptr, face, id );                            \
                                                                         \
       FT_FACE(face)->internal->services. field =                        \
            (FT_Pointer)( (ptr) != NULL                                  \
                        ? (ptr)                                          \
                        : FT_SERVICE_UNAVAILABLE );                      \
     }                                                                   \
   FT_END_STMNT


 /*  A macro used to define new service structure types
  */

#define FT_DEFINE_SERVICE( name )                                                  \
  typedef struct FT_Service_ ## name ## Rec_          FT_Service_ ## name ## Rec;  \
  typedef struct FT_Service_ ## name ## Rec_ const *  FT_Service_ ## name ;        \
  struct FT_Service_ ## name ## Rec_

 /* */
 
#define FT_SERVICE_MULTIPLE_MASTERS_H  <freetype/internal/services/multmast.h>
#define FT_SERVICE_POSTSCRIPT_NAME_H   <freetype/internal/services/postname.h> 
#define FT_SERVICE_GLYPH_DICT_H        <freetype/internal/services/glyfdict.h>
#define FT_SERVICE_BDF_H               <freetype/internal/services/bdf.h>
#define FT_SERVICE_XFREE86_NAME_H      <freetype/internal/services/xf86name.h>
#define FT_SERVICE_SFNT_H              <freetype/internal/services/sfnt.h>

#endif /* __FT_SERVICE_H__ */
