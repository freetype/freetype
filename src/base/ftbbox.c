/***************************************************************************/
/*                                                                         */
/*  ftbbox.c                                                               */
/*                                                                         */
/*    FreeType bbox computation (body).                                    */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This component has a _single_ role: to compute exact outline bounding */
  /* boxes.                                                                */
  /*                                                                       */
  /*************************************************************************/


#include <freetype/ftbbox.h>
#include <freetype/ftimage.h>
#include <freetype/ftoutln.h>


  typedef struct  TBBox_Rec_
  {
    FT_Vector  last;
    FT_BBox    bbox;

  } TBBox_Rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    BBox_Move_To                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used as a `move_to' and `line_to' emitter during  */
  /*    FT_Outline_Decompose().  It simply records the destination point   */
  /*    in `user->last'; no further computations are necessary since we    */
  /*    the cbox as the starting bbox which must be refined.               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    to   :: A pointer to the destination vector.                       */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    user :: A pointer to the current walk context.                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Always 0.  Needed for the interface only.                          */
  /*                                                                       */
  static
  int  BBox_Move_To( FT_Vector*  to,
                     TBBox_Rec*  user )
  {
    user->last = *to;

    return 0;
  }


#define CHECK_X( p, bbox )  \
          ( p->x < bbox.xMin || p->x > bbox.xMax )

