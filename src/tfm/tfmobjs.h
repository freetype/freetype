/****************************************************************************
 *
 * tfmobjs.h
 *
 *   FreeType auxiliary TFM module.
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


#ifndef TFMOBJS_H_
#define TFMOBJS_H_

#include <ft2build.h>
#include "tfmmod.h"

FT_BEGIN_HEADER


#include <ft2build.h>
#include FT_INTERNAL_TFM_H


FT_BEGIN_HEADER

  /* Initialise the TFM stream */
  FT_LOCAL( FT_Error )
  tfm_init( TFM_Parser  parser,
            FT_Memory   memory
            FT_Stream   stream );

  /* Parse TFM metric data */
  FT_LOCAL( FT_Error )
  tfm_parse_metrics( TFM_Parser  parser );

  FT_LOCAL( FT_Error )
  tfm_parse_kerns( TFM_Parser  parser );

  FT_LOCAL( void )
  tfm_close( TFM_Parser  parser );


FT_END_HEADER

#endif /* TFMOBJS_H_ */


/* END */
