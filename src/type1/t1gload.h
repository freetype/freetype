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
/* <Structure> T1_Builder_Funcs                                          */
/*                                                                       */
/* <Description>                                                         */
/*     a structure used to store the address of various functions        */
/*     used by a glyph builder to implement the outline's "path          */
/*     construction".                                                    */
/*                                                                       */
/*                                                                       */
  typedef struct T1_Builder_  T1_Builder;

  typedef FT_Error  (*T1_Builder_EndChar)( T1_Builder*  loader );

  typedef FT_Error  (*T1_Builder_Sbw)    ( T1_Builder*  loader,
                                           FT_Pos       sbx,
                                           FT_Pos       sby,
                                           FT_Pos       wx,
                                           FT_Pos       wy );

  typedef FT_Error  (*T1_Builder_ClosePath)( T1_Builder*  loader );

  typedef FT_Error  (*T1_Builder_RLineTo)( T1_Builder*  loader,
                                           FT_Pos       dx,
                                           FT_Pos       dy );

  typedef FT_Error  (*T1_Builder_RMoveTo)( T1_Builder*  loader,
                                           FT_Pos       dx,
                                           FT_Pos       dy );

  typedef FT_Error  (*T1_Builder_RCurveTo)( T1_Builder*  loader,
                                            FT_Pos       dx1,
                                            FT_Pos       dy1,
                                            FT_Pos       dx2,
                                            FT_Pos       dy2,
                                            FT_Pos       dx3,
                                            FT_Pos       dy3 );

  typedef struct T1_Builder_Funcs_
  {
    T1_Builder_EndChar    end_char;
    T1_Builder_Sbw        set_bearing_point;
    T1_Builder_ClosePath  close_path;
    T1_Builder_RLineTo    rline_to;
    T1_Builder_RMoveTo    rmove_to;
    T1_Builder_RCurveTo   rcurve_to;

  } T1_Builder_Funcs;



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
/*    size   :: current size object                                      */
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
/*    pass     :: pass number for multi-pass hinters                     */
/*                                                                       */
/*    funcs    :: table of builder functions used to perform             */
/*                the outline's path construction                        */
/*                                                                       */
/*    hint_point :: index of next point to hint..                        */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*                                                                       */

  struct T1_Builder_
  {
    FT_Memory     memory;
    T1_Face       face;
    T1_Size       size;
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
    FT_Bool       no_recurse;

    FT_BBox       bbox;          /* bounding box */
    FT_Bool       path_begun;
    FT_Bool       load_points;

    FT_Int        pass;
    FT_Int        hint_point;

    /* path construction function interface */
    T1_Builder_Funcs  funcs;
  };


/*************************************************************************/
/*                                                                       */
/* <Structure> T1_Hinter_Funcs                                           */
/*                                                                       */
/* <Description>                                                         */
/*     a structure used to store the address of various functions        */
/*     used by a Type 1 hinter to perform outline hinting.               */
/*                                                                       */

  typedef FT_Error  (*T1_Hinter_ChangeHints)( T1_Builder*  builder );

  typedef FT_Error  (*T1_Hinter_DotSection)( T1_Builder*  builder );

  typedef FT_Error  (*T1_Hinter_Stem)( T1_Builder*  builder,
                                       FT_Pos       pos,
                                       FT_Pos       width,
                                       FT_Bool      vertical );


  typedef FT_Error  (*T1_Hinter_Stem3)( T1_Builder*  builder,
                                        FT_Pos       pos0,
                                        FT_Pos       width0,
                                        FT_Pos       pos1,
                                        FT_Pos       width1,
                                        FT_Pos       pos2,
                                        FT_Pos       width2,
                                        FT_Bool      vertical );

  typedef struct T1_Hinter_Func_
  {
    T1_Hinter_ChangeHints     change_hints;
    T1_Hinter_DotSection      dot_section;
    T1_Hinter_Stem            stem;
    T1_Hinter_Stem3           stem3;

  } T1_Hinter_Funcs;



  typedef enum T1_Operator_
  {
    op_none = 0,
    op_endchar,
    op_hsbw,
    op_seac,
    op_sbw,
    op_closepath,
    op_hlineto,
    op_hmoveto,
    op_hvcurveto,
    op_rlineto,
    op_rmoveto,
    op_rrcurveto,
    op_vhcurveto,
    op_vlineto,
    op_vmoveto,
    op_dotsection,
    op_hstem,
    op_hstem3,
    op_vstem,
    op_vstem3,
    op_div,
    op_callothersubr,
    op_callsubr,
    op_pop,
    op_return,
    op_setcurrentpoint,

    op_max    /* never remove this one */

  } T1_Operator;




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
    T1_Hinter_Funcs    hinter;

    FT_Int             stack[ T1_MAX_CHARSTRINGS_OPERANDS ];
    FT_Int*            top;

    T1_Decoder_Zone    zones[ T1_MAX_SUBRS_CALLS+1 ];
    T1_Decoder_Zone*   zone;

    FT_Int             flex_state;
    FT_Int             num_flex_vectors;
    FT_Vector          flex_vectors[7];

  } T1_Decoder;



