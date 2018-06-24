/****************************************************************************
 *
 * tfmerror.h
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

  /**************************************************************************
   *
   * This file is used to define the GF error enumeration constants.
   *
   */

#ifndef TFMERROR_H_
#define TFMERROR_H_

#include FT_MODULE_ERRORS_H

#undef FTERRORS_H_

#undef  FT_ERR_PREFIX
#define FT_ERR_PREFIX  TFM_Err_
#define FT_ERR_BASE    FT_Mod_Err_TFM

#include FT_ERRORS_H

#endif /* TFMERROR_H_ */


/* END */
