/***************************************************************************/
/*                                                                         */
/*  ftcmanag.h                                                             */
/*                                                                         */
/*    FreeType Cache Manager                                               */
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


#include <cache/ftcmanag.h>
#include <freetype/internal/ftobjs.h>


#define FTC_LRU_GET_MANAGER( lru )  (FTC_Manager)lru->user_data


 /**************************************************************************/
 /**************************************************************************/
 /*****                                                                *****/
 /*****               FACE & SIZE LRU CALLBACKS                        *****/
 /*****                                                                *****/
 /**************************************************************************/
 /**************************************************************************/

  static
  FT_Error  ftc_manager_init_face( FT_Lru      lru,
                                   FT_LruNode  node )
  {
    FTC_Manager  manager = FTC_LRU_GET_MANAGER( lru );
    FT_Error     error;
    

    error = manager->request_face( (FTC_FaceID)node->key,
                                   manager->library,
                                   manager->request_data,
                                   (FT_Face*)&node->root.data );
    if ( !error )
    {
      /* destroy initial size object; it will be re-created later */
      FT_Face  face = (FT_Face)node->root.data;


      FT_Done_Size( face->size );
    }

    return error;
  }


  /* helper function for ftc_manager_done_face */
  static
  FT_Bool  ftc_manager_size_selector( FT_Lru      lru,
                                      FT_LruNode  node,
                                      FT_Pointer  data )
  {
    FT_UNUSED( lru );

    return ((FT_Size)node->root.data)->face == (FT_Face)data;
  }


  static
  void  ftc_manager_done_face( FT_Lru      lru,
                               FT_LruNode  node )
  {
    FTC_Manager  manager = FTC_LRU_GET_MANAGER( lru );
    FT_Face      face    = (FT_Face)node->root.data;
    

    /* we must begin by removing all sizes for the target face */
    /* from the manager's list                                 */
    FT_Lru_Remove_Selection( manager->sizes_lru,
                             ftc_manager_size_selector,
                             face );

    /* all right, we can discard the face now */    
    FT_Done_Face( face );
    node->root.data = 0;
  }


  typedef struct  FTC_SizeRequest_
  {
    FT_Face    face;
    FT_UShort  width;
    FT_UShort  height;
    
  } FTC_SizeRequest;


  static
  FT_Error  ftc_manager_init_size( FT_Lru      lru,
                                   FT_LruNode  node )
  {
    FTC_SizeRequest*  size_req = (FTC_SizeRequest*)node->key;
    FT_Size           size;
    FT_Error          error;
    FT_Face           face = size_req->face;
    
    FT_UNUSED( lru );

    
    node->root.data = 0;
    error = FT_New_Size( face, &size );
    if ( !error )
    {
      face->size = size;
      error = FT_Set_Pixel_Sizes( face,
                                  size_req->width,
                                  size_req->height );
      if ( error )
        FT_Done_Size( size );
      else
        node->root.data = size;
    }
    return error;   
  }


  static
  void  ftc_manager_done_size( FT_Lru      lru,
                               FT_LruNode  node )
  {
    FT_UNUSED( lru );

    FT_Done_Size( (FT_Size)node->root.data );
  }                                


  static
  FT_Error  ftc_manager_flush_size( FT_Lru      lru,
                                    FT_LruNode  node,
                                    FT_LruKey   key )
  {
    FTC_SizeRequest*  req  = (FTC_SizeRequest*)key;
    FT_Size           size = (FT_Size)node->root.data;
    FT_Error          error;
    

    if ( size->face == req->face )
    {
      size->face->size = size;  /* set current size */
      error = FT_Set_Pixel_Sizes( req->face, req->width, req->height );
      if ( error )
        FT_Done_Size( size );
    }
    else
    {
      FT_Done_Size( size );
      node->key = key;
      error = ftc_manager_init_size( lru, node );
    }
    return error;
  }


  static
  FT_Bool  ftc_manager_compare_size( FT_LruNode  node,
                                     FT_LruKey   key )
  {
    FTC_SizeRequest*  req  = (FTC_SizeRequest*)key;
    FT_Size           size = (FT_Size)node->root.data;
    
    FT_UNUSED( node );


    return ( size->face           == req->face   &&
             size->metrics.x_ppem == req->width  &&
             size->metrics.y_ppem == req->height );
  }

  
  static
  const FT_Lru_Class  ftc_face_lru_class =
  {
    sizeof ( FT_LruRec ),
    ftc_manager_init_face,
    ftc_manager_done_face,
    0,
    0
  };

  
  static
  const FT_Lru_Class  ftc_size_lru_class =
  {
    sizeof ( FT_LruRec ),
    ftc_manager_init_size,
    ftc_manager_done_size,
    ftc_manager_flush_size,
    ftc_manager_compare_size
  };


  FT_EXPORT_FUNC( FT_Error )  FTC_Manager_New( FT_Library          library,
                                               FT_UInt             max_faces,
					       FT_UInt             max_sizes,
                                               FTC_Face_Requester  requester,
                                               FT_Pointer          req_data,
                                               FTC_Manager*        amanager )
  {
    FT_Error     error;
    FT_Memory    memory  = library->memory;
    FTC_Manager  manager = 0;
    
    
    if ( ALLOC( manager, sizeof ( *manager ) ) )
      goto Exit;
    
    if (max_faces == 0)
      max_faces = FTC_MAX_FACES;
      
    if (max_sizes == 0)
      max_sizes = FTC_MAX_SIZES;
      
    error = FT_Lru_New( &ftc_face_lru_class,
                        max_faces,
                        manager,
                        memory,
                        1, /* pre_alloc = TRUE */
                        (FT_Lru*)&manager->faces_lru );
    if ( error )
      goto Exit;
      
    error = FT_Lru_New( &ftc_size_lru_class,
                        max_sizes,
                        manager,
                        memory,
                        1, /* pre_alloc = TRUE */
                        (FT_Lru*)&manager->sizes_lru );                    
    if ( error )
      goto Exit;
    
    manager->library      = library;
    manager->request_face = requester;
    manager->request_data = req_data;
    *amanager = manager;
    
  Exit:
    if ( error && manager )
    {
      FT_Lru_Done( manager->sizes_lru );
      FT_Lru_Done( manager->faces_lru );
      FREE( manager );
    }
     
    return error;
  }
                        

  FT_EXPORT_DEF( void )  FTC_Manager_Done( FTC_Manager  manager )
  {
    FT_Memory  memory = manager->library->memory;
    

    FT_Lru_Done( manager->sizes_lru );
    FT_Lru_Done( manager->faces_lru );
    FREE( manager );
  }


  FT_EXPORT_DEF( void )  FTC_Manager_Reset( FTC_Manager  manager )
  {
    FT_Lru_Reset( manager->sizes_lru );
    FT_Lru_Reset( manager->faces_lru );
  }


  FT_EXPORT_DEF( FT_Error )  FTC_Manager_Lookup_Face( FTC_Manager  manager,
                                                      FTC_FaceID   face_id,
                                                      FT_Face*     aface )
  {
    return  FT_Lru_Lookup( manager->faces_lru,
                           (FT_LruKey)face_id, 
                           (FT_Pointer*)aface );
  }
 
 
 
  FT_EXPORT_DEF( FT_Error )  FTC_Manager_Lookup_Size( FTC_Manager  manager,
                                                      FTC_SizeID   size_id,
                                                      FT_Face*     aface,
                                                      FT_Size*     asize )
  {
    FTC_SizeRequest  req;
    FT_Error         error;
    FT_Face          face;
    

    if (aface)
      *aface = 0;
      
    if (asize)
      *asize = 0;

    error  = FTC_Manager_Lookup_Face( manager, size_id->face_id, &face ); 
    if ( !error )
    {
      FT_Size  size;
      
      req.face   = face;
      req.width  = size_id->pix_width;
      req.height = size_id->pix_height;
      
      error = FT_Lru_Lookup( manager->sizes_lru,
                             (FT_LruKey)&req,
                             (FT_Pointer*)&size );
      if ( !error )
      {
        /* select the size as the current one for this face */
        face->size = size;
        
        if (asize)
          *asize = size;
          
        if (aface)
          *aface = face;
      }
    }

    return error;
  }


/* END */
