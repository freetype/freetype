/***************************************************************************/
/*                                                                         */
/*  gxobjs.c                                                               */
/*                                                                         */
/*    AAT/TrueTypeGX objects manager (body).                               */
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
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_TRUETYPE_TAGS_H
#include FT_LIST_H
#include FT_ERRORS_H
#include FT_LAYOUT_H
#include FT_INTERNAL_OBJECTS_H
#include FT_SERVICE_LAYOUT_H
#include FT_GXLAYOUT_H
#include "gxobjs.h"
#include "gxload.h"
#include "gxerrors.h"
#include "gxdriver.h"
#include "gxaccess.h"
#include "gxltypes.h"


  extern const FT_Driver_ClassRec tt_driver_class;

  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxobjs

  static FT_Error
  gx_face_init( FT_Stream      stream,
                GX_Face        face,
                FT_Int         face_index,
                FT_Int         num_params,
                FT_Parameter*  params );

  static FT_Error
  gx_font_load ( GX_Face  face );

  static void
  gx_font_done( void * object );

  static FT_Module_Interface
  gx_module_get_interface ( FT_Module module,
			    const char* gx_interface );

  static FT_Error
  gx_face_get_kerning( FT_Face     face,
		       FT_UInt     left_glyph,
		       FT_UInt     right_glyph,
		       FT_Vector*  kerning );

  static const FT_Service_LayoutRec  gx_service_layout =
  {
    (FTL_Get_Font_Func)                   gxl_get_font,
    (FTL_Get_EngineType_Func)             gxl_get_engine_type,
    (FTL_New_FeaturesRequest_Func)        gxl_new_features_request,
    (FTL_Done_FeaturesRequest_Func)       gxl_done_features_request,
    (FTL_Copy_FeaturesRequest_Func)       gxl_copy_features_request,
    (FTL_Get_LigatureCaret_Count_Func)    gxl_get_ligature_caret_count,
    (FTL_Get_LigatureCaret_Division_Func) gxl_get_ligature_caret_division,
    (FTL_Substitute_Glyphs_Func)          gxl_substitute_glyphs
  };

  static const FT_ServiceDescRec  gx_services[] =
  {
    { FT_SERVICE_ID_LAYOUT, &gx_service_layout },
    { NULL, NULL }
  };

  FT_LOCAL_DEF( FT_Error )
  gx_driver_init( GX_Driver  driver )
  {
    FT_Error error;
    
    const void * module_name;
    FT_Driver gx_driver_root = &driver->root;
    FT_Driver_Class gx_driver_class = gx_driver_root->clazz;

    module_name = gx_driver_class->root.module_name;

    *gx_driver_class = tt_driver_class;
    driver->root.clazz->root.module_name  = module_name;
    
    if (( error = ((FT_Module_Class*)gx_driver_class)->module_init( (FT_Module)driver ) ))
      return error;
    
    gx_driver_class->init_face 			       = (FT_Face_InitFunc)gx_face_init;
    gx_driver_class->get_kerning 		       = gx_face_get_kerning;
    ((FT_Module_Class*)gx_driver_class)->get_interface = gx_module_get_interface;

    return GX_Err_Ok;
  }
  
  static FT_Error
  gx_face_init( FT_Stream      stream,
                GX_Face        face,
                FT_Int         face_index,
                FT_Int         num_params,
                FT_Parameter*  params )
  {
    FT_Error error;

    /* TODO */
    error = tt_driver_class.init_face ( stream, (FT_Face)face, face_index, num_params, params );
    if ( error )
      goto Exit;
    
    error = gx_font_load ( face );
    if ( error )
      goto Exit;
  Exit:
    return error;
  }

