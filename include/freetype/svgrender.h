/****************************************************************************
 *
 * svgrenderer.h
 *
 *   Interface for SVG Renderer Module (specification).
 *
 * Copyright (C) 2004-2019 by
 * David Turner, Robert Wilhelm, Werner Lemberg and Moazin Khatti.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef FTSVG_RENDERER_H_
#define FTSVG_RENDERER_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif

FT_BEGIN_HEADER

  /**************************************************************************
   *
   * @functype:
   *   SVG_Lib_Init_Func
   *
   * @description:
   *   A callback used to initiate the SVG Rendering port
   *
   * @input:
   *   library ::
   *     A instance of library.  This is required to initialize the
   *     renderer's state which will be held in the library.
   *
   * @return:
   *   FreeType error code.  0 means success.
   */

  typedef FT_Error
  (*SVG_Lib_Init_Func)( FT_Library  library );


  /**************************************************************************
   *
   * @functype:
   *   SVG_Lib_Free_Func
   *
   * @description:
   *   A callback used to free the SVG Rendering port.  Calling this callback
   *   shall do all cleanups that the SVG Rendering port wants to do.
   *
   * @input:
   *   library ::
   *     A instance of library.  This is required to free the renderer's
   *     state which will be held in the library.
   */

  typedef void
  (*SVG_Lib_Free_Func)( FT_Library  library );


  /**************************************************************************
   *
   * @functype:
   *   SVG_Lib_Render_Func
   *
   * @description:
   *   A callback used to render the glyph loaded in the slot.
   *
   * @input:
   *   slot ::
   *     The whole glyph slot object.
   *
   *   outline_bbox ::
   *     The bounding box of the glyph in font units.  So that the renderer
   *     may not need to calculate it again.
   *
   * @return:
   *   FreeType error code.  0 means success.
   */

  typedef FT_Error
  (*SVG_Lib_Render_Func)( FT_GlyphSlot  slot,
                          FT_BBox       outline_bbox);

  /**************************************************************************
   *
   * @functype:
   *   SVG_Lib_Get_Buffer_Size_Func
   *
   * @description:
   *   A callback which is called to get the size of the image buffer needed.
   *   This buffer will ultimately be populated by `SVG_Lib_Render_Func`
   *   hook.
   *
   * @input:
   *   slot ::
   *     The glyph slot which has the SVG document loaded as well as other
   *     info.
   *
   *   bbox ::
   *     The bbox in font units. This is required for the rendering port to
   *     predict the final size of the image buffer.
   *
   * @return:
   *   Size of the state structure in bytes.
   */

  typedef FT_ULong
  (*SVG_Lib_Get_Buffer_Size_Func)( FT_GlyphSlot  slot,
                                   FT_BBox       bbox );


  /**************************************************************************
   *
   * @functype:
   *   SVG_Set_Hooks_Func
   *
   * @description:
   *   A function that is used set SVG Hooks.   Part of the SVG Renderer
   *   Interface.
   *
   * @input:
   *   module ::
   *     `FT_Module` instance.
   *
   *   init_svg ::
   *     A function pointer of the type `SVG_Lib_Init_Func`.  Read the
   *     documentation of `SVG_Lib_Init_Func`.
   *
   *   free_svg ::
   *     A function pointer of the type `SVG_Lib_Free_Func`.  Read the
   *     documentation of `SVG_Lib_Free_Func`.
   *
   *   render_svg ::
   *     A function pointer of the type `SVG_Lib_Render_Func`.  Read the
   *     documentation of `SVG_Lib_Render_Func`.
   *
   *   get_buffer_size ::
   *     A function pointer of the type `SVG_Lib_Get_Buffer_Size_Func`.
   *     Read the documentation of `SVG_Lib_Get_Buffer_Size_Func`.
   *
   * @return:
   *   FreeType error code.  0 means success.
   */

  typedef FT_Error
  (*SVG_Set_Hooks_Func)( FT_Module                     module,
                         SVG_Lib_Init_Func             init_svg,
                         SVG_Lib_Free_Func             free_svg,
                         SVG_Lib_Render_Func           render_svg,
                         SVG_Lib_Get_Buffer_Size_Func  get_buffer_size );

  /**************************************************************************
   *
   * @struct:
   *   SVG_Renderer_Interface
   *
   * @description:
   *   An interface structure that function needed to inject external SVG
   *   rendering library hooks.
   *
   * @fields:
   *   set_hooks ::
   *     A function that can be called to set the hooks.
   *
   * @return:
   *   FreeType error code.  0 means success.
   */

  typedef struct SVG_Renderer_Interface_
  {
    SVG_Set_Hooks_Func  set_hooks;
  } SVG_Renderer_Interface;


  /* TODO: to document */
  FT_EXPORT( FT_Error )
  FT_Set_Svg_Hooks( FT_Library                    library,
                    SVG_Lib_Init_Func             init_svg,
                    SVG_Lib_Free_Func             free_svg,
                    SVG_Lib_Render_Func           render_svg,
                    SVG_Lib_Get_Buffer_Size_Func  get_buffer_size );

  /**************************************************************************
   *
   * @struct:
   *   FT_SVG_DocumentRec_
   *
   * @description:
   *   A structure that models one SVG document.
   *
   * @fields:
   *   svg_document ::
   *     A pointer to the SVG document string.
   *
   *   svg_document_length ::
   *     The length of the SVG document string.
   *
   *   metrics ::
   *     A metrics object storing the size information.
   *
   *   units_per_EM ::
   *     The size of the EM square.
   *
   *   start_glyph_id ::
   *     The starting glyph ID for the glyph range that this document has.
   *
   *   end_glyph_id ::
   *     The ending glyph ID for the glyph range that this document has.
   *
   * @note:
   *   `metrics` and `units_per_EM` might look like repetitions since both
   *   fields are stored in face objects.  However, the Glyph Management API
   *   requires an `FT_Glyph` to store all the information that completely
   *   describes a glyph.  Outline glyphs are themselves scaled thus they
   *   don`t need this information.  However, SVG documents do.  The field
   *   of `units_per_EM` is needed because the SVG is to be scaled in case
   *   its viewbox size differs from `units_per_EM`.  For more info, refer
   *   to the section `Coordinate Systems and Glyph Metrics` of the OpenType
   *   SVG specs.
   */

  typedef struct FT_SVG_DocumentRec_
  {
    FT_Byte*         svg_document;
    FT_ULong         svg_document_length;
    FT_Size_Metrics  metrics;
    FT_UShort        units_per_EM;
    FT_UShort        start_glyph_id;
    FT_UShort        end_glyph_id;
    /* TODO: (OT-SVG) Not storing glyph_index here for now. Might need to
     * at some point. Review this! */
  } FT_SVG_DocumentRec;

  /**************************************************************************
   *
   * @type:
   *   FT_SVG_Document
   *
   * @description:
   *   A handle to a FT_SVG_DocumentRec object.
   */
  typedef struct FT_SVG_DocumentRec_*  FT_SVG_Document;

FT_END_HEADER
#endif
