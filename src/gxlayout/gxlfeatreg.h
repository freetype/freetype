/***************************************************************************/
/*                                                                         */
/*  gxlfeatreg.h                                                           */
/*                                                                         */
/*    High level interface for the font feature registry(specification)    */
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/

#ifndef __GXLFEATREG_H__
#define __GXLFEATREG_H__ 

#include <ft2build.h>
#include FT_TYPES_H
#include "gxltypes.h"
#include "gxfeatreg.h"


FT_BEGIN_HEADER

FT_LOCAL( FT_Error ) gxl_feature_registry_fill_feature(GX_Feature_Registry featreg,
						       FT_Memory memory,
						       GXL_Feature feature);

FT_END_HEADER

#endif /* Not def: __GXLFEATREG_H__ */

/* END */
