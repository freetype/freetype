/***************************************************************************/
/*                                                                         */
/*  ftltypes.h                                                             */
/*                                                                         */
/*    Implementation of FreeType Layout API (body)                         */
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/


#include <ft2build.h>
#include FT_LAYOUT_H
#include FT_INTERNAL_FTL_TYPES_H
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_OBJECTS_H
#include FT_SERVICE_LAYOUT_H
#include FT_LIST_H

  static void
  destroy_feaatures_requests ( FT_Memory memory,
			       void * data,
			       void * user );

  static FT_Error
  ft_face_get_layout_service( FT_Face face,
			      FT_Service_Layout  *aservice )
  {
    FT_Error  error;

    *aservice = NULL;

    if ( !face )
      return FT_Err_Invalid_Face_Handle;

    error = FT_Err_Invalid_Argument;


    FT_FACE_LOOKUP_SERVICE( face,
			    *aservice,
			    LAYOUT );

    if ( *aservice )
      error = FT_Err_Ok;

    return error;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_Get_Font                           ( FT_Face face,
					   FTL_Font * font )
  {
    FT_Error error;
    FT_Service_Layout service;

    if (( error = ft_face_get_layout_service( face, &service ) ))
      return error;

    error = FT_Err_Invalid_Argument;
    if ( service->get_font )
      error = service->get_font ( face, font );
    return error;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_Query_EngineType                  ( FT_Face face, 
					  FTL_EngineType * engine_type)
  {
    FT_Error error;
    FT_Service_Layout service;

    if ( !face )
      return FT_Err_Invalid_Argument;

    if (( error = ft_face_get_layout_service( face, &service ) ))
      return error;

    error = FT_Err_Invalid_Argument;
    if ( service->get_engine_type )
      {
	error = FT_Err_Ok;
	*engine_type = service->get_engine_type( face );
      }
    return error;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_New_FeaturesRequest               ( FT_Face face, 
				          FTL_FeaturesRequest* request)
  {
    FT_Error error;
    FT_Service_Layout service;
    FTL_FeaturesRequest arequest = NULL;
    FT_ListNode node = NULL;
    FT_Memory  memory;
    FTL_Font   font;
    
    memory =  face->driver->root.memory;
    
    if ( FT_NEW( node ) )
      goto Exit;
    
    if (( error = ft_face_get_layout_service( face, &service ) ))
      goto Exit;

    if ( service->get_font )
      {
	if (( error = service->get_font( face, &font ) ))
	  goto Failure;
      }
    
    if ( service->new_features_request )
      {
	if (( error = service->new_features_request( face, &arequest ) ))
	  goto Failure;
      }
    else
      {
	if ( FT_NEW( arequest ) )
	  goto Exit;
	if (( error = FTL_FeaturesRequest_Init ( face, arequest ) ))
	  {
	    FT_FREE( arequest );
	    goto Failure;
	  }
      }
    *request   = arequest;
    node->data = arequest;
    FT_List_Add( &font->features_requests_list, node );
    if ( !font->features_request )
      font->features_request = arequest;
    error = FT_Err_Ok;
  Exit:
    return error;
  Failure:
    if ( arequest )
      FT_FREE ( arequest );
    if ( node )
      FT_FREE( node );
    return error;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_FeaturesRequest_Init               ( FT_Face face, 
					   FTL_FeaturesRequest request)
  {
    FT_Error error;
    FTL_Font font;

    if ( ( error = FTL_Get_Font( face, &font ) ) )
      return error;
    request->font      = font;
    request->direction = FTL_HORIZONTAL;
    return error;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_Done_FeaturesRequest              ( FTL_FeaturesRequest request )
  {
    FT_Error error;
    FTL_Font font;
    FT_Face face;
    FT_Memory memory;
    FT_ListNode  node;
    FT_Service_Layout service;
    
    font = request->font;
    FT_ASSERT(font);
    face = font->face;
    FT_ASSERT(face);
    memory =  face->driver->root.memory;

    if (( error = ft_face_get_layout_service( face, &service ) ))
      return error;
    
    node = FT_List_Find( &font->features_requests_list, request );
    FT_ASSERT(node);
    FT_List_Remove ( &font->features_requests_list, node );
    FT_FREE( node );
    if ( font->features_request == request )
      {
	font->features_request = NULL;
	if ( font->features_requests_list.head )
	  font->features_request = (FTL_FeaturesRequest)(font->features_requests_list.head->data);
      }
    if ( service->done_features_request )
      error = service->done_features_request( request );
    else
      {
	error = FTL_FeaturesRequest_Finalize( request );
	FT_FREE( request );	/* TODO */
      }
    return error;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_FeaturesRequest_Finalize           ( FTL_FeaturesRequest request )
  {
    return FT_Err_Ok;
  }


  FT_EXPORT_DEF ( FTL_Direction )
  FTL_Get_FeaturesRequest_Direction     ( FTL_FeaturesRequest request )
  {
    FT_ASSERT( request );
    return request->direction;
  }

  FT_EXPORT_DEF ( void )
  FTL_Set_FeaturesRequest_Direction     ( FTL_FeaturesRequest request,  
					  FTL_Direction direction)
  {
    FT_ASSERT( request );
    request->direction = direction;
  }

  FT_EXPORT_DEF ( FT_Error )
  FTL_Activate_FeaturesRequest          ( FTL_FeaturesRequest request )
  {
    FTL_Font font;
    
    if ( !request )
      return FT_Err_Invalid_Argument;
    
    font = request->font;
    if ( !font )
      return FT_Err_Invalid_Argument;
    font->features_request = request;
    return FT_Err_Ok;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_Copy_FeaturesRequest              ( FTL_FeaturesRequest from,  FTL_FeaturesRequest to )
  {
    FT_Error error;
    FT_Service_Layout service;

    if ( from->font != to->font )
      return FT_Err_Invalid_Argument;
    if ( from == to )
      return FT_Err_Ok;
    
    FT_ASSERT(from->font->face);
    if (( error = ft_face_get_layout_service( from->font->face, &service ) ))
      return error;
    
    if ( service->copy_features_request )
      error = service->copy_features_request( from, to );
    else
      error = FTL_FeaturesRequest_Copy ( from, to );
    return error;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_FeaturesRequest_Copy              ( FTL_FeaturesRequest from,  FTL_FeaturesRequest to )
  {
    FTL_Set_FeaturesRequest_Direction(to, 
				      FTL_Get_FeaturesRequest_Direction(to));
    return FT_Err_Ok;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_Reset_FeaturesRequest             ( FTL_FeaturesRequest request )
  {
    FTL_Font font;
    FTL_FeaturesRequest default_request;
    
    if ( !request )
      return FT_Err_Invalid_Argument;
    
    font = request->font;
    FT_ASSERT( font );
    FTL_Font_Get_Default_FeaturesRequest( font, &default_request );
    return FTL_Copy_FeaturesRequest( (FTL_FeaturesRequest)default_request, 
				     (FTL_FeaturesRequest)request );
    
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_Font_Init                          ( FTL_Font font,
					   FT_Face  face )
  {
    if ( (!font) || (!face) )
      return FT_Err_Invalid_Argument;
    font->face 		   	       = face;
    font->features_request 	       = NULL;
    font->features_requests_list.head  = NULL;
    font->features_requests_list.tail  = NULL;
    return FT_Err_Ok;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_Font_Finalize                      ( FTL_Font font )
  {
    FT_Face face = font->face;
    FT_Memory memory = face->driver->root.memory;
    
    if ( !font )
      return FT_Err_Invalid_Argument;

    FT_List_Finalize( &font->features_requests_list,
		      (FT_List_Destructor)destroy_feaatures_requests,
		      memory,
		      NULL );

    font->features_requests_list.head = NULL;
    font->features_requests_list.tail = NULL;
    font->features_request            = NULL;
    font->face 			      = NULL;
    return FT_Err_Ok;
  }

  static void
  destroy_feaatures_requests ( FT_Memory memory,
			       void * data,
			       void * user )
  {
    FTL_Font font;
    FT_Face face;
    FTL_FeaturesRequest request = data;
    FT_Service_Layout service 	= NULL;
    
    FT_UNUSED(user);
    
    font = request->font;
    FT_ASSERT(font);
    face = font->face;
    FT_ASSERT(face);
    
    ft_face_get_layout_service( face, &service );
      
    if ( service && service->done_features_request )
      service->done_features_request( request );
    else
      {
	FTL_FeaturesRequest_Finalize( request );
	FT_FREE( request );
      }
  }  

  FT_EXPORT_DEF( FT_Error )
  FTL_Font_Get_Default_FeaturesRequest ( FTL_Font font,
					 FTL_FeaturesRequest * request )
  {
    FT_ListNode  node;

    if ( !font )
      return FT_Err_Invalid_Argument;
    node    = font->features_requests_list.head;
    *request =  node->data;
    return FT_Err_Ok;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_Font_Get_Current_FeaturesRequest    ( FTL_Font font,
					    FTL_FeaturesRequest * request )
  {
    *request = font->features_request;
    FT_ASSERT ( *request );
    return FT_Err_Ok;
  }

  FT_EXPORT_DEF( FT_UShort )
  FTL_Get_LigatureCaret_Count           ( FT_Face face, FT_UShort glyphID )
  {
    FT_Error error;
    FT_Service_Layout service;

    if (( error = ft_face_get_layout_service( face, &service ) ))
      return 0;

    if ( service->get_ligature_caret_count )
      return service->get_ligature_caret_count( face, glyphID );
    else
      return 0;
  }

  FT_EXPORT_DEF( FT_UShort )
  FTL_Get_LigatureCaret_Division        ( FT_Face face, 
					  FT_UShort glyphID, 
					  FT_UShort nth )
  {
    FT_Error error;
    FT_Service_Layout service;

    if (( error = ft_face_get_layout_service( face, &service ) ))
      return 0;

    if ( service->get_ligature_caret_division )
      return service->get_ligature_caret_division( face, glyphID, nth );
    else
      return 0;
  }
  
  FT_EXPORT_DEF( FT_Error )
  FTL_New_Glyphs_Array                  ( FT_Memory memory,
					  FTL_Glyphs_Array * garray )
  {
    FT_Error error;
    FTL_Glyphs_Array agarray;
    
    if ( FT_NEW( agarray ) )
      return error;

    agarray->memory   = memory;
    agarray->glyphs   = NULL;
    agarray->length   = 0;
    agarray->allocated = 0;

    *garray 	       = agarray;
    return FT_Err_Ok;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_Set_Glyphs_Array_Length           ( FTL_Glyphs_Array garray,
					  FT_ULong new_length )
  {
    FT_Error error;
    FT_Memory memory = garray->memory;
    
    if ( new_length > garray->allocated )
      {
	if ( FT_RENEW_ARRAY( garray->glyphs, garray->allocated, new_length ) )
	  return error;
	garray->allocated = new_length;
	garray->length    = new_length;
      }
    return FT_Err_Ok;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_Copy_Glyphs_Array                 ( FTL_Glyphs_Array in,
					  FTL_Glyphs_Array out )
  {
    FT_Error error;
    FT_ULong i;
    
    if (( error = FTL_Set_Glyphs_Array_Length( out, in->length ) ))
      return error;

    for ( i = 0; i < in->length; i++ )
      out->glyphs[i] = in->glyphs[i];
    return FT_Err_Ok;
  }

  FT_EXPORT_DEF( FT_Error )
  FTL_Done_Glyphs_Array                 ( FTL_Glyphs_Array garray )
  {
    FT_Memory memory = garray->memory;
    FT_FREE( garray->glyphs );
    FT_FREE( garray );
    return FT_Err_Ok;
  }


  FT_EXPORT_DEF( FT_Error )
  FTL_Substitute_Glyphs                 ( FT_Face face,
					  FTL_Glyphs_Array in,
					  FTL_Glyphs_Array out )
  {
    FT_Error error;
    FT_Service_Layout service;
    FTL_Font font;
    FTL_FeaturesRequest request;
    
    if (( error = ft_face_get_layout_service( face, &service ) ))
      return error;

    if ( ( error = FTL_Get_Font ( face, &font ) ) )
      return error;
    
    if ( ( error = FTL_Font_Get_Current_FeaturesRequest ( font,
							  &request ) ) )
      return error;
    
    if ( service->substitute_glyphs )
      return service->substitute_glyphs( face, request, in, out );
    else
      return FTL_Copy_Glyphs_Array(in, out);
  }


/* END */
