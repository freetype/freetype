/*******************************************************************
 *
 *  cidgload.h                                                   1.0
 *
 *    CID-Keyed Type1 Glyph Loader.
 *
 *  Copyright 1996-1998 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *
 *  The Type 1 glyph loader uses three distinct objects to build
 *  scaled and hinted outlines from a charstrings program. These are :
 *
 *  - a glyph builder, CID_Builder, used to store the built outline
 *
 *  - a glyph hinter, T1_Hinter, used to record and apply the stem
 *    hints
 *
 *  - a charstrings interpreter, CID_Decoder, used to parse the
 *    Type 1 charstrings stream, manage a stack and call the builder
 *    and/or hinter depending on the opcodes.
 *
 *  Ideally, a Type 2 glyph loader would only need to have its own
 *  T2_Decoder object (assuming the hinter is able to manage all
 *  kinds of hints).
 *
 ******************************************************************/

#ifndef CIDGLOAD_H
#define CIDGLOAD_H

#include <cidobjs.h>

#ifdef __cplusplus
  extern "C" {
#endif


/*************************************************************************/
/*                                                                       */
/* <Structure> CID_Builder                                                */
/*                                                                       */
/* <Description>                                                         */
/*     a structure used during glyph loading to store its outline.       */
/*                                                                       */
/* <Fields>                                                              */
/*    system :: current system object                                    */
/*    face   :: current face object                                      */
/*    glyph  :: current glyph slot                                       */
/*                                                                       */
/*    current :: current glyph outline                                   */
/*    base    :: base glyph outline                                      */
/*                                                                       */
/*    max_points   :: maximum points in builder outline                  */
/*    max_contours :: maximum contours in builder outline                */
/*                                                                       */
/*    last     :: last point position                                    */
/*                                                                       */
/*    scale_x  :: horizontal scale ( FUnits to sub-pixels )              */
/*    scale_y  :: vertical scale   ( FUnits to sub-pixels )              */
/*    pos_x    :: horizontal translation (composite glyphs)              */
/*    pos_y    :: vertical translation   (composite glyph)               */
/*                                                                       */
/*    left_bearing  :: left side bearing point                           */
/*    advance       :: horizontal advance vector                         */
/*                                                                       */
/*    path_begun    :: flag, indicates that a new path has begun         */
/*    load_points   :: flag, if not set, no points are loaded            */
/*                                                                       */
/*    error         :: an error code that is only used to report         */
/*                     memory allocation problems..                      */
/*                                                                       */
/*    metrics_only  :: a boolean indicating that we only want to         */
/*                     compute the metrics of a given glyph, not load    */
/*                     all of its points..                               */
/*                                                                       */

  typedef struct CID_Builder_
  {
    FT_Memory     memory;
    CID_Face      face;
    T1_GlyphSlot  glyph;

    FT_Outline    current;       /* the current glyph outline   */
    FT_Outline    base;          /* the composite glyph outline */

    FT_Int        max_points;    /* capacity of base outline in points   */
    FT_Int        max_contours;  /* capacity of base outline in contours */

    T1_Vector     last;

    T1_Fixed      scale_x;
    T1_Fixed      scale_y;

    T1_Pos        pos_x;
    T1_Pos        pos_y;

    T1_Vector     left_bearing;
    T1_Vector     advance;

    T1_BBox       bbox;          /* bounding box */
    T1_Bool       path_begun;
    T1_Bool       load_points;
    T1_Bool       no_recurse;

    FT_Error      error;         /* only used for memory errors */
    T1_Bool       metrics_only;

  } CID_Builder;


  /* execution context charstring zone */
  typedef struct CID_Decoder_Zone_
  {
    FT_Byte*  base;
    FT_Byte*  limit;
    FT_Byte*  cursor;

  } CID_Decoder_Zone;


  typedef struct CID_Decoder_
  {
    CID_Builder        builder;

    FT_Int             stack[ T1_MAX_CHARSTRINGS_OPERANDS ];
    FT_Int*            top;

    CID_Decoder_Zone   zones[ T1_MAX_SUBRS_CALLS+1 ];
    CID_Decoder_Zone*  zone;

    FT_Matrix          font_matrix;
    CID_Subrs*         subrs;
    FT_UInt            lenIV;

    FT_Int             flex_state;
    FT_Int             num_flex_vectors;
    FT_Vector          flex_vectors[7];

  } CID_Decoder;



  LOCAL_DEF
  void  CID_Init_Builder( CID_Builder*             builder,
                          CID_Face                 face,
                          T1_Size                  size,
                          T1_GlyphSlot             glyph );

  LOCAL_DEF
  void CID_Done_Builder( CID_Builder*  builder );


  LOCAL_DEF
  void  CID_Init_Decoder( CID_Decoder* decoder );


#if 0
  /* Compute the maximum advance width of a font through quick parsing */
  LOCAL_DEF
  FT_Error  CID_Compute_Max_Advance( CID_Face  face,
                                     FT_Int   *max_advance );
#endif

  /* This function is exported, because it is used by the T1Dump utility */
  LOCAL_DEF
  FT_Error   CID_Parse_CharStrings( CID_Decoder*  decoder,
                                    FT_Byte*      charstring_base,
                                    FT_Int        charstring_len );

  LOCAL_DEF
  FT_Error  CID_Load_Glyph( T1_GlyphSlot  glyph,
                            T1_Size       size,
                            FT_Int        glyph_index,
                            FT_Int        load_flags );


#ifdef __cplusplus
  }
#endif

#endif /* T1GLOAD_H */
