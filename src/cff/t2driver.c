/***************************************************************************/
/*                                                                         */
/*  t2driver.c                                                             */
/*                                                                         */
/*    OpenType font driver implementation (body).                          */
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


#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftstream.h>
#include <freetype/internal/sfnt.h>
#include <freetype/ttnameid.h>

#include <t2driver.h>
#include <t2gload.h>


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t2driver


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                          F A C E S                              ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#undef  PAIR_TAG
#define PAIR_TAG( left, right )  ( ( (TT_ULong)left << 16 ) | \
                                     (TT_ULong)right        )


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Get_Kerning                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to return the kerning vector between two      */
  /*    glyphs of the same face.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: A handle to the source face object.                 */
  /*                                                                       */
  /*    left_glyph  :: The index of the left glyph in the kern pair.       */
  /*                                                                       */
  /*    right_glyph :: The index of the right glyph in the kern pair.      */
  /*                                                                       */
  /* <Output>                                                              */
  /*    kerning     :: The kerning vector.  This is in font units for      */
  /*                   scalable formats, and in pixels for fixed-sizes     */
  /*                   formats.                                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Only horizontal layouts (left-to-right & right-to-left) are        */
  /*    supported by this function.  Other layouts, or more sophisticated  */
  /*    kernings are out of scope of this method (the basic driver         */
  /*    interface is meant to be simple).                                  */
  /*                                                                       */
  /*    They can be implemented by format-specific interfaces.             */
  /*                                                                       */
  static
  TT_Error  Get_Kerning( TT_Face     face,
                         TT_UInt     left_glyph,
                         TT_UInt     right_glyph,
                         TT_Vector*  kerning )
  {
    TT_Kern_0_Pair*  pair;


    if ( !face )
      return FT_Err_Invalid_Face_Handle;

    kerning->x = 0;
    kerning->y = 0;

    if ( face->kern_pairs )
    {
      /* there are some kerning pairs in this font file! */
      TT_ULong  search_tag = PAIR_TAG( left_glyph, right_glyph );
      TT_Long   left, right;


      left  = 0;
      right = face->num_kern_pairs - 1;

      while ( left <= right )
      {
        TT_Int    middle   = left + ((right-left) >> 1);
        TT_ULong  cur_pair;


        pair     = face->kern_pairs + middle;
        cur_pair = PAIR_TAG( pair->left, pair->right );

        if ( cur_pair == search_tag )
          goto Found;

        if ( cur_pair < search_tag )
          left = middle+1;
        else
          right = middle-1;
      }
    }

  Exit:
    return FT_Err_Ok;

  Found:
    kerning->x = pair->value;
    goto Exit;
  }


