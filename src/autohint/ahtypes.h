/***************************************************************************/
/*                                                                         */
/*  ahtypes.h                                                              */
/*                                                                         */
/*  General types and definitions for the auto-hint module                 */
/*                                                                         */
/*  Copyright 2000: Catharon Productions Inc.                              */
/*  Author: David Turner                                                   */
/*                                                                         */
/*  This file is part of the Catharon Typography Project and shall only    */
/*  be used, modified, and distributed under the terms of the Catharon     */
/*  Open Source License that should come with this file under the name     */
/*  "CatharonLicense.txt". By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*  Note that this license is compatible with the FreeType license         */
/*                                                                         */
/***************************************************************************/

#ifndef AGTYPES_H
#define AGTYPES_H

#include <freetype/internal/ftobjs.h>  /* for freetype.h + LOCAL_DEF etc.. */

#ifdef FT_FLAT_COMPILE
#include "ahloader.h"  /* glyph loader types & declarations */
#else
#include <autohint/ahloader.h>  /* glyph loader types & declarations */
#endif

#define xxDEBUG_AG

#ifdef DEBUG_AG
#include <stdio.h>
#define AH_LOG(x)  printf##x
#else
#define AH_LOG(x)  /* nothing */
#endif


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/****                                                                   ****/
/****   COMPILE-TIME BUILD OPTIONS                                      ****/
/****                                                                   ****/
/****   Toggle these configuration macros to experiment with            ****/
/****   "features" of the auto-hinter..                                 ****/
/****                                                                   ****/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/* if this option is defined, only strong interpolation will be used to    */
/* place the points between edges. Otherwise, "smooth" points are detected */
/* and later hinted through weak interpolation to correct some unpleasant  */
/* artefacts..                                                             */
/*                                                                         */
#undef  AH_OPTION_NO_WEAK_INTERPOLATION
#undef  AH_OPTION_NO_STRONG_INTERPOLATION

/* undefine this macro if you don't want to hint the metrics */
/* there is no reason to do this, except for experimentation */
#define AH_HINT_METRICS

/* define this macro if you do not want to insert extra edges at a glyph's */
/* x and y extrema (when there isn't one already available). This help     */
/* reduce a number of artefacts and allow hinting of metrics..             */
/*                                                                         */
#undef  AH_OPTION_NO_EXTREMUM_EDGES

