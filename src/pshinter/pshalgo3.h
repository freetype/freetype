/***************************************************************************/
/*                                                                         */
/*  pshalgo3.h                                                             */
/*                                                                         */
/*    PostScript hinting algorithm 3 (specification).                      */
/*                                                                         */
/*  Copyright 2001, 2002 by                                                */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __PSHALGO3_H__
#define __PSHALGO3_H__


#include "pshrec.h"
#include "pshglob.h"
#include FT_TRIGONOMETRY_H


FT_BEGIN_HEADER


  typedef struct PSH3_HintRec_*  PSH3_Hint;

  typedef enum
  {
    PSH3_HINT_GHOST  = PS_HINT_FLAG_GHOST,
    PSH3_HINT_BOTTOM = PS_HINT_FLAG_BOTTOM,
    PSH3_HINT_ACTIVE = 4,
    PSH3_HINT_FITTED = 8

  } PSH3_Hint_Flags;


#define psh3_hint_is_active( x )  ( ( (x)->flags & PSH3_HINT_ACTIVE ) != 0 )
#define psh3_hint_is_ghost( x )   ( ( (x)->flags & PSH3_HINT_GHOST  ) != 0 )
#define psh3_hint_is_fitted( x )  ( ( (x)->flags & PSH3_HINT_FITTED ) != 0 )

#define psh3_hint_activate( x )    (x)->flags |=  PSH3_HINT_ACTIVE
#define psh3_hint_deactivate( x )  (x)->flags &= ~PSH3_HINT_ACTIVE
#define psh3_hint_set_fitted( x )  (x)->flags |=  PSH3_HINT_FITTED


  typedef struct  PSH3_HintRec_
  {
    FT_Int     org_pos;
    FT_Int     org_len;
    FT_Pos     cur_pos;
    FT_Pos     cur_len;
    FT_UInt    flags;
    PSH3_Hint  parent;
    FT_Int     order;

  } PSH3_HintRec;


  /* this is an interpolation zone used for strong points;  */
  /* weak points are interpolated according to their strong */
  /* neighbours                                             */
  typedef struct  PSH3_ZoneRec_
  {
    FT_Fixed  scale;
    FT_Fixed  delta;
    FT_Pos    min;
    FT_Pos    max;

  } PSH3_ZoneRec, *PSH3_Zone;


  typedef struct  PSH3_Hint_TableRec_
  {
    FT_UInt        max_hints;
    FT_UInt        num_hints;
    PSH3_Hint      hints;
    PSH3_Hint*     sort;
    PSH3_Hint*     sort_global;
    FT_UInt        num_zones;
    PSH3_Zone      zones;
    PSH3_Zone      zone;
    PS_Mask_Table  hint_masks;
    PS_Mask_Table  counter_masks;

  } PSH3_Hint_TableRec, *PSH3_Hint_Table;


  typedef struct PSH3_PointRec_*    PSH3_Point;
  typedef struct PSH3_ContourRec_*  PSH3_Contour;

  enum
  {
    PSH3_DIR_NONE  =  4,
    PSH3_DIR_UP    =  1,
    PSH3_DIR_DOWN  = -1,
    PSH3_DIR_LEFT  = -2,
    PSH3_DIR_RIGHT =  2
  };

  enum
  {
    PSH3_POINT_OFF    = 1,   /* point is off the curve  */
    PSH3_POINT_STRONG = 2,   /* point is strong         */
    PSH3_POINT_SMOOTH = 4,   /* point is smooth         */
    PSH3_POINT_FITTED = 8    /* point is already fitted */
  };


  typedef struct  PSH3_PointRec_
  {
    PSH3_Point    prev;
    PSH3_Point    next;
    PSH3_Contour  contour;
    FT_UInt       flags;
    FT_Char       dir_in;
    FT_Char       dir_out;
    FT_Angle      angle_in;
    FT_Angle      angle_out;
    PSH3_Hint     hint;
    FT_Pos        org_u;
    FT_Pos        cur_u;
#ifdef DEBUG_HINTER
    FT_Pos        org_x;
    FT_Pos        cur_x;
    FT_Pos        org_y;
    FT_Pos        cur_y;
    FT_UInt       flags_x;
    FT_UInt       flags_y;
#endif

  } PSH3_PointRec;


#define psh3_point_is_strong( p )   ( (p)->flags & PSH3_POINT_STRONG )
#define psh3_point_is_fitted( p )   ( (p)->flags & PSH3_POINT_FITTED )
#define psh3_point_is_smooth( p )   ( (p)->flags & PSH3_POINT_SMOOTH )

#define psh3_point_set_strong( p )  (p)->flags |= PSH3_POINT_STRONG
#define psh3_point_set_fitted( p )  (p)->flags |= PSH3_POINT_FITTED
#define psh3_point_set_smooth( p )  (p)->flags |= PSH3_POINT_SMOOTH


  typedef struct  PSH3_ContourRec_
  {
    PSH3_Point  start;
    FT_UInt     count;

  } PSH3_ContourRec;


  typedef struct  PSH3_GlyphRec_
  {
    FT_UInt             num_points;
    FT_UInt             num_contours;

    PSH3_Point          points;
    PSH3_Contour        contours;

    FT_Memory           memory;
    FT_Outline*         outline;
    PSH_Globals         globals;
    PSH3_Hint_TableRec  hint_tables[2];

    FT_Bool             vertical;
    FT_Int              major_dir;
    FT_Int              minor_dir;

  } PSH3_GlyphRec, *PSH3_Glyph;


#ifdef DEBUG_HINTER
  extern PSH3_Hint_Table  ps3_debug_hint_table;

  typedef void
  (*PSH3_HintFunc)( PSH3_Hint  hint,
                    FT_Bool    vertical );

  extern PSH3_HintFunc    ps3_debug_hint_func;

  extern PSH3_Glyph       ps3_debug_glyph;
#endif


  extern FT_Error
  ps3_hints_apply( PS_Hints     ps_hints,
                   FT_Outline*  outline,
                   PSH_Globals  globals,
                   FT_UInt32    hint_flags );


FT_END_HEADER


#endif /* __PSHALGO3_H__ */


/* END */
