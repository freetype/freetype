/***************************************************************************/
/*                                                                         */
/*  ftrender.h                                                             */
/*                                                                         */
/*  FreeType renderer modules public interface                             */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef FTRENDER_H
#define FTRENDER_H

#include <freetype/ftmodule.h>
#include <freetype/ftglyph.h>

 /* create a new glyph object */
  typedef  FT_Error  (*FT_Glyph_Init_Func)( FT_Glyph      glyph,
                                            FT_GlyphSlot  slot );

 /* destroys a given glyph object */
  typedef  void      (*FT_Glyph_Done_Func)( FT_Glyph   glyph );

  typedef  void      (*FT_Glyph_Transform_Func)( FT_Glyph    glyph,
                                                 FT_Matrix*  matrix,
                                                 FT_Vector*  delta );
                                                 
  typedef  void      (*FT_Glyph_BBox_Func)( FT_Glyph    glyph,
                                            FT_BBox    *abbox );

  typedef  FT_Error  (*FT_Glyph_Copy_Func)( FT_Glyph   source,
                                            FT_Glyph   target );
                                       
  typedef  FT_Error  (*FT_Glyph_Prepare_Func)( FT_Glyph      glyph,
                                               FT_GlyphSlot  slot );

  struct FT_Glyph_Class_
  {
    FT_UInt                  glyph_size;
    FT_Glyph_Format          glyph_format;
    FT_Glyph_Init_Func       glyph_init;
    FT_Glyph_Done_Func       glyph_done;
    FT_Glyph_Copy_Func       glyph_copy;
    FT_Glyph_Transform_Func  glyph_transform;
    FT_Glyph_BBox_Func       glyph_bbox;
    FT_Glyph_Prepare_Func    glyph_prepare;

  };


 /*************************************************************************
  *
  *  <Struct>
  *     FT_Renderer_Class
  *
  *  <Description>
  *     The renderer module class descriptor.
  *
  *  <Fields>
  *     root          :: the root FT_Module_Class fields
  *
  *     glyph_format  :: the glyph image format this renderer handles
  *
  *     render_glyph  :: a method used to render the image that is in a
  *                      given glyph slot into a bitmap.
  *
  *     set_mode      :: a method used to pass additional parameters
  *
  *     raster_class  :: for ft_glyph_format_outline renderers only, this
  *                      is a pointer to its raster's class.
  *
  *     raster        :: for ft_glyph_format_outline renderers only. this
  *                      is a pointer to the corresponding raster object,
  *                      if any..
  *
  *************************************************************************/

  typedef FT_Error  (*FTRenderer_render)( FT_Renderer   renderer,
                                          FT_GlyphSlot  slot,
                                          FT_UInt       mode,
                                          FT_Vector*    origin );

  typedef FT_Error  (*FTRenderer_transform)( FT_Renderer   renderer,
                                             FT_GlyphSlot  slot,
                                             FT_Matrix*    matrix,
                                             FT_Vector*    delta );

  typedef void      (*FTRenderer_getCBox)( FT_Renderer   renderer,
                                           FT_GlyphSlot  slot,
                                           FT_BBox      *cbox );

  typedef FT_Error  (*FTRenderer_setMode)( FT_Renderer  renderer,
                                           FT_ULong     mode_tag,
                                           FT_Pointer   mode_ptr );

  typedef struct  FT_Renderer_Class_
  {
    FT_Module_Class       root;
    
    FT_Glyph_Format       glyph_format;
    
    FTRenderer_render     render_glyph;
    FTRenderer_transform  transform_glyph;
    FTRenderer_getCBox    get_glyph_cbox;
    FTRenderer_setMode    set_mode;

    FT_Raster_Funcs*      raster_class;

  } FT_Renderer_Class;


 /*************************************************************************
  *
  *  <Function>
  *     FT_Get_Renderer
  *
  *  <Description>
  *     retrieves the current renderer for a given glyph format.
  *
  *  <Input>
  *     library  :: handle to library object
  *     format   :: glyph format
  *
  *  <Return>
  *     renderer handle. 0 if none found.
  *
  *  <Note>
  *     An error will be returned if a module already exists by that
  *     name, or if the module requires a version of freetype that is
  *     too great
  *
  *     To add a new renderer, simply use FT_Add_Module. To retrieve
  *     a renderer by its name, use FT_Get_Module
  *
  *************************************************************************/
  
  FT_EXPORT_DEF(FT_Renderer)  FT_Get_Renderer( FT_Library       library,
                                               FT_Glyph_Format  format );


 /*************************************************************************
  *
  *  <Function>
  *     FT_Set_Renderer
  *
  *  <Description>
  *     Sets the current renderer to use, and set additional mode
  *
  *  <Input>
  *     library     :: handle to library object
  *     renderer    :: handle to renderer object
  *     num_params  :: number of additional parameters
  *     params      :: additional parameters
  *
  *  <Return>
  *     Error code. 0 means success.
  *
  *  <Note>
  *     in case of success, the renderer will be used to convert glyph
  *     images in the renderer's known format into bitmaps.
  *
  *     This doesn't change the current renderer for other formats..
  *
  *************************************************************************/
  
  FT_EXPORT_DEF(FT_Error) FT_Set_Renderer( FT_Library     library,
                                           FT_Renderer    renderer,
                                           FT_UInt        num_params,
                                           FT_Parameter*  parameters );



#endif /* FTMODULE_H */


/* END */
