/*******************************************************************
 *
 *  t1gload.h                                                   1.0
 *
 *    Type1 Glyph Loader.
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
 *  - a glyph builder, T1_Builder, used to store the built outline
 *
 *  - a glyph hinter, T1_Hinter, used to record and apply the stem
 *    hints
 *
 *  - a charstrings interpreter, T1_Decoder, used to parse the
 *    Type 1 charstrings stream, manage a stack and call the builder
 *    and/or hinter depending on the opcodes.
 *
 *  Ideally, a Type 2 glyph loader would only need to have its own
 *  T2_Decoder object (assuming the hinter is able to manage all
 *  kinds of hints).
 *
 ******************************************************************/

#ifndef T1GLOAD_H
#define T1GLOAD_H

#include <t1objs.h>

#ifdef __cplusplus
  extern "C" {
#endif


/*************************************************************************/
/*                                                                       */
/* <Structure> T1_Builder                                                */
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

  typedef struct T1_Builder_
  {
    FT_Memory     memory;
    T1_Face       face;
    T1_GlyphSlot  glyph;

    FT_Outline    current;       /* the current glyph outline   */
    FT_Outline    base;          /* the composite glyph outline */

    FT_Int        max_points;    /* capacity of base outline in points   */
    FT_Int        max_contours;  /* capacity of base outline in contours */

    FT_Vector     last;

    FT_Fixed      scale_x;
    FT_Fixed      scale_y;

    FT_Pos        pos_x;
    FT_Pos        pos_y;

    FT_Vector     left_bearing;
    FT_Vector     advance;

    FT_BBox       bbox;          /* bounding box */
    FT_Bool       path_begun;
    FT_Bool       load_points;
    FT_Bool       no_recurse;

    FT_Error      error;         /* only used for memory errors */
    FT_Bool       metrics_only;

  } T1_Builder;


  /* execution context charstring zone */
  typedef struct T1_Decoder_Zone_
  {
    FT_Byte*  base;
    FT_Byte*  limit;
    FT_Byte*  cursor;

  } T1_Decoder_Zone;


  typedef struct T1_Decoder_
  {
    T1_Builder         builder;

    FT_Int             stack[ T1_MAX_CHARSTRINGS_OPERANDS ];
    FT_Int*            top;

    T1_Decoder_Zone    zones[ T1_MAX_SUBRS_CALLS+1 ];
    T1_Decoder_Zone*   zone;

    FT_Int             flex_state;
    FT_Int             num_flex_vectors;
    FT_Vector          flex_vectors[7];

    T1_Blend*          blend;  /* for multiple masters */

  } T1_Decoder;



  LOCAL_DEF
  void  T1_Init_Builder( T1_Builder*             builder,
                         T1_Face                 face,
                         T1_Size                 size,
                         T1_GlyphSlot            glyph );

  LOCAL_DEF
  void T1_Done_Builder( T1_Builder*  builder );


  LOCAL_DEF
  void  T1_Init_Decoder( T1_Decoder* decoder );


  /* Compute the maximum advance width of a font through quick parsing */
  LOCAL_DEF
  FT_Error  T1_Compute_Max_Advance( T1_Face  face,
                                    FT_Int  *max_advance );


  /* This function is exported, because it is used by the T1Dump utility */
  LOCAL_DEF
  FT_Error   T1_Parse_CharStrings( T1_Decoder*  decoder,
                                   FT_Byte*     charstring_base,
                                   FT_Int       charstring_len,
                                   FT_Int       num_subrs,
                                   FT_Byte**    subrs_base,
                                   FT_Int*      subrs_len );



  LOCAL_DEF
  FT_Error  T1_Load_Glyph( T1_GlyphSlot  glyph,
                           T1_Size       size,
                           FT_Int        glyph_index,
                           FT_Int        load_flags );


#ifdef __cplusplus
  }
#endif

#endif /* T1GLOAD_H */
