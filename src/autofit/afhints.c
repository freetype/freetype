#include "afhints.h"

#ifdef AF_DEBUG

#include <stdio.h>

  void
  af_outline_hints_dump_edges( AF_OutlineHints  hints )
  {
    AF_Edge     edges;
    AF_Edge     edge_limit;
    AF_Segment  segments;
    FT_Int      dimension;


    edges      = hints->horz_edges;
    edge_limit = edges + hints->num_hedges;
    segments   = hints->horz_segments;

    for ( dimension = 1; dimension >= 0; dimension-- )
    {
      AF_Edge   edge;


      printf ( "Table of %s edges:\n",
               !dimension ? "vertical" : "horizontal" );
      printf ( "  [ index |  pos |  dir  | link |"
               " serif | blue | opos  |  pos  ]\n" );

      for ( edge = edges; edge < edge_limit; edge++ )
      {
        printf ( "  [ %5d | %4d | %5s | %4d | %5d |  %c  | %5.2f | %5.2f ]\n",
                 edge - edges,
                 (int)edge->fpos,
                 edge->dir == AF_DIR_UP
                   ? "up"
                   : ( edge->dir == AF_DIR_DOWN
                         ? "down"
                         : ( edge->dir == AF_DIR_LEFT
                               ? "left"
                               : ( edge->dir == AF_DIR_RIGHT
                                     ? "right"
                                     : "none" ) ) ),
                 edge->link ? ( edge->link - edges ) : -1,
                 edge->serif ? ( edge->serif - edges ) : -1,
                 edge->blue_edge ? 'y' : 'n',
                 edge->opos / 64.0,
                 edge->pos / 64.0 );
      }

      edges      = hints->vert_edges;
      edge_limit = edges + hints->num_vedges;
      segments   = hints->vert_segments;
    }
  }


  /* A function used to dump the array of linked segments */
  void
  af_outline_hints_dump_segments( AF_OutlineHints  hints )
  {
    AF_Segment  segments;
    AF_Segment  segment_limit;
    AF_Point    points;
    FT_Int      dimension;


    points        = hints->points;
    segments      = hints->horz_segments;
    segment_limit = segments + hints->num_hsegments;

    for ( dimension = 1; dimension >= 0; dimension-- )
    {
      AF_Segment  seg;


      printf ( "Table of %s segments:\n",
               !dimension ? "vertical" : "horizontal" );
      printf ( "  [ index |  pos |  dir  | link | serif |"
               " numl | first | start ]\n" );

      for ( seg = segments; seg < segment_limit; seg++ )
      {
        printf ( "  [ %5d | %4d | %5s | %4d | %5d | %4d | %5d | %5d ]\n",
                 seg - segments,
                 (int)seg->pos,
                 seg->dir == AF_DIR_UP
                   ? "up"
                   : ( seg->dir == AF_DIR_DOWN
                         ? "down"
                         : ( seg->dir == AF_DIR_LEFT
                               ? "left"
                               : ( seg->dir == AF_DIR_RIGHT
                                     ? "right"
                                     : "none" ) ) ),
                 seg->link ? ( seg->link - segments ) : -1,
                 seg->serif ? ( seg->serif - segments ) : -1,
                 (int)seg->num_linked,
                 seg->first - points,
                 seg->last - points );
      }

      segments      = hints->vert_segments;
      segment_limit = segments + hints->num_vsegments;
    }
  }

