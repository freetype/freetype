/*******************************************************************
 *
 *  t1driver.c
 *
 *    High-level Type1 driver interface for FreeType 2.0
 *
 *  Copyright 1996-1998 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used,
 *  modified, and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute 
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 ******************************************************************/

#include <t1driver.h>
#include <t1gload.h>

#include <ftdebug.h>
#include <ftstream.h>

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t1driver

  /******************************************************************/
  /*                                                                */
  /* <Function> Set_Char_Sizes                                      */
  /*                                                                */
  /* <Description>                                                  */
  /*    A driver method used to reset a size's character sizes      */
  /*    (horizontal and vertical) expressed in fractional points.   */
  /*                                                                */
  /* <Input>                                                        */
  /*    size        :: handle to target size object                 */
  /*    char_width  :: character width expressed in 26.6 points     */
  /*    char_height :: character height expressed in 26.6 points    */
  /*                                                                */
  /* <Return>                                                       */
  /*    FreeType error code. 0 means success                        */
  /*                                                                */
  static
  T1_Error  Set_Char_Sizes( T1_Size      size,
                            T1_F26Dot6   char_width,
                            T1_F26Dot6   char_height,
                            T1_UInt      horz_resolution,
                            T1_UInt      vert_resolution )
  {
    FT_Size_Metrics*  metrics;
    T1_Face           face;
  
    if (!size)
      return T1_Err_Invalid_Size_Handle;
    
    if ( char_width  < 1*64 ) char_width = 1*64;
    if ( char_height < 1*64 ) char_height = 1*64;

    metrics = &size->root.metrics;
    face    = (T1_Face)size->root.face;
    
    metrics->x_ppem = ( char_width  * horz_resolution + 36 )/72;
    metrics->y_ppem = ( char_height * vert_resolution + 36 )/72;
    
    metrics->x_ppem = ( metrics->x_ppem + 32 ) & -64;
    metrics->y_ppem = ( metrics->y_ppem + 32 ) & -64;
    
    metrics->x_scale = FT_MulDiv( metrics->x_ppem, 0x10000,
                                  face->root.units_per_EM );
    
    metrics->y_scale = FT_MulDiv( metrics->y_ppem, 0x10000,
                                  face->root.units_per_EM );

    metrics->x_ppem >>= 6;
    metrics->y_ppem >>= 6;
    
    size->valid = FALSE;

    return T1_Reset_Size( size );
  }


  /******************************************************************/
  /*                                                                */
  /* <Function> Set_Pixel_Sizes                                     */
  /*                                                                */
  /* <Description>                                                  */
  /*    A driver method used to reset a size's character sizes      */
  /*    (horizontal and vertical) expressed in integer pixels.      */
  /*                                                                */
  /* <Input>                                                        */
  /*    size         :: handle to target size object                */
  /*                                                                */
  /*    pixel_width  :: character width expressed in 26.6 points    */
  /*                                                                */
  /*    pixel_height :: character height expressed in 26.6 points   */
  /*                                                                */
  /*    char_size    :: the corresponding character size in points  */
  /*                    This value is only sent to the TrueType     */
  /*                    bytecode interpreter, even though 99% of    */
  /*                    glyph programs will simply ignore it. A     */
  /*                    safe value there is the maximum of the      */
  /*                    pixel width and height (multiplied by       */
  /*                    64 to make it a 26.6 fixed float !)         */
  /* <Return>                                                       */
  /*    FreeType error code. 0 means success                        */
  /*                                                                */
  static
  T1_Error  Set_Pixel_Sizes( T1_Size     size,
                             T1_Int      pixel_width,
                             T1_Int      pixel_height )
  {
    FT_Size_Metrics*   metrics;
    T1_Face            face;
  
    if (!size)
      return T1_Err_Invalid_Size_Handle;
    
    if ( pixel_width  < 1 ) pixel_width = 1;
    if ( pixel_height < 1 ) pixel_height = 1;
  
    metrics = &size->root.metrics;
    face    = (T1_Face)size->root.face;
  
    metrics->x_ppem    = pixel_width;
    metrics->y_ppem    = pixel_height;
  
    metrics->x_scale = FT_MulDiv( metrics->x_ppem * 64,
                                  0x10000, face->root.units_per_EM );
  
    metrics->y_scale = FT_MulDiv( metrics->y_ppem * 64,
                                  0x10000, face->root.units_per_EM );
  
    size->valid = 0;
  
    return T1_Reset_Size(size);
  }


  /******************************************************************/
  /*                                                                */
  /* <Struct> FT_DriverInterface                                    */
  /*                                                                */
  /* <Description>                                                  */
  /*    A structure used to hold a font driver's basic interface    */
  /*    used by the high-level parts of FreeType (or other apps)    */
  /*                                                                */
  /*    Most scalable drivers provide a specialized interface to    */
  /*    access format-specific features. It can be retrieved with   */
  /*    a call to the "get_format_interface", and should be defined */
  /*    in each font driver header (e.g. ttdriver.h, t1driver.h,..) */
  /*                                                                */
  /*    All fields are function pointers ..                         */
  /*                                                                */
  /*                                                                */
  /* <Fields>                                                       */
  /*                                                                */
  /*    new_engine ::                                               */
  /*        used to create and initialise a new driver object       */
  /*                                                                */
  /*    done_engine ::                                              */
  /*        used to finalise and destroy a given driver object      */
  /*                                                                */
  /*    get_format_interface ::                                     */
  /*        return a typeless pointer to the format-specific        */
  /*        driver interface.                                       */
  /*                                                                */
  /*    new_face ::                                                 */
  /*        create a new face object from a resource                */
  /*                                                                */
  /*    done_face ::                                                */
  /*        discards a face object, as well as all child objects    */
  /*        ( sizes, charmaps, glyph slots )                        */
  /*                                                                */
  /*    get_face_properties ::                                      */
  /*        return generic face properties                          */
  /*                                                                */
  /*    get_kerning ::                                              */
  /*        return the kerning vector corresponding to a pair       */
  /*        of glyphs, expressed in unscaled font units.            */
  /*                                                                */
  /*    new_size ::                                                 */
  /*        create and initialise a new scalable size object.       */
  /*                                                                */
  /*    new_fixed_size ::                                           */
  /*        create and initialise a new fixed-size object.          */
  /*                                                                */
  /*    done_size ::                                                */
  /*        finalize a given face size object.                      */
  /*                                                                */
  /*    set_size_resolutions ::                                     */
  /*        reset a scalable size object's output resolutions       */
  /*                                                                */
  /*    set_size_char_sizes ::                                      */
  /*        reset a scalable size object's character size           */
  /*                                                                */
  /*    set_pixel_sizes ::                                          */
  /*        reset a face size object's pixel dimensions. Applies    */
  /*        to both scalable and fixed faces.                       */
  /*                                                                */
  /*    new_glyph_slot ::                                           */
  /*        create and initialise a new glyph slot                  */
  /*                                                                */
  /*    done_glyph_slot ::                                          */
  /*        discard a given glyph slot                              */
  /*                                                                */
  /*    load_glyph ::                                               */
  /*        load a given glyph into a given slot                    */
  /*                                                                */
  /*    get_glyph_metrics ::                                        */
  /*        return a loaded glyph's metrics.                        */
  /*                                                                */

  EXPORT_FUNC
  const  FT_DriverInterface  t1_driver_interface =
  {
    sizeof( FT_DriverRec ),
    sizeof( T1_FaceRec ),
    sizeof( T1_SizeRec ),
    sizeof( T1_GlyphSlotRec ),
    
    "type1",
    1,
    2,

    0,   /* format interface */

    (FTDriver_initDriver)           T1_Init_Driver,
    (FTDriver_doneDriver)           T1_Done_Driver,
    (FTDriver_getInterface)         0,

    (FTDriver_initFace)             T1_Init_Face,
    (FTDriver_doneFace)             T1_Done_Face,
    (FTDriver_getKerning)           0,

    (FTDriver_initSize)             T1_Init_Size,
    (FTDriver_doneSize)             T1_Done_Size,
    (FTDriver_setCharSizes)         Set_Char_Sizes,
    (FTDriver_setPixelSizes)        Set_Pixel_Sizes,

    (FTDriver_initGlyphSlot)        T1_Init_GlyphSlot,
    (FTDriver_doneGlyphSlot)        T1_Done_GlyphSlot,
    (FTDriver_loadGlyph)            T1_Load_Glyph,

    (FTDriver_getCharIndex)         0,
  };


  /******************************************************************/
  /*                                                                */
  /*  <Function> Get_FreeType_Driver_Interface                      */
  /*                                                                */
  /*  <Description>                                                 */
  /*     This function is used when compiling the TrueType driver   */
  /*     as a shared library (.DLL or .so). It will be used by the  */
  /*     high-level library of FreeType to retrieve the address of  */
  /*     the driver's generic interface.                            */
  /*                                                                */
  /*     It shouldn't be implemented in a static build, as each     */
  /*     driver must have the same function as an exported entry    */
  /*     point.                                                     */
  /*                                                                */
  /*  <Return>                                                      */
  /*     address of TrueType's driver generic interface. The        */
  /*     forma-specific interface can then be retrieved through     */
  /*     the method interface->get_format_interface..               */
  /*                                                                */

#ifdef FT_CONFIG_OPTION_DYNAMIC_DRIVERS
  
  EXPORT_FUNC
  FT_DriverInterface*  getDriverInterface( void )
  {
    return &t1_driver_interface;
  }
  
#endif /* FT_CONFIG_OPTION_DYNAMIC_DRIVERS */


