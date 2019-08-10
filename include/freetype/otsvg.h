/****************************************************************************
 *
 * otsvg.h
 *
 *   Interface for OT-SVG support related things (specification).
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
   * @return:
   *   FreeType error code.  0 means success.
   */

  typedef FT_Error
  (*SVG_Lib_Render_Func)( FT_GlyphSlot  slot );

  /**************************************************************************
   *
   * @functype:
   *   SVG_Lib_Preset_Slot_Func
   *
   * @description:
   *   A callback which is to preset the glyphslot.
   *
   * @input:
   *   slot ::
   *     The glyph slot which has the SVG document loaded.
   *
   * @return:
   *   FreeType error code.  0 means success.
   */

  typedef FT_Error
  (*SVG_Lib_Preset_Slot_Func)( FT_GlyphSlot  slot, FT_Bool  cache);


  typedef struct SVG_RendererHooks_
  {
    /* Api Hooks for OT-SVG Rendering */
    SVG_Lib_Init_Func    init_svg;
    SVG_Lib_Free_Func    free_svg;
    SVG_Lib_Render_Func  render_svg;

    SVG_Lib_Preset_Slot_Func  preset_slot;
  } SVG_RendererHooks;


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
