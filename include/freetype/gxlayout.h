/***************************************************************************/
/*                                                                         */
/*  gxlayout.h                                                             */
/*                                                                         */
/*    AAT/TrueTypeGX based layout engine(specification).                   */
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

#ifndef __GXLAYOUT_H__
#define __GXLAYOUT_H__ 


#include <ft2build.h>
#include FT_SFNT_NAMES_H

FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    gx_text_layout                                                     */
  /*                                                                       */
  /* <Title>                                                               */
  /*    GX Text Layout                                                     */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    TrueTypeGX/AAT text layout types and functions.                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The following types and functions(interface) are used in text      */
  /*    layout with TrueTypeGX/AAT font. You have to use combination with  */
  /*    this GXLayout interface and generic FTLayout interface.            */
  /*    You have to use GXLayout interface to get the font features        */
  /*    defined in `feat' table; and specify settings to the features      */
  /*    request created by @FTL_New_FeaturesRequest.                       */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    GXL_FeaturesRequest                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    GXLayout level representation of FTL_FeaturesRequest.              */
  /*                                                                       */
  /*    GXLayout functions dealing in features requests requires           */
  /*    arguments typed to GXL_FeaturesRequest instead of                  */
  /*    FTL_FeaturesRequest.                                               */
  /*                                                                       */
  typedef struct GXL_FeaturesRequestRec_ *GXL_FeaturesRequest;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    GXL_Feature                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An opaque data type representing a feature of TrueTypeGX/AAT font. */
  /*                                                                       */
  /*    @GXL_FeaturesRequest_Get_Feature_Count returns the number of       */
  /*    in a font. @GXL_FeaturesRequest_Get_Feature returns Nth feature    */
  /*    object in a font.                                                  */
  /*                                                                       */
  typedef struct GXL_FeatureRec_         *GXL_Feature;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    GXL_Setting                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An opaque data type representing a setting of a feature.           */
  /*                                                                       */
  /*    @GXL_Feature_Get_Setting_Count returns the number of settings in a */
  /*    feature. @GXL_Feature_Get_Setting returns Nth setting in a         */
  /*    feature.                                                           */
  /*                                                                       */
  typedef struct GXL_SettingRec_         *GXL_Setting;


  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    GXL_Initial_State                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration for the initial state of GX glyph substitution      */
  /*    automaton.                                                         */
  /*                                                                       */
  /*    You can set/get these value to/from a features request by          */
  /*    @GXL_FeaturesRequest_Set_Initial_State and                         */
  /*    @GXL_FeaturesRequest_Set_Initial_State.                            */
  /*                                                                       */
  /* <Values>                                                              */
  /*    GXL_START_OF_TEXT_STATE ::                                         */
  /*      State of text start.                                             */
  /*                                                                       */
  /*    GXL_START_OF_LINE_STATE ::                                         */
  /*      State of line start.                                             */
  /*                                                                       */
  typedef enum
  {
    GXL_START_OF_TEXT_STATE = 0,
    GXL_START_OF_LINE_STATE = 1
  } GXL_Initial_State;

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    GXL_FeaturesRequest_Set_Initial_State                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Sets a initial state for glyph the initial state of GX glyph       */
  /*    substitution automaton to a features request.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    request       :: a Target features request.                        */
  /*                                                                       */
  /*    initial_state :: The initial state.                                */
  /*                                                                       */
  FT_EXPORT ( void )
  GXL_FeaturesRequest_Set_Initial_State ( GXL_FeaturesRequest request,
					  GXL_Initial_State   initial_state );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    GXL_FeaturesRequest_Get_Initial_State                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns a initial state for glyph the initial state of GX glyph    */
  /*    substitution automaton in a features request.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    request :: A target features request.                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    An initial state value typed to @GXL_Initial_State.                */
  /*                                                                       */
  FT_EXPORT ( GXL_Initial_State )
  GXL_FeaturesRequest_Get_Initial_State ( GXL_FeaturesRequest request );
					  

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    GXL_FeaturesRequest_Get_Feature_Count                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the number of features in a features request.              */
  /*                                                                       */
  /* <Input>                                                               */
  /*    request :: A target features request.                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The number of features.                                            */
  /*                                                                       */
  FT_EXPORT ( FT_ULong )
  GXL_FeaturesRequest_Get_Feature_Count ( GXL_FeaturesRequest request );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    GXL_FeaturesRequest_Get_Feature                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns Nth font feature object from a features request.           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    request :: A target features request.                              */
  /*                                                                       */
  /*    index   :: The index of feature.                                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    A feature object.                                                  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Use @GXL_FeaturesRequest_Get_Feature_Count to get the number of    */
  /*    available features.                                                */
  /*                                                                       */
  FT_EXPORT ( GXL_Feature )
  GXL_FeaturesRequest_Get_Feature       ( GXL_FeaturesRequest request, 
					  FT_ULong index);

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    GXL_Feature_Get_Name                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Gets the name for a feature.                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    feature     :: A target feature.                                   */
  /*                                                                       */
  /*    platform_id :: Platform ID for the name. 0 is wild-card.           */
  /*                                                                       */
  /*    encoding_id :: Encoding ID for the name. 0 is wild-card.           */
  /*                                                                       */
  /*    language_id :: Language ID for the name. 0 is wild-card.           */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aname       :: The name of a setting. The memory area must be      */
  /*                   allocated by the client. The pointer to a object    */
  /*                   on a stack is ok.                                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error ) 
  GXL_Feature_Get_Name                  ( GXL_Feature  feature ,
					  FT_UShort    platform_id,
					  FT_UShort    pencoding_id,
					  FT_UShort    language_id,
					  FT_SfntName *aname );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    GXL_Feature_Get_Setting_Count                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the number of settings in a feature                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    request :: A target feature.                                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The number of settings.                                            */
  /*                                                                       */
  FT_EXPORT( FT_UShort )
  GXL_Feature_Get_Setting_Count         ( GXL_Feature feature );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    GXL_Feature_Get_Setting                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns Nth setting object from a feature.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    feature :: A target feature.                                       */
  /*                                                                       */
  /*    index   :: The index of setting.                                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    A setting object.                                                  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Use @GXL_Feature_Get_Setting_Count to get the number of available  */
  /*    settings.                                                          */
  /*                                                                       */
  FT_EXPORT( GXL_Setting )
  GXL_Feature_Get_Setting               ( GXL_Feature feature,
					  FT_ULong    index );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    GXL_Feature_Is_Setting_Exclusive                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns whether a feature's setting is exclusive or not.           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    feature :: A target feature.                                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    True if a feature's setting is exclusive. False if not.            */
  /*                                                                       */
  FT_EXPORT( FT_Bool )
  GXL_Feature_Is_Setting_Exclusive      ( GXL_Feature feature );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    GXL_Setting_Get_Name                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Gets the name for a setting.                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    setting     :: A target setting.                                   */
  /*                                                                       */
  /*    platform_id :: Platform ID for the name. 0 is wild-card.           */
  /*                                                                       */
  /*    encoding_id :: Encoding ID for the name. 0 is wild-card.           */
  /*                                                                       */
  /*    language_id :: Language ID for the name. 0 is wild-card.           */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aname       :: The name of a setting. The memory area must be      */
  /*                   allocated by the client. The pointer to a object    */
  /*                   on a stack is ok.                                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  GXL_Setting_Get_Name                  ( GXL_Setting  setting,
					  FT_UShort    platform_id,
					  FT_UShort    encoding_id,
					  FT_UShort    language_id,
					  FT_SfntName *aname );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    GXL_Setting_Get_State                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns whether a setting is enabled or disabled.                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    setting :: A target setting.                                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    True if a setting is enabled. False if disabled.                   */
  /*                                                                       */
  FT_EXPORT( FT_Bool )
  GXL_Setting_Get_State                 ( GXL_Setting setting );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    GXL_Setting_Set_State                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Make a setting enable or disable.                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    setting :: A target setting.                                       */
  /*                                                                       */
  /*    state   :: A state to set. If this is true, the setting is         */
  /*               enabled. If this is false, the setting is disabled.     */
  /*                                                                       */
  FT_EXPORT( void )
  GXL_Setting_Set_State                 ( GXL_Setting setting, 
					  FT_Bool state );
       
FT_END_HEADER

#endif /* Not def: __GXLAYOUT_H__ */



