#ifndef __AFHINTS_H__
#define __AFHINTS_H__

#include "aftypes.h"

FT_BEGIN_HEADER

 /*
  *  The definition of outline hints. These are shared by all
  *  script analysis routines
  *
  */

  typedef enum
  {
    AF_DIMENSION_HORZ = 0,  /* x coordinates, i.e. vertical segments & edges   */
    AF_DIMENSION_VERT = 1,  /* y coordinates, i.e. horizontal segments & edges */

    AF_DIMENSION_MAX  /* do not remove */
  
  } AF_Dimension;


  /* hint directions -- the values are computed so that two vectors are */
  /* in opposite directions iff `dir1+dir2 == 0'                        */
  typedef enum
  {
    AF_DIR_NONE  =  4,
    AF_DIR_RIGHT =  1,
    AF_DIR_LEFT  = -1,
    AF_DIR_UP    =  2,
    AF_DIR_DOWN  = -2
    
  } AF_Direction;


  /* point hint flags */
  typedef enum
  {
    AF_FLAG_NONE    = 0,
    
   /* point type flags */
    AF_FLAG_CONIC   = (1 << 0),
    AF_FLAG_CUBIC   = (1 << 1),
    AF_FLAG_CONTROL = AF_FLAG_CONIC | AF_FLAG_CUBIC,
    
   /* point extremum flags */
    AF_FLAG_EXTREMA_X = (1 << 2),
    AF_FLAG_EXTREMA_Y = (1 << 3),
    
   /* point roundness flags */
    AF_FLAG_ROUND_X = (1 << 4),
    AF_FLAG_ROUND_Y = (1 << 5),
    
   /* point touch flags */
    AF_FLAG_TOUCH_X = (1 << 6),
    AF_FLAG_TOUCH_Y = (1 << 7),
    
   /* candidates for weak interpolation have this flag set */
    AF_FLAG_WEAK_INTERPOLATION = (1 << 8),
    
   /* all inflection points in the outline have this flag set */
    AF_FLAG_INFLECTION         = (1 << 9)
  
  } AF_Flags;
  

  /* edge hint flags */
  typedef enum
  {
    AF_EDGE_NORMAL = 0,
    AF_EDGE_ROUND  = (1 << 0),
    AF_EDGE_SERIF  = (1 << 1),
    AF_EDGE_DONE   = (1 << 2)
  
  } AF_Edge_Flags;



  typedef struct AF_PointRec_*    AF_Point;
  typedef struct AF_SegmentRec_*  AF_Segment;
  typedef struct AF_EdgeRec_*     AF_Edge;


  typedef struct  AF_PointRec_
  {
    AF_Flags      flags;    /* point flags used by hinter */
    FT_Pos        ox, oy;   /* original, scaled position  */
    FT_Pos        fx, fy;   /* original, unscaled position (font units) */
    FT_Pos        x,  y;    /* current position */
    FT_Pos        u,  v;    /* current (x,y) or (y,x) depending on context */

    AF_Direction  in_dir;   /* direction of inwards vector  */
    AF_Direction  out_dir;  /* direction of outwards vector */

    AF_Point      next;     /* next point in contour     */
    AF_Point      prev;     /* previous point in contour */

  } AF_PointRec;


  typedef struct  AF_SegmentRec_
  {
    AF_Edge_Flags  flags;       /* edge/segment flags for this segment */
    AF_Direction   dir;         /* segment direction                   */
    FT_Pos         pos;         /* position of segment                 */
    FT_Pos         min_coord;   /* minimum coordinate of segment       */
    FT_Pos         max_coord;   /* maximum coordinate of segment       */

    AF_Edge        edge;        /* the segment's parent edge */
    AF_Segment     edge_next;   /* link to next segment in parent edge */

    AF_Segment     link;        /* link segment               */
    AF_Segment     serif;       /* primary segment for serifs */
    FT_Pos         num_linked;  /* number of linked segments  */
    FT_Pos         score;

    AF_Point       first;       /* first point in edge segment             */
    AF_Point       last;        /* last point in edge segment              */
    AF_Point*      contour;     /* ptr to first point of segment's contour */

  } AF_SegmentRec;


  typedef struct  AF_EdgeRec_
  {
    FT_Pos         fpos;       /* original, unscaled position (font units) */
    FT_Pos         opos;       /* original, scaled position                */
    FT_Pos         pos;        /* current position                         */

    AF_Edge_Flags  flags;      /* edge flags */
    AF_Direction   dir;        /* edge direction */
    FT_Fixed       scale;      /* used to speed up interpolation between edges */
    FT_Pos*        blue_edge;  /* non-NULL if this is a blue edge              */

    AF_Edge        link;
    AF_Edge        serif;
    FT_Int         num_linked;

    FT_Int         score;

    AF_Segment     first;
    AF_Segment     last;


  } AF_EdgeRec;


  typedef struct AF_AxisHintsRec_
  {
    FT_Int        num_segments;
    AF_Segment    segments;
  
    FT_Int        num_edges;
    AF_Edge       edges;

    AF_Direction  major_dir;

  } AF_AxisHintsRec, *AF_AxisHints;

  
  typedef struct AF_OutlineHintsRec_
  {
    FT_Memory     memory;

    FT_Fixed      x_scale;
    FT_Fixed      y_scale;
    FT_Pos        edge_distance_threshold;

    FT_Int        max_points;
    FT_Int        num_points;
    AF_Point      points;

    FT_Int        max_contours;
    FT_Int        num_contours;
    AF_Point*     contours;

    AF_AxisHintsRec  axis[ AF_DIMENSION_MAX ];

  } AF_OutlineHintsRec;




  FT_LOCAL( AF_Direction )
  af_direction_compute( FT_Pos  dx,
                        FT_Pos  dy );


  FT_LOCAL( void )
  af_outline_hints_init( AF_OutlineHints  hints );


 /*  used to set the (u,v) fields of each AF_Point in a AF_OutlineHints
  *  object.
  */
  typedef enum  AH_UV_
  {
    AH_UV_FXY,  /* (u,v) = (fx,fy) */
    AH_UV_FYX,  /* (u,v) = (fy,fx) */
    AH_UV_OXY,  /* (u,v) = (ox,oy) */
    AH_UV_OYX,  /* (u,v) = (oy,ox) */
    AH_UV_OX,   /* (u,v) = (ox,x)  */
    AH_UV_OY,   /* (u,v) = (oy,y)  */
    AH_UV_YX,   /* (u,v) = (y,x)   */
    AH_UV_XY    /* (u,v) = (x,y)   *   should always be last! */

  } AH_UV;

  FT_LOCAL_DEF( void )
  af_outline_hints_setup_uv( AF_OutlineHints  hints,
                             AF_UV            source );


 /*  recomputes all AF_Point in a AF_OutlineHints from the definitions
  *  in a source outline
  */
  FT_LOCAL( FT_Error )
  af_outline_hints_reset( AF_OutlineHints  hints,
                          FT_Outline*      outline,
                          FT_Fixed         x_scale,
                          FT_Fixed         y_scale );

  FT_LOCAL( void )
  af_outline_hints_done( AF_OutlineHints  hints );



/* */

FT_END_HEADER

#endif /* __AFHINTS_H__ */
