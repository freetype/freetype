/***************************************************************************/
/*                                                                         */
/*  ttdriver.c                                                             */
/*                                                                         */
/*    TrueType font driver implementation (body).                          */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <ftdebug.h>
#include <ftstream.h>
#include <ttnameid.h>
#include <sfnt.h>

#include <ttdriver.h>
#include <ttgload.h>


#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ttdriver



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


 /******************************************************************
  *
  * <Function>
  *    find_encoding
  *
  * <Description>
  *    return the FT_Encoding corresponding to a given 
  *    (platform_id,encoding_id) pair, as found in TrueType charmaps
  *
  * <Input>
  *   platform_id ::
  *   encoding_id ::
  *
  * <Return>
  *   the corresponding FT_Encoding tag. ft_encoding_none by default
  *
  *****************************************************************/
  
  static
  FT_Encoding   find_encoding( int  platform_id,
                               int  encoding_id )
  {
    typedef struct  TEncoding
    {
      int          platform_id;
      int          encoding_id;
      FT_Encoding  encoding;
  
    } TEncoding;

    static
    const TEncoding   tt_encodings[] =
    {
      { TT_PLATFORM_ISO,                         -1, ft_encoding_unicode },
        
      { TT_PLATFORM_APPLE_UNICODE,               -1, ft_encoding_unicode },
      
      { TT_PLATFORM_MACINTOSH,      TT_MAC_ID_ROMAN, ft_encoding_apple_roman },
      
      { TT_PLATFORM_MICROSOFT,  TT_MS_ID_UNICODE_CS, ft_encoding_unicode },
      { TT_PLATFORM_MICROSOFT,  TT_MS_ID_SJIS,       ft_encoding_sjis },
      { TT_PLATFORM_MICROSOFT,  TT_MS_ID_BIG_5,      ft_encoding_big5 }
    };
    
    const TEncoding  *cur, *limit;
   
    cur   = tt_encodings;
    limit = cur + sizeof(tt_encodings)/sizeof(tt_encodings[0]);
    
    for ( ; cur < limit; cur++ )
    {
      if (cur->platform_id == platform_id)
      {
        if (cur->encoding_id == encoding_id ||
            cur->encoding_id == -1          )
          return cur->encoding;
      }
    }
    return ft_encoding_none;
  }




  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Init_Face                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to initialize a new TrueType face object.     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    resource       :: A handle to the source resource.                 */
  /*                                                                       */
  /*    typeface_index :: An index of the face in the font resource.  Used */
  /*                      to access individual faces in font collections.  */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face           :: A handle to the face object.                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The `typeface_index' parameter field will be set to -1 if the      */
  /*    engine only wants to test the format of the resource.  This means  */
  /*    that font drivers should simply check the font format, then return */
  /*    immediately with an error code of 0 (meaning success).  The field  */
  /*    `num_faces' should be set.                                         */
  /*                                                                       */
  /*    Done_Face() will be called subsequently, whatever the result was.  */
  /*                                                                       */
  static
  TT_Error  Init_Face( FT_Stream  stream,
                       TT_Long    typeface_index,
                       TT_Face    face )
  {
    TT_Error     error;

    /* initialize the TrueType face object */
    error = TT_Init_Face( stream, typeface_index, face );

    /* now set up root fields */
    if ( !error && typeface_index >= 0 )
    {
      FT_Face     root = &face->root;
      FT_Int      flags;
      TT_CharMap  charmap;
      TT_Int      n;
      FT_Memory   memory;

      memory = root->memory;

      /*****************************************************************/
      /*                                                               */
      /* Compute face flags.                                           */
      /*                                                               */
      flags = FT_FACE_FLAG_SCALABLE  |    /* scalable outlines */
              FT_FACE_FLAG_SFNT      |    /* SFNT file format  */
              FT_FACE_FLAG_HORIZONTAL;    /* horizontal data   */

      /* fixed width font ? */
      if ( face->postscript.isFixedPitch )
        flags |= FT_FACE_FLAG_FIXED_WIDTH;

      /* vertical information ? */
      if ( face->vertical_info )
        flags |= FT_FACE_FLAG_VERTICAL;

      /* kerning available ? */
      if ( face->kern_pairs )
        flags |= FT_FACE_FLAG_KERNING;

      root->face_flags = flags;

      /*****************************************************************/
      /*                                                               */
      /* Compute style flags.                                          */
      /*                                                               */
      flags = 0;

      if ( face->os2.version != 0xFFFF )
      {
        /* We have an OS/2 table, use the `fsSelection' field */
        if ( face->os2.fsSelection & 1 )
          flags |= FT_STYLE_FLAG_ITALIC;

        if ( face->os2.fsSelection & 32 )
          flags |= FT_STYLE_FLAG_BOLD;
      }
      else
      {
        /* This is an old Mac font, use the header field */
        if ( face->header.Mac_Style & 1 )
          flags |= FT_STYLE_FLAG_BOLD;

        if ( face->header.Mac_Style & 2 )
          flags |= FT_STYLE_FLAG_ITALIC;
      }

      face->root.style_flags = flags;

      /*****************************************************************/
      /*                                                               */
      /* Polish the charmaps.                                          */
      /*                                                               */
      /*   Try to set the charmap encoding according to the platform & */
      /*   encoding ID of each charmap.                                */
      /*                                                               */
      charmap            = face->charmaps;
      root->num_charmaps = face->num_charmaps;

      /* allocate table of pointers */
      if ( ALLOC_ARRAY( root->charmaps, root->num_charmaps, FT_CharMap ) )
        return error;

      for ( n = 0; n < root->num_charmaps; n++, charmap++ )
      {
        FT_Int  platform = charmap->cmap.platformID;
        FT_Int  encoding = charmap->cmap.platformEncodingID;

        charmap->root.face        = (FT_Face)face;
        charmap->root.platform_id = platform;
        charmap->root.encoding_id = encoding;
        charmap->root.encoding    = find_encoding(platform,encoding);

        /* now, set root->charmap with a unicode charmap wherever available */
        if (!root->charmap && charmap->root.encoding == ft_encoding_unicode)
          root->charmap = (FT_CharMap)charmap;
        
        root->charmaps[n] = (FT_CharMap)charmap;
      }

      root->num_fixed_sizes = 0;
      root->available_sizes = 0;

      /*****************************************************************/
      /*                                                               */
      /*  Set up metrics.                                              */
      /*                                                               */
      root->bbox.xMin    = face->header.xMin;
      root->bbox.yMin    = face->header.yMin;
      root->bbox.xMax    = face->header.xMax;
      root->bbox.yMax    = face->header.yMax;
      root->units_per_EM = face->header.Units_Per_EM;

      /* The ascender/descender/height are computed from the OS/2 table   */
      /* when found.  Otherwise, they're taken from the horizontal header */
      if ( face->os2.version != 0xFFFF )
      {
        root->ascender  =  face->os2.sTypoAscender;
        root->descender = -face->os2.sTypoDescender;
        root->height    =  root->ascender + root->descender +
                           face->os2.sTypoLineGap;
      }
      else
      {
        root->ascender  = face->horizontal.Ascender;
        root->descender = face->horizontal.Descender;
        root->height    = root->ascender + root->descender +
                          face->horizontal.Line_Gap;
      }

      root->max_advance_width  = face->horizontal.advance_Width_Max;

      root->max_advance_height = root->height;
      if ( face->vertical_info )
        root->max_advance_height = face->vertical.advance_Height_Max;

      root->underline_position  = face->postscript.underlinePosition;
      root->underline_thickness = face->postscript.underlineThickness;

      /* root->max_points      - already set up */
      /* root->max_contours    - already set up */

    }
    return error;
  }


