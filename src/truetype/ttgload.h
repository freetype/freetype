/***************************************************************************/
/*                                                                         */
/*  ttgload.h                                                              */
/*                                                                         */
/*    TrueType Glyph Loader (specification).                               */
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


#ifndef TTGLOAD_H
#define TTGLOAD_H

#include <ttobjs.h>

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#include <ttinterp.h>
#endif

#ifdef __cplusplus
  extern "C" {
#endif

  typedef struct TT_Loader_
  {
    TT_Face         face;
    TT_Size         size;
    TT_GlyphSlot    glyph;

    TT_ULong        load_flags;
    TT_UInt         glyph_index;

    FT_Stream       stream;
    TT_Int          byte_len;
    TT_Int          left_points;
    TT_Int          left_contours;

    TT_BBox         bbox;
    TT_Int          left_bearing;
    TT_Int          advance;
    TT_Bool         preserve_pps;
    TT_Vector       pp1;
    TT_Vector       pp2;
        
    TT_ULong        glyf_offset;
        
    /* the zone where we load our glyphs */
    FT_GlyphZone    base;
    FT_GlyphZone    zone;
    
#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
    TT_ExecContext  exec;
    TT_Byte*        instructions;
#endif

  } TT_Loader;


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Get_Metrics                                                     */
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
  void  TT_Get_Metrics( TT_HoriHeader*  header,
                        TT_UInt         index,
                        TT_Short*       bearing,
                        TT_UShort*      advance );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Glyph                                                      */
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
  TT_Error  TT_Load_Glyph( TT_Size       size,
                           TT_GlyphSlot  glyph,
                           TT_UShort     glyph_index,
                           TT_UInt       load_flags );

#ifdef __cplusplus
  }
#endif


#endif /* TTGLOAD_H */


/* END */
