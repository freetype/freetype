/***************************************************************************/
/*                                                                         */
/*  ftcolor.h                                                              */
/*                                                                         */
/*    FreeType's glyph color management (specification).                   */
/*                                                                         */
/*  Copyright 2018 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef FTCOLOR_H_
#define FTCOLOR_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER


  /**************************************************************************
   *
   * @section:
   *   color_management
   *
   * @title:
   *   Glyph Color Management
   *
   * @abstract:
   *   Retrieving and manipulating OpenType's `CPAL' table entries.
   *
   * @description:
   *   The functions described here allow access and manipulation of color
   *   palette entries in OpenType's `CPAL' table.
   */


  /**************************************************************************
   *
   * @struct:
   *   FT_Color
   *
   * @description:
   *   This structure models a BGRA color value of a `CPAL' palette entry.
   *
   *   The used color space is sRGB; the colors are not pre-multiplied, and
   *   alpha values must be explicitly set.
   *
   * @fields:
   *   blue ::
   *     Blue value.
   *
   *   green ::
   *     Green value.
   *
   *   red ::
   *     Red value.
   *
   *   alpha ::
   *     Alpha value, giving the red, green, and blue color's opacity.
   *
   * @since:
   *   2.10
   */
  typedef struct  FT_Color_
  {
    FT_Byte  blue;
    FT_Byte  green;
    FT_Byte  red;
    FT_Byte  alpha;

  } FT_Color;


  /**************************************************************************
   *
   * @func:
   *   FT_Palette_Get_Size
   *
   * @description:
   *   Get the number of palettes in the `CPAL' table and the number of
   *   entries in a palette (all palettes have the same number of entries).
   *
   * @input:
   *   face ::
   *     The source face handle.
   *
   * @output:
   *   anum_palettes ::
   *     The number of palettes.
   *
   *   anum_palette_entries ::
   *     The number of entries in a single palette.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   This function always returns an error if the config macro
   *   `TT_CONFIG_OPTION_COLOR_LAYERS' is not defined in `ftoption.h'.
   *
   * @since:
   *   2.10
   */
  FT_EXPORT( FT_Error )
  FT_Palette_Get_Size( FT_Face     face,
                       FT_UShort*  anum_palettes,
                       FT_UShort*  anum_palette_entries );


  /**************************************************************************
   *
   * @func:
   *   FT_Palette_Get_Name_IDs
   *
   * @description:
   *   Get the palette name IDs, which correspond to entries like `dark' or
   *   `light' in the font's `name' table.
   *
   * @input:
   *   face ::
   *     The source face handle.
   *
   * @output:
   *   palette_name_ids ::
   *     A read-only array of palette name IDs.  NULL if the font's `CPAL'
   *     table doesn't contain appropriate data.
   *
   *     Use function @FT_Get_Sfnt_Name to map name IDs to a name strings.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   The number of palettes can be retrieved with @FT_Palette_Get_Size.
   *
   *   An empty name ID in the `CPAL' table gets represented as value
   *   0xFFFF.
   *
   *   This function always returns an error if the config macro
   *   `TT_CONFIG_OPTION_COLOR_LAYERS' is not defined in `ftoption.h'.
   *
   * @since:
   *   2.10
   */
  FT_EXPORT( FT_Error )
  FT_Palette_Get_Name_IDs( FT_Face           face,
                           const FT_UShort*  palette_name_ids );


  /**************************************************************************
   *
   * @enum:
   *   FT_PALETTE_XXX
   *
   * @description:
   *   A list of bit field constants returned by function
   *   @FT_Palette_Get_Types to indicate for which background a given
   *   palette is usable.
   *
   * @values:
   *   FT_PALETTE_USABLE_WITH_LIGHT_BACKGROUND ::
   *     The palette is appropriate to use when displaying the font on a
   *     light background such as white.
   *
   *   FT_PALETTE_USABLE_WITH_DARK_BACKGROUND ::
   *     The palette is appropriate to use when displaying the font on a
   *     dark background such as black.
   *
   * @note:
   *   The number of palette types can be retrieved with
   *   @FT_Palette_Get_Size.
   *
   * @since:
   *   2.10
   */
