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
#include <ttinterp.h>


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


#undef  SCALE_X
#define SCALE_X( distance )  FT_MulFix( distance, exec->metrics.x_scale )

#undef  SCALE_Y
#define SCALE_Y( distance )  FT_MulFix( distance, exec->metrics.y_scale )


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Get_Metrics                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the horizontal or vertical metrics in font units for a     */
  /*    given glyph.  The metrics are the left side bearing (resp. top     */
  /*    side bearing) and advance width (resp. advance height).            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    header  :: A pointer to either the horizontal or vertical metrics  */
  /*               structure.                                              */
  /*                                                                       */
  /*    index   :: The glyph index.                                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    bearing :: The bearing, either left side or top side.              */
  /*                                                                       */
  /*    advance :: The advance width resp. advance height.                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function will much probably move to another component in the  */
  /*    near future, but I haven't decided which yet.                      */
  /*                                                                       */
  LOCAL_FUNC
  void  TT_Get_Metrics( TT_HoriHeader*  header,
                        TT_UShort       index,
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
  /*                                                                       */
  /* <Function>                                                            */
  /*    Get_HMetrics                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the horizontal metrics in font units for a given glyph.    */
  /*    If `check' is true, take care of monospaced fonts by returning the */
  /*    advance width maximum.                                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face  :: A handle to the target source face.                       */
  /*                                                                       */
  /*    index :: The glyph index.                                          */
  /*                                                                       */
  /*    check :: If set, handle monospaced fonts.                          */
  /*                                                                       */
  /* <Output>                                                              */
  /*    lsb   :: The left side bearing.                                    */
  /*                                                                       */
  /*    aw    :: The advance width.                                        */
  /*                                                                       */
  static
  void Get_HMetrics( TT_Face     face,
                     TT_UShort   index,
                     TT_Bool     check,
                     TT_Short*   lsb,
                     TT_UShort*  aw )
  {
    TT_Get_Metrics( &face->horizontal, index, lsb, aw );

    if ( check && face->postscript.isFixedPitch )
      *aw = face->horizontal.advance_Width_Max;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Get_Advance_Widths                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the advance width table for a given pixel size if it is    */
  /*    found in the font's `hdmx' table (if any).                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to the target source face.                        */
  /*                                                                       */
  /*    ppem :: The pixel size.                                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*   A pointer to the advance with table.  NULL if it doesn't exist.     */
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
  /*                                                                       */
  /* <Function>                                                            */
  /*    translate_array                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Translates an array of coordinates.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    n       :: The number of points to translate.                      */
  /*                                                                       */
  /*    delta_x :: The horizontal coordinate of the shift vector.          */
  /*                                                                       */
  /*    delta_y :: The vertical coordinate of the shift vector.            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    coords  :: The vector array to translate.                          */
  /*                                                                       */
  static
  void  translate_array( TT_UShort   n,
                         TT_Vector*  coords,
                         TT_Pos      delta_x,
                         TT_Pos      delta_y )
  {
    TT_UShort  k;


    if ( delta_x )
      for ( k = 0; k < n; k++ )
        coords[k].x += delta_x;

    if ( delta_y )
      for ( k = 0; k < n; k++ )
        coords[k].y += delta_y;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    mount_zone                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Mounts one glyph zone on top of another.  This is needed to        */
  /*    assemble composite glyphs.                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    source :: The source glyph zone.                                   */
  /*                                                                       */
  /* <Output>                                                              */
  /*    target :: The target glyph zone.                                   */
  /*                                                                       */
  static
  void  mount_zone( TT_GlyphZone*  source,
                    TT_GlyphZone*  target )
  {
    TT_UShort  np;
    TT_Short   nc;


    np = source->n_points;
    nc = source->n_contours;

    target->org   = source->org + np;
    target->cur   = source->cur + np;
    target->touch = source->touch + np;

    target->contours = source->contours + nc;

    target->n_points   = 0;
    target->n_contours = 0;
  }


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
  TT_Error  Load_Simple_Glyph( TT_ExecContext   exec,
                               FT_Stream        stream,
                               TT_ULong         byte_count,
                               TT_Short         n_contours,
                               TT_Short         left_contours,
                               TT_UShort        left_points,
                               TT_UInt          load_flags,
                               TT_SubGlyphRec*  subg,
                               TT_Bool          debug )
  {
    TT_Error       error;
    TT_GlyphZone*  pts;
    TT_Short       k;
    TT_UShort      j;
    TT_UShort      n_points, n_ins;
    TT_Face        face;
    TT_Byte*       flag;
    TT_Vector*     vec;
    TT_F26Dot6     x, y;

    TT_Vector      *pp1, *pp2;
        

    face = exec->face;

    /*********************************************************************/
    /* simple check                                                      */
    
    if ( n_contours > left_contours )
    {
      FT_TRACE0(( "ERROR: Glyph index %ld has %d contours > left %d\n",
                   subg->index,
                   n_contours,
                   left_contours ));
      return TT_Err_Too_Many_Contours;
    }

    /* preparing the execution context */
    mount_zone( &subg->zone, &exec->pts );


    /*********************************************************************/
    /* reading the contours endpoints                                    */
    
    if ( ACCESS_Frame( byte_count ) )
      return error;

    for ( k = 0; k < n_contours; k++ )
      exec->pts.contours[k] = GET_UShort();

    n_points = 0;
    if ( n_contours > 0 )
      n_points = exec->pts.contours[n_contours - 1] + 1;


    /*********************************************************************/
    /* reading the bytecode instructions                                 */
    
    n_ins = GET_UShort();

    if ( n_points > left_points )
    {
      FT_TRACE0(( "ERROR: Too many points in glyph %ld\n", subg->index ));
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
    MEM_Copy( exec->glyphIns, stream->cursor, n_ins );
    stream->cursor += n_ins;

    error = TT_Set_CodeRange( exec, tt_coderange_glyph,
                              exec->glyphIns, n_ins );
    if (error) goto Fail;

    
    /*********************************************************************/
    /* reading the point flags                                           */
    
    j    = 0;
    flag = exec->pts.touch;

    while ( j < n_points )
    {
      TT_Byte  c, cnt;

      flag[j] = c = GET_Byte();
      j++;

      if ( c & 8 )
      {
        cnt = GET_Byte();
        while( cnt > 0 )
        {
          flag[j++] = c;
          cnt--;
        }
      }
    }

    /*********************************************************************/
    /* reading the X coordinates                                         */
    
    x    = 0;
    vec  = exec->pts.org;

    for ( j = 0; j < n_points; j++ )
    {
      if ( flag[j] & 2 )
      {
        if ( flag[j] & 16 )
          x += GET_Byte();
        else
          x -= GET_Byte();
      }
      else
      {
        if ( (flag[j] & 16) == 0 )
          x += GET_Short();
      }

      vec[j].x = x;
    }


    /*********************************************************************/
    /* reading the YX coordinates                                         */
    
    y = 0;

    for ( j = 0; j < n_points; j++ )
    {
      if ( flag[j] & 4 )
      {
        if ( flag[j] & 32 )
          y += GET_Byte();
        else
          y -= GET_Byte();
      }
      else
      {
        if ( (flag[j] & 32) == 0 )
          y += GET_Short();
      }

      vec[j].y = y;
    }

    FORGET_Frame();

    /*********************************************************************/
    /* Add shadow points                                                  */
    
    /* Now add the two shadow points at n and n + 1.    */
    /* We need the left side bearing and advance width. */

    /* pp1 = xMin - lsb */
    pp1    = vec + n_points;
    pp1->x = subg->bbox.xMin - subg->left_bearing;
    pp1->y = 0;

    /* pp2 = pp1 + aw */
    pp2    = pp1 + 1;
    pp2->x = pp1->x + subg->advance;
    pp2->y = 0;
        
    /* clear the touch flags */
    for ( j = 0; j < n_points; j++ )
      exec->pts.touch[j] &= FT_Curve_Tag_On;

    exec->pts.touch[n_points    ] = 0;
    exec->pts.touch[n_points + 1] = 0;

    /* Note that we return two more points that are not */
    /* part of the glyph outline.                       */

    n_points += 2;

    /* now eventually scale and hint the glyph */

    pts = &exec->pts;
    pts->n_points   = n_points;
    pts->n_contours = n_contours;

    if (load_flags & FT_LOAD_NO_SCALE)
    {
      /* no scaling, just copy the orig arrays into the cur ones */
      org_to_cur( n_points, pts );
    }
    else
    {
      /* first scale the glyph points */
      for ( j = 0; j < n_points; j++ )
      {
        pts->org[j].x = SCALE_X( pts->org[j].x );
        pts->org[j].y = SCALE_Y( pts->org[j].y );
      }

      /* if hinting, round pp1, and shift the glyph accordingly */
      if ( subg->is_hinted )
      {
        x = pts->org[n_points - 2].x;
        x = ((x + 32) & -64) - x;
        translate_array( n_points, pts->org, x, 0 );

        org_to_cur( n_points, pts );

        pts->cur[n_points - 1].x = (pts->cur[n_points - 1].x + 32) & -64;

        /* now consider hinting */
        if ( n_ins > 0 )
        {
          exec->is_composite     = FALSE;
          exec->pedantic_hinting = load_flags & FT_LOAD_PEDANTIC;

          error = TT_Run_Context( exec, debug );
          if ( error && exec->pedantic_hinting )
            return error;
        }
      }
      else
        org_to_cur( n_points, pts );
    }

    /* save glyph phantom points */
    if ( !subg->preserve_pps )
    {
      subg->pp1 = pts->cur[n_points - 2];
      subg->pp2 = pts->cur[n_points - 1];
    }

    return TT_Err_Ok;
    
  Fail:
    FORGET_Frame();
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Load_Composite_End                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finalizes the loading process of a composite glyph element.  This  */
  /*    function is used for the `Load_End' state of TT_Load_Glyph().      */
  /*                                                                       */
  static
  TT_Error  Load_Composite_End( TT_UShort        n_points,
                                TT_Short         n_contours,
                                TT_ExecContext   exec,
                                TT_SubGlyphRec*  subg,
                                FT_Stream        stream,
                                TT_UInt          load_flags,
                                TT_Bool          debug )
  {
    TT_Error       error;

    TT_UShort      k, n_ins;
    TT_GlyphZone*  pts;

    if ( subg->is_hinted                    &&
         subg->element_flag & WE_HAVE_INSTR )
    {
      if ( READ_UShort( n_ins ) )    /* read size of instructions */
        return error;

      FT_TRACE4(( "Instructions size = %d\n", n_ins ));

      if ( n_ins > exec->face->max_profile.maxSizeOfInstructions )
      {
        FT_TRACE0(( "Too many instructions in composite glyph %ld\n",
                  subg->index ));
        return TT_Err_Too_Many_Ins;
      }

      if ( FILE_Read( exec->glyphIns, n_ins ) )
        return error;

      error = TT_Set_CodeRange( exec,
                                tt_coderange_glyph,
                                exec->glyphIns,
                                n_ins );
      if ( error )
        return error;
    }
    else
      n_ins = 0;


    /* prepare the execution context */
    n_points += 2;
    exec->pts = subg->zone;
    pts       = &exec->pts;

    pts->n_points   = n_points;
    pts->n_contours = n_contours;

    /* add phantom points */
    pts->cur[n_points - 2] = subg->pp1;
    pts->cur[n_points - 1] = subg->pp2;

    pts->touch[n_points - 1] = 0;
    pts->touch[n_points - 2] = 0;

    /* if hinting, round the phantom points */
    if ( subg->is_hinted )
    {
      pts->cur[n_points - 2].x = ((subg->pp1.x + 32) & -64);
      pts->cur[n_points - 1].x = ((subg->pp2.x + 32) & -64);
    }

    for ( k = 0; k < n_points; k++ )
      pts->touch[k] &= FT_Curve_Tag_On;

    cur_to_org( n_points, pts );

    /* now consider hinting */
    if ( subg->is_hinted && n_ins > 0 )
    {
      exec->is_composite     = TRUE;
      exec->pedantic_hinting = load_flags & FT_LOAD_PEDANTIC;

      error = TT_Run_Context( exec, debug );
      if ( error && exec->pedantic_hinting )
        return error;
    }

    /* save glyph origin and advance points */
    subg->pp1 = pts->cur[n_points - 2];
    subg->pp2 = pts->cur[n_points - 1];

    return TT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Init_Glyph_Component                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a glyph component for further processing.              */
  /*                                                                       */
  static
  void  Init_Glyph_Component( TT_SubGlyphRec*  element,
                              TT_SubGlyphRec*  original,
                              TT_ExecContext   exec )
  {
    element->index     = -1;
    element->is_scaled = FALSE;
    element->is_hinted = FALSE;

    if ( original )
      mount_zone( &original->zone, &element->zone );
    else
      element->zone = exec->pts;

    element->zone.n_contours = 0;
    element->zone.n_points   = 0;

    element->arg1 = 0;
    element->arg2 = 0;

    element->element_flag = 0;
    element->preserve_pps = FALSE;

    element->transform.xx = 0x10000;
    element->transform.xy = 0;
    element->transform.yx = 0;
    element->transform.yy = 0x10000;

    element->transform.ox = 0;
    element->transform.oy = 0;

    element->left_bearing = 0;
    element->advance      = 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Glyph                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to load a single glyph within a given glyph slot,  */
  /*    for a given size.                                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    glyph       :: A handle to a target slot object where the glyph    */
  /*                   will be loaded.                                     */
  /*                                                                       */
  /*    size        :: A handle to the source face size at which the glyph */
  /*                   must be scaled/loaded.                              */
  /*                                                                       */
  /*    glyph_index :: The index of the glyph in the font file.            */
  /*                                                                       */
  /*    load_flags  :: A flag indicating what to load for this glyph.  The */
  /*                   FT_LOAD_XXX constants can be used to control the    */
  /*                   glyph loading process (e.g., whether the outline    */
  /*                   should be scaled, whether to load bitmaps or not,   */
  /*                   whether to hint the outline, etc).                  */
  /* <Output>                                                              */
  /*    result      :: A set of bit flags indicating the type of data that */
  /*                   was loaded in the glyph slot (outline or bitmap,    */
  /*                   etc).                                               */
  /*                                                                       */
  /*                   You can set this field to 0 if you don't want this  */
  /*                   information.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_Glyph( TT_Size       size,
                           TT_GlyphSlot  glyph,
                           TT_UShort     glyph_index,
                           TT_UInt       load_flags )
  {
    typedef enum  TPhases_
    {
      Load_Exit,
      Load_Glyph,
      Load_Header,
      Load_Simple,
      Load_Composite,
      Load_End

    } TPhases;

    TT_Error   error = 0;
    FT_Stream  stream;

    TT_Face    face;

    TT_UShort  num_points;
    TT_Short   num_contours;
    TT_UShort  left_points;
    TT_Short   left_contours;

    TT_ULong   load_top;
    TT_Long    k, l;
    TT_Int     new_flags;
    TT_UShort  index;
    TT_UShort  u;
    TT_Long    count;

    TT_Long  glyph_offset, offset;

    TT_F26Dot6  x, y, nx, ny;

    TT_Fixed  xx, xy, yx, yy;
    TT_BBox   bbox;

    TT_ExecContext  exec;

    TT_SubGlyphRec *subglyph, *subglyph2;

    TT_GlyphZone base_pts;

    TPhases   phase;
    TT_Byte*  widths;
    TT_Int    num_elem_points;

    SFNT_Interface*  sfnt;
    
    /* first of all, check arguments */
    if ( !glyph )
      return TT_Err_Invalid_Glyph_Handle;

    face = (TT_Face)glyph->face;
    if ( !face )
      return TT_Err_Invalid_Glyph_Handle;

    sfnt   = (SFNT_Interface*)face->sfnt;
    stream = face->root.stream;
    count  = 0;
    
    if ( glyph_index >= face->root.num_glyphs )
      return TT_Err_Invalid_Glyph_Index;

    if ( !size || (load_flags & FT_LOAD_NO_SCALE) )
    {
      size        = NULL;
      load_flags |= FT_LOAD_NO_SCALE   |
                    FT_LOAD_NO_HINTING |
                    FT_LOAD_NO_BITMAP;
    }

    /* Try to load embedded bitmap if any */
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

    if ( load_flags & FT_LOAD_NO_OUTLINE )
      return ( error ? error : TT_Err_Unavailable_Bitmap );

    error = face->goto_table( face, TTAG_glyf, stream, 0 );
    if (error)
    {
      FT_ERROR(( "TT.GLoad: could not access glyph table\n" ));
      return error;
    }

    glyph_offset = FILE_Pos();

    /* query new execution context */

    if ( size && size->debug )
      exec = size->context;
    else
      exec = TT_New_Context( face );

    if ( !exec )
      return TT_Err_Could_Not_Find_Context;

    TT_Load_Context( exec, face, size );

    if ( size )
    {
      /* load default graphics state - if needed */
      if ( size->GS.instruct_control & 2 )
        exec->GS = tt_default_graphics_state;

      glyph->outline.high_precision = ( size->root.metrics.y_ppem < 24 );
    }

    /* save its critical pointers, as they'll be modified during load */
    base_pts = exec->pts;

    /* init variables */
    left_points   = face->root.max_points;    /* remove phantom points */
    left_contours = face->root.max_contours;

    num_points   = 0;
    num_contours = 0;

    load_top = 0;
    subglyph = exec->loadStack;

    Init_Glyph_Component( subglyph, NULL, exec );

    subglyph->index     = glyph_index;
    subglyph->is_hinted = !(load_flags & FT_LOAD_NO_HINTING);

    /* when the cvt program has disabled hinting, the argument */
    /* is ignored.                                             */
    if ( size && (size->GS.instruct_control & 1) )
      subglyph->is_hinted = FALSE;

    /* Main loading loop */

    phase = Load_Glyph;
    index = 0;

    while ( phase != Load_Exit )
    {
      subglyph = exec->loadStack + load_top;

      switch ( phase )
      {

        /************************************************************/
        /*                                                          */
        /* Load_Glyph state                                         */
        /*                                                          */
        /*   reading a glyph's generic data, checking whether the   */
        /*   glyph is cached already (not implemented yet)          */
        /*                                                          */
        /* exit states: Load_Header and Load_End                    */
        /*                                                          */
      case Load_Glyph:
        /* check glyph index and table */

        index = (TT_UInt)subglyph->index;
        if ( index >= face->root.num_glyphs )
        {
          error = TT_Err_Invalid_Glyph_Index;
          goto Fail;
        }

        /* get horizontal metrics */
        {
          TT_Short   left_bearing;
          TT_UShort  advance_width;

          Get_HMetrics( face, index, TRUE,
                        &left_bearing,
                        &advance_width );

          subglyph->left_bearing = left_bearing;
          subglyph->advance      = advance_width;
        }

        phase = Load_Header;

        /************************************************************/
        /*                                                          */
        /* Load_Header state                                        */
        /*                                                          */
        /*   reading a glyph's generic header to determine whether  */
        /*   it is a simple or composite glyph                      */
        /*                                                          */
        /* exit states: Load_Simple and Load_Composite              */
        /*                                                          */
      case Load_Header:
        /* load glyph */

        offset = face->glyph_locations[index];
        count  = 0;
        if (index < face->num_locations-1)
          count = face->glyph_locations[index+1] - offset;
          
        if (count == 0)
        {
          /* as described by Frederic Loyer, these are spaces, and */
          /* not the unknown glyph.                                */

          num_contours = 0;
          num_points   = 0;

          subglyph->bbox.xMin = 0;
          subglyph->bbox.xMax = 0;
          subglyph->bbox.yMin = 0;
          subglyph->bbox.yMax = 0;

          subglyph->pp1.x = 0;
          subglyph->pp2.x = subglyph->advance;

          if ( !(load_flags & FT_LOAD_NO_SCALE) )
            subglyph->pp2.x = SCALE_X( subglyph->pp2.x );

          exec->glyphSize = 0;
          phase = Load_End;
          break;
        }

        offset = glyph_offset + offset;

        /* read first glyph header */
        if ( FILE_Seek( offset ) ||
             ACCESS_Frame( 10L ) )
          goto Fail_File;

        num_contours = GET_Short();

        subglyph->bbox.xMin = GET_Short();
        subglyph->bbox.yMin = GET_Short();
        subglyph->bbox.xMax = GET_Short();
        subglyph->bbox.yMax = GET_Short();

        FORGET_Frame();

        FT_TRACE6(( "Glyph %ld\n", index ));
        FT_TRACE6(( " # of contours : %d\n", num_contours ));
        FT_TRACE6(( " xMin: %4d  xMax: %4d\n",
                     subglyph->bbox.xMin,
                     subglyph->bbox.xMax ));
        FT_TRACE6(( " yMin: %4d  yMax: %4d\n",
                     subglyph->bbox.yMin,
                     subglyph->bbox.yMax ));
        FT_TRACE6(( "-" ));

        count -= 10;

        if ( num_contours > left_contours )
        {
          FT_TRACE0(( "ERROR: Too many contours for glyph %ld\n", index ));
          error = TT_Err_Too_Many_Contours;
          goto Fail;
        }

        subglyph->pp1.x = subglyph->bbox.xMin - subglyph->left_bearing;
        subglyph->pp1.y = 0;
        subglyph->pp2.x = subglyph->pp1.x + subglyph->advance;
        if (!(load_flags & FT_LOAD_NO_SCALE))
        {
          subglyph->pp1.x = SCALE_X( subglyph->pp1.x );
          subglyph->pp2.x = SCALE_X( subglyph->pp2.x );
        }

        /* is it a simple glyph ? */
        if ( num_contours < 0 )
        {
          phase = Load_Composite;
          break;
        }

        phase = Load_Simple;
        
        /************************************************************/
        /*                                                          */
        /* Load_Simple state                                        */
        /*                                                          */
        /*   reading a simple glyph (num_contours must be set to    */
        /*   the glyph's number of contours.)                       */
        /*                                                          */
        /* exit state: Load_End                                     */
        /*                                                          */
      case Load_Simple:
        new_flags = load_flags;

        /* disable hinting when scaling */
        if ( !subglyph->is_hinted )
          new_flags |= FT_LOAD_NO_HINTING;

        error = Load_Simple_Glyph( exec,
                                   stream,
                                   count,
                                   num_contours,
                                   left_contours,
                                   left_points,
                                   new_flags,
                                   subglyph,
                                   (TT_Bool)(size && size->debug &&
                                             load_top == 0) );
        if ( error )
          goto Fail;

        /* Note: We could have put the simple loader source there */
        /*       but the code is fat enough already :-)           */

        num_points = exec->pts.n_points - 2;

        phase = Load_End;
        break;

        /************************************************************/
        /*                                                          */
        /* Load_Composite state                                     */
        /*                                                          */
        /*   reading a composite glyph header and pushing a new     */
        /*   load element on the stack.                             */
        /*                                                          */
        /* exit state: Load_Glyph                                  */
        /*                                                          */
      case Load_Composite:

        /* create a new element on the stack */
        load_top++;

        if ( load_top > face->max_components )
        {
          error = TT_Err_Invalid_Composite;
          goto Fail;
        }

        subglyph2 = exec->loadStack + load_top;

        Init_Glyph_Component( subglyph2, subglyph, NULL );
        subglyph2->is_hinted = subglyph->is_hinted;

        /* now read composite header */

        if ( ACCESS_Frame( 4L ) )
          goto Fail_File;

        subglyph->element_flag = new_flags = GET_UShort();

        subglyph2->index = GET_UShort();

        FORGET_Frame();

        k = 1+1;

        if ( new_flags & ARGS_ARE_WORDS )
          k *= 2;

        if ( new_flags & WE_HAVE_A_SCALE )
          k += 2;

        else if ( new_flags & WE_HAVE_AN_XY_SCALE )
          k += 4;

        else if ( new_flags & WE_HAVE_A_2X2 )
          k += 8;

        if ( ACCESS_Frame( k ) )
          goto Fail_File;

        if ( new_flags & ARGS_ARE_WORDS )
        {
          k = GET_Short();
          l = GET_Short();
        }
        else
        {
          k = GET_Char();
          l = GET_Char();
        }

        subglyph->arg1 = k;
        subglyph->arg2 = l;

        if ( new_flags & ARGS_ARE_XY_VALUES )
        {
          subglyph->transform.ox = k;
          subglyph->transform.oy = l;
        }

        xx = 0x10000;
        xy = 0;
        yx = 0;
        yy = 0x10000;

        if ( new_flags & WE_HAVE_A_SCALE )
        {
          xx = (TT_Fixed)GET_Short() << 2;
          yy = xx;
          subglyph2->is_scaled = TRUE;
        }
        else if ( new_flags & WE_HAVE_AN_XY_SCALE )
        {
          xx = (TT_Fixed)GET_Short() << 2;
          yy = (TT_Fixed)GET_Short() << 2;
          subglyph2->is_scaled = TRUE;
        }
        else if ( new_flags & WE_HAVE_A_2X2 )
        {
          xx = (TT_Fixed)GET_Short() << 2;
          xy = (TT_Fixed)GET_Short() << 2;
          yx = (TT_Fixed)GET_Short() << 2;
          yy = (TT_Fixed)GET_Short() << 2;
          subglyph2->is_scaled = TRUE;
        }

        FORGET_Frame();

        subglyph->transform.xx = xx;
        subglyph->transform.xy = xy;
        subglyph->transform.yx = yx;
        subglyph->transform.yy = yy;

        k = FT_MulFix( xx, yy ) -  FT_MulFix( xy, yx );

        /* disable hinting in case of scaling/slanting */
        if ( ABS( k ) != 0x10000 )
          subglyph2->is_hinted = FALSE;

        subglyph->file_offset = FILE_Pos();

        phase = Load_Glyph;
        break;

        /************************************************************/
        /*                                                          */
        /* Load_End state                                           */
        /*                                                          */
        /*   after loading a glyph, apply transformation and offset */
        /*   where necessary, pop element and continue or stop      */
        /*   process.                                               */
        /*                                                          */
        /* exit states: Load_Composite and Load_Exit                */
        /*                                                          */
      case Load_End:
        if ( load_top > 0 )
        {
          subglyph2 = subglyph;

          load_top--;
          subglyph = exec->loadStack + load_top;

          /* check advance width and left side bearing */

          if ( !subglyph->preserve_pps &&
               subglyph->element_flag & USE_MY_METRICS )
          {
            subglyph->left_bearing = subglyph2->left_bearing;
            subglyph->advance      = subglyph2->advance;

            subglyph->pp1 = subglyph2->pp1;
            subglyph->pp2 = subglyph2->pp2;

            subglyph->preserve_pps = TRUE;
          }

          /* apply scale */

          if ( subglyph2->is_scaled )
          {
            TT_Vector*  cur = subglyph2->zone.cur;
            TT_Vector*  org = subglyph2->zone.org;


            for ( u = 0; u < num_points; u++ )
            {
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

              cur++;
              org++;
            }
          }

          /* adjust counts */

          num_elem_points = subglyph->zone.n_points;
          
          for ( k = 0; k < num_contours; k++ )
            subglyph2->zone.contours[k] += num_elem_points;

          subglyph->zone.n_points   += num_points;
          subglyph->zone.n_contours += num_contours;

          left_points   -= num_points;
          left_contours -= num_contours;

          /* apply offset */

          if ( !(subglyph->element_flag & ARGS_ARE_XY_VALUES) )
          {
            k = subglyph->arg1;
            l = subglyph->arg2;

            if ( k >= num_elem_points ||
                 l >= num_points )
            {
              error = TT_Err_Invalid_Composite;
              goto Fail;
            }

            l += num_elem_points; 
	    
            x = subglyph->zone.cur[k].x - subglyph->zone.cur[l].x;
            y = subglyph->zone.cur[k].y - subglyph->zone.cur[l].y;
          }
          else
          {
            x = subglyph->transform.ox;
            y = subglyph->transform.oy;

            if (!(load_flags & FT_LOAD_NO_SCALE))
            {
              x = SCALE_X( x );
              y = SCALE_Y( y );
              
              if ( subglyph->element_flag & ROUND_XY_TO_GRID )
	          {
	            x = (x + 32) & -64;
	            y = (y + 32) & -64;
	          }
            }
          }

          translate_array( num_points, subglyph2->zone.cur, x, y );

          cur_to_org( num_points, &subglyph2->zone );

          num_points   = subglyph->zone.n_points;
          num_contours = subglyph->zone.n_contours;

          /* check for last component */

          if ( FILE_Seek( subglyph->file_offset ) )
            goto Fail_File;

          if ( subglyph->element_flag & MORE_COMPONENTS )
            phase = Load_Composite;
          else
          {
            error = Load_Composite_End( num_points,
                                        num_contours,
                                        exec,
                                        subglyph,
                                        stream,
                                        load_flags,
                                        (TT_Bool)(size && size->debug &&
                                                  load_top == 0) );
            if ( error )
              goto Fail;

            phase = Load_End;
          }
        }
        else
          phase = Load_Exit;

        break;

      case Load_Exit:
        break;
      }
    }

    /* finally, copy the points arrays to the glyph object */

    exec->pts = base_pts;

    for ( u = 0; u < num_points + 2; u++ )
    {
      glyph->outline.points[u] = exec->pts.cur[u];
      glyph->outline.flags [u] = exec->pts.touch[u];
    }

    for ( k = 0; k < num_contours; k++ )
      glyph->outline.contours[k] = exec->pts.contours[k];

    glyph->outline.n_points    = num_points;
    glyph->outline.n_contours  = num_contours;
    glyph->outline.second_pass = TRUE;

    /* translate array so that (0,0) is the glyph's origin */
    translate_array( (TT_UShort)(num_points + 2),
                     glyph->outline.points,
                     -subglyph->pp1.x,
                     0 );

    FT_Get_Outline_CBox( &glyph->outline, &bbox );

    if ( subglyph->is_hinted )
    {
      /* grid-fit the bounding box */
      bbox.xMin &= -64;
      bbox.yMin &= -64;
      bbox.xMax  = (bbox.xMax + 63) & -64;
      bbox.yMax  = (bbox.yMax + 63) & -64;
    }

    /* get the device-independent scaled horizontal metrics */
    /* take care of fixed-pitch fonts...                    */
    {
      TT_Pos  left_bearing;
      TT_Pos  advance;


      left_bearing = subglyph->left_bearing;
      advance      = subglyph->advance;

      if ( face->postscript.isFixedPitch )
        advance = face->horizontal.advance_Width_Max;

      if ( !(load_flags & FT_LOAD_NO_SCALE) )
      {
        left_bearing = SCALE_X( left_bearing );
        advance      = SCALE_X( advance );
      }

      glyph->metrics2.horiBearingX = left_bearing;
      glyph->metrics2.horiAdvance  = advance;
    }

    glyph->metrics.horiBearingX = bbox.xMin;
    glyph->metrics.horiBearingY = bbox.yMax;
    glyph->metrics.horiAdvance  = subglyph->pp2.x - subglyph->pp1.x;

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
      if ( !(load_flags & FT_LOAD_NO_SCALE) )
      {
        Top     = SCALE_Y( top_bearing );
        top     = SCALE_Y( top_bearing + subglyph->bbox.yMax ) - bbox.yMax;
        advance = SCALE_Y( advance_height );
      }
      else
      {
        Top     = top_bearing;
        top     = top_bearing + subglyph->bbox.yMax - bbox.yMax;
        advance = advance_height;
      }

      glyph->metrics2.vertBearingY = Top;
      glyph->metrics2.vertAdvance  = advance;

      /* XXX: for now, we have no better algorithm for the lsb, but it    */
      /*      should work fine.                                           */
      /*                                                                  */
      left = ( bbox.xMin - bbox.xMax ) / 2;

      /* grid-fit them if necessary */
      if ( subglyph->is_hinted )
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
    if ( !exec->face->postscript.isFixedPitch && size &&
         subglyph->is_hinted )
    {
      widths = Get_Advance_Widths( exec->face,
                                   exec->size->root.metrics.x_ppem );
      if ( widths )
        glyph->metrics.horiAdvance = widths[glyph_index] << 6;
    }

    glyph->outline.dropout_mode = (TT_Char)exec->GS.scan_type;

    /* set glyph dimensions */
    glyph->metrics.width  = bbox.xMax - bbox.xMin;
    glyph->metrics.height = bbox.yMax - bbox.yMin;

    glyph->format = ft_glyph_format_outline;

    error = TT_Err_Ok;

  Fail_File:
  Fail:

    /* reset the execution context */
    exec->pts = base_pts;

    if ( !size || !size->debug )
      TT_Done_Context( exec );

    return error;
  }


/* END */
