/*******************************************************************
 *
 *  t1hinter.h                                                 1.2
 *
 *    Type1 hinter.         
 *
 *  Copyright 1996-1999 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 *
 *  The Hinter is in charge of fitting th scaled outline to the
 *  pixel grid in order to considerably improve the quality of
 *  the Type 1 font driver's output..
 *
 ******************************************************************/

#ifndef T1HINTER_H
#define T1HINTER_H

#include <t1objs.h>
#include <t1gload.h>

#ifdef __cplusplus
  extern "C" {
#endif


/************************************************************************
 *
 * <Struct>
 *   T1_Snap_Zone
 *
 * <Description>
 *   A "snap zone" is used to model either a blue zone or a stem width
 *   at a given character size. It is made of a minimum and maximum
 *   edge, defined in 26.6 pixels, as well as one "original" and
 *   "scaled" position.
 *
 *   the position corresponds to the stem width (for stem snap zones)
 *   or to the blue position (for blue zones)
 *
 * <Fields>
 *   orus  :: original position in font units
 *   pix   :: current position in sub-pixel units
 *   min   :: minimum boundary in sub-pixel units
 *   max   :: maximim boundary in sub-pixel units
 *
 ************************************************************************/

  typedef struct T1_Snap_Zone_
  {
    T1_Pos  orus;
    T1_Pos  pix; 
    T1_Pos  min; 
    T1_Pos  max;  
  
  } T1_Snap_Zone;


/************************************************************************
 *
 * <Struct>
 *   T1_Edge
 *
 * <Description>
 *   A very simply structure used to model an stem edge
 *
 * <Fields>
 *   orus  :: original edge position in font units
 *   pix   :: scaled edge position in sub-pixel units
 *
 ************************************************************************/

  typedef struct T1_Edge_
  {
    T1_Pos  orus;
    T1_Pos  pix;  
  
  } T1_Edge;


/************************************************************************
 *
 * <Struct>
 *    T1_Stem_Hint
 *
 * <Description>
 *    A simple structure used to model a stem hint
 *
 * <Fields>
 *    min_edge   :: the hint's minimum edge
 *    max_edge   :: the hint's maximum edge
 *    hint_flags :: some flags describing the stem properties
 *
 * <Note>
 *    the min and max edges of a ghost stem have the same position,
 *    even if they're coded in a weird way in the charstrings
 *
 ************************************************************************/

  typedef struct T1_Stem_Hint_
  {
    T1_Edge  min_edge;
    T1_Edge  max_edge;
    T1_Int   hint_flags;

  } T1_Stem_Hint;


#define T1_HINT_FLAG_ACTIVE      1    /* indicates an active stem */
#define T1_HINT_FLAG_MIN_BORDER  2    /* unused for now..         */
#define T1_HINT_FLAG_MAX_BORDER  4    /* unused for now..         */


/* hinter's configuration constants */
#define T1_HINTER_MAX_BLUES    24    /* maximum number of blue zones      */
#define T1_HINTER_MAX_SNAPS    16    /* maximum number of stem snap zones */
#define T1_HINTER_MAX_EDGES    64    /* maximum number of stem hints      */


/************************************************************************
 *
 * <Struct>
 *   T1_Size_Hints
 *
 * <Description>
 *   A structure used to model the hinting information related to
 *   a size object
 *
 * <Fields>
 *   supress_overshoots :: a boolean flag to tell when overshoot
 *                         supression should occur.
 *
 *   num_blue_zones     :: the total number of blue zones (top+bottom)
 *   num_bottom_zones   :: the number of bottom zones
 *
 *   blue_zones         :: the blue zones table. bottom zones are
 *                         stored first in the table, followed by
 *                         all top zones
 *
 *   num_stem_snapH     :: number of horizontal stem snap zones
 *   stem_snapH         :: horizontal stem snap zones
 *
 *   num_stem_snapV     :: number of vertical stem snap zones
 *   stem_snapV         :: vertical stem snap zones
 *
 ************************************************************************/

  struct T1_Size_Hints_
  {
    T1_Bool       supress_overshoots;

    T1_Int        num_blue_zones;
    T1_Int        num_bottom_zones;
    T1_Snap_Zone  blue_zones[ T1_HINTER_MAX_BLUES ];

    T1_Int        num_snap_widths;
    T1_Snap_Zone  snap_widths[ T1_HINTER_MAX_SNAPS ];

    T1_Int        num_snap_heights;
    T1_Snap_Zone  snap_heights[ T1_HINTER_MAX_SNAPS ];
  };



/************************************************************************
 *
 * <Struct>
 *    T1_Stem_Table
 *
 * <Description>
 *    A simple structure used to model a set of stem hints in a
 *    single direction during the loading of a given glyph outline.
 *    Not all stem hints are active at a time. Moreover, stems must
 *    be sorted regularly
 *
 * <Fields>
 *    num_stems   :: total number of stems in table
 *    num_active  :: number of active stems in table
 *
 *    stems       :: the table of all stems
 *    sort        :: a table of indices into the stems table, used
 *                   to keep a sorted list of the active stems
 *
 ************************************************************************/

  typedef struct T1_Stem_Table_
  {
    T1_Int        num_stems;
    T1_Int        num_active;

    T1_Stem_Hint  stems[ T1_HINTER_MAX_EDGES ];
    T1_Int        sort [ T1_HINTER_MAX_EDGES ];

  } T1_Stem_Table;



/************************************************************************
 *
 * <Struct>
 *   T1_Glyph_Hints
 *
 * <Description>
 *   A structure used to model the stem hints of a given glyph outline
 *   during glyph loading. 
 *
 * <Fields>
 *   hori_stems  :: horizontal stem hints table
 *   vert_stems  :: vertical stem hints table
 *
 ************************************************************************/

  struct T1_Glyph_Hints_
  {
    T1_Stem_Table  hori_stems;
    T1_Stem_Table  vert_stems;
  };



/************************************************************************
 *
 * <Data>
 *    t1_hinter_funcs
 *
 * <Description>
 *    A table containing the address of various functions used during
 *    the loading of an hinted scaled outline
 *
 ************************************************************************/

  LOCAL_DEF
  const T1_Hinter_Funcs  t1_hinter_funcs;


/************************************************************************
 *
 * <Function>
 *    T1_New_Size_Hinter
 *
 * <Description>
 *    Allocates a new hinter structure for a given size object
 *
 * <Input>
 *    size :: handle to target size object
 *
 * <Return>
 *    Error code. 0 means success
 *
 ************************************************************************/

  LOCAL_DEF
  T1_Error  T1_New_Size_Hinter( T1_Size  size );


/************************************************************************
 *
 * <Function>
 *    T1_Done_Size_Hinter
 *
 * <Description>
 *    Releases a given size object's hinter structure
 *
 * <Input>
 *    size :: handle to target size object
 *
 ************************************************************************/

  LOCAL_DEF
  void      T1_Done_Size_Hinter( T1_Size  size );


/************************************************************************
 *
 * <Function>
 *    T1_Reset_Size_Hinter
 *
 * <Description>
 *    Recomputes hinting information when a given size object has
 *    changed its resolutions/char sizes/pixel sizes
 *
 * <Input>
 *    size :: handle to size object
 *
 * <Return>
 *    Error code. 0 means success
 *
 ************************************************************************/

  LOCAL_DEF
  T1_Error  T1_Reset_Size_Hinter( T1_Size  size );


/************************************************************************
 *
 * <Function>
 *    T1_New_Glyph_Hinter
 *
 * <Description>
 *    Allocates a new hinter structure for a given glyph slot
 *
 * <Input>
 *    glyph :: handle to target glyph slot
 *
 * <Return>
 *    Error code. 0 means success
 *
 ************************************************************************/

  LOCAL_DEF
  T1_Error  T1_New_Glyph_Hinter( T1_GlyphSlot  glyph );


/************************************************************************
 *
 * <Function>
 *    T1_Done_Glyph_Hinter
 *
 * <Description>
 *    Releases a given glyph slot's hinter structure
 *
 * <Input>
 *    glyph :: handle to glyph slot
 *
 ************************************************************************/

  LOCAL_DEF
  void      T1_Done_Glyph_Hinter( T1_GlyphSlot  glyph );




/************************************************************************
 *
 * <Function>
 *   T1_Hint_Points
 *
 * <Description>
 *   this function grid-fits several points in a given Type 1 builder
 *   at once. 
 *
 * <Input>
 *   builder  :: handle to target Type 1 builder
 *
 ************************************************************************/

  LOCAL_DEF
  void  T1_Hint_Points( T1_Builder*  builder );


/************************************************************************
 *
 * <Function>
 *    T1_Hint_Stems
 *
 * <Description>
 *    This function is used to compute the location of each stem hint
 *    between the first and second passes of the glyph loader on the
 *    charstring.
 *
 * <Input>
 *    builder :: handle to target builder
 *
 ************************************************************************/

  LOCAL_DEF
  void  T1_Hint_Stems( T1_Builder*  builder );

#ifdef __cplusplus
  }
#endif

#endif /* T1HINTER_H */
