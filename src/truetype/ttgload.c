/***************************************************************************/
/*                                                                         */
/*  ttgload.c                                                              */
/*                                                                         */
/*    TrueType Glyph Loader (body).                                        */
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


#include <freetype.h>
#include <ftdebug.h>
#include <ftcalc.h>
#include <ftstream.h>

#include <sfnt.h>
#include <ttgload.h>

#include <tttags.h>

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#include <ttinterp.h>
#endif

  /* required for the tracing mode */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ttgload


  /*************************************************************************/
  /*                                                                       */
  /* Composite font flags.                                                 */
  /*                                                                       */
#define ARGS_ARE_WORDS       0x001
#define ARGS_ARE_XY_VALUES   0x002
#define ROUND_XY_TO_GRID     0x004
#define WE_HAVE_A_SCALE      0x008
/* reserved                  0x010 */
#define MORE_COMPONENTS      0x020
#define WE_HAVE_AN_XY_SCALE  0x040
#define WE_HAVE_A_2X2        0x080
#define WE_HAVE_INSTR        0x100
#define USE_MY_METRICS       0x200



  /*************************************************************************/
  /*    Returns the horizontal or vertical metrics in font units for a     */
  /*    given glyph.  The metrics are the left side bearing (resp. top     */
  /*    side bearing) and advance width (resp. advance height).            */
  /*                                                                       */
  LOCAL_FUNC
  void  TT_Get_Metrics( TT_HoriHeader*  header,
                        TT_UInt       index,
                        TT_Short*       bearing,
                        TT_UShort*      advance )
  {
    TT_LongMetrics*  longs_m;
    TT_UShort        k = header->number_Of_HMetrics;


    if ( index < k )
    {
      longs_m  = (TT_LongMetrics*)header->long_metrics + index;
      *bearing = longs_m->bearing;
      *advance = longs_m->advance;
    }
    else
    {
      *bearing = ((TT_ShortMetrics*)header->short_metrics)[index - k];
      *advance = ((TT_LongMetrics*)header->long_metrics)[k - 1].advance;
    }
  }


  /*************************************************************************/
  /*    Returns the horizontal metrics in font units for a given glyph.    */
  /*    If `check' is true, take care of monospaced fonts by returning the */
  /*    advance width maximum.                                             */
  /*                                                                       */
  static
  void Get_HMetrics( TT_Face     face,
                     TT_UInt     index,
                     TT_Bool     check,
                     TT_Short*   lsb,
                     TT_UShort*  aw )
  {
    TT_Get_Metrics( &face->horizontal, index, lsb, aw );

    if ( check && face->postscript.isFixedPitch )
      *aw = face->horizontal.advance_Width_Max;
  }


  /*************************************************************************/
  /*    Returns the advance width table for a given pixel size if it is    */
  /*    found in the font's `hdmx' table (if any).                         */
  /*                                                                       */
  static
  TT_Byte*  Get_Advance_Widths( TT_Face    face,
                                TT_UShort  ppem )
  {
    TT_UShort  n;


    for ( n = 0; n < face->hdmx.num_records; n++ )
      if ( face->hdmx.records[n].ppem == ppem )
        return face->hdmx.records[n].widths;

    return NULL;
  }


#define cur_to_org( n, zone )  \
          MEM_Copy( (zone)->org, (zone)->cur, n * sizeof ( TT_Vector ) )

#define org_to_cur( n, zone )  \
          MEM_Copy( (zone)->cur, (zone)->org, n * sizeof ( TT_Vector ) )


  /*************************************************************************/
  /*    Translates an array of coordinates.                                */
  /*                                                                       */
  static
  void  translate_array( TT_UInt     n,
                         TT_Vector*  coords,
                         TT_Pos      delta_x,
                         TT_Pos      delta_y )
  {
    TT_UInt  k;

    if ( delta_x )
      for ( k = 0; k < n; k++ )
        coords[k].x += delta_x;

    if ( delta_y )
      for ( k = 0; k < n; k++ )
        coords[k].y += delta_y;
  }



  /*************************************************************************/
  /*    Mounts one glyph zone on top of another.  This is needed to        */
  /*    assemble composite glyphs.                                         */
  /*                                                                       */
  static
  void  mount_zone( FT_GlyphZone*  source,
                    FT_GlyphZone*  target )
  {
    TT_UInt  np;
    TT_Int   nc;

    np = source->n_points;
    nc = source->n_contours;

    target->org   = source->org + np;
    target->cur   = source->cur + np;
    target->tags = source->tags + np;

    target->contours = source->contours + nc;

    target->n_points   = 0;
    target->n_contours = 0;
  }


