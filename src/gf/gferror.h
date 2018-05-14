/***************************************************************************/
/*                                                                         */
/*  gferror.h                                                              */
/*                                                                         */
/*    FreeType font driver for TeX's GF FONT files                         */
/*                                                                         */
/*  Copyright 1996-2018 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This file is used to define the GF error enumeration constants.      */
  /*                                                                       */
  /*************************************************************************/

#ifndef GFERROR_H_
#define GFERROR_H_

#include FT_MODULE_ERRORS_H

#undef FTERRORS_H_

#undef  FT_ERR_PREFIX
#define FT_ERR_PREFIX  GF_Err_
#define FT_ERR_BASE    FT_Mod_Err_GF

#include FT_ERRORS_H

#endif /* GFERROR_H_ */


/* END */
