/***************************************************************************/
/*                                                                         */
/*  ftcache.h                                                              */
/*                                                                         */
/*    FreeType Cache subsystem                                             */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef FTCACHE_H
#define FTCACHE_H

#include <freetype/ftglyph.h>



 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/
 /*****                                                                *****/
 /*****                     BASIC TYPE DEFINITIONS                     *****/
 /*****                                                                *****/
 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/



 /**************************************************************************
  *
  *  <Type>
  *     FTC_FaceID
  *
  *  <Description>
  *     a generic pointer type that is used to identity face objects.
  *     the content of such objects is application-dependent
  *
  **************************************************************************/
  
  typedef FT_Pointer  FTC_FaceID;


  
 /**************************************************************************
  *
  *  <FuncType>
  *     FTC_Face_Requester
  *
  *  <Description>
  *     a callback function provided by client applications. It is used
  *     to translate a given FTC_FaceID into a new valid FT_Face object
  *
  *  <Input>
  *     face_id :: the face id to resolve
  *     library :: handle to a FreeType library object
  *     data    :: application-provided request data
  *
  *  <Output>
  *     aface   :: a new FT_Face handle
  *
  *  <Return>
  *     Error code. 0 means success
  *
  **************************************************************************/
  
  typedef FT_Error  (*FTC_Face_Requester)( FTC_FaceID  face_id,
                                           FT_Library  library,
                                           FT_Pointer  request_data, 
                                           FT_Face*    aface );


 /**************************************************************************
  *
  *  <Struct>
  *     FTC_SizeRec
  *
  *  <Description>
  *     A simple structure used to describe a given "font size" to the
  *     cache manager
  *
  *  <Fields>
  *     face_id    :: id of face to use
  *     pix_width  :: character width in integer pixels
  *     pix_height :: character height in integer pixels
  *
  **************************************************************************/

  typedef struct  FTC_SizeRec_
  {
    FTC_FaceID  face_id;
    FT_UShort   pix_width;
    FT_UShort   pix_height;
    
  } FTC_SizeRec;


 /**************************************************************************
  *
  *  <Type>
  *     FTC_SizeID
  *
  *  <Description>
  *     A simple handle to a FTC_SizeRec structure
  *
  **************************************************************************/
  
  typedef FTC_SizeRec*  FTC_SizeID;
 


 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/
 /*****                                                                *****/
 /*****                       CACHE MANAGER OBJECT                     *****/
 /*****                                                                *****/
 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/




 /**************************************************************************
  *
  *  <Type>
  *     FTC_Manager
  *
  *  <Description>
  *     This object is used to cache one or more FT_Face object, along with
  *     corresponding FT_Size objects.
  *    
  **************************************************************************/
  
  typedef struct FTC_ManagerRec_*  FTC_Manager;


 /**************************************************************************
  *
  *  <Function>
  *     FTC_Manager_New
  *
  *  <Description>
  *     Create a new cache manager.
  *
  *  <Input>
  *     library   :: the parent FreeType library handle to use
  *
  *     max_faces :: maximum number of faces to keep alive in manager
  *                  use 0 for defaults
  *
  *     max_sizes :: maximum number of sizes to keep alive in manager
  *                  use 0 for defaults
  *
  *     requester :: an application-provided callback used to translate
  *                  face IDs into real FT_Face objects
  *
  *     req_data  :: a generic pointer that is passed to the requester
  *                  each time it is called (see FTC_Face_Requester)
  *
  *  <Output>
  *     amanager  :: handle to new manager object. 0 in case of failure
  *
  *  <Return>
  *     Error code. 0 means success
  *
  **************************************************************************/
  
  FT_EXPORT_DEF( FT_Error )  FTC_Manager_New( FT_Library          library,
                                              FT_UInt             max_faces,
					      FT_UInt             max_sizes,
                                              FTC_Face_Requester  requester,
                                              FT_Pointer          req_data,
                                              FTC_Manager*        amanager );


                        
 /**************************************************************************
  *
  *  <Function>
  *     FTC_Manager_Reset
  *
  *  <Description>
  *     Empty a given cache manager. This simply gets rid of all the
  *     currently cached FT_Face & FT_Size objects within the manager
  *
  *  <Input>
  *     manager :: handle to manager
  *
  **************************************************************************/

  FT_EXPORT_DEF( void )      FTC_Manager_Reset( FTC_Manager  manager );


 /**************************************************************************
  *
  *  <Function>
  *     FTC_Manager_Done
  *
  *  <Description>
  *     destroys a given manager after emptying it..
  *
  *  <Input>
  *     manager :: handle to target cache manager object
  *
  **************************************************************************/
  
  FT_EXPORT_DEF( void )      FTC_Manager_Done( FTC_Manager  manager );


 /**************************************************************************
  *
  *  <Function>
  *     FTC_Manager_Lookup_Face
  *
  *  <Description>
  *     retrieves the FT_Face that corresponds to a given face ID through
  *     a cache manager.
  *
  *  <Input>
  *     manager  :: handle to cache manager
  *     face_id  :: ID of face object
  *
  *  <Output>
  *     aface     :: handle to face object
  *
  *  <Return>
  *     Error code. 0 means success
  *
  *  <Note>
  *     The returned FT_Face object is always owned by the manager, you
  *     should never try to discard it yourself..
  *
  *     The FT_Face object doesn't necessarily have a current size object
  *     (i.e. face->size can be 0). If you need a specific "font size",
  *     use FTC_Manager_Lookup_Size instead..
  *
  *     Never change the face's transform (i.e. NEVER CALL FT_Set_Transform)
  *     on a returned face. If you need to transform glyphs, do it yourself
  *     after glyph loading..
  *
  **************************************************************************/
  
  FT_EXPORT_DEF( FT_Error )  FTC_Manager_Lookup_Face( FTC_Manager  manager,
                                                      FTC_FaceID   face_id,
                                                      FT_Face*     aface );
 

 /**************************************************************************
  *
  *  <Function>
  *     FTC_Manager_Lookup_Size
  *
  *  <Description>
  *     retrieves the FT_Face & FT_Size that correspond to a given
  *     FTC_SizeID
  *
  *  <Input>
  *     manager  :: handle to cache manager.
  *     size_id  :: ID of the "font size" to use
  *
  *  <InOut>
  *     aface    :: ptr to handle to face object. Set to 0 if you don't need it
  *     asize    :: ptr to handle to size object. Set to 0 if you don't need it
  *
  *  <Return>
  *     Error code. 0 means success
  *
  *  <Note>
  *     The returned FT_Face object is always owned by the manager, you
  *     should never try to discard it yourself..
  *
  *     Never change the face's transform (i.e. NEVER CALL FT_Set_Transform)
  *     on a returned face. If you need to transform glyphs, do it yourself
  *     after glyph loading..
  *
  *     The returned FT_Size object is always owned by the manager, you
  *     should never try to discard it, NEVER CHANGE ITS SETTINGS THROUGH
  *     FT_Set_Pixel_Sizes OR FT_Set_Char_Size !!
  *
  *     The returned size object is the face's current size, which means
  *     that you can call FT_Load_Glyph with the face if you need to..
  *
  **************************************************************************/
  
  FT_EXPORT_DEF( FT_Error )  FTC_Manager_Lookup_Size( FTC_Manager  manager,
                                                      FTC_SizeID   size_id,
                                                      FT_Face*     aface,
                                                      FT_Size*     asize );

  
 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/
 /*****                                                                *****/
 /*****                        IMAGE CACHE OBJECT                      *****/
 /*****                                                                *****/
 /**************************************************************************/
 /**************************************************************************/
 /**************************************************************************/


 /**************************************************************************
  *
  *  <Enum>
  *     FTC_Image_Type
  *
  *  <Description>
  *     An enumeration used to list the types of glyph images found in a
  *     glyph image cache.
  *
  *  <Fields>
  *     ftc_image_mono     :: monochrome bitmap glyphs
  *     ftc_image_grays    :: anti-aliased bitmap glyphs
  *     ftc_image_outline  :: scaled (and hinted) outline glyphs
  *     ftc_master_outline :: unscaled original outline glyphs
  *
  *  <Note>
  *     other types may be defined in the future
  *
  **************************************************************************/

  typedef enum  FTC_Image_Type_
  {
    ftc_image_mono = 0,         /* monochrome bitmap   */
    ftc_image_grays,            /* anti-aliased bitmap */
    ftc_image_outline,          /* scaled outline      */
    ftc_image_master_outline    /* original outline    */
  
  } FTC_Image_Type;

 
 /**************************************************************************
  *
  *  <Struct>
  *     FTC_Image_Desc
  *
  *  <Description>
  *     A simple structure used to describe a given glyph image category
  *
  *  <Fields>
  *     size       :: a FTC_SizeRec used to describe the glyph's face & size
  *     image_type :: the glyph image's type
  *
  **************************************************************************/

  typedef struct FTC_Image_Desc_
  {
    FTC_SizeRec  size;
    FT_UInt      image_type;
  
  } FTC_Image_Desc;



 /**************************************************************************
  *
  *  <Type>
  *     FTC_Image_Cache
  *
  *  <Description>
  *     A handle to an glyph image cache object.. They are designed to
  *     hold many distinct glyph images, while not exceeding a certain
  *     memory threshold..
  *
  **************************************************************************/

  typedef struct FTC_Image_CacheRec_*  FTC_Image_Cache; 



 /**************************************************************************
  *
  *  <Function>
  *     FTC_Image_Cache_New
  *
  *  <Description>
  *     Create a new glyph image cache.
  *
  *  <Input>
  *     manager   :: parent manager for the image cache
  *
  *     max_bytes :: the maximum amount of memory that will be used to
  *                  store glyph images
  *
  *  <Output>
  *     acache    :: handle to new glyph image cache object
  *
  *  <Return>
  *     Error code. 0 means success
  *
  **************************************************************************/
  
  FT_EXPORT_DEF( FT_Error )  FTC_Image_Cache_New( FTC_Manager       manager,
                                                  FT_ULong          max_bytes,
                                                  FTC_Image_Cache*  acache );
                                                
 /**************************************************************************
  *
  *  <Function>
  *     FTC_Image_Cache_Done
  *
  *  <Description>
  *     Destroys a given glyph image cache (and all glyphs within it).
  *
  *  <Input>
  *     manager   :: parent manager for the image cache
  *
  **************************************************************************/
  
  FT_EXPORT_DEF( void )      FTC_Image_Cache_Done( FTC_Image_Cache  cache );


 /**************************************************************************
  *
  *  <Function>
  *     FTC_Image_Cache_Lookup
  *
  *  <Description>
  *     Retrieve a given glyph image from a glyph image cache.
  *
  *  <Input>
  *     cache   :: handle to source glyph image cache
  *     desc    :: pointer to a glyph image descriptor
  *     gindex  :: index of glyph to retrieve
  *
  *  <Output>
  *     aglyph  :: the corresponding FT_Glyph object. 0 in case of failure
  *
  *  <Return>
  *     Error code. 0 means success
  *
  *  <Note>
  *     the returned glyph is owned and manager by the glyph image cache,
  *     never try to transform or discard it manually. You can however
  *     create a copy with FT_Glyph_Copy and modify the new one at will.
  *
  *     Because the glyph image cache limits the total amount of memory
  *     taken by the glyphs it holds, the returned glyph might disappear
  *     on a later invocation of this function !! It's a cache after all ;-)
  *
  **************************************************************************/
  
  FT_EXPORT_DEF( FT_Error )  FTC_Image_Cache_Lookup(
                               FTC_Image_Cache  cache,
                               FTC_Image_Desc*  desc,
                               FT_UInt          gindex,
                               FT_Glyph*        aglyph );


#endif /* FTCACHE_H */