#define NEW_TABLE(tag)							\
  do									\
    {									\
      if ( FT_NEW( font->tag ) )                                        \
	goto Exit;							\
									\
      FT_TRACE2 (( "\tloading %s...", #tag ));				\
      error = gx_table_load(face,stream,tag,font);			\
      if ( error )							\
	{								\
	  if ( !(face->goto_table( face, TTAG_##tag, stream, 0 )) )	\
	    FT_TRACE2(("no table\n"));					\
	  else if ( (TTAG_kern	        == TTAG_##tag) &&		\
		    (font->kern->version == 1) )			\
	    FT_TRACE2(("old kern\n"));					\
	  else								\
	    FT_TRACE2(("failed\n"));					\
	  FT_FREE ( font->tag );                                        \
	}								\
      else								\
	{								\
	  FT_TRACE2(("successful\n"));					\
	  tables[set_index++] = (GX_Table)font->tag;			\
	}								\
    } while (0)

#define DONE_TABLE(tag)					\
  do							\
  {							\
    if ( font->tag )					\
      {							\
	gx_table_done ( (GX_Table)font->tag, memory );	\
	FT_FREE(font->tag);				\
     }							\
  } while ( 0 )


  static FT_Error
  gx_font_load( GX_Face  face )
  {
    FT_Error error;
    FT_Stream stream = face->root.stream;
    FT_Memory memory = stream->memory;
    GXL_Font font;

#define nTABLES 12
    GX_Table tables[nTABLES];
    FT_Int   init_index, set_index = 0;

    GXL_FeaturesRequest features_request;
    
    for ( init_index = 0; init_index < nTABLES; init_index++ )
      tables[init_index] = NULL;
    
    if ( face->extra.data )
      {
	error = GX_Err_Busy_Extra_Data;
	goto Exit;
      }

    if ( FT_NEW(font) )
      goto Exit;
    
    if (( error = FTL_Font_Init ( (FTL_Font)font, (FT_Face)face ) ))
      goto Failure;
    
    NEW_TABLE(feat);
    NEW_TABLE(mort);
    NEW_TABLE(morx);

    NEW_TABLE(trak);
    NEW_TABLE(kern);
    NEW_TABLE(prop);
    NEW_TABLE(lcar);
    NEW_TABLE(opbd);
    NEW_TABLE(bsln);
    NEW_TABLE(fmtx);
    NEW_TABLE(fdsc);
    NEW_TABLE(just);

    NEW_TABLE(fvar);

    error = FT_Err_Unknown_File_Format;
    
    if (((  font->mort ) || ( font->morx ))
	&& ( font->feat ))
      {
	face->root.face_flags |= FT_FACE_FLAG_GLYPH_SUBSTITUTION;
	error =  GX_Err_Ok;
      }
    if ( font->kern ) 
      {
	face->root.face_flags |= FT_FACE_FLAG_KERNING;
	error = GX_Err_Ok;
      }
    
    if ( error == GX_Err_Ok )
      {
	face->extra.finalizer = gx_font_done;
	face->extra.data = font;
	if (( error = FTL_New_FeaturesRequest ( (FT_Face)face, 
						(FTL_FeaturesRequest*)(&features_request) ) ))
	  goto Failure;
      }
    else
      goto Failure;
  Exit:
    return error;
  Failure:
    for ( ; set_index > 0; set_index--)
      {
	if ( tables[set_index - 1] )
	  {
	    gx_table_done(tables[set_index - 1], memory);
	    FT_FREE(tables[set_index - 1]);
	  }
      }
    FT_FREE(font);
    face->extra.finalizer = NULL;
    face->extra.data = NULL;
    return error;
  }
  
  static void
  gx_font_done( void * object )
  {
    GXL_Font font     = object;
    FT_Face face      = font->root.face;
    FT_Memory memory  = face->driver->root.memory; /* TODO */
    
    DONE_TABLE(mort);
    DONE_TABLE(morx);
    DONE_TABLE(feat);
    DONE_TABLE(trak);
    DONE_TABLE(kern);
    DONE_TABLE(prop);
    DONE_TABLE(bsln);
    DONE_TABLE(lcar);
    DONE_TABLE(opbd);
    DONE_TABLE(fmtx);
    DONE_TABLE(fdsc);
    DONE_TABLE(just);
    DONE_TABLE(fvar);
    
    FTL_Font_Finalize((FTL_Font)font);
    FT_FREE(object);
  }

  static FT_Module_Interface
  gx_module_get_interface( FT_Module module,
			   const char* gx_interface )
  {
    FT_Module_Interface gx;

    gx = ft_service_list_lookup( gx_services, gx_interface );
    if ( gx )
      return gx;
    
    /* TODO */
    if ( tt_driver_class.root.get_interface )
      return tt_driver_class.root.get_interface( module, gx_interface );
    return NULL;
  }

  static FT_Error
  gx_face_get_kerning( FT_Face     face,
		       FT_UInt     left_glyph,
		       FT_UInt     right_glyph,
		       FT_Vector*  kerning )
  {
    FT_Error error;
    FTL_Font font;
    GX_Kern kern;
    FTL_Direction dir;
    FTL_FeaturesRequest request;
    
    kerning->x = 0;
    kerning->y = 0;

    if (( error = FTL_Get_Font ( face, &font ) ))
      return error;

    kern = ((GXL_Font)font)->kern;
    if ( !kern )
      /* Run old kerning handler */
      return tt_driver_class.get_kerning ( face, left_glyph, right_glyph, kerning );

    FTL_Font_Get_Current_FeaturesRequest( font, &request );
    dir = FTL_Get_FeaturesRequest_Direction ( request );

    return gx_kern_get_pair_kerning(kern, 
				    left_glyph, right_glyph, 
				    dir,
				    kerning);
  }


/* END */