#endif /* AF_DEBUG */


  /* compute the direction value of a given vector */
  FT_LOCAL_DEF( AF_Direction )
  af_direction_compute( FT_Pos  dx,
                        FT_Pos  dy )
  {
    AF_Direction  dir;
    FT_Pos        ax = ABS( dx );
    FT_Pos        ay = ABS( dy );


    dir = AF_DIR_NONE;

    /* atan(1/12) == 4.7 degrees */

    /* test for vertical direction */
    if ( ax * 12 < ay )
    {
      dir = dy > 0 ? AF_DIR_UP : AF_DIR_DOWN;
    }
    /* test for horizontal direction */
    else if ( ay * 12 < ax )
    {
      dir = dx > 0 ? AF_DIR_RIGHT : AF_DIR_LEFT;
    }

    return dir;
  }


  /* compute all inflex points in a given glyph */
  static void
  af_outline_hints_compute_inflections( AF_OutlineHints  hints )
  {
    AF_Point*  contour       = hints->contours;
    AF_Point*  contour_limit = contour + hints->num_contours;


    /* load original coordinates in (u,v) */
    af_outline_hints_setup_uv( hints, outline, AF_UV_FXY );

    /* do each contour separately */
    for ( ; contour < contour_limit; contour++ )
    {
      AF_Point   point = contour[0];
      AF_Point   first = point;
      AF_Point   start = point;
      AF_Point   end   = point;
      AF_Point   before;
      AF_Point   after;
      AF_Angle   angle_in, angle_seg, angle_out;
      AF_Angle   diff_in, diff_out;
      FT_Int     finished = 0;


      /* compute first segment in contour */
      first = point;

      start = end = first;
      do
      {
        end = end->next;
        if ( end == first )
          goto Skip;

      } while ( end->u == first->u && end->v == first->v );

      angle_seg = af_angle( end->u - start->u, 
                            end->v - start->v );

      /* extend the segment start whenever possible */
      before = start;
      do
      {
        do
        {
          start  = before;
          before = before->prev;
          if ( before == first )
            goto Skip;

        } while ( before->u == start->u && before->v == start->v );

        angle_in = af_angle( start->u - before->u, 
                             start->v - before->v );

      } while ( angle_in == angle_seg );

      first   = start;
      diff_in = af_angle_diff( angle_in, angle_seg );

      /* now, process all segments in the contour */
      do
      {
        /* first, extend current segment's end whenever possible */
        after = end;
        do
        {
          do
          {
            end   = after;
            after = after->next;
            if ( after == first )
              finished = 1;

          } while ( end->u == after->u && end->v == after->v );

          vec.x     = after->u - end->u;
          vec.y     = after->v - end->v;
          angle_out = af_angle( after->u - end->u,
                                after->v - end->v );

        } while ( angle_out == angle_seg );

        diff_out = af_angle_diff( angle_seg, angle_out );

        if ( ( diff_in ^ diff_out ) < 0 )
        {
          /* diff_in and diff_out have different signs, we have */
          /* inflection points here...                          */
          do
          {
            start->flags |= AF_FLAG_INFLECTION;
            start = start->next;

          } while ( start != end );

          start->flags |= AF_FLAG_INFLECTION;
        }

        start     = end;
        end       = after;
        angle_seg = angle_out;
        diff_in   = diff_out;

      } while ( !finished );

    Skip:
      ;
    }
  }



  FT_LOCAL_DEF( void )
  af_outline_hints_init( AF_OutlineHints  hints,
                         FT_Memory        memory )
  {
    FT_ZERO( hints );
    hints->memory = memory;
  }                         



  FT_LOCAL_DEF( void )
  af_outline_hints_done( AF_OutlineHints  hints )
  {
    if ( hints && hints->memory )
    {
      FT_Memory     memory = hints->memory;
      AF_Dimension  dim;

     /* note that we don't need to free the segment and edge
      * buffers, since they're really within the hints->points array
      */
      for ( dim = 0; dim < 2; dim++ )
      {
        AF_AxisHints  axis = &hints->axis[ dim ];
        
        axis->num_segments = 0;
        axis->num_edges    = 0;
        axis->segments     = NULL;
        axis->edges        = NULL;
      }

      FT_FREE( hints->contours );
      hints->max_contours = 0;
      hints->num_contours = 0;
      
      FT_FREE( hints->points );
      hints->num_points = 0;
      hints->max_points = 0;
      
      hints->memory = NULL;
    }
  }



  FT_LOCAL_DEF( FT_Error )
  af_outline_hints_reset( AF_OutlineHints  hints,
                          FT_Outline*      outline,
                          FT_Fixed         x_scale,
                          FT_Fixed         y_scale )
  {
    FT_Error     error        = AF_Err_Ok;
    
    FT_UInt      old_max, new_max;

    hints->num_points    = 0;
    hints->num_contours  = 0;
    
    hints->axis[0].num_segments = 0;
    hints->axis[0].num_edges    = 0;
    hints->axis[1].num_segments = 0;
    hints->axis[1].num_edges    = 0;
    
   /* first of all, reallocate the contours array when necessary
    */
    new_max = (FT_UInt) outline->n_contours;
    old_max = hints->max_contours;
    if ( new_max > old_max )
    {
      new_max = (new_max + 3) & ~3;
      
      if ( FT_RENEW_ARRAY( hints->contours, old_max, new_max ) )
        goto Exit;
      
      hints->max_contours = new_max;
    }

   /* then, reallocate the points, segments & edges arrays if needed --   
    * note that we reserved two additional point positions, used to       
    * hint metrics appropriately                                          
    */                                                                    
    new_max = (FT_UInt)( outline->n_points + 2 );
    old_max = hints->max_points;
    if ( new_max > old_max )
    {
      FT_Byte*    items;
      FT_ULong    off1, off2, off3;
      
     /* we store in a single buffer the following arrays:
      *
      *  - an array of   N  AF_PointRec   items
      *  - an array of 2*N  AF_SegmentRec items
      *  - an array of 2*N  AF_EdgeRec    items 
      *
      */
      
      new_max = ( new_max + 2 + 7 ) & ~7;
      
#undef  OFF_INCREMENT
#define OFF_INCREMENT( _off, _type, _count )   \
     ((((_off) + sizeof(_type)) & ~(sizeof(_type)) + ((_count)*sizeof(_type)))

      off1 = OFF_INCREMENT( 0, AF_PointRec, new_max );
      off2 = OFF_INCREMENT( off1, AF_SegmentRec, new_max );
      off3 = OFF_INCREMENT( off2, AF_EdgeRec, new_max*2 );

      FT_FREE( hints->points );

      if ( FT_ALLOC( items, off3 ) )
      {
        hints->max_points       = 0;
        hints->axis[0].segments = NULL;
        hints->axis[0].edges    = NULL;
        hints->axis[1].segments = NULL;
        hints->axis[1].edges    = NULL;
        goto Exit;
      }
      
     /* readjust some pointers
      */
      hints->max_points       = new_max;
      hints->points           = (AF_Point) items;
      
      hints->axis[0].segments = (AF_Segment)( items + off1 );
      hints->axis[1].segments = hints->axis[0].segments + new_max;
      
      hints->axis[0].edges    = (AF_Edge)   ( items + off2 );
      hints->axis[1].edges    = hints->axis[0].edges + new_max;
    }

    hints->num_points   = outline->n_points;
    hints->num_contours = outline->n_contours;


    /* We can't rely on the value of `FT_Outline.flags' to know the fill  */
    /* direction used for a glyph, given that some fonts are broken (e.g. */
    /* the Arphic ones).  We thus recompute it each time we need to.      */
    /*                                                                    */
    hints->axis[ AF_DIMENSION_HORZ ].major_dir = AF_DIR_UP;
    hints->axis[ AF_DIMENSION_VERT ].major_dir = AF_DIR_LEFT;

    if ( FT_Outline_Get_Orientation( outline ) == FT_ORIENTATION_POSTSCRIPT )
    {
      hints->axis[ AF_DIMENSION_HORZ ].major_dir = AF_DIR_DOWN;
      hints->axis[ AF_DIMENSION_VERT ].major_dir = AF_DIR_RIGHT;
    }

    hints->x_scale = x_scale;
    hints->y_scale = y_scale;

    points = hints->points;
    if ( hints->num_points == 0 )
      goto Exit;

    {
      /* do one thing at a time -- it is easier to understand, and */
      /* the code is clearer                                       */
      AF_Point  point;
      AF_Point  point_limit = points + hints->num_points;


      /* compute coordinates & bezier flags */
      {
        FT_Vector*  vec = outline->points;
        char*       tag = outline->tags;


        for ( point = points; point < point_limit; point++, vec++, tag++ )
        {
          point->fx = vec->x;
          point->fy = vec->y;
          point->ox = point->x = FT_MulFix( vec->x, x_scale );
          point->oy = point->y = FT_MulFix( vec->y, y_scale );

          switch ( FT_CURVE_TAG( *tag ) )
          {
          case FT_CURVE_TAG_CONIC:
            point->flags = AF_FLAG_CONIC;
            break;
          case FT_CURVE_TAG_CUBIC:
            point->flags = AF_FLAG_CUBIC;
            break;
          default:
            point->flags = 0;
            ;
          }
        }
      }

      /* compute `next' and `prev' */
      {
        FT_Int    contour_index;
        AF_Point  prev;
        AF_Point  first;
        AF_Point  end;


        contour_index = 0;

        first = points;
        end   = points + outline->contours[0];
        prev  = end;

        for ( point = points; point < point_limit; point++ )
        {
          point->prev = prev;
          if ( point < end )
          {
            point->next = point + 1;
            prev        = point;
          }
          else
          {
            point->next = first;
            contour_index++;
            if ( point + 1 < point_limit )
            {
              end   = points + source->contours[contour_index];
              first = point + 1;
              prev  = end;
            }
          }
        }
      }

      /* set-up the contours array */
      {
        AF_Point*  contour       = hints->contours;
        AF_Point*  contour_limit = contour + hints->num_contours;
        short*     end           = outline->contours;
        short      idx           = 0;


        for ( ; contour < contour_limit; contour++, end++ )
        {
          contour[0] = points + idx;
          idx        = (short)( end[0] + 1 );
        }
      }

      /* compute directions of in & out vectors */
      {
        for ( point = points; point < point_limit; point++ )
        {
          AF_Point   prev;
          AF_Point   next;
          FT_Pos     in_x, in_y, out_x, out_y;


          prev   = point->prev;
          in_x   = point->fx - prev->fx;
          in_y   = point->fy - prev->fy;
          
          point->in_dir = af_compute_direction( in_x, in_y );

          next   = point->next;
          out_x  = next->fx - point->fx;
          out_y  = next->fy - point->fy;

          point->out_dir = af_compute_direction( out_x, out_y );

          if ( point->flags & ( AF_FLAG_CONIC | AF_FLAG_CUBIC ) )
          {
          Is_Weak_Point:
            point->flags |= AF_FLAG_WEAK_INTERPOLATION;
          }
          else if ( point->out_dir == point->in_dir )
          {
            AF_Angle  angle_in, angle_out, delta;


            if ( point->out_dir != AF_DIR_NONE )
              goto Is_Weak_Point;

            angle_in  = af_angle( in_x, in_y );
            angle_out = af_angle( out_x, out_y );
            delta     = af_angle_diff( angle_in, angle_out );

            if ( delta < 2 && delta > -2 )
              goto Is_Weak_Point;
          }
          else if ( point->in_dir == -point->out_dir )
            goto Is_Weak_Point;
        }
      }
    }

   /* compute inflection points
    */    
    af_outline_hints_compute_inflections( hints );

  Exit:
    return error;
  }


  FT_LOCAL_DEF( void )
  af_outline_hints_setup_uv( AF_OutlineHints  hints,
                             AF_UV            source )
  {
    AF_Point  point       = hints->points;
    AF_Point  point_limit = point + hints->num_points;


    switch ( source )
    {
    case AF_UV_FXY:
      for ( ; point < point_limit; point++ )
      {
        point->u = point->fx;
        point->v = point->fy;
      }
      break;

    case AF_UV_FYX:
      for ( ; point < point_limit; point++ )
      {
        point->u = point->fy;
        point->v = point->fx;
      }
      break;

    case AF_UV_OXY:
      for ( ; point < point_limit; point++ )
      {
        point->u = point->ox;
        point->v = point->oy;
      }
      break;

    case AF_UV_OYX:
      for ( ; point < point_limit; point++ )
      {
        point->u = point->oy;
        point->v = point->ox;
      }
      break;

    case AF_UV_YX:
      for ( ; point < point_limit; point++ )
      {
        point->u = point->y;
        point->v = point->x;
      }
      break;

    case AF_UV_OX:
      for ( ; point < point_limit; point++ )
      {
        point->u = point->x;
        point->v = point->ox;
      }
      break;

    case AF_UV_OY:
      for ( ; point < point_limit; point++ )
      {
        point->u = point->y;
        point->v = point->oy;
      }
      break;

    default:
      for ( ; point < point_limit; point++ )
      {
        point->u = point->x;
        point->v = point->y;
      }
    }
  }



