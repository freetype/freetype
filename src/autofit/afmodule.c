#include "afmodule.h"
#include "afhints.h"
#include "afglobal.h"
#include "aflatin.h"

  static FT_Error
  af_hinter_load( AF_Hinter  hinter,
                  FT_UInt    glyph_index,
                  FT_Int32   load_flags,
                  FT_UInt    depth )
  {
    FT_Face           face     = hinter->face;
    FT_GlyphSlot      slot     = face->glyph;
    FT_Slot_Internal  internal = slot->internal;
    FT_Fixed          x_scale  = hinter->globals->x_scale;
    FT_Fixed          y_scale  = hinter->globals->y_scale;
    FT_Error          error;
    AF_Outline        outline  = hinter->glyph;
    AF_Loader         gloader  = hinter->loader;


    /* load the glyph */
    error = FT_Load_Glyph( face, glyph_index, load_flags );
    if ( error )
      goto Exit;

    /* Set `hinter->transformed' after loading with FT_LOAD_NO_RECURSE. */
    hinter->transformed = internal->glyph_transformed;

    if ( hinter->transformed )
    {
      FT_Matrix  imatrix;


      imatrix              = internal->glyph_matrix;
      hinter->trans_delta  = internal->glyph_delta;
      hinter->trans_matrix = imatrix;

      FT_Matrix_Invert( &imatrix );
      FT_Vector_Transform( &hinter->trans_delta, &imatrix );
    }

    /* set linear horizontal metrics */
    slot->linearHoriAdvance = slot->metrics.horiAdvance;
    slot->linearVertAdvance = slot->metrics.vertAdvance;

    switch ( slot->format )
    {
    case FT_GLYPH_FORMAT_OUTLINE:

      /* translate glyph outline if we need to */
      if ( hinter->transformed )
      {
        FT_UInt     n     = slot->outline.n_points;
        FT_Vector*  point = slot->outline.points;


        for ( ; n > 0; point++, n-- )
        {
          point->x += hinter->trans_delta.x;
          point->y += hinter->trans_delta.y;
        }
      }

      /* copy the outline points in the loader's current               */
      /* extra points which is used to keep original glyph coordinates */
      error = af_loader_check_points( gloader, slot->outline.n_points + 2,
                                      slot->outline.n_contours );
      if ( error )
        goto Exit;

      FT_ARRAY_COPY( gloader->current.extra_points, slot->outline.points,
                     slot->outline.n_points );

      FT_ARRAY_COPY( gloader->current.outline.contours, slot->outline.contours,
                     slot->outline.n_contours );

      FT_ARRAY_COPY( gloader->current.outline.tags, slot->outline.tags,
                     slot->outline.n_points );

      gloader->current.outline.n_points   = slot->outline.n_points;
      gloader->current.outline.n_contours = slot->outline.n_contours;

      /* compute original phantom points */
      hinter->pp1.x = 0;
      hinter->pp1.y = 0;
      hinter->pp2.x = FT_MulFix( slot->metrics.horiAdvance, x_scale );
      hinter->pp2.y = 0;

      /* be sure to check for spacing glyphs */
      if ( slot->outline.n_points == 0 )
        goto Hint_Metrics;

      /* now load the slot image into the auto-outline and run the */
      /* automatic hinting process                                 */
      error = af_outline_load( outline, x_scale, y_scale, face );
      if ( error )
        goto Exit;

      /* perform feature detection */
      af_outline_detect_features( outline );

      if ( hinter->do_vert_hints )
      {
        af_outline_compute_blue_edges( outline, hinter->globals );
        af_outline_scale_blue_edges( outline, hinter->globals );
      }

      /* perform alignment control */
      af_hinter_hint_edges( hinter );
      af_hinter_align( hinter );

      /* now save the current outline into the loader's current table */
      af_outline_save( outline, gloader );

      /* we now need to hint the metrics according to the change in */
      /* width/positioning that occured during the hinting process  */
      {
        FT_Pos   old_advance, old_rsb, old_lsb, new_lsb;
        AF_Edge  edge1 = outline->vert_edges;     /* leftmost edge  */
        AF_Edge  edge2 = edge1 +
                         outline->num_vedges - 1; /* rightmost edge */


        old_advance = hinter->pp2.x;
        old_rsb     = old_advance - edge2->opos;
        old_lsb     = edge1->opos;
        new_lsb     = edge1->pos;

        hinter->pp1.x = FT_PIX_ROUND( new_lsb    - old_lsb );
        hinter->pp2.x = FT_PIX_ROUND( edge2->pos + old_rsb );

#if 0
        /* try to fix certain bad advance computations */
        if ( hinter->pp2.x + hinter->pp1.x == edge2->pos && old_rsb > 4 )
          hinter->pp2.x += 64;
#endif
      }

      /* good, we simply add the glyph to our loader's base */
      af_loader_add( gloader );
      break;

    case FT_GLYPH_FORMAT_COMPOSITE:
      {
        FT_UInt      nn, num_subglyphs = slot->num_subglyphs;
        FT_UInt      num_base_subgs, start_point;
        FT_SubGlyph  subglyph;


        start_point = gloader->base.outline.n_points;

        /* first of all, copy the subglyph descriptors in the glyph loader */
        error = af_loader_check_subglyphs( gloader, num_subglyphs );
        if ( error )
          goto Exit;

        FT_ARRAY_COPY( gloader->current.subglyphs, slot->subglyphs,
                       num_subglyphs );

        gloader->current.num_subglyphs = num_subglyphs;
        num_base_subgs = gloader->base.num_subglyphs;

        /* now, read each subglyph independently */
        for ( nn = 0; nn < num_subglyphs; nn++ )
        {
          FT_Vector  pp1, pp2;
          FT_Pos     x, y;
          FT_UInt    num_points, num_new_points, num_base_points;


          /* gloader.current.subglyphs can change during glyph loading due */
          /* to re-allocation -- we must recompute the current subglyph on */
          /* each iteration                                                */
          subglyph = gloader->base.subglyphs + num_base_subgs + nn;

          pp1 = hinter->pp1;
          pp2 = hinter->pp2;

          num_base_points = gloader->base.outline.n_points;

          error = af_hinter_load( hinter, subglyph->index,
                                  load_flags, depth + 1 );
          if ( error )
            goto Exit;

          /* recompute subglyph pointer */
          subglyph = gloader->base.subglyphs + num_base_subgs + nn;

          if ( subglyph->flags & FT_SUBGLYPH_FLAG_USE_MY_METRICS )
          {
            pp1 = hinter->pp1;
            pp2 = hinter->pp2;
          }
          else
          {
            hinter->pp1 = pp1;
            hinter->pp2 = pp2;
          }

          num_points     = gloader->base.outline.n_points;
          num_new_points = num_points - num_base_points;

          /* now perform the transform required for this subglyph */

          if ( subglyph->flags & ( FT_SUBGLYPH_FLAG_SCALE    |
                                   FT_SUBGLYPH_FLAG_XY_SCALE |
                                   FT_SUBGLYPH_FLAG_2X2      ) )
          {
            FT_Vector*  cur   = gloader->base.outline.points +
                                num_base_points;
            FT_Vector*  org   = gloader->base.extra_points +
                                num_base_points;
            FT_Vector*  limit = cur + num_new_points;


            for ( ; cur < limit; cur++, org++ )
            {
              FT_Vector_Transform( cur, &subglyph->transform );
              FT_Vector_Transform( org, &subglyph->transform );
            }
          }

          /* apply offset */

          if ( !( subglyph->flags & FT_SUBGLYPH_FLAG_ARGS_ARE_XY_VALUES ) )
          {
            FT_Int      k = subglyph->arg1;
            FT_UInt     l = subglyph->arg2;
            FT_Vector*  p1;
            FT_Vector*  p2;


            if ( start_point + k >= num_base_points         ||
                               l >= (FT_UInt)num_new_points )
            {
              error = AF_Err_Invalid_Composite;
              goto Exit;
            }

            l += num_base_points;

            /* for now, only use the current point coordinates;    */
            /* we may consider another approach in the near future */
            p1 = gloader->base.outline.points + start_point + k;
            p2 = gloader->base.outline.points + start_point + l;

            x = p1->x - p2->x;
            y = p1->y - p2->y;
          }
          else
          {
            x = FT_MulFix( subglyph->arg1, x_scale );
            y = FT_MulFix( subglyph->arg2, y_scale );

            x = FT_PIX_ROUND(x);
            y = FT_PIX_ROUND(y);
          }

          {
            FT_Outline  dummy = gloader->base.outline;


            dummy.points  += num_base_points;
            dummy.n_points = (short)num_new_points;

            FT_Outline_Translate( &dummy, x, y );
          }
        }
      }
      break;

    default:
      /* we don't support other formats (yet?) */
      error = AF_Err_Unimplemented_Feature;
    }

  Hint_Metrics:
    if ( depth == 0 )
    {
      FT_BBox  bbox;


      /* transform the hinted outline if needed */
      if ( hinter->transformed )
        FT_Outline_Transform( &gloader->base.outline, &hinter->trans_matrix );

      /* we must translate our final outline by -pp1.x and compute */
      /* the new metrics                                           */
      if ( hinter->pp1.x )
        FT_Outline_Translate( &gloader->base.outline, -hinter->pp1.x, 0 );

      FT_Outline_Get_CBox( &gloader->base.outline, &bbox );
      bbox.xMin  = FT_PIX_FLOOR(  bbox.xMin );
      bbox.yMin  = FT_PIX_FLOOR(  bbox.yMin );
      bbox.xMax  = FT_PIX_CEIL( bbox.xMax );
      bbox.yMax  = FT_PIX_CEIL( bbox.yMax );

      slot->metrics.width        = bbox.xMax - bbox.xMin;
      slot->metrics.height       = bbox.yMax - bbox.yMin;
      slot->metrics.horiBearingX = bbox.xMin;
      slot->metrics.horiBearingY = bbox.yMax;

      /* for mono-width fonts (like Andale, Courier, etc.) we need */
      /* to keep the original rounded advance width                */
      if ( !FT_IS_FIXED_WIDTH( slot->face ) )
        slot->metrics.horiAdvance = hinter->pp2.x - hinter->pp1.x;
      else
        slot->metrics.horiAdvance = FT_MulFix( slot->metrics.horiAdvance,
                                               x_scale );

      slot->metrics.horiAdvance = FT_PIX_ROUND( slot->metrics.horiAdvance );

      /* now copy outline into glyph slot */
      af_loader_rewind( slot->internal->loader );
      error = af_loader_copy_points( slot->internal->loader, gloader );
      if ( error )
        goto Exit;

      slot->outline = slot->internal->loader->base.outline;
      slot->format  = FT_GLYPH_FORMAT_OUTLINE;
    }

#ifdef DEBUG_HINTER
    af_debug_hinter = hinter;
#endif

  Exit:
    return error;
  }


  /* load and hint a given glyph */
  FT_LOCAL_DEF( FT_Error )
  af_hinter_load_glyph( AF_Hinter     hinter,
                        FT_GlyphSlot  slot,
                        FT_Size       size,
                        FT_UInt       glyph_index,
                        FT_Int32      load_flags )
  {
    FT_Face          face         = slot->face;
    FT_Error         error;
    FT_Fixed         x_scale      = size->metrics.x_scale;
    FT_Fixed         y_scale      = size->metrics.y_scale;
    AF_Face_Globals  face_globals = FACE_GLOBALS( face );
    FT_Render_Mode   hint_mode    = FT_LOAD_TARGET_MODE( load_flags );


    /* first of all, we need to check that we're using the correct face and */
    /* global hints to load the glyph                                       */
    if ( hinter->face != face || hinter->globals != face_globals )
    {
      hinter->face = face;
      if ( !face_globals )
      {
        error = af_hinter_new_face_globals( hinter, face, 0 );
        if ( error )
          goto Exit;

      }
      hinter->globals = FACE_GLOBALS( face );
      face_globals    = FACE_GLOBALS( face );

    }

#ifdef FT_CONFIG_CHESTER_BLUE_SCALE

   /* try to optimize the y_scale so that the top of non-capital letters
    * is aligned on a pixel boundary whenever possible
    */
    {
      AF_Globals  design = &face_globals->design;
      FT_Pos      shoot  = design->blue_shoots[AF_BLUE_SMALL_TOP];


      /* the value of 'shoot' will be -1000 if the font doesn't have */
      /* small latin letters; we simply check the sign here...       */
      if ( shoot > 0 )
      {
        FT_Pos  scaled = FT_MulFix( shoot, y_scale );
        FT_Pos  fitted = FT_PIX_ROUND( scaled );


        if ( scaled != fitted )
        {
         /* adjust y_scale
          */
          y_scale = FT_MulDiv( y_scale, fitted, scaled );

         /* adust x_scale
          */
          if ( fitted < scaled )
            x_scale -= x_scale / 50;  /* x_scale*0.98 with integers */
        }
      }
    }

#endif /* FT_CONFIG_CHESTER_BLUE_SCALE */

    /* now, we must check the current character pixel size to see if we */
    /* need to rescale the global metrics                               */
    if ( face_globals->x_scale != x_scale ||
         face_globals->y_scale != y_scale )
      af_hinter_scale_globals( hinter, x_scale, y_scale );

    af_loader_rewind( hinter->loader );

    /* reset hinting flags according to load flags and current render target */
    hinter->do_horz_hints = FT_BOOL( !(load_flags & FT_LOAD_NO_AUTOHINT) );
    hinter->do_vert_hints = FT_BOOL( !(load_flags & FT_LOAD_NO_AUTOHINT) );

#ifdef DEBUG_HINTER
    hinter->do_horz_hints = !af_debug_disable_vert;  /* not a bug, the meaning */
    hinter->do_vert_hints = !af_debug_disable_horz;  /* of h/v is inverted!    */
#endif

    /* we snap the width of vertical stems for the monochrome and         */
    /* horizontal LCD rendering targets only.  Corresponds to X snapping. */
    hinter->do_horz_snapping = FT_BOOL( hint_mode == FT_RENDER_MODE_MONO ||
                                        hint_mode == FT_RENDER_MODE_LCD  );

    /* we snap the width of horizontal stems for the monochrome and     */
    /* vertical LCD rendering targets only.  Corresponds to Y snapping. */
    hinter->do_vert_snapping = FT_BOOL( hint_mode == FT_RENDER_MODE_MONO   ||
                                        hint_mode == FT_RENDER_MODE_LCD_V  );

    hinter->do_stem_adjust   = FT_BOOL( hint_mode != FT_RENDER_MODE_LIGHT );

    load_flags |= FT_LOAD_NO_SCALE
                | FT_LOAD_IGNORE_TRANSFORM;
    load_flags &= ~FT_LOAD_RENDER;

    error = af_hinter_load( hinter, glyph_index, load_flags, 0 );

  Exit:
    return error;
  }