/*********************************************************************
 *
 * <Function>
 *    T1_Init_Builder
 *
 * <Description>
 *    Initialise a given glyph builder.
 *
 * <Input>
 *    builder :: glyph builder to initialise
 *    face    :: current face object
 *    size    :: current size object
 *    glyph   :: current glyph object
 *    funcs   :: glyph builder functions (or "methods").
 *
 * <Note>
 *    This function is exported for now because it is used by the
 *    "t1dump" utility. Later, it will be accessed through a
 *    format-specific extension
 *
 *********************************************************************/

  LOCAL_DEF
  void  T1_Init_Builder( T1_Builder*             builder,
                         T1_Face                 face,
                         T1_Size                 size,
                         T1_GlyphSlot            glyph,
                         const T1_Builder_Funcs* funcs );

/*********************************************************************
 *
 * <Function>
 *    T1_Done_Builder
 *
 * <Description>
 *    Finalise a given glyph builder. Its content can still be
 *    used after the call, but the function saves important information
 *    within the corresponding glyph slot.
 *
 * <Input>
 *    builder :: glyph builder to initialise
 *
 * <Note>
 *    This function is exported for now because it is used by the
 *    "t1dump" utility. Later, it will be accessed through a
 *    format-specific extension
 *
 *********************************************************************/

  LOCAL_DEF
  void T1_Done_Builder( T1_Builder*  builder );


/*********************************************************************
 *
 * <Function>
 *    T1_Init_Decoder
 *
 * <Description>
 *    Initialise a given Type 1 decoder for parsing
 *
 * <Input>
 *    decoder :: Type 1 decoder to initialise
 *    funcs   :: hinter functions interface
 *
 * <Note>
 *    This function is exported for now because it is used by the
 *    "t1dump" utility. Later, it will be accessed through a
 *    format-specific extension
 *
 *********************************************************************/

  LOCAL_DEF
  void  T1_Init_Decoder( T1_Decoder*             decoder,
                         const T1_Hinter_Funcs*  funcs );



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



/*************************************************************************/
/*                                                                       */
/* <Function> T1_Add_Points                                              */
/*                                                                       */
/* <Description>                                                         */
/*    Checks that there is enough room in the current load glyph outline */
/*    to accept "num_points" additional outline points. If not, this     */
/*    function grows the load outline's arrays accordingly..             */
/*                                                                       */
/* <Input>                                                               */
/*    builder    :: pointer to glyph builder object                      */
/*    num_points :: number of points that will be added later            */
/*                                                                       */
/* <Return>                                                              */
/*    Type1 error code. 0 means success                                  */
/*                                                                       */
/* <Note>                                                                */
/*    This function does NOT update the points count in the glyph loader */
/*    This must be done by the caller itself, after this function is     */
/*    invoked..                                                          */
/*                                                                       */
  LOCAL_DEF
  FT_Error  T1_Add_Points( T1_Builder*  builder,
                           FT_Int       num_points );

/*************************************************************************/
/*                                                                       */
/* <Function> T1_Add_Contours                                            */
/*                                                                       */
/* <Description>                                                         */
/*    Checks that there is enough room in the current load glyph outline */
/*    to accept "num_contours" additional contours. If not, this func    */
/*    the load outline's arrays accordingly..                            */
/*                                                                       */
/* <Input>                                                               */
/*    builder      :: pointer to glyph builder object                    */
/*    num_contours :: number of contours that will be added later        */
/*                                                                       */
/* <Return>                                                              */
/*    Type1 error code. 0 means success                                  */
/*                                                                       */
/* <Note>                                                                */
/*    This function does NOT update the contours count in the load glyph */
/*    This must be done by the caller itself, after this function is     */
/*    invoked..                                                          */
/*                                                                       */
  LOCAL_DEF
  FT_Error  T1_Add_Contours( T1_Builder*  builder,
                             FT_Int       num_contours );


  LOCAL_DEF
  FT_Error  T1_Load_Glyph( T1_GlyphSlot  glyph,
                           T1_Size       size,
                           FT_Int        glyph_index,
                           FT_Int        load_flags );


#ifdef __cplusplus
  }
#endif

#endif /* T1GLOAD_H */
