/***************************************************************************/
/*                                                                         */
/*  ftsynth.c                                                              */
/*                                                                         */
/*    FreeType synthesizing code for emboldening and slanting (body).      */
/*                                                                         */
/*  Copyright 2000-2001, 2002, 2003, 2004 by                               */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_CALC_H
#include FT_OUTLINE_H
#include FT_TRIGONOMETRY_H
#include FT_SYNTHESIS_H


#define FT_BOLD_THRESHOLD  0x0100


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****   EXPERIMENTAL OBLIQUING SUPPORT                                ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/

  /* documentation is in ftsynth.h */

  FT_EXPORT_DEF( void )
  FT_GlyphSlot_Oblique( FT_GlyphSlot  slot )
  {
    FT_Matrix    transform;
    FT_Outline*  outline = &slot->outline;


    /* only oblique outline glyphs */
    if ( slot->format != FT_GLYPH_FORMAT_OUTLINE )
      return;

    /* we don't touch the advance width */

    /* For italic, simply apply a shear transform, with an angle */
    /* of about 12 degrees.                                      */

    transform.xx = 0x10000L;
    transform.yx = 0x00000L;

    transform.xy = 0x06000L;
    transform.yy = 0x10000L;

    FT_Outline_Transform( outline, &transform );
  }


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****   EXPERIMENTAL EMBOLDENING/OUTLINING SUPPORT                    ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/


  /* documentation is in ftsynth.h */

  FT_EXPORT_DEF( void )
  FT_GlyphSlot_Embolden( FT_GlyphSlot  slot )
  {
    FT_Vector*   points;
    FT_Vector    v_prev, v_first, v_next, v_cur;
    FT_Pos       distance;
    FT_Outline*  outline = &slot->outline;
    FT_Face      face = FT_SLOT_FACE( slot );
    FT_Angle     rotate, angle_in, angle_out;
    FT_Int       c, n, first;


    /* only embolden outline glyph images */
    if ( slot->format != FT_GLYPH_FORMAT_OUTLINE )
      return;

    /* compute control distance */
    distance = FT_MulFix( face->units_per_EM / 60,
                          face->size->metrics.y_scale );

    if ( FT_Outline_Get_Orientation( outline ) == FT_ORIENTATION_TRUETYPE )
      rotate = -FT_ANGLE_PI2;
    else
      rotate = FT_ANGLE_PI2;

    points = outline->points;

    first = 0;
    for ( c = 0; c < outline->n_contours; c++ )
    {
      int  last = outline->contours[c];


      v_first = points[first];
      v_prev  = points[last];
      v_cur   = v_first;

      for ( n = first; n <= last; n++ )
      {
        FT_Pos     d;
        FT_Vector  in, out;
        FT_Fixed   scale;
        FT_Angle   angle_diff;


        if ( n < last ) v_next = points[n + 1];
        else            v_next = v_first;

        /* compute the in and out vectors */
        in.x  = v_cur.x - v_prev.x;
        in.y  = v_cur.y - v_prev.y;

        out.x = v_next.x - v_cur.x;
        out.y = v_next.y - v_cur.y;

        angle_in   = FT_Atan2( in.x, in.y );
        angle_out  = FT_Atan2( out.x, out.y );
        angle_diff = FT_Angle_Diff( angle_in, angle_out );
        scale      = FT_Cos( angle_diff/2 );

        if ( scale < 0x400L && scale > -0x400L )
        {
          if ( scale >= 0 )
            scale = 0x400L;
          else
            scale = -0x400L;
        }

        d = FT_DivFix( distance, scale );

        FT_Vector_From_Polar( &in, d, angle_in + angle_diff/2 - rotate );

        outline->points[n].x = v_cur.x + distance + in.x;
        outline->points[n].y = v_cur.y + distance + in.y;

        v_prev = v_cur;
        v_cur  = v_next;
      }

      first = last + 1;
    }

    slot->metrics.horiAdvance =
      ( slot->metrics.horiAdvance + distance*4 ) & ~63;
  }


/* END */
