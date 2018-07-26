/****************************************************************************
 *
 * tfmmod.c
 *
 *   FreeType auxiliary TFM module.
 *
 * Copyright 2000-2018 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#include <ft2build.h>
#include "tfmmod.h"


  static
  const TFM_Interface  tfm_interface =
  {
    tfm_init,           /* init          */
    tfm_parse_metrics,  /* parse metrics */
    tfm_parse_kerns,    /* parse kerns   */
    tfm_close,          /* done          */
  };


  FT_CALLBACK_TABLE_DEF
  const FT_Module_Class  tfm_module_class =
  {
    0,
    sizeof ( FT_ModuleRec ),
    "tfm",
    0x20000L,
    0x20000L,

    &tfm_interface,   /* module-specific interface */

    (FT_Module_Constructor)NULL,  /* module_init   */
    (FT_Module_Destructor) NULL,  /* module_done   */
    (FT_Module_Requester)  NULL   /* get_interface */
  };


/* END */