/* don't touch for now.. */
#define AH_MAX_WIDTHS   12
#define AH_MAX_HEIGHTS  12

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/****                                                                   ****/
/****   TYPES DEFINITIONS                                               ****/
/****                                                                   ****/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

 /* see agangles.h */
  typedef FT_Int   AH_Angle;


 /* hint flags */
  typedef enum AH_Flags_
  {
    ah_flah_none     = 0,

    /* bezier control points flags */
    ah_flah_conic    = 1,
    ah_flah_cubic    = 2,
    ah_flah_control  = ah_flah_conic | ah_flah_cubic,

    /* extrema flags */
    ah_flah_extrema_x = 4,
    ah_flah_extrema_y = 8,

    /* roundness */
    ah_flah_round_x   = 16,
    ah_flah_round_y   = 32,

    /* touched */
    ah_flah_touch_x   = 64,
    ah_flah_touch_y   = 128,

    /* weak interpolation */
    ah_flah_weak_interpolation = 256,

    /* never remove this one !! */
    ah_flah_max

  } AH_Flags;


 /* edge hint flags */
  typedef enum AH_Edge_Flags_
  {
    ah_edge_normal = 0,
    ah_edge_round  = 1,
    ah_edge_serif  = 2,
    ah_edge_done   = 4

  } AH_Edge_Flags;


 /* hint directions - the values are computed so that two vectors are */
 /* in opposite directions iff "dir1+dir2 == 0"                       */
  typedef enum AH_Direction_
  {
    ah_dir_none  =  4,
    ah_dir_right =  1,
    ah_dir_left  = -1,
    ah_dir_up_and_down = 0,
    ah_dir_left_and_right = 0,
    ah_dir_up    =  2,
    ah_dir_down  = -2

  } AH_Direction;


  typedef struct AH_Point    AH_Point;
  typedef struct AH_Segment  AH_Segment;
  typedef struct AH_Edge     AH_Edge;

 /***************************************************************************
  *
  * <Struct>
  *    AH_Point
  *
  * <Description>
  *    A structure used to model an outline point to the AH_Outline type
  *
  * <Fields>
  *    flags   :: current point hint flags
  *    ox, oy  :: current original scaled coordinates
  *    fx, fy  :: current coordinates in font units
  *    x,  y   :: current hinter coordinates
  *    u, v    :: point coordinates - meaning varies with context
  *
  *    in_dir  :: direction of inwards  vector (prev->point)
  *    out_dir :: direction of outwards vector (point->next)
  *
  *    in_angle  :: angle of inwards vector
  *    out_angle :: angle of outwards vector
  *
  *    next    :: next point in same contour
  *    prev    :: previous point in same contour
  *
  */
  struct AH_Point
  {
    AH_Flags      flags;   /* point flags used by hinter         */
    FT_Pos        ox, oy;
    FT_Pos        fx, fy;
    FT_Pos        x,  y;
    FT_Pos        u,  v;

    AH_Direction  in_dir;   /* direction of inwards vector  */
    AH_Direction  out_dir;  /* direction of outwards vector */

    AH_Angle      in_angle;
    AH_Angle      out_angle;

    AH_Point*     next;    /* next point in contour      */
    AH_Point*     prev;    /* previous point in contour  */
  };


 /***************************************************************************
  *
  * <Struct>
  *    AH_Segment
  *
  * <Description>
  *    a structure used to describe an edge segment to the auto-hinter. A
  *    segment is simply a sequence of successive points located on the same
  *    horizontal or vertical "position", in a given direction.
  *
  * <Fields>
  *    flags      :: segment edge flags ( straight, rounded.. )
  *    dir        :: segment direction
  *
  *    first      :: first point in segment
  *    last       :: last point in segment
  *    contour    :: ptr to first point of segment's contour
  *
  *    pos        :: segment position in font units
  *    size       :: segment size
  *
  *    edge       :: edge of current segment
  *    edge_next  :: next segment on same edge
  *
  *    link       :: the pairing segment for this edge
  *    serif      :: the primary segment for serifs
  *    num_linked :: the number of other segments that link to this one
  *
  *    score      :: used to score the segment when selecting them..
  *
  */
  struct AH_Segment
  {
    AH_Edge_Flags  flags;
    AH_Direction   dir;

    AH_Point*      first;     /* first point in edge segment      */
    AH_Point*      last;      /* last point in edge segment       */
    AH_Point**     contour;   /* ptr to first point of segment's contour */

    FT_Pos         pos;       /* position of segment */
    FT_Pos         min_coord; /* minimum coordinate of segment */
    FT_Pos         max_coord; /* maximum coordinate of segment */

    AH_Edge*       edge;
    AH_Segment*    edge_next;

    AH_Segment*    link;         /* link segment               */
    AH_Segment*    serif;        /* primary segment for serifs */
    FT_Pos         num_linked;   /* number of linked segments  */
    FT_Int         score;
  };


 /***************************************************************************
  *
  * <Struct>
  *    AH_Edge
  *
  * <Description>
  *    a structure used to describe an edge, which really is a horizontal
  *    or vertical coordinate which will be hinted depending on the segments
  *    located on it..
  *
  * <Fields>
  *    flags      :: segment edge flags ( straight, rounded.. )
  *    dir        :: main segment direction on this edge
  *
  *    first      :: first edge segment
  *    last       :: last edge segment
  *
  *    fpos       :: original edge position in font units
  *    opos       :: original scaled edge position
  *    pos        :: hinted edge position
  *
  *    link       :: the linked edge
  *    serif      :: the serif edge
  *    num_paired :: the number of other edges that pair to this one
  *
  *    score      :: used to score the edge when selecting them..
  *
  *    blue_edge  :: indicate the blue zone edge this edge is related to
  *                  only set for some of the horizontal edges in a Latin
  *                  font..
  *
  ***************************************************************************/
  struct AH_Edge
  {
    AH_Edge_Flags   flags;
    AH_Direction    dir;

    AH_Segment*     first;
    AH_Segment*     last;

    FT_Pos          fpos;
    FT_Pos          opos;
    FT_Pos          pos;

    AH_Edge*        link;
    AH_Edge*        serif;
    FT_Int          num_linked;

    FT_Int          score;
    FT_Pos*         blue_edge;
  };


 /* an outline as seen by the hinter */
  typedef struct AH_Outline_
  {
    FT_Memory    memory;

    AH_Direction vert_major_dir;   /* vertical major direction   */
    AH_Direction horz_major_dir;   /* horizontal major direction */

    FT_Fixed     x_scale;
    FT_Fixed     y_scale;
    FT_Pos       edge_distance_threshold;

    FT_Int       max_points;
    FT_Int       num_points;
    AH_Point*    points;

    FT_Int       max_contours;
    FT_Int       num_contours;
    AH_Point**   contours;

    FT_Int       num_hedges;
    AH_Edge*     horz_edges;

    FT_Int       num_vedges;
    AH_Edge*     vert_edges;

    FT_Int       num_hsegments;
    AH_Segment*  horz_segments;

    FT_Int       num_vsegments;
    AH_Segment*  vert_segments;

  } AH_Outline;



  typedef enum AH_Blue_
  {
    ah_blue_capital_top,     /* THEZOCQS */
    ah_blue_capital_bottom,  /* HEZLOCUS */
    ah_blue_small_top,       /* xzroesc  */
    ah_blue_small_bottom,    /* xzroesc  */
    ah_blue_small_minor,     /* pqgjy    */

    ah_blue_max

  } AH_Blue;

  typedef enum
  {
    ah_hinter_monochrome = 1,
    ah_hinter_optimize   = 2

  } AH_Hinter_Flags;


 /************************************************************************
  *
  *  <Struct>
  *     AH_Globals
  *
  *  <Description>
  *     Holds the global metrics for a given font face (be it in design
  *     units, or scaled pixel values)..
  *
  *  <Fields>
  *     num_widths  :: number of widths
  *     num_heights :: number of heights
  *     widths      :: snap widths, including standard one
  *     heights     :: snap height, including standard one
  *     blue_refs   :: reference position of blue zones
  *     blue_shoots :: overshoot position of blue zones
  *
  ************************************************************************/

  typedef struct AH_Globals_
  {
    FT_Int    num_widths;
    FT_Int    num_heights;

    FT_Pos    widths [ AH_MAX_WIDTHS ];
    FT_Pos    heights[ AH_MAX_HEIGHTS ];

    FT_Pos    blue_refs  [ ah_blue_max ];
    FT_Pos    blue_shoots[ ah_blue_max ];

  } AH_Globals;


 /************************************************************************
  *
  *  <Struct>
  *     AH_Face_Globals
  *
  *  <Description>
  *     Holds the complete global metrics for a given font face (i.e. the
  *     design units version + a scaled version + the current scales used)
  *
  *  <Fields>
  *     face     :: handle to source face object
  *     design   :: globals in font design units
  *     scaled   :: scaled globals in sub-pixel values
  *     x_scale  :: current horizontal scale
  *     y_scale  :: current vertical scale
  *
  ************************************************************************/

  typedef struct AH_Face_Globals_
  {
    FT_Face     face;
    AH_Globals  design;
    AH_Globals  scaled;
    FT_Fixed    x_scale;
    FT_Fixed    y_scale;
    FT_Bool     control_overshoot;

  } AH_Face_Globals;




  typedef struct AH_Hinter
  {
    FT_Memory         memory;
    FT_Long           flags;

    FT_Int            algorithm;
    FT_Face           face;

    AH_Face_Globals*  globals;

    AH_Outline*       glyph;

    AH_Loader*        loader;
    FT_Vector         pp1;
    FT_Vector         pp2;

  } AH_Hinter;

#endif /* AGTYPES_H */
