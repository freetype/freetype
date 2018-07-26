/****************************************************************************
 *
 * ftbbox.h
 *
 *   Test header file for docwriter.
 *
 * Copyright 2018 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


  /**************************************************************************
   *
   * This component has a _single_ role: to test docwriter
   *
   * This file is ONLY used to test docwriter, and should not be taken
   * seriously.
   *
   */


#ifndef FTBBOX_H_
#define FTBBOX_H_


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
   * @section:
   *   outline_processing
   *
   * @title:
   *   Outline Processing
   *
   * @abstract:
   *   Functions to create, transform, and render vectorial glyph images.
   *
   * @description:
   *   This section contains routines used to create and destroy scalable
   *   glyph images known as 'outlines'.  These can also be measured,
   *   transformed, and converted into bitmaps and pixmaps.
   *


  /**************************************************************************
   *
   * @function:
   *   FT_Foo_Bar
   *
   * @description:
   *   Compute the exact bar for the given foo.
   *
   * @input:
   *   foo ::
   *     A pointer to the source foo.
   * 
   * @values:
   *   FT_FOO ::
   *     The foo.
   *
   *   FT_BAR ::
   *     The bar.
   *
   * @output:
   *   bar ::
   *     The foo's exact bar.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   If the foo is tricky and the bar has been loaded with
   *   @FT_FOO, the resulting bar is meaningless.  To get
   *   reasonable values for the bar it is necessary to load the foo
   *   at a large baz value (so that the hinting instructions can
   *   properly shift and scale the subfoos), then extracting the bar,
   *   which can be eventually converted back to baz units.
   */
  FT_EXPORT( FT_Error )
  FT_Outline_Get_BBox( FT_Outline*  outline,
                       FT_BBox     *abbox );

  /* */


FT_END_HEADER

#endif /* FTBBOX_H_ */

  /****************************************************************************
   *
   * @chapter:
   *   support_api
   *
   * @title:
   *   Support API
   *
   * @sections:
   *   outline_processing
   *
   */

  /*************************************************************************
   *
   * @macro:
   *   FT_BBOX_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   API of the optional exact bounding box computation routines.
   *
   */
#define FT_BBOX_H  <freetype/ftbbox.h>

/* */

/* END */


/* Local Variables: */
/* coding: utf-8    */
/* End:             */
