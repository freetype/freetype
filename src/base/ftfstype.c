/***************************************************************************/
/*                                                                         */
/*  ftfstype.c                                                             */
/*                                                                         */
/*    FreeType utility file to access FSType data (body).                  */
/*                                                                         */
/*  Copyright 2008, 2009 by                                                */
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
#include FT_TYPE1_TABLES_H
#include FT_TRUETYPE_TABLES_H


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_UShort )
  FT_Get_FSType_Flags( FT_Face  face )
  {
    PS_FontInfoRec  font_info;
    TT_OS2*         os2;


    /* look at FSType before fsType for Type42 */

    if ( !FT_Get_PS_Font_Info( face, &font_info ) &&
         font_info.fs_type != 0                   )
      return font_info.fs_type;

    if ( ( os2 = (TT_OS2*)FT_Get_Sfnt_Table( face, ft_sfnt_os2 ) ) != NULL &&
         os2->version != 0xFFFFU                                           )
      return os2->fsType;

    return 0;
  }


/* END */
