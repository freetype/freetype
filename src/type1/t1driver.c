/***************************************************************************/
/*                                                                         */
/*  t1driver.c                                                             */
/*                                                                         */
/*    Type 1 driver interface (body).                                      */
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


#include <t1driver.h>
#include <t1gload.h>
#include <t1afm.h>

#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftstream.h>
#include <freetype/internal/psnames.h>


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t1driver


#ifndef T1_CONFIG_OPTION_NO_AFM


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Get_Interface                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Each driver can provide one or more extensions to the base         */
  /*    FreeType API.  These can be used to access format specific         */
  /*    features (e.g., all TrueType/OpenType resources share a common     */
  /*    file structure and common tables which can be accessed through the */
  /*    `sfnt' interface), or more simply generic ones (e.g., the          */
  /*    `postscript names' interface which can be used to retrieve the     */
  /*     PostScript name of a given glyph index).                          */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    driver    :: A handle to a driver object.                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    interface :: A string designing the interface.  Examples are       */
  /*                 `sfnt', `post_names', `charmaps', etc.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    A typeless pointer to the extension's interface (normally a table  */
  /*    of function pointers).  Returns NULL if the requested extension    */
  /*    isn't available (i.e., wasn't compiled in the driver at build      */
  /*    time).                                                             */
  /*                                                                       */
  static
  FTDriver_Interface  Get_Interface( FT_Driver         driver,
                                     const FT_String*  interface )
  {
    UNUSED( driver );

    if ( strcmp( (const char*)interface, "attach_file" ) == 0 )
      return (FTDriver_Interface)T1_Read_AFM;

    return 0;
  }



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
  FT_Error  Get_Kerning( T1_Face     face,
                         FT_UInt     left_glyph,
                         FT_UInt     right_glyph,
                         FT_Vector*  kerning )
  {
    T1_AFM*  afm;


    kerning->x = 0;
    kerning->y = 0;

    afm = (T1_AFM*)face->afm_data;
    if ( afm )
      T1_Get_Kerning( afm, left_glyph, right_glyph, kerning );

    return T1_Err_Ok;
  }


