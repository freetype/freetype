/***************************************************************************/
/*                                                                         */
/*  sfdriver.c                                                             */
/*                                                                         */
/*    High-level SFNT driver interface (body).                             */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <freetype/internal/sfnt.h>
#include <freetype/internal/ftobjs.h>
#include <sfdriver.h>
#include <ttload.h>
#include <ttsbit.h>
#include <ttpost.h>
#include <ttcmap.h>
#include <sfobjs.h>


  static
  void*  get_sfnt_table( TT_Face      face,
                         FT_Sfnt_Tag  tag )
  {
    void*  table;


    switch ( tag )
    {
    case ft_sfnt_head:
      table = &face->header;
      break;

    case ft_sfnt_hhea:
      table = &face->horizontal;
      break;

    case ft_sfnt_vhea:
      table = face->vertical_info ? &face->vertical : 0;
      break;

    case ft_sfnt_os2:
      table = face->os2.version == 0xFFFF ? 0 : &face->os2;
      break;

    case ft_sfnt_post:
      table = &face->postscript;
      break;

    case ft_sfnt_maxp:
      table = &face->max_profile;
      break;

    case ft_sfnt_pclt:
      table = face->pclt.Version ? &face->pclt : 0;
      break;

    default:
      table = 0;
    }

    return table;
  }


  static
  FTDriver_Interface  SFNT_Get_Interface( FT_Driver    driver,
                                          const char*  interface )
  {
    UNUSED( driver );

    if ( strcmp( interface, "get_sfnt" ) == 0 )
      return (FTDriver_Interface)get_sfnt_table;

    return 0;
  }


  static const SFNT_Interface  sfnt_interface =
  {
    TT_Goto_Table,

    SFNT_Init_Face,
    SFNT_Load_Face,
    SFNT_Done_Face,
    SFNT_Get_Interface,

    TT_Load_Any,
    TT_Load_SFNT_Header,
    TT_Load_Directory,

    TT_Load_Header,
    TT_Load_Metrics_Header,
    TT_Load_CMap,
    TT_Load_MaxProfile,
    TT_Load_OS2,
    TT_Load_PostScript,

    TT_Load_Names,
    TT_Free_Names,

    TT_Load_Hdmx,
    TT_Free_Hdmx,

    TT_Load_Kern,
    TT_Load_Gasp,
    TT_Load_PCLT,

#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS
    /* see `ttsbit.h' */
    TT_Load_SBit_Strikes,
    TT_Load_SBit_Image,
    TT_Free_SBit_Strikes,
#else /* TT_CONFIG_OPTION_EMBEDDED_BITMAPS */
    0,
    0,
    0,
#endif /* TT_CONFIG_OPTION_EMBEDDED_BITMAPS */

#ifdef TT_CONFIG_OPTION_POSTSCRIPT_NAMES
    /* see `ttpost.h' */
    TT_Get_PS_Name,
    TT_Free_Post_Names,
#else /* TT_CONFIG_OPTION_POSTSCRIPT_NAMES */
    0,
    0,
#endif /* TT_CONFIG_OPTION_POSTSCRIPT_NAMES */

    /* see `ttcmap.h' */
    TT_CharMap_Load,
    TT_CharMap_Free,
  };


  const FT_DriverInterface  sfnt_driver_interface =
  {
    sizeof ( FT_DriverRec ), 0, 0, 0,

    "sfnt",     /* driver name                          */
    1,          /* driver version                       */
    2,          /* driver requires FreeType 2 or higher */

    (void*)&sfnt_interface,

    0, 0, 0,
    0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0,
    0,
  };


/* END */
