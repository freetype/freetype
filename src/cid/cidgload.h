/***************************************************************************/
/*                                                                         */
/*  cidgload.h                                                             */
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


#ifndef CIDGLOAD_H
#define CIDGLOAD_H

#include <cidobjs.h>

#ifdef __cplusplus
  extern "C" {
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Structure>                                                           */
  /*    CID_Builder                                                        */
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
  typedef struct  CID_Builder_
  {
    FT_Memory     memory;
    CID_Face      face;
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

  } CID_Builder;


  /* execution context charstring zone */

  typedef struct  CID_Decoder_Zone_
  {
    T1_Byte*  base;
    T1_Byte*  limit;
    T1_Byte*  cursor;

  } CID_Decoder_Zone;


  typedef struct  CID_Decoder_
  {
    CID_Builder        builder;

    T1_Int             stack[T1_MAX_CHARSTRINGS_OPERANDS];
    T1_Int*            top;

    CID_Decoder_Zone   zones[T1_MAX_SUBRS_CALLS + 1];
    CID_Decoder_Zone*  zone;

    T1_Matrix          font_matrix;
    CID_Subrs*         subrs;
    T1_UInt            lenIV;

    T1_Int             flex_state;
    T1_Int             num_flex_vectors;
    T1_Vector          flex_vectors[7];

  } CID_Decoder;


  LOCAL_DEF
  void  CID_Init_Builder( CID_Builder*  builder,
                          CID_Face      face,
                          T1_Size       size,
                          T1_GlyphSlot  glyph );

  LOCAL_DEF
  void CID_Done_Builder( CID_Builder*  builder );


  LOCAL_DEF
  void CID_Init_Decoder( CID_Decoder*  decoder );


#if 0

  /* Compute the maximum advance width of a font through quick parsing */
  LOCAL_DEF
  T1_Error  CID_Compute_Max_Advance( CID_Face  face,
                                     T1_Int*   max_advance );

#endif

  /* This function is exported, because it is used by the T1Dump utility */
  LOCAL_DEF
  T1_Error  CID_Parse_CharStrings( CID_Decoder*  decoder,
                                   T1_Byte*      charstring_base,
                                   T1_Int        charstring_len );

  LOCAL_DEF
  T1_Error  CID_Load_Glyph( T1_GlyphSlot  glyph,
                            T1_Size       size,
                            T1_Int        glyph_index,
                            T1_Int        load_flags );


#ifdef __cplusplus
  }
#endif


#endif /* CIDGLOAD_H */


/* END */