#define FT_PALETTE_USABLE_WITH_LIGHT_BACKGROUND  0x01
#define FT_PALETTE_USABLE_WITH_DARK_BACKGROUND   0x02


  /**************************************************************************
   *
   * @func:
   *   FT_Palette_Get_Types
   *
   * @description:
   *   Get the palette types.  Possible values are an ORed combination of
   *   @FT_PALETTE_USABLE_WITH_LIGHT_BACKGROUND and
   *   @FT_PALETTE_USABLE_WITH_DARK_BACKGROUND.
   *
   * @input:
   *   face ::
   *     The source face handle.
   *
   * @output:
   *   apalette_types ::
   *     A read-only array of palette types.  NULL if the font's `CPAL'
   *     table doesn't contain appropriate data.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   The number of palette types can be retrieved with
   *   @FT_Palette_Get_Size.
   *
   *   This function always returns an error if the config macro
   *   `TT_CONFIG_OPTION_COLOR_LAYERS' is not defined in `ftoption.h'.
   *
   * @since:
   *   2.10
   */
  FT_EXPORT( FT_Error )
  FT_Palette_Get_Types( FT_Face           face,
                        const FT_UShort*  apalette_types );


  /**************************************************************************
   *
   * @func:
   *   FT_Palette_Get_Entry_Name_IDs
   *
   * @description:
   *   Get the palette entry name IDs.  In each palette, entries with the
   *   same index have the same function.  For example, index~0 might
   *   correspond to string `outline' in the font's `name' table to indicate
   *   that this palette entry is used for outlines, index~1 might
   *   correspond to `fill' to indicate the filling color palette entry,
   *   etc.
   *
   * @input:
   *   face ::
   *     The source face handle.
   *
   * @output:
   *   aentry_names ::
   *     A read-only array of palette entry name IDs.  NULL if the font's
   *     `CPAL' table doesn't contain appropriate data.
   *
   *     Use function @FT_Get_Sfnt_Name to map entry name IDs to a name
   *     strings.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   The number of palette entries can be retrieved with
   *   @FT_Palette_Get_Size.
   *
   *   An empty entry name ID in the `CPAL' table gets represented as value
   *   0xFFFF.
   *
   *   This function always returns an error if the config macro
   *   `TT_CONFIG_OPTION_COLOR_LAYERS' is not defined in `ftoption.h'.
   *
   * @since:
   *   2.10
   */
  FT_EXPORT( FT_Error )
  FT_Palette_Get_Entry_Name_IDs( FT_Face           face,
                                 const FT_UShort*  palette_entry_name_ids );


  /**************************************************************************
   *
   * @func:
   *   FT_Palette_Select
   *
   * @description:
   *   This function has two purposes.
   *
   *   (1) It activates a palette for rendering color glyphs, and
   *
   *   (2) it retrieves all (unmodified) color entries of this palette.  This
   *       function returns a read-write array, which means that a calling
   *       application can modify the palette entries on demand.
   *
   * A corollary of (2) is that calling the function, then modifying some
   * values, then calling the function again with the same arguments resets
   * all color entries to the original `CPAL' values; all user modifications
   * are lost.
   *
   * @input:
   *   face ::
   *     The source face handle.
   *
   *   palette_index ::
   *     The palette index.
   *
   * @output:
   *   apalette_entries ::
   *     An array of color entries for a palette with index `palette_index'.
   *     If `apalette_entries' is set to NULL, no array gets returned (and
   *     no color entries can be modified).
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   The number of color entries can be retrieved with
   *   @FT_Palette_Get_Size.
   *
   *   The array pointed to by `apalette_entries' is owned and managed by
   *   FreeType.
   *
   *   This function always returns an error if the config macro
   *   `TT_CONFIG_OPTION_COLOR_LAYERS' is not defined in `ftoption.h'.
   *
   * @since:
   *   2.10
   */
  FT_EXPORT( FT_Error )
  FT_Palette_Select( FT_Face     face,
                     FT_UShort   palette_index,
                     FT_Color*  *apalette_entries );


  /**************************************************************************
   *
   * @func:
   *   FT_Palette_Set_Foreground_Color
   *
   * @description:
   *   `CPAL' uses color index 0xFFFF to indicate a `text foreground color'.
   *   This function sets this value.
   *
   * @input:
   *   face ::
   *     The source face handle.
   *
   *   foreground_color ::
   *     An `FT_Color' structure to define the text foreground color.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   This function always returns an error if the config macro
   *   `TT_CONFIG_OPTION_COLOR_LAYERS' is not defined in `ftoption.h'.
   *
   * @since:
   *   2.10
   */
  FT_EXPORT( FT_Error )
  FT_Palette_Set_Foreground_Color( FT_Face   face,
                                   FT_Color  foreground_color );

  /* */


FT_END_HEADER

#endif /* FTCOLOR_H_ */


/* END */