#undef  IS_HINTED
#define IS_HINTED(flags)  ((flags & FT_LOAD_NO_HINTING) == 0)

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Load_Simple_Glyph                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a simple (i.e, non-composite) glyph.  This function is used  */
  /*    for the `Load_Simple' state of TT_Load_Glyph().  All composite     */
  /*    glyphs elements will be loaded with routine.                       */
  /*                                                                       */
  static
  TT_Error  Load_Simple( TT_Loader*       load,
                         TT_UInt          byte_count,
                         TT_Int           n_contours,
                         TT_Bool          debug )
  {
    TT_Error       error;
    FT_Stream      stream = load->stream;
    FT_GlyphZone*  zone   = &load->zone;
    TT_Face        face = load->face;

    TT_UShort      n_ins;
    TT_Int         n, n_points;
        
    /*********************************************************************/
    /* simple check                                                      */
    
    if ( n_contours > load->left_contours )
    {
      FT_TRACE0(( "ERROR: Glyph index %ld has %d contours > left %d\n",
                   load->glyph_index,
                   n_contours,
                   load->left_contours ));
      return TT_Err_Too_Many_Contours;
    }

    /* preparing the execution context */
    mount_zone( &load->base, zone );

    /*********************************************************************/
    /* reading the contours endpoints                                    */
    
    if ( ACCESS_Frame( byte_count ) )
      return error;

    for ( n = 0; n < n_contours; n++ )
      zone->contours[n] = GET_UShort();

    n_points = 0;
    if ( n_contours > 0 )
      n_points = zone->contours[n_contours - 1] + 1;


    /*********************************************************************/
    /* reading the bytecode instructions                                 */
    
    n_ins = GET_UShort();
    load->face->root.glyph->control_len = n_ins;

    if ( n_points > load->left_points )
    {
      FT_TRACE0(( "ERROR: Too many points in glyph %ld\n", load->glyph_index ));
      error = TT_Err_Too_Many_Points;
      goto Fail;
    }

    FT_TRACE4(( "Instructions size : %d\n", n_ins ));

    if ( n_ins > face->max_profile.maxSizeOfInstructions )
    {
      FT_TRACE0(( "ERROR: Too many instructions!\n" ));
      error = TT_Err_Too_Many_Ins;
      goto Fail;
    }

    if (stream->cursor + n_ins > stream->limit)
    {
      FT_TRACE0(( "ERROR: Instruction count mismatch!\n" ));
      error = TT_Err_Too_Many_Ins;
      goto Fail;
    }

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
    if ( (load->load_flags & (FT_LOAD_NO_SCALE|FT_LOAD_NO_HINTING))==0 )
    {
      MEM_Copy( load->exec->glyphIns, stream->cursor, n_ins );

      error = TT_Set_CodeRange( load->exec, tt_coderange_glyph,
                                load->exec->glyphIns, n_ins );
      if (error) goto Fail;
    }
#endif

    stream->cursor += n_ins;
    
    /*********************************************************************/
    /* reading the point tags                                           */

    {    
      TT_Byte*  flag  = load->zone.tags;
      TT_Byte*  limit = flag + n_points;
      TT_Byte   c, count;
      
      for (; flag < limit; flag++)
      {
        *flag = c = GET_Byte();
        if ( c & 8 )
        {
          for ( count = GET_Byte(); count > 0; count-- )
            *++flag = c;
        }
      }
    }

    /*********************************************************************/
    /* reading the X coordinates                                         */
    
    {
      TT_Vector*  vec   = zone->org;
      TT_Vector*  limit = vec + n_points;
      TT_Byte*    flag  = zone->tags;
      TT_Pos      x     = 0;
      
      for ( ; vec < limit; vec++, flag++ )
      {
        TT_Pos  y = 0;
        
        if ( *flag & 2 )
        {
          y = GET_Byte();
          if ((*flag & 16) == 0) y = -y;
        }
        else if ((*flag & 16) == 0)
          y = GET_Short();
          
        x     += y;
        vec->x = x;
      }
    }

    /*********************************************************************/
    /* reading the Y coordinates                                         */

    {
      TT_Vector*  vec   = zone->org;
      TT_Vector*  limit = vec + n_points;
      TT_Byte*    flag  = zone->tags;
      TT_Pos      x     = 0;
      
      for ( ; vec < limit; vec++, flag++ )
      {
        TT_Pos  y = 0;
        
        if ( *flag & 4 )
        {
          y = GET_Byte();
          if ((*flag & 32) == 0) y = -y;
        }
        else if ((*flag & 32) == 0)
          y = GET_Short();
          
        x     += y;
        vec->y = x;
      }
    }

    FORGET_Frame();

    /*********************************************************************/
    /* Add shadow points                                                  */
    
    /* Now add the two shadow points at n and n + 1.    */
    /* We need the left side bearing and advance width. */

    {
      TT_Vector*  pp1;
      TT_Vector*  pp2;
      
      /* pp1 = xMin - lsb */
      pp1    = zone->org + n_points;
      pp1->x = load->bbox.xMin - load->left_bearing;
      pp1->y = 0;
  
      /* pp2 = pp1 + aw */
      pp2    = pp1 + 1;
      pp2->x = pp1->x + load->advance;
      pp2->y = 0;
        
      /* clear the touch tags */
      for ( n = 0; n < n_points; n++ )
        zone->tags[n] &= FT_Curve_Tag_On;

      zone->tags[n_points    ] = 0;
      zone->tags[n_points + 1] = 0;
    }
    /* Note that we return two more points that are not */
    /* part of the glyph outline.                       */

    zone->n_points   = n_points;
    zone->n_contours = n_contours;
    n_points        += 2;

    /*******************************************/
    /* now eventually scale and hint the glyph */

    if (load->load_flags & FT_LOAD_NO_SCALE)
    {
      /* no scaling, just copy the orig arrays into the cur ones */
      org_to_cur( n_points, zone );
    }
    else
    {
      TT_Vector*  vec = zone->org;
      TT_Vector*  limit = vec + n_points;
      TT_Fixed    x_scale = load->size->root.metrics.x_scale;
      TT_Fixed    y_scale = load->size->root.metrics.y_scale;

      /* first scale the glyph points */
      for (; vec < limit; vec++)
      {
        vec->x = FT_MulFix( vec->x, x_scale );
        vec->y = FT_MulFix( vec->y, y_scale );
      }

      /* if hinting, round pp1, and shift the glyph accordingly */
      if ( !IS_HINTED(load->load_flags) )
      {
        org_to_cur( n_points, zone );
      }
      else
      {
        TT_Pos  x = zone->org[n_points-2].x;
        x = ((x + 32) & -64) - x;
        translate_array( n_points, zone->org, x, 0 );

        org_to_cur( n_points, zone );

        zone->cur[n_points-1].x = (zone->cur[n_points-1].x + 32) & -64;

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
        /* now consider hinting */
        if ( n_ins > 0 )
        {
          load->exec->is_composite     = FALSE;
          load->exec->pedantic_hinting = (TT_Bool)(load->load_flags & FT_LOAD_PEDANTIC);
          load->exec->pts              = *zone;
          load->exec->pts.n_points    += 2;

          error = TT_Run_Context( load->exec, debug );
          if ( error && load->exec->pedantic_hinting )
            return error;
        }
#endif
      }
    }

    /* save glyph phantom points */
    if ( !load->preserve_pps )
    {
      load->pp1 = zone->cur[n_points - 2];
      load->pp2 = zone->cur[n_points - 1];
    }

    return TT_Err_Ok;
    
  Fail:
    FORGET_Frame();
    return error;
  }






  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    load_truetype_glyph                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads a given truetype glyph. Handles composites and uses a        */
  /*    TT_Loader object..                                                 */
  /*                                                                       */
  static
  TT_Error  load_truetype_glyph( TT_Loader*  loader,
                                 TT_UInt     glyph_index )
  {
    FT_Stream    stream = loader->stream;
    TT_Error     error;
    TT_Face      face   = loader->face;
    TT_ULong     offset;
    FT_SubGlyph  subglyphs[ TT_MAX_SUBGLYPHS ];
    TT_Int       num_subglyphs = 0, contours_count;
    TT_UInt      index, num_points, num_contours, count;
    TT_Fixed     x_scale, y_scale;
    TT_ULong     ins_offset;
    
    /* check glyph index */
    index = glyph_index;
    if ( index >= (TT_UInt)face->root.num_glyphs )
    {
      error = TT_Err_Invalid_Glyph_Index;
      goto Fail;
    }

    loader->glyph_index = glyph_index;
    num_contours = 0;
    num_points   = 0;
    ins_offset   = 0;
    
    x_scale = 0x10000;
    y_scale = 0x10000;
    if ( (loader->load_flags & FT_LOAD_NO_SCALE)==0 )
    {
      x_scale = loader->size->root.metrics.x_scale;
      y_scale = loader->size->root.metrics.y_scale;
    }

    /* get horizontal metrics */
    {
      TT_Short   left_bearing;
      TT_UShort  advance_width;

      Get_HMetrics( face, index,
                    (TT_Bool)!(loader->load_flags &
                                 FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH),
                    &left_bearing,
                    &advance_width );

      loader->left_bearing = left_bearing;
      loader->advance      = advance_width;
    }

    /* load glyph header */
    offset = face->glyph_locations[index];
    count  = 0;
    if (index < (TT_UInt)face->num_locations-1)
       count = face->glyph_locations[index+1] - offset;
          

    if (count == 0)
    {
      /* as described by Frederic Loyer, these are spaces, and */
      /* not the unknown glyph.                                */
      loader->bbox.xMin = 0;
      loader->bbox.xMax = 0;
      loader->bbox.yMin = 0;
      loader->bbox.yMax = 0;

      loader->pp1.x = 0;
      loader->pp2.x = loader->advance;

      if ( (loader->load_flags & FT_LOAD_NO_SCALE)==0 )
        loader->pp2.x = FT_MulFix( loader->pp2.x, x_scale );

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
      if (loader->exec)
        loader->exec->glyphSize = 0;
#endif
      goto Load_End;
    }

    offset = loader->glyf_offset + offset;

    /* read first glyph header */
    if ( FILE_Seek( offset ) || ACCESS_Frame( 10L ) )
      goto Fail;

    contours_count = GET_Short();

    loader->bbox.xMin = GET_Short();
    loader->bbox.yMin = GET_Short();
    loader->bbox.xMax = GET_Short();
    loader->bbox.yMax = GET_Short();

    FORGET_Frame();

    FT_TRACE6(( "Glyph %ld\n", index ));
    FT_TRACE6(( " # of contours : %d\n", num_contours ));
    FT_TRACE6(( " xMin: %4d  xMax: %4d\n", loader->bbox.xMin,
                                           loader->bbox.xMax ));
    FT_TRACE6(( " yMin: %4d  yMax: %4d\n", loader->bbox.yMin,
                                           loader->bbox.yMax ));
    FT_TRACE6(( "-" ));

    count -= 10;

    if ( contours_count > loader->left_contours )
    {
      FT_TRACE0(( "ERROR: Too many contours for glyph %ld\n", index ));
      error = TT_Err_Too_Many_Contours;
      goto Fail;
    }

    loader->pp1.x = loader->bbox.xMin - loader->left_bearing;
    loader->pp1.y = 0;
    loader->pp2.x = loader->pp1.x + loader->advance;
    loader->pp2.y = 0;
    
    if ((loader->load_flags & FT_LOAD_NO_SCALE)==0)
    {
      loader->pp1.x = FT_MulFix( loader->pp1.x, x_scale );
      loader->pp2.x = FT_MulFix( loader->pp2.x, x_scale );
    }

    /*************************************************************************/
    /*************************************************************************/
    /*************************************************************************/

    /**********************************************************************/
    /* if it is a simple glyph, load it                                   */
    if (contours_count >= 0)
    {
      TT_UInt  num_base_points;
      
#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
      error = Load_Simple( loader,
                           count,
                           contours_count,
                           (TT_Bool)( loader->size &&
                                      loader->size->debug ) );
#else
      error = Load_Simple( loader, count, contours_count, 0 );
#endif                                      
      if ( error )
        goto Fail;

      /* Note: We could have put the simple loader source there */
      /*       but the code is fat enough already :-)           */
      num_points   = loader->zone.n_points;
      num_contours = loader->zone.n_contours;
      
      num_base_points = loader->base.n_points;
      {
        TT_UInt  k;
        for ( k = 0; k < num_contours; k++ )
          loader->zone.contours[k] += num_base_points;
      }

      loader->base.n_points   += num_points;
      loader->base.n_contours += num_contours;

      loader->zone.n_points    = 0;
      loader->zone.n_contours  = 0;

      loader->left_points   -= num_points;
      loader->left_contours -= num_contours;
    }
    /*************************************************************************/
    /*************************************************************************/
    /*************************************************************************/

    /************************************************************************/
    else /* otherwise, load a composite !!                                  */
    {
      /* for each subglyph, read composite header */
      FT_SubGlyph*  subglyph = subglyphs;
      
      if (ACCESS_Frame(count)) goto Fail;
      
      num_subglyphs = 0;
      do
      {
        TT_Fixed  xx, xy, yy, yx;
        
        subglyph->arg1 = subglyph->arg2 = 0;
        
        subglyph->flags = GET_UShort();
        subglyph->index = GET_UShort();

        /* read arguments */
        if (subglyph->flags & ARGS_ARE_WORDS)
        {
          subglyph->arg1 = GET_Short();
          subglyph->arg2 = GET_Short();
        }
        else
        {
          subglyph->arg1 = GET_Char();
          subglyph->arg2 = GET_Char();
        }
        
        /* read transform */
        xx = yy = 0x10000;
        xy = yx = 0;
        
        if (subglyph->flags & WE_HAVE_A_SCALE)
        {
          xx = (TT_Fixed)GET_Short() << 2;
          yy = xx;
        }
        else if (subglyph->flags & WE_HAVE_AN_XY_SCALE)
        {
          xx = (TT_Fixed)GET_Short() << 2;
          yy = (TT_Fixed)GET_Short() << 2;
        }
        else if (subglyph->flags & WE_HAVE_A_2X2)
        {
          xx = (TT_Fixed)GET_Short() << 2;
          xy = (TT_Fixed)GET_Short() << 2;
          yx = (TT_Fixed)GET_Short() << 2;
          yy = (TT_Fixed)GET_Short() << 2;
        }
        
        subglyph->transform.xx = xx;
        subglyph->transform.xy = xy;
        subglyph->transform.yx = yx;
        subglyph->transform.yy = yy;
        
        subglyph++;
        num_subglyphs++;
        if (num_subglyphs >= TT_MAX_SUBGLYPHS)
          break;
      }
      while (subglyph[-1].flags & MORE_COMPONENTS);

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
      {
        /* we must undo the ACCESS_Frame in order to point to the */
        /* composite instructions, if we find some ..             */
        /* we will process them later..                           */
        ins_offset = FILE_Pos() + stream->cursor - stream->limit;
      }
#endif
      FORGET_Frame();

      /* if the flag FT_LOAD_NO_RECURSE is set, we return the subglyph */
      /* "as is" in the glyph slot (the client application will be     */
      /* responsible for interpreting this data..)                     */
      if ( loader->load_flags & FT_LOAD_NO_RECURSE )
      {
        FT_GlyphSlot  glyph = loader->glyph;

        /* reallocate subglyph array if necessary */        
        if (glyph->max_subglyphs < num_subglyphs)
        {
          FT_Memory  memory = loader->face->root.memory;
          
          if ( REALLOC_ARRAY( glyph->subglyphs, glyph->max_subglyphs,
                              num_subglyphs, FT_SubGlyph ) )
            goto Fail;
            
          glyph->max_subglyphs = num_subglyphs;
        }

        /* copy subglyph array */
        MEM_Copy( glyph->subglyphs, subglyphs,
                  num_subglyphs*sizeof(FT_SubGlyph));
                  
        /* set up remaining glyph fields */
        glyph->num_subglyphs = num_subglyphs;
        glyph->format        = ft_glyph_format_composite;
        goto Load_End;
      }


    /*************************************************************************/
    /*************************************************************************/
    /*************************************************************************/

      /*********************************************************************/
      /* Now, read each subglyph independently..                           */
      {
        TT_Int  n, num_base_points, num_new_points;
        
        subglyph = subglyphs;
        for ( n = 0; n < num_subglyphs; n++, subglyph++ )
        {
          TT_Vector  pp1, pp2;
          TT_Pos     x, y;
          
          pp1 = loader->pp1;
          pp2 = loader->pp2;

          num_base_points = loader->base.n_points;
          
          error = load_truetype_glyph( loader, subglyph->index );
          if ( subglyph->flags & USE_MY_METRICS )
          {
            pp1 = loader->pp1;
            pp2 = loader->pp2;
          }
          else
          {
            loader->pp1 = pp1;
            loader->pp2 = pp2;
          }
          
          num_points   = loader->base.n_points;
          num_contours = loader->base.n_contours;
          
          num_new_points = num_points - num_base_points;
          
          /********************************************************/
          /* now perform the transform required for this subglyph */
          
          if ( subglyph->flags & ( WE_HAVE_A_SCALE     |
                                   WE_HAVE_AN_XY_SCALE |
                                   WE_HAVE_A_2X2       ) )
          {
            TT_Vector*  cur = loader->zone.cur;
            TT_Vector*  org = loader->zone.org;
            TT_Vector*  limit = cur + num_new_points;

            for ( ; cur < limit; cur++, org++ )
            {
              TT_Pos  nx, ny;
              
              nx = FT_MulFix( cur->x, subglyph->transform.xx ) +
                   FT_MulFix( cur->y, subglyph->transform.yx );

              ny = FT_MulFix( cur->x, subglyph->transform.xy ) +
                   FT_MulFix( cur->y, subglyph->transform.yy );

              cur->x = nx;
              cur->y = ny;

              nx = FT_MulFix( org->x, subglyph->transform.xx ) +
                   FT_MulFix( org->y, subglyph->transform.yx );

              ny = FT_MulFix( org->x, subglyph->transform.xy ) +
                   FT_MulFix( org->y, subglyph->transform.yy );

              org->x = nx;
              org->y = ny;
            }
          }

          /* apply offset */

          if ( !(subglyph->flags & ARGS_ARE_XY_VALUES) )
          {
            TT_Int   k = subglyph->arg1;
            TT_UInt  l = subglyph->arg2;

            if ( k >= num_base_points ||
                 l >= (TT_UInt)num_new_points  )
            {
              error = TT_Err_Invalid_Composite;
              goto Fail;
            }

            l += num_base_points; 
	    
            x = loader->base.cur[k].x - loader->base.cur[l].x;
            y = loader->base.cur[k].y - loader->base.cur[l].y;
          }
          else
          {
            x = subglyph->arg1;
            y = subglyph->arg2;

            if (!(loader->load_flags & FT_LOAD_NO_SCALE))
            {
              x = FT_MulFix( x, x_scale );
              y = FT_MulFix( y, y_scale );
              
              if ( subglyph->flags & ROUND_XY_TO_GRID )
	          {
	            x = (x + 32) & -64;
	            y = (y + 32) & -64;
	          }
            }
          }

          translate_array( num_new_points, loader->zone.cur, x, y );
          cur_to_org( num_new_points, &loader->zone );
        }
        
    /*************************************************************************/
    /*************************************************************************/
    /*************************************************************************/

        /* we have finished loading all sub-glyphs, now, look for */
        /* instructions for this composite !!                     */

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
        subglyph--;
        if (num_subglyphs > 0 && loader->exec && subglyph->flags & WE_HAVE_INSTR)
        {
          TT_UShort       n_ins;
          TT_ExecContext  exec = loader->exec;
          TT_UInt         n_points = loader->base.n_points;
          FT_GlyphZone*   pts;
          TT_Vector*      pp1;
          
          /* read size of instructions */
          if ( FILE_Seek( ins_offset ) ||
               READ_UShort(n_ins)      ) goto Fail;
          FT_TRACE4(( "Instructions size = %d\n", n_ins ));
    
          /* check it */
          if ( n_ins > face->max_profile.maxSizeOfInstructions )
          {
            FT_TRACE0(( "Too many instructions in composite glyph %ld\n",
                        subglyph->index ));
            return TT_Err_Too_Many_Ins;
          }
     
          if (exec)
          {
          }
          /* read the instructions */
          if ( FILE_Read( exec->glyphIns, n_ins ) )
            goto Fail;
    
          error = TT_Set_CodeRange( exec,
                                    tt_coderange_glyph,
                                    exec->glyphIns,
                                    n_ins );
          if ( error ) goto Fail;
          
          /* prepare the execution context */
          exec->pts   = loader->base;
          pts         = &exec->pts;
      
          pts->n_points   = num_points + 2;
          pts->n_contours = num_contours;
      
          /* add phantom points */
          pp1    = pts->cur + num_points;
          pp1[0] = loader->pp1;
          pp1[1] = loader->pp2;
      
          pts->tags[num_points + 1] = 0;
          pts->tags[num_points + 2] = 0;
      
          /* if hinting, round the phantom points */
          if ( IS_HINTED(loader->load_flags) )
          {
            pp1[0].x = ((loader->pp1.x + 32) & -64);
            pp1[1].x = ((loader->pp2.x + 32) & -64);
          }
      
          {
            TT_UInt  k;
            for ( k = 0; k < n_points; k++ )
              pts->tags[k] &= FT_Curve_Tag_On;
          }
      
          cur_to_org( n_points, pts );
      
          /* now consider hinting */
          if ( IS_HINTED(loader->load_flags) && n_ins > 0 )
          {
            exec->is_composite     = TRUE;
            exec->pedantic_hinting =
                (TT_Bool)(loader->load_flags & FT_LOAD_PEDANTIC);
      
            error = TT_Run_Context( exec, loader->size->debug );
            if ( error && exec->pedantic_hinting )
              goto Fail;
          }
          
          /* save glyph origin and advance points */
          loader->pp1 = pp1[0];
          loader->pp2 = pp1[1];
        }
#endif
          
      }
    }

    /*************************************************************************/
    /*************************************************************************/
    /*************************************************************************/
    /*************************************************************************/

  Load_End:
    error = TT_Err_Ok;

  Fail:
    return error;    
  }





  static
  void  compute_glyph_metrics( TT_Loader*    loader,
                               TT_UInt       glyph_index )
  {
    TT_UInt       num_points   = loader->base.n_points;
    TT_UInt       num_contours = loader->base.n_contours;
    TT_BBox       bbox;
    TT_Face       face = loader->face;
    TT_Fixed      x_scale, y_scale;
    TT_GlyphSlot  glyph = loader->glyph;
    TT_Size       size = loader->size;
    
    /* when a simple glyph was loaded, the value of        */
    /* "base.n_points" and "base.n_contours" is 0, we must */
    /* take those in the "zone" instead..                  */
    if ( num_points == 0 && num_contours == 0 )
    {
      num_points   = loader->zone.n_points;
      num_contours = loader->zone.n_contours;
    }
    
    x_scale = 0x10000;
    y_scale = 0x10000;
    if ( (loader->load_flags & FT_LOAD_NO_SCALE) == 0)
    {
      x_scale = size->root.metrics.x_scale;
      y_scale = size->root.metrics.y_scale;
    }
    
    if ( glyph->format != ft_glyph_format_composite )
    {
      TT_UInt  u;
      for ( u = 0; u < num_points + 2; u++ )
      {
        glyph->outline.points[u] = loader->base.cur[u];
        glyph->outline.tags [u] = loader->base.tags[u];
      }

      for ( u = 0; u < num_contours; u++ )
        glyph->outline.contours[u] = loader->base.contours[u];

      /* glyph->outline.second_pass = TRUE; */
      glyph->outline.flags      &= ~ft_outline_single_pass;
      glyph->outline.n_points    = num_points;
      glyph->outline.n_contours  = num_contours;
  
      /* translate array so that (0,0) is the glyph's origin */
      translate_array( (TT_UShort)(num_points + 2),
                       glyph->outline.points,
                       -loader->pp1.x,
                       0 );
  
      FT_Outline_Get_CBox( &glyph->outline, &bbox );

      if ( IS_HINTED(loader->load_flags) )
      {
        /* grid-fit the bounding box */
        bbox.xMin &= -64;
        bbox.yMin &= -64;
        bbox.xMax  = (bbox.xMax + 63) & -64;
        bbox.yMax  = (bbox.yMax + 63) & -64;
      }
    }
    else
      bbox = loader->bbox;

    /* get the device-independent scaled horizontal metrics */
    /* take care of fixed-pitch fonts...                    */
    {
      TT_Pos  left_bearing;
      TT_Pos  advance;

      left_bearing = loader->left_bearing;
      advance      = loader->advance;

     /* the flag FT_LOAD_NO_ADVANCE_CHECK was introduced to       */
     /* correctly support DynaLab fonts, who have an incorrect    */
     /* "advance_Width_Max" field !! It is used, to my knowledge  */
     /* exclusively in the X-TrueType font server..               */
     /*                                                           */
      if ( face->postscript.isFixedPitch                        &&
           (loader->load_flags & FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH) == 0 )
        advance = face->horizontal.advance_Width_Max;

      if ( !(loader->load_flags & FT_LOAD_NO_SCALE) )
      {
        left_bearing = FT_MulFix( left_bearing, x_scale );
        advance      = FT_MulFix( advance, x_scale );
      }

      glyph->metrics2.horiBearingX = left_bearing;
      glyph->metrics2.horiAdvance  = advance;
    }

    glyph->metrics.horiBearingX = bbox.xMin;
    glyph->metrics.horiBearingY = bbox.yMax;
    glyph->metrics.horiAdvance  = loader->pp2.x - loader->pp1.x;

    /* Now take care of vertical metrics.  In the case where there is    */
    /* no vertical information within the font (relatively common), make */
    /* up some metrics by `hand'...                                      */

    {
      TT_Short   top_bearing;    /* vertical top side bearing (EM units) */
      TT_UShort  advance_height; /* vertical advance height   (EM units) */

      TT_Pos  left;     /* scaled vertical left side bearing         */
      TT_Pos  Top;      /* scaled original vertical top side bearing */
      TT_Pos  top;      /* scaled vertical top side bearing          */
      TT_Pos  advance;  /* scaled vertical advance height            */


      /* Get the unscaled `tsb' and `ah' */
      if ( face->vertical_info                   &&
           face->vertical.number_Of_VMetrics > 0 )
      {
        /* Don't assume that both the vertical header and vertical */
        /* metrics are present in the same font :-)                */

        TT_Get_Metrics( (TT_HoriHeader*)&face->vertical,
                        glyph_index,
                        &top_bearing,
                        &advance_height );
      }
      else
      {
        /* Make up the distances from the horizontal header..     */

        /* NOTE: The OS/2 values are the only `portable' ones,    */
        /*       which is why we use them, when there is an       */
        /*       OS/2 table in the font. Otherwise, we use the    */
        /*       values defined in the horizontal header..        */
        /*                                                        */
        /* NOTE2: The sTypoDescender is negative, which is why    */
        /*        we compute the baseline-to-baseline distance    */
        /*        here with:                                      */
        /*             ascender - descender + linegap             */
        /*                                                        */
        if ( face->os2.version != 0xFFFF )
        {
          top_bearing    = face->os2.sTypoLineGap / 2;
          advance_height = (TT_UShort)(face->os2.sTypoAscender -
                                       face->os2.sTypoDescender +
                                       face->os2.sTypoLineGap);
        }
        else
        {
          top_bearing    = face->horizontal.Line_Gap / 2;
          advance_height = (TT_UShort)(face->horizontal.Ascender  +
                                       face->horizontal.Descender +
                                       face->horizontal.Line_Gap);
        }
      }

      /* We must adjust the top_bearing value from the bounding box given
         in the glyph header to te bounding box calculated with
         TT_Get_Outline_BBox()                                            */

      /* scale the metrics */
      if ( !(loader->load_flags & FT_LOAD_NO_SCALE) )
      {
        Top     = FT_MulFix( top_bearing, y_scale );
        top     = FT_MulFix( top_bearing + loader->bbox.yMax, y_scale )
                    - bbox.yMax;
        advance = FT_MulFix( advance_height, y_scale );
      }
      else
      {
        Top     = top_bearing;
        top     = top_bearing + loader->bbox.yMax - bbox.yMax;
        advance = advance_height;
      }

      glyph->metrics2.vertBearingY = Top;
      glyph->metrics2.vertAdvance  = advance;

      /* XXX: for now, we have no better algorithm for the lsb, but it    */
      /*      should work fine.                                           */
      /*                                                                  */
      left = ( bbox.xMin - bbox.xMax ) / 2;

      /* grid-fit them if necessary */
      if ( IS_HINTED(loader->load_flags) )
      {
        left   &= -64;
        top     = (top + 63) & -64;
        advance = (advance + 32) & -64;
      }

      glyph->metrics.vertBearingX = left;
      glyph->metrics.vertBearingY = top;
      glyph->metrics.vertAdvance  = advance;
    }

    /* Adjust advance width to the value contained in the hdmx table. */
    if ( !face->postscript.isFixedPitch && size &&
         IS_HINTED(loader->load_flags) )
    {
      TT_Byte* widths = Get_Advance_Widths( face,
                                   size->root.metrics.x_ppem );
      if ( widths )
        glyph->metrics.horiAdvance = widths[glyph_index] << 6;
    }

/* drop-out mode is irrelevant, we always use mode 2 */
#if 0
#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
    if (loader->exec)
      glyph->outline.dropout_mode = (TT_Char)loader->exec->GS.scan_type;
#else
    glyph->outline.dropout_mode = 2;
#endif
#endif

    /* set glyph dimensions */
    glyph->metrics.width  = bbox.xMax - bbox.xMin;
    glyph->metrics.height = bbox.yMax - bbox.yMin;

  }












  LOCAL_FUNC
  TT_Error  TT_Load_Glyph( TT_Size       size,
                           TT_GlyphSlot  glyph,
                           TT_UShort     glyph_index,
                           TT_UInt       load_flags )
  {
    SFNT_Interface*  sfnt;
    TT_Face          face;
    FT_Stream        stream;
    FT_Memory        memory;
    TT_Error         error;
    TT_Loader        loader;
    FT_GlyphZone*    zone;
    
    face   = (TT_Face)glyph->face;
    sfnt   = (SFNT_Interface*)face->sfnt;
    stream = face->root.stream;
    memory = face->root.memory;
    error  = 0;

    if ( !size || (load_flags & FT_LOAD_NO_SCALE)  ||
                  (load_flags & FT_LOAD_NO_RECURSE ))
    {
      size        = NULL;
      load_flags |= FT_LOAD_NO_SCALE   |
                    FT_LOAD_NO_HINTING |
                    FT_LOAD_NO_BITMAP;
    }

    glyph->num_subglyphs = 0;

#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS
    /*********************************************************************/
    /* Try to load embedded bitmap if any                                */
    if ( size && (load_flags & FT_LOAD_NO_BITMAP) == 0 && sfnt->load_sbits )
    {
      TT_SBit_Metrics  metrics;
 
      error = sfnt->load_sbit_image( face,
                                     size->root.metrics.x_ppem,
                                     size->root.metrics.y_ppem,
                                     glyph_index,
                                     stream,
                                     &glyph->bitmap,
                                     &metrics );
      if ( !error )
      {
        glyph->outline.n_points   = 0;
        glyph->outline.n_contours = 0;

        glyph->metrics.width  = (TT_Pos)metrics.width  << 6;
        glyph->metrics.height = (TT_Pos)metrics.height << 6;

        glyph->metrics.horiBearingX = (TT_Pos)metrics.horiBearingX << 6;
        glyph->metrics.horiBearingY = (TT_Pos)metrics.horiBearingY << 6;
        glyph->metrics.horiAdvance  = (TT_Pos)metrics.horiAdvance  << 6;

        glyph->metrics.vertBearingX = (TT_Pos)metrics.vertBearingX << 6;
        glyph->metrics.vertBearingY = (TT_Pos)metrics.vertBearingY << 6;
        glyph->metrics.vertAdvance  = (TT_Pos)metrics.vertAdvance  << 6;

        glyph->format = ft_glyph_format_bitmap;
        return error;
      }
    }
#endif /* TT_CONFIG_OPTION_EMBEDDED_BITMAPS */

    if ( load_flags & FT_LOAD_NO_OUTLINE )
      return ( error ? error : TT_Err_Unavailable_Bitmap );

   /* seek to the beginning of the glyph table. For Type 43 fonts       */
   /* the table might be accessed from a Postscript stream or something */
   /* else...                                                           */
    error = face->goto_table( face, TTAG_glyf, stream, 0 );
    if (error)
    {
      FT_ERROR(( "TT.GLoad: could not access glyph table\n" ));
      goto Exit;
    }

    MEM_Set( &loader, 0, sizeof(loader) );

    /* update the glyph zone bounds */
    zone   = &((TT_Driver)face->root.driver)->zone;
    error  = FT_Update_GlyphZone( zone,
                                  face->root.max_points,
                                  face->root.max_contours );
    if (error)
    {
      FT_ERROR(( "TT.GLoad: could not update loader glyph zone\n" ));
      goto Exit;
    }
    loader.base = *zone;
    
    loader.zone.n_points   = 0;
    loader.zone.n_contours = 0;

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
    if ( size )
    {
      /* query new execution context */
      loader.exec = size->debug ? size->context : TT_New_Context(face);
      if ( !loader.exec )
        return TT_Err_Could_Not_Find_Context;
        
      TT_Load_Context( loader.exec, face, size );

      /* load default graphics state - if needed */
      if ( size->GS.instruct_control & 2 )
        loader.exec->GS = tt_default_graphics_state;
    }
#endif /* TT_CONFIG_OPTION_BYTECODE_INTERPRETER */

    /* clear all outline flags, except the "owner" one */
    glyph->outline.flags &= ft_outline_owner;
    
    if (size && size->root.metrics.y_ppem < 24 )
      glyph->outline.flags |= ft_outline_high_precision;

    /************************************************************************/
    /* let's initialise the rest of our loader now                          */
    loader.left_points   = face->root.max_points;
    loader.left_contours = face->root.max_contours;
    loader.load_flags    = load_flags;

    loader.face   = face;
    loader.size   = size;
    loader.glyph  = glyph;
    loader.stream = stream;
    
    loader.glyf_offset = FILE_Pos();

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
    /* when the cvt program has disabled hinting, the argument */
    /* is ignored.                                             */
    if ( size && (size->GS.instruct_control & 1) )
      loader.load_flags |= FT_LOAD_NO_HINTING;
#endif

    /* Main loading loop */
    glyph->format = ft_glyph_format_outline;
    error = load_truetype_glyph( &loader, glyph_index );
    if (!error)
      compute_glyph_metrics( &loader, glyph_index );

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
    if ( !size || !size->debug )
      TT_Done_Context( loader.exec );
#endif

  Exit:
    return error;
  }



/* END */