#undef  PAIR_TAG
#define PAIR_TAG( left, right )  ( ((TT_ULong)left << 16) | (TT_ULong)right )


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
      return TT_Err_Invalid_Face_Handle;

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
    return TT_Err_Ok;

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
  TT_Error  Set_Char_Sizes( TT_Size     size,
                            TT_F26Dot6  char_width,
                            TT_F26Dot6  char_height,
                            TT_UInt     horz_resolution,
                            TT_UInt     vert_resolution )
  {
    FT_Size_Metrics*  metrics = &size->root.metrics;
    TT_Face           face    = (TT_Face)size->root.face;
    TT_Long           dim_x, dim_y;

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
    
    size->ttmetrics.valid = FALSE;

    return TT_Reset_Size( size );
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
  TT_Error  Set_Pixel_Sizes( TT_Size  size,
                             TT_UInt  pixel_width,
                             TT_UInt  pixel_height )
  {
    (void) pixel_width;
    (void) pixel_height;

    /* many things were pre-computed by the base layer */

    size->ttmetrics.valid = FALSE;

    return TT_Reset_Size( size );
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
  TT_Error  Load_Glyph( TT_GlyphSlot  slot,
                        TT_Size       size,
                        TT_UShort     glyph_index,
                        TT_UInt       load_flags )
  {
    TT_Error  error;


    if ( !slot )
      return TT_Err_Invalid_Glyph_Handle;

    /* check that we want a scaled outline or bitmap */
    if ( !size )
      load_flags |= FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING;

    if ( load_flags & FT_LOAD_NO_SCALE )
      size = NULL;

    /* reset the size object if necessary */
    if ( size )
    {
      /* these two object must have the same parent */
      if ( size->root.face != slot->face )
        return TT_Err_Invalid_Face_Handle;

      if ( !size->ttmetrics.valid )
      {
        if ( FT_SET_ERROR( TT_Reset_Size( size ) ) )
          return error;
      }
    }

    /* now load the glyph outline if necessary */
    error = TT_Load_Glyph( size, slot, glyph_index, load_flags );

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
                           TT_Long     charcode )
  {
    TT_Error       error;
    TT_Face        face;
    TT_CMapTable*  cmap;
    
    cmap = &charmap->cmap;
    face = (TT_Face)charmap->root.face;

    /* Load table if needed */
    if ( !cmap->loaded )
    {
      SFNT_Interface*  sfnt = (SFNT_Interface*)face->sfnt;
      
      error = sfnt->load_charmap( face, cmap, face->root.stream );
      if (error)
        return error;

      cmap->loaded = TRUE;
    }

    if ( cmap->get_index )
      return cmap->get_index( cmap, charcode );
    else
      return 0;
  }


  static
  void*  tt_get_sfnt_table( TT_Face  face, FT_Sfnt_Tag  tag )
  {
    void*  table;
    
    switch (tag)
    {
      case ft_sfnt_head: table = &face->header; break;
      case ft_sfnt_hhea: table = &face->horizontal; break;
      case ft_sfnt_vhea: table = (face->vertical_info ? &face->vertical : 0 ); break;
      case ft_sfnt_os2:  table = (face->os2.version == 0xFFFF ? 0 : &face->os2 ); break;
      case ft_sfnt_post: table = &face->postscript; break;
      case ft_sfnt_maxp: table = &face->max_profile; break;
      
      default:
        table = 0;
    }
    return table;
  }


  static
  FTDriver_Interface  tt_get_interface( TT_Driver  driver, const char* interface )
  {
    (void)driver;
    
    if (strcmp(interface,"get_sfnt")==0)
      return (FTDriver_Interface)tt_get_sfnt_table;
      
    return 0;
  }


  /* The FT_DriverInterface structure is defined in ftdriver.h. */

  const FT_DriverInterface  tt_driver_interface =
  {
    sizeof ( TT_DriverRec ),
    sizeof ( TT_FaceRec ),
    sizeof ( TT_SizeRec ),
    sizeof ( FT_GlyphSlotRec ),

    "truetype",      /* driver name                           */
    100,             /* driver version == 1.0                 */
    200,             /* driver requires FreeType 2.0 or above */

    (void*)0,

    (FTDriver_initDriver)        TT_Init_Driver,
    (FTDriver_doneDriver)        TT_Done_Driver,
    (FTDriver_getInterface)      tt_get_interface,

    (FTDriver_initFace)          Init_Face,
    (FTDriver_doneFace)          TT_Done_Face,
    (FTDriver_getKerning)        Get_Kerning,

    (FTDriver_initSize)          TT_Init_Size,
    (FTDriver_doneSize)          TT_Done_Size,
    (FTDriver_setCharSizes)      Set_Char_Sizes,
    (FTDriver_setPixelSizes)     Set_Pixel_Sizes,

    (FTDriver_initGlyphSlot)     TT_Init_GlyphSlot,
    (FTDriver_doneGlyphSlot)     TT_Done_GlyphSlot,
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

  EXPORT_FUNC
  FT_DriverInterface*  getDriverInterface( void )
  {
    return &tt_driver_interface;
  }

#endif /* CONFIG_OPTION_DYNAMIC_DRIVERS */


/* END */