#undef PAIR_TAG


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                           S I Z E S                             ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Set_Char_Sizes                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to reset a size's character sizes (horizontal */
  /*    and vertical) expressed in fractional points.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    char_width  :: The character width expressed in 26.6 fractional    */
  /*                   points.                                             */
  /*    char_height :: The character height expressed in 26.6 fractional   */
  /*                   points.                                             */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    size        :: A handle to the target size object.                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  static
  TT_Error  Set_Char_Sizes( T2_Size     size,
                            FT_F26Dot6  char_width,
                            FT_F26Dot6  char_height,
                            FT_UInt     horz_resolution,
                            FT_UInt     vert_resolution )
  {
    FT_Size_Metrics*  metrics = &size->metrics;
    T2_Face           face    = (T2_Face)size->face;
    FT_Long           dim_x, dim_y;

    /* This bit flag, when set, indicates that the pixel size must be */
    /* truncated to an integer. Nearly all TrueType fonts have this   */
    /* bit set, as hinting won't work really well otherwise.          */
    /*                                                                */
    /* However, for those rare fonts who do not set it, we override   */
    /* the default computations performed by the base layer. I really */
    /* don't know if this is useful, but hey, that's the spec :-)     */
    /*                                                                */
    if ( (face->header.Flags & 8) == 0 )
    {
      /* Compute pixel sizes in 26.6 units */
      dim_x = (char_width * horz_resolution) / 72;
      dim_y = (char_height * vert_resolution) / 72;

      metrics->x_scale = FT_DivFix( dim_x, face->root.units_per_EM );
      metrics->y_scale = FT_DivFix( dim_y, face->root.units_per_EM );

      metrics->x_ppem    = (TT_UShort)(dim_x >> 6);
      metrics->y_ppem    = (TT_UShort)(dim_y >> 6);
    }

    return T2_Reset_Size( size );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Set_Pixel_Sizes                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to reset a size's character sizes (horizontal */
  /*    and vertical) expressed in integer pixels.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    pixel_width  :: The character width expressed in integer pixels.   */
  /*                                                                       */
  /*    pixel_height :: The character height expressed in integer pixels.  */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    size         :: A handle to the target size object.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code. 0 means success                               */
  /*                                                                       */
  static
  FT_Error  Set_Pixel_Sizes( T2_Size  size,
                             FT_UInt  pixel_width,
                             FT_UInt  pixel_height )
  {
    UNUSED(pixel_width);
    UNUSED(pixel_height);

    return T2_Reset_Size( size );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Load_Glyph                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to load a glyph within a given glyph slot.    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    slot        :: A handle to the target slot object where the glyph  */
  /*                   will be loaded.                                     */
  /*                                                                       */
  /*    size        :: A handle to the source face size at which the glyph */
  /*                   must be scaled/loaded/etc.                          */
  /*                                                                       */
  /*    glyph_index :: The index of the glyph in the font file.            */
  /*                                                                       */
  /*    load_flags  :: A flag indicating what to load for this glyph.  The */
  /*                   FTLOAD_??? constants can be used to control the     */
  /*                   glyph loading process (e.g., whether the outline    */
  /*                   should be scaled, whether to load bitmaps or not,   */
  /*                   whether to hint the outline, etc).                  */
  /* <Output>                                                              */
  /*    result      :: A set of bit flags indicating the type of data that */
  /*                   was loaded in the glyph slot (outline, bitmap,      */
  /*                   pixmap, etc).                                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  static
  FT_Error  Load_Glyph( T2_GlyphSlot  slot,
                        T2_Size       size,
                        FT_UShort     glyph_index,
                        FT_UInt       load_flags )
  {
    FT_Error  error;


    if ( !slot )
      return FT_Err_Invalid_Handle;

    /* check that we want a scaled outline or bitmap */
    if ( !size )
      load_flags |= FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING;

    if ( load_flags & FT_LOAD_NO_SCALE )
      size = NULL;

    /* reset the size object if necessary */
    if ( size )
    {
      /* these two object must have the same parent */
      if ( size->face != slot->root.face )
        return FT_Err_Invalid_Face_Handle;
    }

    /* now load the glyph outline if necessary */
    error = T2_Load_Glyph( slot, size, glyph_index, load_flags );

    /* force drop-out mode to 2 - irrelevant now */
    /* slot->outline.dropout_mode = 2; */
    return error;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****             C H A R A C T E R   M A P P I N G S                 ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Get_Char_Index                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Uses a charmap to return a given character code's glyph index.     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    charmap  :: A handle to the source charmap object.                 */
  /*    charcode :: The character code.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Glyph index.  0 means `undefined character code'.                  */
  /*                                                                       */
  static
  FT_UInt  Get_Char_Index( TT_CharMap  charmap,
                           FT_Long     charcode )
  {
    FT_Error       error;
    T2_Face        face;
    TT_CMapTable*  cmap;

    cmap = &charmap->cmap;
    face = (T2_Face)charmap->root.face;

    /* Load table if needed */
    if ( !cmap->loaded )
    {
      SFNT_Interface*  sfnt = (SFNT_Interface*)face->sfnt;

      error = sfnt->load_charmap( face, cmap, face->root.stream );
      if (error) return error;

      cmap->loaded = TRUE;
    }

    return (cmap->get_index ? cmap->get_index( cmap, charcode ) : 0 );
  }


  static
  FTDriver_Interface  t2_get_interface( T2_Driver  driver, const char* interface )
  {
    FT_Driver        sfntd = FT_Get_Driver( driver->root.library, "sfnt" );
    SFNT_Interface*  sfnt;
    
    /* only return the default interface from the SFNT module */
    if (sfntd)
    {
      sfnt = (SFNT_Interface*)(sfntd->interface.format_interface);
      if (sfnt)
        return sfnt->get_interface( (FT_Driver)driver, interface );
    }
    return 0;
  }


  /* The FT_DriverInterface structure is defined in ftdriver.h. */

  const FT_DriverInterface  cff_driver_interface =
  {
    sizeof ( T2_DriverRec ),
    sizeof ( TT_FaceRec ),
    sizeof ( FT_SizeRec ),
    sizeof ( T2_GlyphSlotRec ),

    "cff",           /* driver name                           */
    100,             /* driver version == 1.0                 */
    200,             /* driver requires FreeType 2.0 or above */

    (void*)0,

    (FTDriver_initDriver)        T2_Init_Driver,
    (FTDriver_doneDriver)        T2_Done_Driver,
    (FTDriver_getInterface)      t2_get_interface,

    (FTDriver_initFace)          T2_Init_Face,
    (FTDriver_doneFace)          T2_Done_Face,
    (FTDriver_getKerning)        Get_Kerning,

    (FTDriver_initSize)          T2_Init_Size,
    (FTDriver_doneSize)          T2_Done_Size,
    (FTDriver_setCharSizes)      Set_Char_Sizes,
    (FTDriver_setPixelSizes)     Set_Pixel_Sizes,

    (FTDriver_initGlyphSlot)     T2_Init_GlyphSlot,
    (FTDriver_doneGlyphSlot)     T2_Done_GlyphSlot,
    (FTDriver_loadGlyph)         Load_Glyph,

    (FTDriver_getCharIndex)      Get_Char_Index,
  };



  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    getDriverInterface                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used when compiling the TrueType driver as a      */
  /*    shared library (`.DLL' or `.so').  It will be used by the          */
  /*    high-level library of FreeType to retrieve the address of the      */
  /*    driver's generic interface.                                        */
  /*                                                                       */
  /*    It shouldn't be implemented in a static build, as each driver must */
  /*    have the same function as an exported entry point.                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The address of the TrueType's driver generic interface.  The       */
  /*    format-specific interface can then be retrieved through the method */
  /*    interface->get_format_interface.                                   */
  /*                                                                       */
#ifdef FT_CONFIG_OPTION_DYNAMIC_DRIVERS

  EXPORT_FUNC(FT_DriverInterface*)  getDriverInterface( void )
  {
    return &cff_driver_interface;
  }

#endif /* CONFIG_OPTION_DYNAMIC_DRIVERS */


/* END */
