/***************************************************************************/
/*                                                                         */
/*  ftmm.h                                                                 */
/*                                                                         */
/*    FreeType Multiple-Master interface.                                  */
/*                                                                         */
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

#ifndef FTMM_H
#define FTMM_H

#include <freetype/t1tables.h>

#ifdef __cplusplus
  extern "C" {
#endif

 /**********************************************************************
  *
  * <Struct>
  *    FT_MM_Axis
  *
  * <Description>
  *    A simple structure used to model a given axis in design space
  *    for multiple masters fonts..
  *
  * <Fields>
  *    name     :: axis' name
  *    minimum  :: axis' minimum design coordinate
  *    maximum  :: axis's maximum design coordinate
  *
  */
  typedef struct FT_MM_Axis_
  {
    FT_String*  name;
    FT_Long     minimum;
    FT_Long     maximum;
  
  } FT_MM_Axis;

 /**********************************************************************
  *
  * <Struct>
  *    FT_Multi_Master
  *
  * <Description>
  *    A structure used to model the axis and space of a multiple    
  *    masters font.               
  *
  * <Fields>
  *    num_axis    :: number of axis. cannot exceed 4
  *
  *    num_designs :: number of designs, should ne normally 2^num_axis
  *                   even though the Type 1 specification strangely
  *                   allows for intermediate designs to be present
  *                   this number cannot exceed 16
  *
  *    axis        :: an table of axis descriptors..
  *
  */
  typedef struct FT_Multi_Master_
  {
    FT_UInt     num_axis;
    FT_UInt     num_designs;
    FT_MM_Axis  axis[ T1_MAX_MM_AXIS ];
  
  } FT_Multi_Master;


  typedef  FT_Error  (*FT_Get_MM_Func)( FT_Face  face, FT_Multi_Master* master );
  
  typedef  FT_Error  (*FT_Set_MM_Design_Func)( FT_Face   face,
                                               FT_UInt   num_coords,
                                               FT_Long*  coords );

  typedef  FT_Error  (*FT_Set_MM_Blend_Func)( FT_Face   face,
                                              FT_UInt   num_coords,
                                              FT_Long*  coords );

 /*************************************************************************
  *
  *  <Function>
  *     FT_Get_Multi_Master
  *
  *  <Description>
  *     Retrieves the multiple master descriptor of a given font
  *
  *  <Input>
  *     face    :: handle to source face
  *
  *  <Output>
  *     master  :: multiple masters descriptor
  *
  *  <Return>
  *     Error code. 0 means success.
  *
  */
  FT_EXPORT_DEF(FT_Error)   FT_Get_Multi_Master( FT_Face           face,
                                                 FT_Multi_Master*  master );


 /*************************************************************************
  *
  *  <Function>
  *     FT_Set_MM_Design_Coordinates
  *
  *  <Description>
  *     For multiple masters fonts, choose an interpolated font design
  *     through design coordinates
  *
  *  <Input>
  *     face       :: handle to source face
  *     num_coords :: number of design coordinates (must be equal to the
  *                   number of axis in the font).
  *     coords     :: design coordinates
  *
  *  <Return>
  *     Error code. 0 means success.
  *
  */
  FT_EXPORT_DEF(FT_Error)   FT_Set_MM_Design_Coordinates( FT_Face  face,
                                                          FT_UInt  num_coords,
                                                          FT_Long* coords );

 /*************************************************************************
  *
  *  <Function>
  *     FT_Set_MM_Blend_Coordinates
  *
  *  <Description>
  *     For multiple masters fonts, choose an interpolated font design
  *     through normalized blend coordinates
  *
  *  <Input>
  *     face       :: handle to source face
  *     num_coords :: number of design coordinates (must be equal to the
  *                   number of axis in the font).
  *     coords     :: design coordinates (each one must be between 0 and 1.0)
  *
  *  <Return>
  *     Error code. 0 means success.
  *
  */
  FT_EXPORT_DEF(FT_Error)   FT_Set_MM_Blend_Coordinates( FT_Face   face,
                                                         FT_UInt   num_coords,
                                                         FT_Fixed* coords );

#ifdef __cplusplus
  }
#endif

#endif /* FTMM_H */
/* END */
