/***************************************************************************/
/*                                                                         */
/*  ftbdf.h                                                                */
/*                                                                         */
/*    FreeType API for accessing BDF-specific strings (specification).     */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTBDF_H__
#define __FTBDF_H__

#include <ft2build.h>
#include FT_FREETYPE_H


FT_BEGIN_HEADER


  FT_EXPORT( FT_Error )
  FT_Get_BDF_Charset_ID( FT_Face       face,
                         const char*  *acharset_encoding,
                         const char*  *acharset_registry );

FT_END_HEADER

#endif /* __FTBDF_H__ */


/* END */
