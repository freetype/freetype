/***************************************************************************/
/*                                                                         */
/*  ftnames.c                                                              */
/*                                                                         */
/*    Simple interface to access SFNT name tables (which are used          */
/*    to hold font names, copyright info, notices, etc.) (body).           */
/*                                                                         */
/*    This is _not_ used to retrieve glyph names!                          */
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


#include <freetype/ftnames.h>
#include <freetype/internal/tttypes.h>


#ifdef TT_CONFIG_OPTION_SFNT_NAMES


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Sfnt_Name_Count                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Retrieves the number of name strings in the SFNT `name' table.     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to the source face.                               */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The number of strings in the `name' table.                         */
  /*                                                                       */
  FT_EXPORT_FUNC( FT_UInt )  FT_Get_Sfnt_Name_Count( FT_Face  face )
  {
    return face && ( FT_IS_SFNT( face ) ? ((TT_Face)face)->num_names : 0 );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Sfnt_Name                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Retrieves a string of the SFNT `name' table for a given index.     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face  :: A handle to the source face.                              */
  /*                                                                       */
  /*    index :: The index of the `name' string.                           */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aname :: The indexed FT_SfntName structure.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The `string' array returned in the `aname' structure is not        */
  /*    null-terminated.                                                   */
  /*                                                                       */
  /*    Use FT_Get_Sfnt_Name_Count() to get the total number of available  */
  /*    `name' table entries, then do a loop until you get the right       */
  /*    platform, encoding, and name ID.                                   */
  /*                                                                       */
  FT_EXPORT_FUNC( FT_Error ) FT_Get_Sfnt_Name( FT_Face       face,
                                               FT_UInt       index,
                                               FT_SfntName*  aname )
  {
    FT_Error  error = FT_Err_Invalid_Argument;


    if ( aname && face && FT_IS_SFNT( face ) )
    {
      TT_Face  ttface = (TT_Face)face;


      if ( index < ttface->num_names )
      {
        TT_NameRec*  name = ttface->name_table.names + index;


        aname->platform_id = name->platformID;
        aname->encoding_id = name->encodingID;
        aname->language_id = name->languageID;
        aname->name_id     = name->nameID;
        aname->string      = (FT_Byte*)name->string;
        aname->string_len  = name->stringLength;

        error = FT_Err_Ok;
      }
    }

    return error;
  }


#endif /* TT_CONFIG_OPTION_SFNT_NAMES */


/* END */
