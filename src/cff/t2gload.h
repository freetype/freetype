/***************************************************************************/
/*                                                                         */
/*  t2gload.h                                                              */
/*                                                                         */
/*    OpenType Glyph Loader (specification).                               */
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
/***************************************************************************/


#ifndef T2GLOAD_H
#define T2GLOAD_H

#include <t2objs.h>

#ifdef __cplusplus
  extern "C" {
#endif

  typedef struct T2_Loader_
  {
    T2_Face         face;
    T2_Size         size;
    T2_GlyphSlot    glyph;

    FT_ULong        load_flags;
    FT_UInt         glyph_index;

    FT_Stream       stream;
    FT_Int          byte_len;
    FT_Int          left_points;
    FT_Int          left_contours;

    FT_BBox         bbox;
    FT_Int          left_bearing;
    FT_Int          advance;
    FT_Bool         preserve_pps;
    FT_Vector       pp1;
    FT_Vector       pp2;

    FT_ULong        glyf_offset;

    /* the zone where we load our glyphs */
    FT_GlyphZone    base;
    FT_GlyphZone    zone;

  } T2_Loader;


#if 0
  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T2_Get_Metrics                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the horizontal or vertical metrics in font units for a     */
  /*    given glyph.  The metrics are the left side bearing (resp. top     */
  /*    side bearing) and advance width (resp. advance height).            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    header  :: A pointer to either the horizontal or vertical metrics  */
  /*               structure.                                              */
  /*                                                                       */
  /*    index   :: The glyph index.                                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    bearing :: The bearing, either left side or top side.              */
  /*                                                                       */
  /*    advance :: The advance width resp. advance height.                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function will much probably move to another component in the  */
  /*    near future, but I haven't decided which yet.                      */
  /*                                                                       */
  LOCAL_DEF
  void  T2_Get_Metrics( TT_HoriHeader*  header,
                        FT_UInt         index,
                        FT_Short*       bearing,
                        FT_UShort*      advance );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T2_Load_Glyph                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to load a single glyph within a given glyph slot,  */
  /*    for a given size.                                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    glyph       :: A handle to a target slot object where the glyph    */
  /*                   will be loaded.                                     */
  /*                                                                       */
  /*    size        :: A handle to the source face size at which the glyph */
  /*                   must be scaled/loaded.                              */
  /*                                                                       */
  /*    glyph_index :: The index of the glyph in the font file.            */
  /*                                                                       */
  /*    load_flags  :: A flag indicating what to load for this glyph.  The */
  /*                   FT_LOAD_XXX constants can be used to control the    */
  /*                   glyph loading process (e.g., whether the outline    */
  /*                   should be scaled, whether to load bitmaps or not,   */
  /*                   whether to hint the outline, etc).                  */
  /* <Output>                                                              */
  /*    result      :: A set of bit flags indicating the type of data that */
  /*                   was loaded in the glyph slot (outline or bitmap,    */
  /*                   etc).                                               */
  /*                                                                       */
  /*                   You can set this field to 0 if you don't want this  */
  /*                   information.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_DEF
  FT_Error  T2_Load_Glyph( T2_Size       size,
                           T2_GlyphSlot  glyph,
                           FT_UShort     glyph_index,
                           FT_UInt       load_flags );
#endif

#ifdef __cplusplus
  }
#endif


#endif /* T2GLOAD_H */


/* END */
