#include "aflatin.h"

  FT_LOCAL_DEF( void )
  af_latin_hints_compute_segments( AF_OutlineHints  hints,
                                   AF_Dimension     dim )
  {
    AF_AxisHints  axis = &hints->axis[dim];
    AF_Segment    segments = axis->segments;
    AF_Segment    segment       =  segments;
    FT_Int        num_segments  =  0;
    AF_Point*     contour       =  hints->contours;
    AF_Point*     contour_limit =  contour + hints->num_contours;
    AF_Direction  major_dir;

#ifdef AF_HINT_METRICS
    AF_Point    min_point     =  0;
    AF_Point    max_point     =  0;
    FT_Pos      min_coord     =  32000;
    FT_Pos      max_coord     = -32000;
#endif

    major_dir   = ABS( axis->major_dir );
    segment_dir = major_dir;

    /* set up (u,v) in each point */
    af_setup_uv( outline, (dim == AF_DIMENSION_HORZ)
                        ? AF_UV_FXY,
                        : AF_UV_FYX );


    /* do each contour separately */
    for ( ; contour < contour_limit; contour++ )
    {
      AF_Point  point   =  contour[0];
      AF_Point  last    =  point->prev;
      int       on_edge =  0;
      FT_Pos    min_pos =  32000;  /* minimum segment pos != min_coord */
      FT_Pos    max_pos = -32000;  /* maximum segment pos != max_coord */
      FT_Bool   passed;


#ifdef AF_HINT_METRICS
      if ( point->u < min_coord )
      {
        min_coord = point->u;
        min_point = point;
      }
      if ( point->u > max_coord )
      {
        max_coord = point->u;
        max_point = point;
      }
#endif

      if ( point == last )  /* skip singletons -- just in case */
        continue;

      if ( ABS( last->out_dir )  == major_dir &&
           ABS( point->out_dir ) == major_dir )
      {
        /* we are already on an edge, try to locate its start */
        last = point;

        for (;;)
        {
          point = point->prev;
          if ( ABS( point->out_dir ) != major_dir )
          {
            point = point->next;
            break;
          }
          if ( point == last )
            break;
        }
      }

      last   = point;
      passed = 0;

      for (;;)
      {
        FT_Pos  u, v;


        if ( on_edge )
        {
          u = point->u;
          if ( u < min_pos )
            min_pos = u;
          if ( u > max_pos )
            max_pos = u;

          if ( point->out_dir != segment_dir || point == last )
          {
            /* we are just leaving an edge; record a new segment! */
            segment->last = point;
            segment->pos  = ( min_pos + max_pos ) >> 1;

            /* a segment is round if either its first or last point */
            /* is a control point                                   */
            if ( ( segment->first->flags | point->flags ) &
                   AF_FLAG_CONTROL                        )
              segment->flags |= AF_EDGE_ROUND;

            /* compute segment size */
            min_pos = max_pos = point->v;

            v = segment->first->v;
            if ( v < min_pos )
              min_pos = v;
            if ( v > max_pos )
              max_pos = v;

            segment->min_coord = min_pos;
            segment->max_coord = max_pos;

            on_edge = 0;
            num_segments++;
            segment++;
            /* fallthrough */
          }
        }

        /* now exit if we are at the start/end point */
        if ( point == last )
        {
          if ( passed )
            break;
          passed = 1;
        }

        if ( !on_edge && ABS( point->out_dir ) == major_dir )
        {
          /* this is the start of a new segment! */
          segment_dir = point->out_dir;

          /* clear all segment fields */
          FT_ZERO( segment );

          segment->dir      = segment_dir;
          segment->flags    = AF_EDGE_NORMAL;
          min_pos = max_pos = point->u;
          segment->first    = point;
          segment->last     = point;
          segment->contour  = contour;
          segment->score    = 32000;
          segment->link     = NULL;
          on_edge           = 1;

#ifdef AF_HINT_METRICS
          if ( point == max_point )
            max_point = 0;

          if ( point == min_point )
            min_point = 0;
#endif
        }

        point = point->next;
      }

    } /* contours */

#ifdef AF_HINT_METRICS
    /* we need to ensure that there are edges on the left-most and  */
    /* right-most points of the glyph in order to hint the metrics; */
    /* we do this by inserting fake segments when needed            */
    if ( dim == AF_DIMENSION_HORZ )
    {
      AF_Point  point       = hints->points;
      AF_Point  point_limit = point + hints->num_points;

      FT_Pos    min_pos =  32000;
      FT_Pos    max_pos = -32000;


      min_point = 0;
      max_point = 0;

      /* compute minimum and maximum points */
      for ( ; point < point_limit; point++ )
      {
        FT_Pos  x = point->fx;


        if ( x < min_pos )
        {
          min_pos   = x;
          min_point = point;
        }
        if ( x > max_pos )
        {
          max_pos   = x;
          max_point = point;
        }
      }

      /* insert minimum segment */
      if ( min_point )
      {
        /* clear all segment fields */
        FT_ZERO( segment );

        segment->dir   = segment_dir;
        segment->flags = AF_EDGE_NORMAL;
        segment->first = min_point;
        segment->last  = min_point;
        segment->pos   = min_pos;
        segment->score = 32000;
        segment->link  = NULL;

        num_segments++;
        segment++;
      }

      /* insert maximum segment */
      if ( max_point )
      {
        /* clear all segment fields */
        FT_ZERO( segment );

        segment->dir   = segment_dir;
        segment->flags = AF_EDGE_NORMAL;
        segment->first = max_point;
        segment->last  = max_point;
        segment->pos   = max_pos;
        segment->score = 32000;
        segment->link  = NULL;

        num_segments++;
        segment++;
      }
    }
#endif /* AF_HINT_METRICS */

    axis->num_segments = num_segments;
  }


  FT_LOCAL_DEF( void )
  af_latin_hints_link_segments( AF_OutlineHints  hints,
                                AF_Dimension     dim )
  {
    AF_AxisHints  axis          = &hints->axis[dim];
    AF_Segment    segments      = axis->segments;
    AF_Segment    segment_limit = segments + axis->num_segments;
    AF_Direction  major_dir     = axis->major_dir;
    AF_Segment    seg1, seg2;

    /* now compare each segment to the others */
    for ( seg1 = segments; seg1 < segment_limit; seg1++ )
    {
      /* the fake segments are introduced to hint the metrics -- */
      /* we must never link them to anything                     */
      if ( seg1->first == seg1->last || seg1->dir != major_dir )
        continue;

      for ( seg2 = segments; seg2 < segment_limit; seg2++ )
        if ( seg2 != seg1 && seg1->dir + seg2->dir == 0 )
        {
          FT_Pos  pos1 = seg1->pos;
          FT_Pos  pos2 = seg2->pos;
          FT_Pos  dist = pos2 - pos1;


          if ( dist < 0 )
            continue;

          {
            FT_Pos  min = seg1->min_coord;
            FT_Pos  max = seg1->max_coord;
            FT_Pos  len, score;


            if ( min < seg2->min_coord )
              min = seg2->min_coord;

            if ( max > seg2->max_coord )
              max = seg2->max_coord;

            len = max - min;
            if ( len >= 8 )
            {
              score = dist + 3000 / len;

              if ( score < seg1->score )
              {
                seg1->score = score;
                seg1->link  = seg2;
              }

              if ( score < seg2->score )
              {
                seg2->score = score;
                seg2->link  = seg1;
              }
            }
          }
        }
    }

    /* now, compute the `serif' segments */
    for ( seg1 = segments; seg1 < segment_limit; seg1++ )
    {
      seg2 = seg1->link;

      if ( seg2 )
      {
        seg2->num_linked++;
        if ( seg2->link != seg1 )
        {
          seg1->link  = 0;
          seg1->serif = seg2->link;
        }
      }
    }
  }


  FT_LOCAL_DEF( void )
  af_latin_hints_compute_edges( AF_OutlineHints  hints,
                                AF_Dimension     dim )
  {
    AF_AxisHints  axis = &hints->axis[dim];
    AF_Edge       edges = axis->edges;
    AF_Edge       edge, edge_limit;
    
    AF_Segment    segments = axis->segments;
    AF_Segment    segment_limit = segments + axis->num_segments;
    AF_Segment    seg;
    
    AF_Direction  up_dir;
    FT_Fixed      scale;
    FT_Pos        edge_distance_threshold;


    scale = ( dim == AF_DIMENSION_HORZ ) ? hints->x_scale
                                         : hints->y_scale;

    up_dir = ( dim == AF_DIMENSION_HORZ ) ? AF_DIR_UP
                                          : AF_DIR_RIGHT;

    /*********************************************************************/
    /*                                                                   */
    /* We will begin by generating a sorted table of edges for the       */
    /* current direction.  To do so, we simply scan each segment and try */
    /* to find an edge in our table that corresponds to its position.    */
    /*                                                                   */
    /* If no edge is found, we create and insert a new edge in the       */
    /* sorted table.  Otherwise, we simply add the segment to the edge's */
    /* list which will be processed in the second step to compute the    */
    /* edge's properties.                                                */
    /*                                                                   */
    /* Note that the edges table is sorted along the segment/edge        */
    /* position.                                                         */
    /*                                                                   */
    /*********************************************************************/

    edge_distance_threshold = FT_MulFix( outline->edge_distance_threshold,
                                         scale );
    if ( edge_distance_threshold > 64 / 4 )
      edge_distance_threshold = 64 / 4;

    edge_distance_threshold = FT_DivFix( edge_distance_threshold,
                                         scale );

    edge_limit = edges;
    for ( seg = segments; seg < segment_limit; seg++ )
    {
      AF_Edge  found = 0;


      /* look for an edge corresponding to the segment */
      for ( edge = edges; edge < edge_limit; edge++ )
      {
        FT_Pos  dist;


        dist = seg->pos - edge->fpos;
        if ( dist < 0 )
          dist = -dist;

        if ( dist < edge_distance_threshold )
        {
          found = edge;
          break;
        }
      }

      if ( !found )
      {
        /* insert a new edge in the list and */
        /* sort according to the position    */
        while ( edge > edges && edge[-1].fpos > seg->pos )
        {
          edge[0] = edge[-1];
          edge--;
        }
        edge_limit++;

        /* clear all edge fields */
        FT_MEM_ZERO( edge, sizeof ( *edge ) );

        /* add the segment to the new edge's list */
        edge->first    = seg;
        edge->last     = seg;
        edge->fpos     = seg->pos;
        edge->opos     = edge->pos = FT_MulFix( seg->pos, scale );
        seg->edge_next = seg;
      }
      else
      {
        /* if an edge was found, simply add the segment to the edge's */
        /* list                                                       */
        seg->edge_next        = edge->first;
        edge->last->edge_next = seg;
        edge->last            = seg;
      }
    }
    *p_num_edges = (FT_Int)( edge_limit - edges );


    /*********************************************************************/
    /*                                                                   */
    /* Good, we will now compute each edge's properties according to     */
    /* segments found on its position.  Basically, these are:            */
    /*                                                                   */
    /*  - edge's main direction                                          */
    /*  - stem edge, serif edge or both (which defaults to stem then)    */
    /*  - rounded edge, straight or both (which defaults to straight)    */
    /*  - link for edge                                                  */
    /*                                                                   */
    /*********************************************************************/

    /* first of all, set the `edge' field in each segment -- this is */
    /* required in order to compute edge links                       */

    /* Note that I've tried to remove this loop, setting
     * the "edge" field of each segment directly in the
     * code above.  For some reason, it slows down execution
     * speed -- on a Sun.
     */
    for ( edge = edges; edge < edge_limit; edge++ )
    {
      seg = edge->first;
      if ( seg )
        do
        {
          seg->edge = edge;
          seg       = seg->edge_next;
        }
        while ( seg != edge->first );
    }

    /* now, compute each edge properties */
    for ( edge = edges; edge < edge_limit; edge++ )
    {
      FT_Int  is_round    = 0;  /* does it contain round segments?    */
      FT_Int  is_straight = 0;  /* does it contain straight segments? */
      FT_Pos  ups         = 0;  /* number of upwards segments         */
      FT_Pos  downs       = 0;  /* number of downwards segments       */


      seg = edge->first;

      do
      {
        FT_Bool  is_serif;


        /* check for roundness of segment */
        if ( seg->flags & AF_EDGE_ROUND )
          is_round++;
        else
          is_straight++;

        /* check for segment direction */
        if ( seg->dir == up_dir )
          ups   += seg->max_coord-seg->min_coord;
        else
          downs += seg->max_coord-seg->min_coord;

        /* check for links -- if seg->serif is set, then seg->link must */
        /* be ignored                                                   */
        is_serif = (FT_Bool)( seg->serif && seg->serif->edge != edge );

        if ( seg->link || is_serif )
        {
          AF_Edge     edge2;
          AF_Segment  seg2;


          edge2 = edge->link;
          seg2  = seg->link;

          if ( is_serif )
          {
            seg2  = seg->serif;
            edge2 = edge->serif;
          }

          if ( edge2 )
          {
            FT_Pos  edge_delta;
            FT_Pos  seg_delta;


            edge_delta = edge->fpos - edge2->fpos;
            if ( edge_delta < 0 )
              edge_delta = -edge_delta;

            seg_delta = seg->pos - seg2->pos;
            if ( seg_delta < 0 )
              seg_delta = -seg_delta;

            if ( seg_delta < edge_delta )
              edge2 = seg2->edge;
          }
          else
            edge2 = seg2->edge;

#ifdef FT_CONFIG_CHESTER_SERIF
          if ( is_serif )
          {
            edge->serif   = edge2;
            edge2->flags |= AF_EDGE_SERIF;
          }
          else
            edge->link  = edge2;
#else /* !FT_CONFIG_CHESTER_SERIF */
          if ( is_serif )
            edge->serif = edge2;
          else
            edge->link  = edge2;
#endif /* !FT_CONFIG_CHESTER_SERIF */
        }

        seg = seg->edge_next;

      } while ( seg != edge->first );

      /* set the round/straight flags */
      edge->flags = AF_EDGE_NORMAL;

      if ( is_round > 0 && is_round >= is_straight )
        edge->flags |= AF_EDGE_ROUND;

      /* set the edge's main direction */
      edge->dir = AF_DIR_NONE;

      if ( ups > downs )
        edge->dir = up_dir;

      else if ( ups < downs )
        edge->dir = -up_dir;

      else if ( ups == downs )
        edge->dir = 0;  /* both up and down! */

      /* gets rid of serifs if link is set                */
      /* XXX: This gets rid of many unpleasant artefacts! */
      /*      Example: the `c' in cour.pfa at size 13     */

      if ( edge->serif && edge->link )
        edge->serif = 0;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    af_outline_detect_features                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Performs feature detection on a given AF_OutlineRec object.        */
  /*                                                                       */
  FT_LOCAL_DEF( void )
  af_latin_hints_detect_features( AF_OutlineHints  hints,
                                  AF_Dimension     dim )
  {
    af_latin_hints_compute_segments( hints, dim );
    af_latin_hints_link_segments   ( hints, dim );
    af_latin_hints_compute_edges   ( hints dim );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    af_outline_compute_blue_edges                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Computes the `blue edges' in a given outline (i.e. those that must */
  /*    be snapped to a blue zone edge (top or bottom).                    */
  /*                                                                       */
  FT_LOCAL_DEF( void )
  af_latin_hints_compute_blue_edges( AF_OutlineHints  outline,
                                     AF_Face_Globals  face_globals )
  {
    AF_Edge     edge       = outline->horz_edges;
    AF_Edge     edge_limit = edge + outline->num_hedges;
    AF_Globals  globals    = &face_globals->design;
    FT_Fixed    y_scale    = outline->y_scale;

    FT_Bool     blue_active[AF_BLUE_MAX];


    /* compute which blue zones are active, i.e. have their scaled */
    /* size < 3/4 pixels                                           */
    {
      AF_Blue  blue;
      FT_Bool  check = 0;


      for ( blue = AF_BLUE_CAPITAL_TOP; blue < AF_BLUE_MAX; blue++ )
      {
        FT_Pos  ref, shoot, dist;


        ref   = globals->blue_refs[blue];
        shoot = globals->blue_shoots[blue];
        dist  = ref - shoot;
        if ( dist < 0 )
          dist = -dist;

        blue_active[blue] = 0;

        if ( FT_MulFix( dist, y_scale ) < 48 )
        {
          blue_active[blue] = 1;
          check = 1;
        }
      }

      /* return immediately if no blue zone is active */
      if ( !check )
        return;
    }

    /* for each horizontal edge search the blue zone which is closest */
    for ( ; edge < edge_limit; edge++ )
    {
      AF_Blue  blue;
      FT_Pos*  best_blue = 0;
      FT_Pos   best_dist;  /* initial threshold */


      /* compute the initial threshold as a fraction of the EM size */
      best_dist = FT_MulFix( face_globals->face->units_per_EM / 40, y_scale );

#ifdef FT_CONFIG_CHESTER_SMALL_F
      if ( best_dist > 64 / 2 )
        best_dist = 64 / 2;
#else
      if ( best_dist > 64 / 4 )
        best_dist = 64 / 4;
#endif

      for ( blue = AF_BLUE_CAPITAL_TOP; blue < AF_BLUE_MAX; blue++ )
      {
        /* if it is a top zone, check for right edges -- if it is a bottom */
        /* zone, check for left edges                                      */
        /*                                                                 */
        /* of course, that's for TrueType XXX                              */
        FT_Bool  is_top_blue  =
                   FT_BOOL( AF_IS_TOP_BLUE( blue ) );
        FT_Bool  is_major_dir =
                   FT_BOOL( edge->dir == outline->horz_major_dir );


        if ( !blue_active[blue] )
          continue;

        /* if it is a top zone, the edge must be against the major    */
        /* direction; if it is a bottom zone, it must be in the major */
        /* direction                                                  */
        if ( is_top_blue ^ is_major_dir )
        {
          FT_Pos   dist;
          FT_Pos*  blue_pos = globals->blue_refs + blue;


          /* first of all, compare it to the reference position */
          dist = edge->fpos - *blue_pos;
          if ( dist < 0 )
            dist = -dist;

          dist = FT_MulFix( dist, y_scale );
          if ( dist < best_dist )
          {
            best_dist = dist;
            best_blue = blue_pos;
          }

          /* now, compare it to the overshoot position if the edge is     */
          /* rounded, and if the edge is over the reference position of a */
          /* top zone, or under the reference position of a bottom zone   */
          if ( edge->flags & AF_EDGE_ROUND && dist != 0 )
          {
            FT_Bool  is_under_ref = FT_BOOL( edge->fpos < *blue_pos );


            if ( is_top_blue ^ is_under_ref )
            {
              blue_pos = globals->blue_shoots + blue;
              dist = edge->fpos - *blue_pos;
              if ( dist < 0 )
                dist = -dist;

              dist = FT_MulFix( dist, y_scale );
              if ( dist < best_dist )
              {
                best_dist = dist;
                best_blue = blue_pos;
              }
            }
          }
        }
      }

      if ( best_blue )
        edge->blue_edge = best_blue;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    af_outline_scale_blue_edges                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function must be called before hinting in order to re-adjust  */
  /*    the contents of the detected edges (basically change the `blue     */
  /*    edge' pointer from `design units' to `scaled ones').               */
  /*                                                                       */
  FT_LOCAL_DEF( void )
  af_outline_hints_scale_blue_edges( AF_OutlineHints  hints )       outline,
  {
    AF_AxisHints  axis       = &hints->axis[ AF_DIMENSION_VERT ];
    AF_Edge       edge       = axis->edges;
    AF_Edge       edge_limit = edge + axis->num_edges;
    FT_Pos        delta;


    delta = globals->scaled.blue_refs - globals->design.blue_refs;

    for ( ; edge < edge_limit; edge++ )
    {
      if ( edge->blue_edge )
        edge->blue_edge += delta;
    }
  }

