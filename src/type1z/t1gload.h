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

    T1_Int        max_points;    /* capacity of base outline in points   */
    T1_Int        max_contours;  /* capacity of base outline in contours */

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
    
    T1_Error      error;         /* only used for memory errors */
    T1_Bool       metrics_only;
    
  } T1_Builder;


  /* execution context charstring zone */
  typedef struct T1_Decoder_Zone_
  {
    T1_Byte*  base;
    T1_Byte*  limit;
    T1_Byte*  cursor;

  } T1_Decoder_Zone;


  typedef struct T1_Decoder_
  {
    T1_Builder         builder;

    T1_Int             stack[ T1_MAX_CHARSTRINGS_OPERANDS ];
    T1_Int*            top;

    T1_Decoder_Zone    zones[ T1_MAX_SUBRS_CALLS+1 ];
    T1_Decoder_Zone*   zone;

    T1_Int             flex_state;
    T1_Int             num_flex_vectors;
    T1_Vector          flex_vectors[7];

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
  T1_Error  T1_Compute_Max_Advance( T1_Face  face,
                                    T1_Int  *max_advance );


  /* This function is exported, because it is used by the T1Dump utility */
  LOCAL_DEF
  T1_Error   T1_Parse_CharStrings( T1_Decoder*  decoder,
                                   T1_Byte*     charstring_base,
                                   T1_Int       charstring_len,
                                   T1_Int       num_subrs,
                                   T1_Byte**    subrs_base,
                                   T1_Int*      subrs_len );



  LOCAL_DEF
  T1_Error  T1_Load_Glyph( T1_GlyphSlot  glyph,
                           T1_Size       size,
                           T1_Int        glyph_index,
                           T1_Int        load_flags );


#ifdef __cplusplus
  }
#endif

#endif /* T1GLOAD_H */
