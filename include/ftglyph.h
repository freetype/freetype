/***************************************************************************/
/*                                                                         */
/*  ftglyph.h                                                              */
/*                                                                         */
/*    FreeType convenience functions to handle glyphs..                    */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*  This file contains the definition of several convenience functions     */
/*  that can be used by client applications to easily retrieve glyph       */
/*  bitmaps and outlines from a given face.                                */
/*                                                                         */
/*  These functions should be optional if you're writing a font server     */
/*  or text layout engine on top of FreeType. However, they are pretty     */
/*  handy for many other simple uses of the library..                      */
/*                                                                         */
/***************************************************************************/

#ifndef FTGLYPH_H
#define FTGLYPH_H

#include <freetype.h>

  typedef enum {
  
    ft_glyph_type_none    = 0,
    ft_glyph_type_bitmap  = 1,
    ft_glyph_type_outline = 2
  
  } FT_GlyphType;

 /***********************************************************************
  *
  * <Struct>
  *    FT_GlyphRec
  *
  * <Description>
  *    The root glyph structure contains a given glyph image's metrics.
  *    Note that the FT_Glyph type is a pointer to FT_GlyphRec
  *
  * <Field>
  *    memory   :: a handle to the memory allocator that is used to
  *                create/clone/destroy this glyph..
  *
  *    glyph_type :: the glyph type..
  *
  *    height   :: height of glyph image
  *    width    :: width of glyph image
  *
  *    bearingX :: horizontal bearing, this is the distance from the
  *                the current pen position to the left of the glyph
  *
  *    bearingY :: vertical bearing, this is the distance from the
  *                current pen position to the top of the glyph
  *
  *    advance  :: this is the horizontal or vertical advance for the
  *                glyph
  *
  * <Note>
  *    the distances expressed in the metrics are expressed in 26.6 fixed
  *    float sub-pixels (i.e. 1/64th of pixels).
  *
  *    the vertical bearing has a positive value when the glyph top is
  *    above the baseline, and negative when it is under.. 
  *
  ***********************************************************************/

  typedef struct FT_GlyphRec_
  {
    FT_Memory     memory;
    FT_GlyphType  glyph_type;
    FT_Int        height;
    FT_Int        width;
    FT_Int        bearingX;
    FT_Int        bearingY;
    FT_Int        advance;
    
  } FT_GlyphRec, *FT_Glyph;


 /***********************************************************************
  *
  * <Struct>
  *    FT_BitmapGlyphRec
  *
  * <Description>
  *    A structure used to describe a bitmap glyph image..              
  *    Note that the FT_BitmapGlyph type is a pointer to FT_BitmapGlyphRec
  *
  * <Field>
  *    metrics  :: the corresponding glyph metrics
  *    bitmap   :: a descriptor for the bitmap.
  *
  * <Note>
  *    the "width" and "height" fields of the metrics are expressed in
  *    26.6 sub-pixels. However, the width and height in pixels can be
  *    read directly from "bitmap.width" and "bitmap.height"
  *
  *    this structure is used for both monochrome and anti-aliased
  *    bitmaps (the bitmap descriptor contains field describing the
  *    format of the pixel buffer)
  *
  *    the corresponding pixel buffer is always owned by the BitmapGlyph
  *    and is thus creatde and destroyed with it..
  *
  ***********************************************************************/
  
  typedef struct FT_BitmapGlyphRec_
  {
    FT_GlyphRec  metrics;
    FT_Int       left;
    FT_Int       top;
    FT_Bitmap    bitmap;
  
  } FT_BitmapGlyphRec_, *FT_BitmapGlyph;


 /***********************************************************************
  *
  * <Struct>
  *    FT_OutlineGlyphRec
  *
  * <Description>
  *    A structure used to describe a vectorial outline glyph image..              
  *    Note that the FT_OutlineGlyph type is a pointer to FT_OutlineGlyphRec
  *
  * <Field>
  *    metrics  :: the corresponding glyph metrics
  *    outline  :: a descriptor for the outline
  *
  * <Note>
  *    the "width" and "height" fields of the metrics are expressed in
  *    26.6 sub-pixels. However, the width and height in pixels can be
  *    read directly from "bitmap.width" and "bitmap.rows"
  *
  *    the corresponding outline points tables is always owned by the
  *    object and are destroyed with it..
  *
  *    an OutlineGlyph can be used to generate a BitmapGlyph with the
  *    function FT_OutlineGlyph_Render()
  *
  ***********************************************************************/
  
  typedef struct FT_OutlineGlyphRec_
  {
    FT_GlyphRec  metrics;
    FT_Outline   outline;
    
  } FT_OutlineGlyphRec_, *FT_OutlineGlyph;


 /***********************************************************************
  *
  * <Function>
  *    FT_Get_Glyph_Bitmap
  *
  * <Description>
  *    A function used to directly return a monochrome bitmap glyph image
  *    from a face.
  *
  * <Input>
  *    face        :: handle to source face object
  *    glyph_index :: glyph index in face
  *    load_flags  :: load flags, see FT_LOAD_FLAG_XXXX constants..
  *    grays       :: number of gray levels for anti-aliased bitmaps,
  *                   set to 0 if you want to render a monochrome bitmap
  *    origin      :: a pointer to the origin's position. Set to 0
  *                   if the current transform is the identity..
  *
  * <Output>
  *    bitglyph :: pointer to the new bitmap glyph
  *
  * <Return>
  *    Error code. 0 means success.
  *
  * <Note>
  *    If the font contains glyph outlines, these will be automatically
  *    converted to a bitmap according to the value of "grays"
  *
  *    If "grays" is set to 0, the result is a 1-bit monochrome bitmap
  *    otherwise, it is an 8-bit gray-level bitmap
  *
  *    The number of gray levels in the result anti-aliased bitmap might
  *    not be "grays", depending on the current scan-converter implementation
  *
  *    Note that it is not possible to generate 8-bit monochrome bitmaps
  *    with this function. Rather, use FT_Get_Glyph_Outline, then
  *    FT_Glyph_Render_Outline and provide your own span callbacks..
  *
  *    When the face doesn't contain scalable outlines, this function will
  *    fail if the current transform is not the identity, or if the glyph
  *    origin's phase to the pixel grid is not 0 in both directions !!
  *
  ***********************************************************************/

  EXPORT_DEF
  FT_Error  FT_Get_Glyph_Bitmap( FT_Face         face,
                                 FT_UInt         glyph_index,
                                 FT_UInt         load_flags,
                                 FT_Int          grays,
                                 FT_Vector*      origin,
                                 FT_BitmapGlyph  *abitglyph );


 /***********************************************************************
  *
  * <Function>
  *    FT_Get_Glyph_Outline
  *
  * <Description>
  *    A function used to directly return a bitmap glyph image from a
  *    face. This is faster than calling FT_Load_Glyph+FT_Get_Outline_Bitmap..
  *
  * <Input>
  *    face        :: handle to source face object
  *    glyph_index :: glyph index in face
  *    load_flags  :: load flags, see FT_LOAD_FLAG_XXXX constants..
  * 
  * <Output>
  *    vecglyph :: pointer to the new outline glyph
  *
  * <Return>
  *    Error code. 0 means success.
  *
  * <Note>
  *    If the glyph is not an outline in the face, this function will
  *    fail..
  *
  *    This function will fail if the load flags FT_LOAD_NO_OUTLINE and
  *    FT_LOAD_NO_RECURSE are set..
  *
  ***********************************************************************/
  
  EXPORT_DEF
  FT_Error  FT_Get_Glyph_Outline( FT_Face           face,
                                  FT_UInt           glyph_index,
                                  FT_UInt           load_flags,
                                  FT_OutlineGlyph  *vecglyph );


 /***********************************************************************
  *
  * <Function>
  *    FT_Set_Transform
  *
  * <Description>
  *    A function used to set the transform that is applied to glyph images
  *    just after they're loaded in the face's glyph slot, and before they're 
  *    returned by either FT_Get_Glyph_Bitmap or FT_Get_Glyph_Outline
  *
  * <Input>
  *    face   :: handle to source face object
  *    matrix :: pointer to the transform's 2x2 matrix. 0 for identity
  *    delta  :: pointer to the transform's translation. 0 for null vector
  *
  * <Note>
  *    The transform is only applied to glyph outlines when they are found
  *    in a font face. It is unable to transform embedded glyph bitmaps
  *
  ***********************************************************************/
  
  EXPORT_DEF
  void FT_Set_Transform( FT_Face     face,
                         FT_Matrix*  matrix,
                         FT_Vector*  delta );


 /***********************************************************************
  *
  * <Function>
  *    FT_Done_Glyph
  *
  * <Description>
  *    Destroys a given glyph..
  *
  * <Input>
  *    glyph  :: handle to target glyph object 
  *
  ***********************************************************************/
  
  EXPORT_DEF
  void  FT_Done_Glyph( FT_Glyph  glyph );


 /***********************************************************************
  *
  * <Function>
  *    FT_Glyph_Get_Box
  *
  * <Description>
  *    Returns the glyph image's bounding box in pixels.
  *
  * <Input>
  *    glyph :: handle to target glyph object 
  *
  * <Output>
  *    box   :: the glyph bounding box. Coordinates are expressed in
  *             _integer_ pixels, with exclusive max bounds
  *
  * <Note>
  *    Coordinates are relative to the glyph origin, using the Y-upwards
  *    convention..
  *
  *    The width of the box in pixels is box.xMax-box.xMin
  *    The height is box.yMax - box.yMin
  *
  ***********************************************************************/
  
  EXPORT_DEF
  void  FT_Glyph_Get_Box( FT_Glyph  glyph,
                          FT_BBox  *box );

#endif /* FTGLYPH_H */
