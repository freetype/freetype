/***************************************************************************/
/*                                                                         */
/*  gfdrivr.h                                                              */
/*                                                                         */
/*    FreeType font driver for TeX's GF FONT files                         */
/*                                                                         */
/*  Copyright 1996-2018 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*  Copyright 2007 Dmitry Timoshkov for Codeweavers                        */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef GFDRIVR_H_
#define GFDRIVR_H_

#include <ft2build.h>
#include FT_INTERNAL_DRIVER_H

#include "gf.h"


FT_BEGIN_HEADER


  typedef struct  GF_encoding_el_
  {
    FT_Long    enc;
    FT_UShort  glyph;
    
  } GF_encoding_el;


  typedef struct  GF_FaceRec_
  {
    FT_FaceRec        root;
    //TO-DO
  } GF_FaceRec, *GF_Face;


  FT_EXPORT_VAR( const FT_Driver_ClassRec )  gf_driver_class;


FT_END_HEADER


#endif /* GFDRIVR_H_ */


/* END */
