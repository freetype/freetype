/****************************************************************************
 *
 * otsvg.h
 *
 *   Interface for OT-SVG support related things (specification).
 *
 * Copyright (C) 2022 by
 * David Turner, Robert Wilhelm, Werner Lemberg, and Moazin Khatti.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef OTSVG_H_
#define OTSVG_H_

#include <freetype/freetype.h>

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
   *     The first glyph ID in the glyph range covered by this document.
   *
   *   end_glyph_id ::
   *     The last glyph ID in the glyph range covered by this document.
   *
   *   transform ::
   *     A 2x2 transformation matrix to apply to the glyph while rendering
   *     it.
   *
   *   delta ::
   *     The translation to apply to the glyph while rendering.
   *
   * @note:
   *   When the slot is passed down to a renderer, the renderer can only
   *   access the `metrics` and `units_per_EM` fields via `slot->face`.
   *   However, when `FT_Glyph_To_Bitmap` sets up a dummy object, it has no
   *   way to set a `face` object.  Thus, metrics information and
   *   `units_per_EM` (which is necessary for OT-SVG) has to be stored
   *   separately.
   *
   * @since:
   *   2.12
   */
  typedef struct  FT_SVG_DocumentRec_
  {
    FT_Byte*  svg_document;
    FT_ULong  svg_document_length;

    FT_Size_Metrics  metrics;
    FT_UShort        units_per_EM;

    FT_UShort  start_glyph_id;
    FT_UShort  end_glyph_id;

    FT_Matrix  transform;
    FT_Vector  delta;

  } FT_SVG_DocumentRec;


  /**************************************************************************
   *
   * @type:
   *   FT_SVG_Document
   *
   * @description:
   *   A handle to an @FT_SVG_DocumentRec object.
   *
   * @since:
   *   2.12
   */
  typedef struct FT_SVG_DocumentRec_*  FT_SVG_Document;


FT_END_HEADER

#endif /* OTSVG_H_ */


/* END */
