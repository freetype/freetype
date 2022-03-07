/****************************************************************************
 *
 * ftcolor.c
 *
 *   FreeType's glyph color management (body).
 *
 * Copyright (C) 2018-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#include <freetype/internal/ftdebug.h>
#include <freetype/internal/sfnt.h>
#include <freetype/internal/tttypes.h>
#include <freetype/ftcolor.h>


#ifdef TT_CONFIG_OPTION_COLOR_LAYERS

  static
  const FT_Palette_Data  null_palette_data = { 0, NULL, NULL, 0, NULL };


  /* documentation is in ftcolor.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Data_Get( FT_Face           face,
                       FT_Palette_Data  *apalette_data )
  {
    if ( !face )
      return FT_THROW( Invalid_Face_Handle );
    if ( !apalette_data )
      return FT_THROW( Invalid_Argument );

    if ( FT_IS_SFNT( face ) )
      *apalette_data = ( (TT_Face)face )->palette_data;
    else
      *apalette_data = null_palette_data;

    return FT_Err_Ok;
  }


  /* documentation is in ftcolor.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Select( FT_Face     face,
                     FT_UShort   palette_index,
                     FT_Color*  *apalette )
  {
    FT_Error  error;

    TT_Face       ttface;
    SFNT_Service  sfnt;


    if ( !face )
      return FT_THROW( Invalid_Face_Handle );

    if ( !FT_IS_SFNT( face ) )
    {
      if ( apalette )
        *apalette = NULL;

      return FT_Err_Ok;
    }

    ttface = (TT_Face)face;
    sfnt   = (SFNT_Service)ttface->sfnt;

    error = sfnt->set_palette( ttface, palette_index );
    if ( error )
      return error;

    ttface->palette_index = palette_index;

    if ( apalette )
      *apalette = ttface->palette;

    return FT_Err_Ok;
  }


  /* documentation is in ftcolor.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Set( FT_Face    face,
                  FT_Int     index,
                  FT_Color*  palette )
  {
    TT_Face    ttface;
    FT_UShort  i;


    if ( !face || !FT_IS_SFNT( face ) )
      return FT_THROW( Invalid_Face_Handle );

    if ( !palette || index >= 0 )
      return FT_THROW( Invalid_Argument );

    ttface = (TT_Face)face;

    for ( i = 0; i < ttface->palette_data.num_palette_entries; i++ )
      ttface->palette[i] = palette[i];
    ttface->palette_index = index;

    return FT_Err_Ok;
  }


  /* documentation is in ftcolor.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Get( FT_Face     face,
                  FT_Int     *anindex,
                  FT_Color*  *apalette )
  {
    TT_Face  ttface;


    if ( !face || !FT_IS_SFNT( face ) )
      return FT_THROW( Invalid_Face_Handle );

    ttface = (TT_Face)face;

    if ( anindex )
      *anindex  = ttface->palette_index;
    if ( apalette )
      *apalette = ttface->palette;

    return FT_Err_Ok;
  }


  /* documentation is in ftcolor.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Set_Foreground_Color( FT_Face   face,
                                   FT_Color  foreground_color )
  {
    TT_Face  ttface;


    if ( !face )
      return FT_THROW( Invalid_Face_Handle );

    if ( !FT_IS_SFNT( face ) )
      return FT_Err_Ok;

    ttface = (TT_Face)face;

    ttface->foreground_color      = foreground_color;
    ttface->have_foreground_color = 1;

    return FT_Err_Ok;
  }


  /* documentation is in ftcolor.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Get_Foreground_Color( FT_Face    face,
                                   FT_Color*  aforeground_color )
  {
    TT_Face  ttface;

    FT_Color  white = { 0xFF, 0xFF, 0xFF, 0xFF };
    FT_Color  black = { 0x00, 0x00, 0x00, 0xFF };


    if ( !face || !FT_IS_SFNT( face ) )
      return FT_THROW( Invalid_Face_Handle );

    if ( !aforeground_color )
      return FT_THROW( Invalid_Argument );

    ttface = (TT_Face)face;

    if ( ttface->have_foreground_color )
      *aforeground_color = ttface->foreground_color;
    else if ( ttface->palette_index < 0 )
      *aforeground_color = black;
    else
    {
      if ( ttface->palette_data.palette_flags                            &&
           ( ttface->palette_data.palette_flags[ttface->palette_index] &
               FT_PALETTE_FOR_DARK_BACKGROUND                          ) )
        *aforeground_color = white;
      else
        *aforeground_color = black;
    }

    return FT_Err_Ok;
  }

#else /* !TT_CONFIG_OPTION_COLOR_LAYERS */

  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Data_Get( FT_Face           face,
                       FT_Palette_Data  *apalette_data )
  {
    FT_UNUSED( face );
    FT_UNUSED( apalette_data );


    return FT_THROW( Unimplemented_Feature );
  }


  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Select( FT_Face     face,
                     FT_UShort   palette_index,
                     FT_Color*  *apalette )
  {
    FT_UNUSED( face );
    FT_UNUSED( palette_index );
    FT_UNUSED( apalette );


    return FT_THROW( Unimplemented_Feature );
  }


  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Set( FT_Face    face,
                  FT_Int     index,
                  FT_Color*  palette )
  {
    FT_UNUSED( face );
    FT_UNUSED( index );
    FT_UNUSED( palette );


    return FT_THROW( Unimplemented_Feature );
  }


  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Get( FT_Face     face,
                  FT_Int     *anindex,
                  FT_Color*  *apalette )
  {
    FT_UNUSED( face );
    FT_UNUSED( anindex );
    FT_UNUSED( apalette );


    return FT_THROW( Unimplemented_Feature );
  }


  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Set_Foreground_Color( FT_Face   face,
                                   FT_Color  foreground_color )
  {
    FT_UNUSED( face );
    FT_UNUSED( foreground_color );


    return FT_THROW( Unimplemented_Feature );
  }


  FT_EXPORT_DEF( FT_Error )
  FT_Palette_Get_Foreground_Color( FT_Face    face,
                                   FT_Color*  aforeground_color )
  {
    FT_UNUSED( face );
    FT_UNUSED( aforeground_color );


    return FT_THROW( Unimplemented_Feature );
  }

#endif /* !TT_CONFIG_OPTION_COLOR_LAYERS */


/* END */
