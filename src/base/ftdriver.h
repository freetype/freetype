/***************************************************************************/
/*                                                                         */
/*  ftdriver.h                                                             */
/*                                                                         */
/*  FreeType driver interface (specification).                             */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef FTDRIVER_H
#define FTDRIVER_H

#include <freetype.h>

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                         D R I V E R S                           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_initDriver                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to create a new driver object for a given     */
  /*    format.                                                            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    driver :: A handle to the `new' driver object.  The fields         */
  /*              `library', `system', and `lock' are already set when the */
  /*              base layer calls this method.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  typedef FT_Error  (*FTDriver_initDriver)( FT_Driver  driver );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_doneDriver                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to finalize a given driver object.  Note that */
  /*    all faces and resources for this driver have been released before  */
  /*    this call, and that this function should NOT destroy the driver    */
  /*    object.                                                            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    driver :: A handle to target driver object.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  typedef FT_Error  (*FTDriver_doneDriver)( FT_Driver  driver );

  
  typedef void  (*FTDriver_Interface)( void );

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_getInterface                                              */
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
  typedef FTDriver_Interface  (*FTDriver_getInterface)
                                ( FT_Driver         driver,
                                  const FT_String*  interface );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_FormatInterface                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver interface field whose value is a driver-specific          */
  /*    interface method table.  This table contains entry points to       */
  /*    various functions that are strictly related to the driver's        */
  /*    format.                                                            */
  /*                                                                       */
  typedef void*  FT_FormatInterface;


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Attach_Reader                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is associated to the `attach_file' driver-specific   */
  /*    interface.  It is used to read additional data for a given face    */
  /*    from another input stream/file.  For example, it is used to        */
  /*    attach a Type 1 AFM file to a given Type 1 face.                   */
  /*                                                                       */
  typedef FT_Error  (*FT_Attach_Reader)( FT_Face  face, FT_Stream  stream );



  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                           F A C E S                             ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_initFace                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to initialize a new face object.  The object  */
  /*    must be created by the caller.                                     */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    stream         :: The input stream.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    typeface_index :: The face index in the font resource.  Used to    */
  /*                      access individual faces in collections.          */
  /*                                                                       */
  /* <Output>                                                              */
  /*    face           :: A handle to the new target face.                 */
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
  /*    FTDriver_doneFace() will be called subsequently, whatever the      */
  /*    result was.                                                        */
  /*                                                                       */
  typedef FT_Error  (*FTDriver_initFace)( FT_Stream  stream,
                                          FT_Long    typeface_index,
                                          FT_Face    face );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_doneFace                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to finalize a given face object.  This        */
  /*    function does NOT destroy the object, that is the responsibility   */
  /*    of the caller.                                                     */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face :: A handle to the target face object.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  typedef void  (*FTDriver_doneFace)( FT_Face  face );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_getKerning                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to return the kerning vector between two      */
  /*    glyphs of the same face.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: A handle to the source face object.                 */
  /*    left_glyph  :: The index of the left glyph in the kern pair.       */
  /*    right_glyph :: The index of the right glyph in the kern pair.      */
  /*                                                                       */
  /* <Output>                                                              */
  /*    kerning     :: A pointer to the kerning vector.  This is in font   */
  /*                   units for scalable formats, and in pixels for       */
  /*                   fixed-sizes formats.                                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Only horizontal layouts (left-to-right & right-to-left) are        */
  /*    supported by this method.  Other layouts, or more sophisticated    */
  /*    kernings are out of the scope of this method (the basic driver     */
  /*    interface is meant to be simple).                                  */
  /*                                                                       */
  /*    They can be implemented by format-specific interfaces.             */
  /*                                                                       */
  typedef FT_Error  (*FTDriver_getKerning)( FT_Face      face,
                                            FT_UInt      left_glyph,
                                            FT_UInt      right_glyph,
                                            FT_Vector*   kerning );



  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                            S I Z E S                            ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_initSize                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to initialize a new size object.  The object  */
  /*    must be created by the caller.                                     */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    size :: A handle to the new size object.                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function should return an error if the face's format isn't    */
  /*    scalable.                                                          */
  /*                                                                       */
  typedef FT_Error  (*FTDriver_initSize)( FT_Size  size );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_setCharSizes                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to reset a size's character sizes (horizontal */
  /*    and vertical) expressed in fractional points.                      */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    size            :: A handle to the target size object.             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    char_width      :: The character width expressed in 26.6           */
  /*                       fractional points.                              */
  /*    char_height     :: The character height expressed in 26.6          */
  /*                       fractional points.                              */
  /*    horz_resolution :: The horizontal resolution.                      */
  /*    vert_resolution :: The vertical resolution.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function should always FAIL if the face format isn't          */
  /*    scalable!                                                          */
  /*                                                                       */
  typedef FT_Error  (*FTDriver_setCharSizes)( FT_Size     size,
                                              FT_F26Dot6  char_width,
                                              FT_F26Dot6  char_height,
                                              FT_UInt     horz_resolution,
                                              FT_UInt     vert_resolution );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_setPixelSizes                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to reset a size's character sizes (horizontal */
  /*    and vertical) expressed in integer pixels.                         */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    size         :: A handle to the target size object.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    pixel_width  :: The character width expressed in 26.6 fractional   */
  /*                    pixels.                                            */
  /*    pixel_height :: The character height expressed in 26.6 fractional  */
  /*                    pixels.                                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function should work with all kinds of `size' objects, either */
  /*    fixed or scalable ones.                                            */
  /*                                                                       */
  typedef FT_Error  (*FTDriver_setPixelSizes)( FT_Size  size,
                                               FT_UInt  pixel_width,
                                               FT_UInt  pixel_height );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_doneSize                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to finalize a given size object.  This method */
  /*    does NOT destroy the object; this is the responsibility of the     */
  /*    caller.                                                            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    size :: A handle to the target size object.                        */
  /*                                                                       */
  typedef void  (*FTDriver_doneSize)( FT_Size  size );



  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                       G L Y P H   S L O T S                     ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_initGlyphSlot                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to initialize a new glyph slot object.  The   */
  /*    object must be created by the caller.  The glyph slot is a         */
  /*    container where a single glyph can be loaded, either in outline or */
  /*    bitmap format.                                                     */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    slot :: A handle to the new glyph slot object.                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  typedef FT_Error  (*FTDriver_initGlyphSlot)( FT_GlyphSlot  slot );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_doneGlyphSlot                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to finalize a given glyph slot.  The object   */
  /*    is not destroyed by this function.                                 */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    slot :: A handle to the new glyph slot object.                     */
  /*                                                                       */
  typedef void  (*FTDriver_doneGlyphSlot)( FT_GlyphSlot  slot );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_loadGlyph                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A driver method used to load a glyph within a given glyph slot.    */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    slot        :: A handle to target slot object where the glyph will */
  /*                   be loaded.                                          */
  /*    size        :: A handle to the source face size at which the glyph */
  /*                   must be scaled/loaded.                              */
  /*                                                                       */
  /* <Input>                                                               */
  /*    glyph_index :: The index of the glyph in the font file.            */
  /*    load_flags  :: A flag indicating what to load for this glyph.  The */
  /*                   FTLOAD_??? constants can be used to control the     */
  /*                   glyph loading process (e.g., whether the outline    */
  /*                   should be scaled, whether to load bitmaps or not,   */
  /*                   whether to hint the outline, etc).                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  typedef FT_Error  (*FTDriver_loadGlyph)( FT_GlyphSlot  slot,
                                           FT_Size       size,
                                           FT_UInt       glyph_index,
                                           FT_Int        load_flags );



  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                  C H A R A C T E R   M A P S                    ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FTDriver_getCharIndex                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Uses a charmap to return a given character code's glyph index.     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    charmap  :: A handle to the source charmap object.                 */
  /*    charcode :: The character code.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The glyph index.  0 means `undefined character code'.              */
  /*                                                                       */
  typedef FT_UInt  (*FTDriver_getCharIndex)( FT_CharMap  charmap,
                                             FT_Long     charcode );



  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                       I N T E R F A C E                         ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_DriverInterface                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure which holds a font driver's basic interface used by    */
  /*    the high-level parts of FreeType (or other applications).          */
  /*                                                                       */
  /*    Most scalable drivers provide a specialized interface to access    */
  /*    format specific features.  It can be retrieved with a call to      */
  /*    `get_format_interface()', and should be defined in each font       */
  /*    driver header (e.g., ttdriver.h, t1driver.h, etc).                 */
  /*                                                                       */
  /*    All fields are function pointers.                                  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    driver_object_size   :: The size in bytes of a single driver       */
  /*                            object.                                    */
  /*    face_object_size     :: The size in bytes of a single face object. */
  /*    size_object_size     :: The size in bytes of a single size object. */
  /*    slot_object_size     :: The size in bytes of a single glyph slot   */
  /*                            object.                                    */
  /*                                                                       */
  /*    driver_name          :: A string to describe the driver to the     */
  /*                            system.  It doesn't necessarily describe   */
  /*                            in detail all the font formats the driver  */
  /*                            may support.                               */
  /*    driver_version       :: The driver version number.  Starts at 1.   */
  /*    driver_requires      :: The FreeType major version this driver is  */
  /*                            written for.  This number should be equal  */
  /*                            to or greater than 2!                      */
  /*                                                                       */
  /*    format_interface     :: A pointer to the driver's format-specific  */
  /*                            interface.                                 */
  /*                                                                       */
  /*    init_driver          :: Used to initialize a given driver object.  */
  /*    done_driver          :: Used to finalize and destroy a given       */
  /*                            driver object.                             */
  /*    get_interface        :: Returns an interface for a given driver    */
  /*                            extension.                                 */
  /*                                                                       */
  /*    init_face            :: Initializes a given face object.           */
  /*    done_face            :: Discards a face object, as well as all     */
  /*                            child objects (sizes, charmaps, glyph      */
  /*                            slots).                                    */
  /*    get_kerning          :: Returns the kerning vector corresponding   */
  /*                            to a pair of glyphs, expressed in unscaled */
  /*                            font units.                                */
  /*                                                                       */
  /*    init_size            :: Initializes a given size object.           */
  /*    done_size            :: Finalizes a given size object.             */
  /*    set_size_char_sizes  :: Resets a scalable size object's character  */
  /*                            size.                                      */
  /*    set_pixel_sizes      :: Resets a face size object's pixel          */
  /*                            dimensions.  Applies to both scalable and  */
  /*                            fixed faces.                               */
  /*                                                                       */
  /*    init_glyph_slot      :: Initializes a given glyph slot object.     */
  /*    done_glyph_slot      :: Finalizes a given glyph slot.              */
  /*    load_glyph           :: Loads a given glyph into a given slot.     */
  /*                                                                       */
  /*    get_char_index       :: Returns the glyph index for a given        */
  /*                            charmap.                                   */
  /*                                                                       */
  typedef struct  FT_DriverInterface_
  {
    FT_Int                       driver_object_size;
    FT_Int                       face_object_size;
    FT_Int                       size_object_size;
    FT_Int                       slot_object_size;

    FT_String*                   driver_name;
    FT_Int                       driver_version;
    FT_Int                       driver_requires;

    void*                        format_interface;

    FTDriver_initDriver          init_driver;
    FTDriver_doneDriver          done_driver;
    FTDriver_getInterface        get_interface;

    FTDriver_initFace            init_face;
    FTDriver_doneFace            done_face;
    FTDriver_getKerning          get_kerning;

    FTDriver_initSize            init_size;
    FTDriver_doneSize            done_size;
    FTDriver_setCharSizes        set_char_sizes;
    FTDriver_setPixelSizes       set_pixel_sizes;

    FTDriver_initGlyphSlot       init_glyph_slot;
    FTDriver_doneGlyphSlot       done_glyph_slot;
    FTDriver_loadGlyph           load_glyph;

    FTDriver_getCharIndex        get_char_index;

  } FT_DriverInterface;


#endif /* FTDRIVER_H */


/* END */