#define CHECK_Y( p, bbox )  \
          ( p->y < bbox.yMin || p->y > bbox.yMax )


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    BBox_Conic_Check                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finds the extrema of a 1-dimensional conic Bezier curve and update */
  /*    a bounding range.  This version uses direct computation, as it     */
  /*    doesn't need square roots.                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y1  :: The start coordinate.                                       */
  /*    y2  :: The coordinate of the control point.                        */
  /*    y3  :: The end coordinate.                                         */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    min :: The address of the current minimum.                         */
  /*    max :: The address of the current maximum.                         */
  /*                                                                       */
  static
  void  BBox_Conic_Check( FT_Pos   y1,
                          FT_Pos   y2,
                          FT_Pos   y3,
                          FT_Pos*  min,
                          FT_Pos*  max )
  {
    if ( y1 == y3 )
    {
      if ( y2 == y1 )               /* Flat arc */
        goto Suite;
    }
    else if ( y1 < y3 )
    {
      if ( y2 >= y1 && y2 <= y3 )   /* Ascending arc */
        goto Suite;
    }
    else
    {
      if ( y2 >= y3 && y2 <= y1 )   /* Descending arc */
      {
        y2 = y1;
        y1 = y3;
        y3 = y2;
        goto Suite;
      }
    }

    y1 = y3 = y1 - FT_MulDiv( y2 - y1, y2 - y1, y1 - 2*y2 + y3 );

  Suite:
    if ( y1 < *min ) *min = y1;
    if ( y3 > *max ) *max = y3;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    BBox_Conic_To                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used as a `conic_to' emitter during               */
  /*    FT_Raster_Decompose().  It checks a conic Bezier curve with the    */
  /*    current bounding box, and computes its extrema if necessary to     */
  /*    update it.                                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    control :: A pointer to a control point.                           */
  /*    to      :: A pointer to the destination vector.                    */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    user    :: The address of the current walk context.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Always 0.  Needed for the interface only.                          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    In the case of a non-monotonous arc, we compute directly the       */
  /*    extremum coordinates, as it is sufficiently fast.                  */
  /*                                                                       */
  static
  int  BBox_Conic_To( FT_Vector*  control,
                      FT_Vector*  to,
                      TBBox_Rec*  user )
  {
    /* we don't need to check `to' since it is always an `on' point, thus */
    /* within the bbox                                                    */

    if ( CHECK_X( control, user->bbox ) )

      BBox_Conic_Check( user->last.x,
                        control->x,
                        to->x,
                        &user->bbox.xMin,
                        &user->bbox.xMax );

    if ( CHECK_Y( control, user->bbox ) )

      BBox_Conic_Check( user->last.y,
                        control->y,
                        to->y,
                        &user->bbox.yMin,
                        &user->bbox.yMax );

    user->last = *to;

    return 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    BBox_Cubic_Check                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finds the extrema of a 1-dimensional cubic Bezier curve and        */
  /*    updates a bounding range.  This version uses splitting because we  */
  /*    don't want to use square roots and extra accuracies.               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    p1  :: The start coordinate.                                       */
  /*    p2  :: The coordinate of the first control point.                  */
  /*    p3  :: The coordinate of the second control point.                 */
  /*    p4  :: The end coordinate.                                         */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    min :: The address of the current minimum.                         */
  /*    max :: The address of the current maximum.                         */
  /*                                                                       */
  static
  void  BBox_Cubic_Check( FT_Pos   p1,
                          FT_Pos   p2,
                          FT_Pos   p3,
                          FT_Pos   p4,
                          FT_Pos*  min,
                          FT_Pos*  max )
  {
    FT_Pos  stack[33], *arc;


    arc = stack;

    arc[0] = p1;
    arc[1] = p2;
    arc[2] = p3;
    arc[3] = p4;

    do
    {
      FT_Pos  y1 = arc[0];
      FT_Pos  y2 = arc[1];
      FT_Pos  y3 = arc[2];
      FT_Pos  y4 = arc[3];


      if ( y1 == y4 )
      {
        if ( y1 == y2 && y1 == y3 )                         /* Flat */
          goto Test;
      }
      else if ( y1 < y4 )
      {
        if ( y2 >= y1 && y2 <= y4 && y3 >= y1 && y3 <= y4 ) /* Ascending */
          goto Test;
      }
      else
      {
        if ( y2 >= y4 && y2 <= y1 && y3 >= y4 && y3 <= y1 ) /* Descending */
        {
          y2 = y1;
          y1 = y4;
          y4 = y2;
          goto Test;
        }
      }

      /* Unknown direction, split the arc in two */
      arc[6] = y4;
      arc[1] = y1 = ( y1 + y2 ) / 2;
      arc[5] = y4 = ( y4 + y3 ) / 2;
      y2 = ( y2 + y3 ) / 2;
      arc[2] = y1 = ( y1 + y2 ) / 2;
      arc[4] = y4 = ( y4 + y2 ) / 2;
      arc[3] = ( y1 + y4 ) / 2;

      arc += 3;
      goto Suite;

   Test:
      if ( y1 < *min ) *min = y1;
      if ( y4 > *max ) *max = y4;
      arc -= 3;

    Suite:
      ;
    } while ( arc >= stack );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    BBox_Cubic_To                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used as a `cubic_to' emitter during               */
  /*    FT_Raster_Decompose().  It checks a cubic Bezier curve with the    */
  /*    current bounding box, and computes its extrema if necessary to     */
  /*    update it.                                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    control1 :: A pointer to the first control point.                  */
  /*    control2 :: A pointer to the second control point.                 */
  /*    to       :: A pointer to the destination vector.                   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    user     :: The address of the current walk context.               */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Always 0.  Needed for the interface only.                          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    In the case of a non-monotonous arc, we don't compute directly     */
  /*    extremum coordinates, we subdivise instead.                        */
  /*                                                                       */
  static
  int  BBox_Cubic_To( FT_Vector*  control1,
                      FT_Vector*  control2,
                      FT_Vector*  to,
                      TBBox_Rec*  user )
  {
    /* we don't need to check `to' since it is always an `on' point, thus */
    /* within the bbox                                                    */

    if ( CHECK_X( control1, user->bbox ) ||
         CHECK_X( control2, user->bbox ) )

        BBox_Cubic_Check( user->last.x,
                          control1->x,
                          control2->x,
                          to->x,
                          &user->bbox.xMin,
                          &user->bbox.xMax );

    if ( CHECK_Y( control1, user->bbox ) ||
         CHECK_Y( control2, user->bbox ) )

        BBox_Cubic_Check( user->last.y,
                          control1->y,
                          control2->y,
                          to->y,
                          &user->bbox.yMin,
                          &user->bbox.yMax );

    user->last = *to;

    return 0;
  }


  /* documentation is in ftbbox.h */

  FT_EXPORT_DEF( FT_Error )  FT_Outline_Get_BBox( FT_Outline*  outline,
                                                  FT_BBox     *abbox )
  {
    FT_BBox     cbox;
    FT_BBox     bbox;
    FT_Vector*  vec;
    FT_UShort   n;


    if ( !abbox )
      return FT_Err_Invalid_Argument;

    if ( !outline )
      return FT_Err_Invalid_Outline;

    /* if outline is empty, return (0,0,0,0) */
    if ( outline->n_points == 0 || outline->n_contours <= 0 )
    {
      abbox->xMin = abbox->xMax = 0;
      abbox->yMin = abbox->yMax = 0;
      return 0;
    }

    /* We compute the control box as well as the bounding box of  */
    /* all `on' points in the outline.  Then, if the two boxes    */
    /* coincide, we exit immediately.                             */

    vec = outline->points;
    bbox.xMin = bbox.xMax = cbox.xMin = cbox.xMax = vec->x;
    bbox.yMin = bbox.yMax = cbox.yMin = cbox.yMax = vec->y;

    for ( n = 1; n < outline->n_points; n++ )
    {
      FT_Pos  x = vec->x;
      FT_Pos  y = vec->y;


      /* update control box */
      if ( x < cbox.xMin ) cbox.xMin = x;
      if ( x > cbox.xMax ) cbox.xMax = x;

      if ( y < cbox.yMin ) cbox.yMin = y;
      if ( y > cbox.yMax ) cbox.yMax = y;

      if ( FT_CURVE_TAG( outline->tags[n] ) == FT_Curve_Tag_On )
      {
        /* update bbox for `on' points only */
        if ( x < bbox.xMin ) bbox.xMin = x;
        if ( x > bbox.xMax ) bbox.xMax = x;

        if ( y < bbox.yMin ) bbox.yMin = y;
        if ( y > bbox.yMax ) bbox.yMax = y;
      }

      vec++;
    }

    /* test two boxes for equality */
    if ( cbox.xMin < bbox.xMin || cbox.xMax > bbox.xMax ||
         cbox.yMin < bbox.yMin || cbox.yMax > bbox.yMax )
    {
      /* the two boxes are different, now walk over the outline to */
      /* get the Bezier arc extrema.                               */

      static const FT_Outline_Funcs  interface =
      {
        (FT_Outline_MoveTo_Func) BBox_Move_To,
        (FT_Outline_LineTo_Func) BBox_Move_To,
        (FT_Outline_ConicTo_Func)BBox_Conic_To,
        (FT_Outline_CubicTo_Func)BBox_Cubic_To,
        0, 0
      };

      FT_Error   error;
      TBBox_Rec  user;


      user.bbox = bbox;

      error = FT_Outline_Decompose( outline, &interface, &user );
      if ( error )
        return error;

      *abbox = user.bbox;
    }
    else
      *abbox = bbox;

    return FT_Err_Ok;
  }


/* END */