#endif /* !T1_CONFIG_OPTION_NO_AFM */


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
  /*    char_width      :: The character width expressed in 26.6           */
  /*                       fractional points.                              */
  /*                                                                       */
  /*    char_height     :: The character height expressed in 26.6          */
  /*                       fractional points.                              */
  /*                                                                       */
  /*    horz_resolution :: The horizontal resolution of the output device. */
  /*                                                                       */
  /*    vert_resolution :: The vertical resolution of the output device.   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    size            :: A handle to the target size object.             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  static
  FT_Error  Set_Char_Sizes( T1_Size     size,
                            FT_F26Dot6  char_width,
                            FT_F26Dot6  char_height,
                            FT_UInt     horz_resolution,
                            FT_UInt     vert_resolution )
  {
    UNUSED( char_width );
    UNUSED( char_height );
    UNUSED( horz_resolution );
    UNUSED( vert_resolution );

    size->valid = FALSE;
    return T1_Reset_Size( size );
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
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  static
  FT_Error  Set_Pixel_Sizes( T1_Size  size,
                             FT_Int   pixel_width,
                             FT_Int   pixel_height )
  {
    UNUSED( pixel_width );
    UNUSED( pixel_height );

    size->valid = FALSE;
    return T1_Reset_Size( size );
  }


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
  FT_UInt  Get_Char_Index( FT_CharMap  charmap,
                           FT_Long     charcode )
  {
    T1_Face             face;
    FT_UInt             result = 0;
    PSNames_Interface*  psnames;


    face = (T1_Face)charmap->face;
    psnames = (PSNames_Interface*)face->psnames;
    if ( psnames )
      switch ( charmap->encoding )
      {
        /*******************************************************************/
        /*                                                                 */
        /* Unicode encoding support                                        */
        /*                                                                 */
      case ft_encoding_unicode:
        /* use the `PSNames' module to synthetize the Unicode charmap */
        result = psnames->lookup_unicode( &face->unicode_map,
                                          (FT_ULong)charcode );

        /* the function returns 0xFFFF if the Unicode charcode has */
        /* no corresponding glyph.                                 */
        if ( result == 0xFFFF )
          result = 0;
        goto Exit;

        /*******************************************************************/
        /*                                                                 */
        /* Custom Type 1 encoding                                          */
        /*                                                                 */
      case ft_encoding_adobe_custom:
        {
          T1_Encoding*  encoding = &face->type1.encoding;


          if ( charcode >= encoding->code_first &&
               charcode <= encoding->code_last  )
            result = encoding->char_index[charcode];
          goto Exit;
        }

        /*******************************************************************/
        /*                                                                 */
        /* Adobe Standard & Expert encoding support                        */
        /*                                                                 */
      default:
        if ( charcode < 256 )
        {
          FT_UInt      code;
          FT_Int       n;
          const char*  glyph_name;


          code = psnames->adobe_std_encoding[charcode];
          if ( charmap->encoding == ft_encoding_adobe_expert )
            code = psnames->adobe_expert_encoding[charcode];

          glyph_name = psnames->adobe_std_strings( code );
          if ( !glyph_name )
            break;

          for ( n = 0; n < face->type1.num_glyphs; n++ )
          {
            const char*  gname = face->type1.glyph_names[n];


            if ( gname && gname[0] == glyph_name[0] &&
                 strcmp( gname, glyph_name ) == 0   )
            {
              result = n;
              break;
            }
          }
        }
      }

  Exit:
    return result;
  }


  const  FT_DriverInterface  t1_driver_interface =
  {
    sizeof( FT_DriverRec ),
    sizeof( T1_FaceRec ),
    sizeof( T1_SizeRec ),
    sizeof( T1_GlyphSlotRec ),

    "type1",
    100,
    200,

    0,   /* format interface */

    (FTDriver_initDriver)   T1_Init_Driver,
    (FTDriver_doneDriver)   T1_Done_Driver,

#ifdef T1_CONFIG_OPTION_NO_AFM
    (FTDriver_getInterface) 0,
#else
    (FTDriver_getInterface) Get_Interface,
#endif

    (FTDriver_initFace)     T1_Init_Face,
    (FTDriver_doneFace)     T1_Done_Face,

#ifdef T1_CONFIG_OPTION_NO_AFM
    (FTDriver_getKerning)   0,
#else
    (FTDriver_getKerning)   Get_Kerning,
#endif

    (FTDriver_initSize)     T1_Init_Size,
    (FTDriver_doneSize)     T1_Done_Size,
    (FTDriver_setCharSizes) Set_Char_Sizes,
    (FTDriver_setPixelSizes)Set_Pixel_Sizes,

    (FTDriver_initGlyphSlot)T1_Init_GlyphSlot,
    (FTDriver_doneGlyphSlot)T1_Done_GlyphSlot,
    (FTDriver_loadGlyph)    T1_Load_Glyph,

    (FTDriver_getCharIndex) Get_Char_Index,
  };


#ifdef FT_CONFIG_OPTION_DYNAMIC_DRIVERS


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    getDriverInterface                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used when compiling the CID driver as a shared    */
  /*    library (`.DLL' or `.so').  It will be used by the high-level      */
  /*    library of FreeType to retrieve the address of the driver's        */
  /*    generic interface.                                                 */
  /*                                                                       */
  /*    It shouldn't be implemented in a static build, as each driver must */
  /*    have the same function as an exported entry point.                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The address of the CID's driver generic interface.  The            */
  /*    format-specific interface can then be retrieved through the method */
  /*    interface->get_format_interface.                                   */
  /*                                                                       */
  EXPORT_FUNC( FT_DriverInterface* )  getDriverInterface( void )
  {
    return &t1_driver_interface;
  }


#endif /* FT_CONFIG_OPTION_DYNAMIC_DRIVERS */


/* END */
