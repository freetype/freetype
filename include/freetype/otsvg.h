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
