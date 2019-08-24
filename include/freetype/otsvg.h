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
   *   A callback which is called when the first OT-SVG glyph is rendered in
   *   the lifetime of an @FT_Library object. The callback should perform all
   *   sorts of initializations that the SVG rendering library needs such as
   *   allocating memory for `svg_renderer_state` of @FT_LibraryRec.
   *
   * @input:
   *   library ::
   *     An instance of @FT_Library. It's passed to give the callbacks access
   *     to `svg_renderer_state` of @FT_LibraryRec.
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
   *   A callback which is called when the `ot-svg` module is being freed.
   *   It is only called only if the init hook was called earlier. So, if no
   *   OT-SVG glyph is rendered, neither the init hook is called nor the free
   *   hook.
   *
   * @input:
   *   library ::
   *     An instance of @FT_Library. It's passed to give the callbacks access
   *     to `svg_renderer_state` of @FT_LibraryRec.
   */
  typedef void
  (*SVG_Lib_Free_Func)( FT_Library  library );


  /**************************************************************************
   *
   * @functype:
   *   SVG_Lib_Render_Func
   *
   * @description:
   *   A callback which is called to render an OT-SVG glyph. This callback
   *   hook is called right after the preset hook has been called with
   *   `cache` set to `TRUE`. The data necessary to render is available
   *   through the handle @FT_SVG_Document which is set in `other` field of
   *   @FT_GlyphSlotRec.
   *
   * @input:
   *   slot ::
   *     The slot to render.
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
   *   A callback which is called to preset the glyphslot. It is called from
   *   two places.
   *
   *   1. When `FT_Load_Glyph` needs to preset the glyphslot.
   *   2. Right before the `ot-svg` module calls the render callback hook.
   *
   *   When it is the former, the argument `cache` is set to `FALSE`. When it
   *   is the latter, the argument `cache` is set to `TRUE`. This distinction
   *   has been made because while presetting a glyphslot many calculations
   *   are needed and later the render callback hook needs the same
   *   calculations, thus, if `cache` is `TRUE`, the hook might _cache_ these
   *   calculations in `svg_renderer_state` of @FT_LibraryRec.
   *
   * @input:
   *   slot ::
   *     The glyph slot which has the SVG document loaded.
   *
   *   cache ::
   *     See description.
   *
   * @return:
   *   FreeType error code.  0 means success.
   */
  typedef FT_Error
  (*SVG_Lib_Preset_Slot_Func)( FT_GlyphSlot  slot, FT_Bool  cache);


  /**************************************************************************
   *
   * @struct:
   *   SVG_RendererHooks
   *
   * @description:
   *   A structure that stores the four hooks needed to render OT-SVG glyphs
   *   properly. The structure is publicly used to set the hooks via driver
   *   properties.
   *
   * @fields:
   *   init_svg ::
   *     The initialization hook.
   *
   *   free_svg ::
   *     The cleanup hook.
   *
   *   render_hook ::
   *     The render hook.
   *
   *   preset_slot ::
   *     The preset hook.
   */
  typedef struct SVG_RendererHooks_
  {
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
   *     A pointer to the SVG document.
   *
   *   svg_document_length ::
   *     The length of `svg_document`.
   *
   *   metrics ::
   *     A metrics object storing the size information.
   *
   *   units_per_EM ::
   *     The size of the EM square.
   *
   *   start_glyph_id ::
   *     The first glyph ID in the glyph range is covered by this document.
   *
   *   end_glyph_id ::
   *     The last glyph ID in the glyph range is covered by this document.
   *
   *   transform ::
   *     A 2x2 transformation matrix to apply on the glyph while rendering it.
   *
   *   delta ::
   *     Translation to apply on the glyph while rendering.
   *
   * @note:
   *   `metrics` and `units_per_EM` might look like repetitions since both
   *   fields are stored in face object, but they are not; When the slot is
   *   passed down to a renderer, the renderer can only access the `metrics`
   *   and `units_per_EM` by `slot->face`. However, when `FT_Glyph_To_Bitmap`
   *   sets up a dummy object, it has no way to set a `face` object. Thus,
   *   metrics information and units_per_EM (which is necessary for OT-SVG)
   *   has to be stored separately.
   */

  typedef struct FT_SVG_DocumentRec_
  {
    FT_Byte*         svg_document;
    FT_ULong         svg_document_length;
    FT_Size_Metrics  metrics;
    FT_UShort        units_per_EM;
    FT_UShort        start_glyph_id;
    FT_UShort        end_glyph_id;
    FT_Matrix        transform;
    FT_Vector        delta;

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
