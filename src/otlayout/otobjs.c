/***************************************************************************/
/*                                                                         */
/*  otobjs.c                                                               */
/*                                                                         */
/*    OpenType objects manager (body).                                     */
/*                                                                         */
/***************************************************************************/

#include <ft2build.h>
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_TRUETYPE_TAGS_H
#include FT_LIST_H
#include FT_ERRORS_H
#include FT_LAYOUT_H
#include FT_INTERNAL_OBJECTS_H
#include FT_SERVICE_LAYOUT_H
#include FT_OTLAYOUT_H
#include "otobjs.h"
#include "ot-types.h"
#include "ot-info.h"
#include "oterrors.h"
#include "otdriver.h"
#include "otltypes.h"


  extern const FT_Driver_ClassRec tt_driver_class;

  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_otobjs

  static FT_Error
  ot_face_init( FT_Stream      stream,
                OT_Face        face,
                FT_Int         face_index,
                FT_Int         num_params,
                FT_Parameter*  params );

  static FT_Error
  ot_font_load ( OT_Face  face );

  static void
  ot_font_done( void * object );

  static FT_Module_Interface
  ot_module_get_interface ( FT_Module module,
			    const char* ot_interface );

  static const FT_Service_LayoutRec  ot_service_layout =
  {
    (FTL_Get_Font_Func)                   otl_get_font,
    (FTL_Get_EngineType_Func)             otl_get_engine_type,
    (FTL_New_FeaturesRequest_Func)        otl_new_features_request,
    (FTL_Done_FeaturesRequest_Func)       otl_done_features_request,
    (FTL_Copy_FeaturesRequest_Func)       otl_copy_features_request,
    (FTL_Get_LigatureCaret_Count_Func)    NULL, /* TODO */
    (FTL_Get_LigatureCaret_Division_Func) NULL, /* TODO */
    (FTL_Substitute_Glyphs_Func)          otl_substitute_glyphs 
  };

  static const FT_ServiceDescRec  ot_services[] =
  {
    { FT_SERVICE_ID_LAYOUT, &ot_service_layout },
    { NULL, NULL }
  };

  FT_LOCAL_DEF( FT_Error )
  ot_driver_init( OT_Driver  driver )
  {
    FT_Error error;
    
    const void * module_name;
    FT_Driver ot_driver_root = &driver->root;
    FT_Driver_Class ot_driver_class = ot_driver_root->clazz;

    module_name = ot_driver_class->root.module_name;

    *ot_driver_class = tt_driver_class;
    driver->root.clazz->root.module_name  = module_name;
    
    if (( error = ((FT_Module_Class*)ot_driver_class)->module_init( (FT_Module)driver ) ))
      return error;
    
    ot_driver_class->init_face 			       = (FT_Face_InitFunc)ot_face_init;
    ((FT_Module_Class*)ot_driver_class)->get_interface = ot_module_get_interface;

    return OT_Err_Ok;
  }
  
  static FT_Error
  ot_face_init( FT_Stream      stream,
                OT_Face        face,
                FT_Int         face_index,
                FT_Int         num_params,
                FT_Parameter*  params )
  {
    FT_Error error;
    
    /* TODO */
    error = tt_driver_class.init_face ( stream, (FT_Face)face, face_index, num_params, params );
    if ( error )
      goto Exit;
    
    error = ot_font_load ( face );
    if ( error )
      goto Exit;
  Exit:
    return error;
  }

  static FT_Error
  ot_font_load( OT_Face  face )
  {
    FT_Error error;
    FT_Memory memory = FT_FACE_MEMORY( face );
    
    OTL_Font font = NULL;
    OTL_FeaturesRequest features_request;
    OTInfo * ot_info;
    
    if ( face->extra.data )
      {
	error = OT_Err_Busy_Extra_Data;
	goto Exit;
      }

    ot_info = ot_info_get ( (FT_Face) face );
    if ( !ot_info )
      {
	error = OT_Err_Unknown_File_Format;
	goto Exit;
      }

    if ( FT_NEW ( font ) )
      goto Exit;

    font->info = ot_info;
    if (( error = FTL_Font_Init ( (FTL_Font)font, (FT_Face)face ) ))
      goto Failure;

    face->extra.finalizer = ot_font_done;
    face->extra.data = font;
    if (( error = FTL_New_FeaturesRequest ( (FT_Face)face, 
					    (FTL_FeaturesRequest*)(&features_request) ) ))
      goto Failure;
    face->root.face_flags |= FT_FACE_FLAG_GLYPH_SUBSTITUTION; /* ??? */
    error =  OT_Err_Ok;
  Exit:
    return error;
  Failure:
    if ( font )
      FT_FREE( font );
    face->extra.finalizer = NULL;
    face->extra.data = NULL;
    return error;
  }
  
  static void
  ot_font_done( void * object )
  {
    OTL_Font font     = object;
    FT_Face face      = FTL_FONT_FACE( font );
    FT_Memory memory  = FT_FACE_MEMORY( face );
    
    FTL_Font_Finalize((FTL_Font)font);
    FT_FREE( object );
  }

  static FT_Module_Interface
  ot_module_get_interface( FT_Module module,
			   const char* ot_interface )
  {
    FT_Module_Interface ot;

    ot = ft_service_list_lookup( ot_services, ot_interface );
    if ( ot )
      return ot;
    
    /* TODO */
    if ( tt_driver_class.root.get_interface )
      return tt_driver_class.root.get_interface( module, ot_interface );
    return NULL;
  }
