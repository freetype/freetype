/***************************************************************************/
/*                                                                         */
/*  t2gload.h                                                              */
/*                                                                         */
/*    OpenType Glyph Loader (specification).                               */
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


#ifndef T2GLOAD_H
#define T2GLOAD_H

#include <freetype/freetype.h>
#include <t2objs.h>

#ifdef __cplusplus
  extern "C" {
#endif


#define T2_MAX_OPERANDS     48
#define T2_MAX_SUBRS_CALLS  32


  /*************************************************************************/
  /*                                                                       */
  /* <Structure>                                                           */
  /*    T2_Builder                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*     A structure used during glyph loading to store its outline.       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    memory       :: The current memory object.                         */
  /*                                                                       */
  /*    face         :: The current face object.                           */
  /*                                                                       */
  /*    glyph        :: The current glyph slot.                            */
  /*                                                                       */
  /*    current      :: The current glyph outline.                         */
  /*                                                                       */
  /*    base         :: The base glyph outline.                            */
  /*                                                                       */
  /*    max_points   :: maximum points in builder outline                  */
  /*                                                                       */
  /*    max_contours :: Maximal number of contours in builder outline.     */
  /*                                                                       */
  /*    last         :: The last point position.                           */
  /*                                                                       */
  /*    scale_x      :: The horizontal scale (FUnits to sub-pixels).       */
  /*                                                                       */
  /*    scale_y      :: The vertical scale (FUnits to sub-pixels).         */
  /*                                                                       */
  /*    pos_x        :: The horizontal translation (if composite glyph).   */
  /*                                                                       */
  /*    pos_y        :: The vertical translation (if composite glyph).     */
  /*                                                                       */
  /*    left_bearing :: The left side bearing point.                       */
  /*                                                                       */
  /*    advance      :: The horizontal advance vector.                     */
  /*                                                                       */
  /*    bbox         :: Unused.                                            */
  /*                                                                       */
  /*    path_begun   :: A flag which indicates that a new path has begun.  */
  /*                                                                       */
  /*    load_points  :: If this flag is not set, no points are loaded.     */
  /*                                                                       */
  /*    no_recurse   :: Set but not used.                                  */
  /*                                                                       */
  /*    error        :: An error code that is only used to report memory   */
  /*                    allocation problems.                               */
  /*                                                                       */
  /*    metrics_only :: A boolean indicating that we only want to compute  */
  /*                    the metrics of a given glyph, not load all of its  */
  /*                    points.                                            */
  /*                                                                       */
  typedef struct  T2_Builder_
  {
    FT_Memory     memory;
    TT_Face       face;
    T2_GlyphSlot  glyph;

    FT_Outline    current;       /* the current glyph outline   */
    FT_Outline    base;          /* the composite glyph outline */

    TT_Int        max_points;    /* capacity of base outline in points   */
    TT_Int        max_contours;  /* capacity of base outline in contours */

    TT_Vector     last;

    TT_Fixed      scale_x;
    TT_Fixed      scale_y;

    TT_Pos        pos_x;
    TT_Pos        pos_y;

    TT_Vector     left_bearing;
    TT_Vector     advance;

    TT_BBox       bbox;          /* bounding box */
    TT_Bool       path_begun;
    TT_Bool       load_points;
    TT_Bool       no_recurse;

    TT_Error      error;         /* only used for memory errors */
    TT_Bool       metrics_only;

  } T2_Builder;


  /* execution context charstring zone */

  typedef struct  T2_Decoder_Zone_
  {
    TT_Byte*  base;
    TT_Byte*  limit;
    TT_Byte*  cursor;

  } T2_Decoder_Zone;


  typedef struct  T2_Decoder_
  {
    T2_Builder        builder;
    CFF_Font*         cff;

    TT_Fixed          stack[T2_MAX_OPERANDS + 1];
    TT_Fixed*         top;

    T2_Decoder_Zone   zones[T2_MAX_SUBRS_CALLS + 1];
    T2_Decoder_Zone*  zone;

    TT_Int            flex_state;
    TT_Int            num_flex_vectors;
    TT_Vector         flex_vectors[7];

    TT_Pos            glyph_width;
    TT_Pos            nominal_width;

    TT_Bool           read_width;
    TT_Int            num_hints;
    TT_Fixed*         buildchar;
    TT_Int            len_buildchar;

    TT_UInt           num_locals;
    TT_UInt           num_globals;

    TT_Int            locals_bias;
    TT_Int            globals_bias;

    TT_Byte**         locals;
    TT_Byte**         globals;

  } T2_Decoder;


  LOCAL_DEF
  void  T2_Init_Decoder( T2_Decoder*   decoder,
                         TT_Face       face,
                         T2_Size       size,
                         T2_GlyphSlot  slot );


#if 0  /* unused until we support pure CFF fonts */

  /* Compute the maximum advance width of a font through quick parsing */
  LOCAL_DEF
  TT_Error  T2_Compute_Max_Advance( TT_Face  face,
                                    TT_Int*  max_advance );

#endif

  /* This function is exported, because it is used by the T1Dump utility */
  LOCAL_DEF
  TT_Error  T2_Parse_CharStrings( T2_Decoder*  decoder,
                                  TT_Byte*     charstring_base,
                                  TT_Int       charstring_len );

  LOCAL_DEF
  TT_Error  T2_Load_Glyph( T2_GlyphSlot  glyph,
                           T2_Size       size,
                           TT_Int        glyph_index,
                           TT_Int        load_flags );


#ifdef __cplusplus
  }
#endif


#endif /* T2GLOAD_H */


/* END */
