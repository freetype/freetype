/****************************************************************************
 *
 * tfmdrivr.h
 *
 *   FreeType font driver for TeX's TFM FONT files
 *
 * Copyright 1996-2018 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef TFMDRIVR_H_
#define TFMDRIVR_H_

#include <ft2build.h>
#include FT_INTERNAL_DRIVER_H

#include "tfm.h"


FT_BEGIN_HEADER

  typedef struct  TFM_FaceRec_
  {
    FT_FaceRec        root;
    /* TO-DO */
  } TFM_FaceRec, *TFM_Face;


  FT_EXPORT_VAR( const FT_Driver_ClassRec )  tfm_driver_class;


FT_END_HEADER


#endif /* TFMDRIVR_H_ */


/* END */
