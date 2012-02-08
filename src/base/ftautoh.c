/***************************************************************************/
/*                                                                         */
/*  ftautoh.c                                                              */
/*                                                                         */
/*    FreeType API for setting and accessing auto-hinter properties        */
/*    (body).                                                              */
/*                                                                         */
/*  Copyright 2012 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H


  /* documentation is in ftautoh.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Library_GetAutohinterProperties( FT_Library  library,
                                      FT_Int32   *properties )
  {
    if ( !library || !properties )
      return FT_Err_Invalid_Argument;

    *properties = library->auto_hinter_flags;

    return FT_Err_Ok;
  }


  /* documentation is in ftautoh.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Library_SetAutohinterProperties( FT_Library  face,
                                      FT_Int32    properties )
  {
    if ( !library )
      return FT_Err_Invalid_Argument;

    library->auto_hinter_flags = properties;

    return FT_Err_Ok;
  }


  /* documentation is in ftautoh.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Face_GetAutohinterProperties( FT_Face    face,
                                   FT_Int32  *properties )
  {
    if ( !face || !properties )
      return FT_Err_Invalid_Argument;

    *properties = face->internal->auto_hinter_flags;

    return FT_Err_Ok;
  }


  /* documentation is in ftautoh.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Face_SetAutohinterProperties( FT_Face   face,
                                   FT_Int32  properties )
  {
    if ( !face )
      return FT_Err_Invalid_Argument;

    face->internal->auto_hinter_flags = properties;

    return FT_Err_Ok;
  }


/* END */
