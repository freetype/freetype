/***************************************************************************/
/*                                                                         */
/*  pshoptim.h                                                             */
/*                                                                         */
/*    Postscript Hints optimiser and grid-fitter                           */
/*                                                                         */
/*  Copyright 2001 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*                                                                         */
/*  process the hints provided by the fitter in order to optimize them     */
/*  for the pixel grid, depending on the current glyph outline and         */
/*  globals. This is where most of the PS Hinter's intelligence is         */
/*                                                                         */
/*  XXXX: Until now, only a dummy implementation is provided in order      */
/*        to test all other functions of the Postscript Hinter.            */
/*                                                                         */
/***************************************************************************/

#ifndef __PS_HINTER_OPTIMISER_H__
#define __PS_HINTER_OPTIMISER_H__

#include "pshfit.h"

FT_BEGIN_HEADER

  FT_LOCAL  FT_Error
  psh_hint_table_optimize( PSH_Hint_Table  table,
                           PSH_Globals     globals,
                           FT_Outline*     outline,
                           FT_Bool         vertical );

FT_END_HEADER

#endif /* __PS_HINTER_OPTIMISER_H__ */
