/****************************************************************************
 *
 * svginterface.h
 *
 *   Exposes the interface of ot-svg module
 *
 * Copyright (C) 1996-2019 by
 * David Turner, Robert Wilhelm, Werner Lemberg and Moazin Khatti.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef SVGINTERFACE_H
#define SVGINTERFACE_H

#include <ft2build.h>
#include FT_OTSVG_H


FT_BEGIN_HEADER

  typedef FT_Error
  (*Preset_Bitmap_Func)( FT_Module     module,
                         FT_GlyphSlot  slot,
                         FT_Bool       cache );

  typedef struct SVG_Interface_
  {
    Preset_Bitmap_Func  preset_slot;

  } SVG_Interface;

  typedef SVG_Interface*  SVG_Service;

FT_END_HEADER

#endif
