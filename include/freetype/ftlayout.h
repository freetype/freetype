/***************************************************************************/
/*                                                                         */
/*  ftltypes.h                                                             */
/*                                                                         */
/*    FreeType Layout API  (specification)                                 */    
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/

#ifndef __FTL_LAYOUT_H_
#define __FTL_LAYOUT_H_

#include <ft2build.h>
#include FT_FREETYPE_H

FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    text_layout                                                        */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Text Layout                                                        */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    Generic text layout functions.                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The following types and functions are used in text layout.         */
  /*    FTLayout is a layout engine stacked on FreeType2.  Currently       */
  /*    TrueTypeGX/AAT is supported as a file format.  OpenType is also    */
  /*    supported, but highly experimental.                                */
  /*    FTLayout provides an abstract interface which is shared by a       */
  /*    layout engine(GXLayout) for TrueTypeGX/AAT and a layout engine     */
  /*    (OTLayout)for OpenType for glyph substitution, one of the text     */
  /*    layout function.                                                   */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FTL_FeaturesRequest                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An opaque data type to specify which font features are used in     */
  /*    text layout.                                                       */
  /*                                                                       */
  /*    Use @FTL_New_FeaturesRequest to create from a face and             */
  /*    @FTL_Done_FeaturesRequest to discard. The way to specify the font  */
  /*    features' settting depenes on the concrete font engine behind the  */
  /*    face.                                                              */
  /*                                                                       */
  typedef struct FTL_FeaturesRequestRec_ * FTL_FeaturesRequest;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FTL_GlyphArray                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Data type represents glyphs array used in glyph substitution.      */
  /*    See @FTL_GlyphRec for more detail.                                 */
  /*                                                                       */
  typedef struct FTL_GlyphArrayRec_    * FTL_GlyphArray;

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*   FTL_EngineType                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*   An enumeration representing the concrete text layout engine type.   */
  /*   You can get the value for a given face, use @FTL_Query_EngineType.  */
  /*                                                                       */
  /* <Values>                                                              */
  /*   FTL_NO_ENGINE ::                                                    */
  /*     No text layout engine behind the face.                            */
  /*                                                                       */
  /*   FTL_OPENTYPE_ENGINE ::                                              */
  /*     OpneType layout engine. You should use the interface declared in  */
  /*     otlayout.h.                                                       */
  /*                                                                       */
  /*   FTL_TRUETYPEGX_ENGINE ::                                            */
  /*     TrueTypeGX/AAT layout engine. You should use the interface        */
  /*     declared in gxlayout.h.                                           */
  /*                                                                       */
  typedef enum
  {
    FTL_NO_ENGINE         = 0,
    FTL_OPENTYPE_ENGINE   = 1,		/* Use otlayout.h */
    FTL_TRUETYPEGX_ENGINE = 2		/* Use gxlayout.h */
  } FTL_EngineType;

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FTL_Direction                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration representing the text layout direction.             */
  /*                                                                       */
  /*    You can set the direction value to a features request by           */
  /*    @FTL_Set_FeaturesRequest_Direction. You can get the direction      */
  /*    value from a features request by                                   */
  /*    @FTL_Get_FeaturesRequest_Direction.                                */
  /*                                                                       */
  /* <Values>                                                              */
  /*   FTL_HORIZONTAL ::                                                   */
  /*     Value representing horizontal writing.                            */
  /*                                                                       */
  /*   FTL_VERTICAL ::                                                     */
  /*     Value representing vertical writing.                              */
  /*                                                                       */      
  typedef enum 
  { 
    FTL_HORIZONTAL = 0,  
    FTL_VERTICAL   = 1
  } FTL_Direction;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*   FTL_GlyphRec                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Data type represents a glyph id used in glyph substitution.        */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    gid     :: The glyph id. If gid is 0xFFFF, this element in an      */
  /*               glyph array should be ignored.                          */
  /*                                                                       */
  /*    ot_prop :: Glyph's property used in OTLayout substitution.         */
  /*                       GXLayout does not use this field.               */
  /*                                                                       */
  typedef struct FTL_GlyphRec_
  {
    FT_UShort gid;
    FT_UShort reserved1;
    FT_ULong  ot_prop;
    FT_ULong  reserved2;
  } FTL_GlyphRec, * FTL_Glyph;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*   FTL_Glyph_ArrayRec_                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Data type represents glyphs array used in glyph substitution.      */
  /*                                                                       */
  /*    This data type is used as input and output arguments for           */
  /*    @FTL_Substitute_Glyphs.                                            */
  /*    @FTL_New_Glyphs_Array allocates 0 length FTL_GlyphRec object.      */
  /*    @FTL_Set_Glyphs_Array_Length sets the length of FTL_GlyphRec       */
  /*    object. You have to set the length to create an input glyph array  */
  /*    for substitution.                                                  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    memory    :: The current memory object which handles the glyph     */
  /*                 arrray.                                               */
  /*                                                                       */
  /*    glyphs    :: Glyphs ids' array.                                    */
  /*                                                                       */
  /*    length    :: The valid length of glyphs.                           */
  /*                                                                       */
  /*    allocated :: The allocation size of glyphs. The client should not  */
  /*                 refer this field.                                     */
  /*                                                                       */
  typedef struct FTL_GlyphArrayRec_
  {
    FT_Memory   memory;
    FTL_Glyph   glyphs;
    /* FT_ULong    pos; */
    FT_ULong    reserved1;
    FT_ULong    length;
    FT_ULong    allocated;
  } FTL_GlyphArrayRec;


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Query_EngineType                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the text layout engine type behind a face.                 */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: The target face.                                    */
  /*                                                                       */
  /* <Output>                                                              */
  /*    engine_type :: The type of text layout engine for the face.        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FTL_Query_EngineType                  ( FT_Face face, 
					  FTL_EngineType * engine_type);

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_New_FeaturesRequest                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new features request for a face.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face    :: The target face.                                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    request :: A features request for a face.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    When the target face is discarded by @FT_Done_Face, all features   */
  /*    requests corresponding to the face are also discardeded            */
  /*    automatically.                                                     */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FTL_New_FeaturesRequest               ( FT_Face face, 
					  FTL_FeaturesRequest* request);

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Done_FeaturesRequest                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Discards a features request.                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    request :: A features request.                                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FTL_Done_FeaturesRequest              ( FTL_FeaturesRequest request );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Activate_FeaturesRequest                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Even though it is possible to create serveral features request     */
  /*    object for a given face (see @FTL_New_FeaturesRequest),            */
  /*    @FTL_Substitute_Glyphs only use the last activated request.        */
  /*    With this function, a features request can be activated.           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    request :: A features request to be activated.                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_EXPORT ( FT_Error )
  FTL_Activate_FeaturesRequest          ( FTL_FeaturesRequest request );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Copy_FeaturesRequest                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Copy the setting of one features request to another.               */ 
  /*                                                                       */
  /* <Input>                                                               */
  /*    from :: The source features request.                               */
  /*                                                                       */
  /* <Output>                                                              */
  /*    to   :: The destination features request. This must be created by  */
  /*            @FTL_New_FeaturesRequest before copying.                   */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    from and to must be created from the same face.                    */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FTL_Copy_FeaturesRequest              ( FTL_FeaturesRequest from,  
					  FTL_FeaturesRequest to );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Reset_FeaturesRequest                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Resets the settings of a features request. In other word set a     */ 
  /*    features request to default settings.                              */
  /*                                                                       */
  /* <Input>                                                               */
  /*    request :: A features request to be reseted.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FTL_Reset_FeaturesRequest             ( FTL_FeaturesRequest request );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Get_FeaturesRequest_Direction                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the writing direction in a features request.               */ 
  /*                                                                       */
  /* <Input>                                                               */
  /*    request :: A features request.                                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The writing direction.                                             */
  /*                                                                       */
  FT_EXPORT ( FTL_Direction )
  FTL_Get_FeaturesRequest_Direction     ( FTL_FeaturesRequest request );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Set_FeaturesRequest_Direction                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets the writing direction to a features request.                   */ 
  /*                                                                       */
  /* <Input>                                                               */
  /*    request   :: A features request.                                   */
  /*                                                                       */
  /*    direction :: Writing direction.                                    */
  /*                                                                       */
  FT_EXPORT ( void )
  FTL_Set_FeaturesRequest_Direction     ( FTL_FeaturesRequest request,  
					  FTL_Direction direction);

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_New_Glyphs_Array                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates an empty glyphs array.                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    momory ::  A memory object from which a glyphs array is allocated. */
  /*                                                                       */
  /* <Output>                                                              */
  /*    garray ::  A glyphs array.                                         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The lengoth of newly allocated glyphs array is 0. Set the length   */
  /*    by @FTL_Set_Glyphs_Array_Length after allocating if you need.      */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FTL_New_Glyphs_Array                  ( FT_Memory memory,
					  FTL_GlyphArray * garray );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Set_Glyphs_Array_Length                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets the length of a glyphs array.                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    length ::  New glyphs array length.                                */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    garray ::  A glyphs array.                                         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FTL_Set_Glyphs_Array_Length           ( FTL_GlyphArray garray,
					  FT_ULong new_length );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Copy_Glyphs_Array                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Copies glyphs array.                                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    in  ::  The source glyphs array.                                   */
  /*                                                                       */
  /* <Output>                                                              */
  /*    out ::  The destination glyphs array.                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FTL_Copy_Glyphs_Array                 ( FTL_GlyphArray in,
					  FTL_GlyphArray out );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Done_Glyphs_Array                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Discards a glyphs array.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    garray  ::  A glyphs array.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FTL_Done_Glyphs_Array                 ( FTL_GlyphArray garray );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Substitute_Glyphs                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Substitutes glyphs based on the rule specified by the current      */
  /*    activated features request.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face ::  A face which has substitution tables.                     */
  /*                                                                       */
  /*    in   ::  Input glyphs array.                                       */  
  /*                                                                       */
  /* <Output>                                                              */
  /*    out  ::  Output(substituted) glyphs array. The length of glyphs    */
  /*             are automatically extended.                               */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FTL_Substitute_Glyphs                 ( FT_Face face,
					  FTL_GlyphArray in,
					  FTL_GlyphArray out );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Get_LigatureCaret_Count                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the number of ligature divisions for a given glyph.        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face    :: A face.                                                 */
  /*                                                                       */
  /*    glyphID :: The target glyph's id.                                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The number of ligature divisions. If the glyph is not ligatured    */
  /*    glyph, returns 0.                                                  */
  /*                                                                       */
  FT_EXPORT( FT_UShort )
  FTL_Get_LigatureCaret_Count           ( FT_Face face, FT_UShort glyphID );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTL_Get_LigatureCaret_Division                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the point of Nth ligature divisions for a given glyph.     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face    :: A face.                                                 */
  /*                                                                       */
  /*    glyphID :: The target glyph's id.                                  */
  /*                                                                       */
  /*    nth     :: The index of ligature divisions.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The point of a ligature division. If the glyph is not ligatured    */
  /*    glyph, returns 0.                                                  */
  /*                                                                       */
  FT_EXPORT( FT_UShort )
  FTL_Get_LigatureCaret_Division        ( FT_Face face, 
					  FT_UShort glyphID, 
					  FT_UShort nth );

FT_END_HEADER

#endif	/* Not def: __FTL_LAYOUT_H_ */


/* END */
