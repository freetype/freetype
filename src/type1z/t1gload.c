/*******************************************************************
 *
 *  t1gload.c                                                   1.0
 *
 *    Type1 Glyph Loader.
 *
 *  Copyright 1996-1999 by
 *  David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 *  This file is part of the FreeType project, and may only be used
 *  modified and distributed under the terms of the FreeType project
 *  license, LICENSE.TXT.  By continuing to use, modify, or distribute
 *  this file you indicate that you have read the license and
 *  understand and accept it fully.
 *
 ******************************************************************/

#include <t1gload.h>
#include <ftdebug.h>
#include <t1encode.h>
#include <ftstream.h>

  /**********************************************************************/
  /**********************************************************************/
  /**********************************************************************/
  /**********                                                   *********/
  /**********                                                   *********/
  /**********           GENERIC CHARSTRINGS PARSING             *********/
  /**********                                                   *********/
  /**********                                                   *********/
  /**********************************************************************/
  /**********************************************************************/
  /**********************************************************************/

/*********************************************************************
 *
 * <Function>
 *    T1_Init_Builder
 *
 * <Description>
 *    Initialise a given glyph builder.
 *
 * <Input>
 *    builder :: glyph builder to initialise
 *    face    :: current face object
 *    size    :: current size object
 *    glyph   :: current glyph object
 *
 *********************************************************************/

  LOCAL_FUNC
  void  T1_Init_Builder( T1_Builder*   builder,
                         T1_Face       face,
                         T1_Size       size,
                         T1_GlyphSlot  glyph )
  {
    builder->path_begun  = 0;
    builder->load_points = 1;

    builder->face   = face;
    builder->glyph  = glyph;
    builder->memory = face->root.memory;

    if (glyph)
    {
      builder->base         = glyph->root.outline;
      builder->max_points   = glyph->max_points;
      builder->max_contours = glyph->max_contours;
    }

    if (size)
    {
      builder->scale_x = size->root.metrics.x_scale;
      builder->scale_y = size->root.metrics.y_scale;
    }

    builder->pos_x = 0;
    builder->pos_y = 0;

    builder->left_bearing.x = 0;
    builder->left_bearing.y = 0;
    builder->advance.x      = 0;
    builder->advance.y      = 0;

    builder->base.n_points   = 0;
    builder->base.n_contours = 0;
    builder->current         = builder->base;
  }


/*********************************************************************
 *
 * <Function>
 *    T1_Done_Builder
 *
 * <Description>
 *    Finalise a given glyph builder. Its content can still be
 *    used after the call, but the function saves important information
 *    within the corresponding glyph slot.
 *
 * <Input>
 *    builder :: glyph builder to initialise
 *
 *********************************************************************/

  LOCAL_FUNC
  void T1_Done_Builder( T1_Builder*  builder )
  {
    T1_GlyphSlot  glyph = builder->glyph;

    if (glyph)
    {
      glyph->root.outline = builder->base;
      glyph->max_points   = builder->max_points;
      glyph->max_contours = builder->max_contours;
    }
  }



/*********************************************************************
 *
 * <Function>
 *    T1_Init_Decoder
 *
 * <Description>
 *    Initialise a given Type 1 decoder for parsing
 *
 * <Input>
 *    decoder :: Type 1 decoder to initialise
 *    funcs   :: hinter functions interface
 *
 *********************************************************************/

  EXPORT_FUNC
  void  T1_Init_Decoder( T1_Decoder* decoder )
  {
    decoder->top              = 0;
    decoder->zone             = 0;
    decoder->flex_state       = 0;
    decoder->num_flex_vectors = 0;

    /* Clear loader */
    MEM_Set( &decoder->builder, 0, sizeof(decoder->builder) );
  }


  /* check that there is enough room for "count" more points */
  static
  T1_Error  check_points( T1_Builder*  builder,
                          T1_Int       count )
  {
    FT_Outline*  base    = &builder->base;
    FT_Outline*  outline = &builder->current;

    if (!builder->load_points)
      return T1_Err_Ok;

    count += base->n_points + outline->n_points;
        
    /* realloc points table if necessary */
    if ( count >= builder->max_points )
    {
      T1_Error   error;
      FT_Memory  memory    = builder->memory;
      T1_Int     increment = outline->points - base->points;
      T1_Int     current   = builder->max_points;

      while ( builder->max_points < count )
        builder->max_points += 8;

      if ( REALLOC_ARRAY( base->points, current,
                          builder->max_points, T1_Vector )  ||
  
           REALLOC_ARRAY( base->flags, current,
                          builder->max_points, T1_Byte )    )
      {
        builder->error = error;
        return error;
      }
    
      outline->points = base->points + increment;
      outline->flags  = base->flags  + increment;
    }
    return T1_Err_Ok;
  }


  /* add a new point, do not check room */
  static
  void  add_point( T1_Builder*  builder,
                   FT_Pos       x,
                   FT_Pos       y,
                   FT_Byte      flag )
  {
    FT_Outline*  outline = &builder->current;

    if (builder->load_points)
    {
      FT_Vector*  point   = outline->points + outline->n_points;
      FT_Byte*    control = (FT_Byte*)outline->flags + outline->n_points;
      
      point->x = x;
      point->y = y;
      *control = ( flag ? FT_Curve_Tag_On : FT_Curve_Tag_Cubic );
      
      builder->last = *point;
    }
    
    outline->n_points++;
  }
    

  /* check room for a new on-curve point, then add it */
  static
  T1_Error  add_point1( T1_Builder*  builder,
                        FT_Pos       x,
                        FT_Pos       y )
  {
    T1_Error  error;
    
    error = check_points(builder,1);
    if (!error)
      add_point( builder, x, y, 1 );

    return error;
  }


  /* check room for a new contour, then add it */
  static
  T1_Error  add_contour( T1_Builder*  builder )
  {
    FT_Outline*  base    = &builder->base;
    FT_Outline*  outline = &builder->current;

    if (!builder->load_points)
    {
      outline->n_contours++;
      return T1_Err_Ok;
    }
    
    /* realloc contours array if necessary */
    if ( base->n_contours + outline->n_contours >= builder->max_contours &&
         builder->load_points )
    {
      T1_Error  error;
      FT_Memory memory = builder->memory;
      T1_Int    increment = outline->contours - base->contours;
      T1_Int    current   = builder->max_contours;

      builder->max_contours += 4;

      if ( REALLOC_ARRAY( base->contours,
                          current, builder->max_contours, T1_Short ) )
      {
        builder->error = error;
        return error;
      }
  
      outline->contours = base->contours + increment;
    }

    if (outline->n_contours > 0)
      outline->contours[ outline->n_contours-1 ] = outline->n_points-1;

    return T1_Err_Ok;
  }


  /* if a path was begun, add its first on-curve point */
  static
  T1_Error  start_point( T1_Builder*  builder,
                         T1_Pos       x,
                         T1_Pos       y )
  {
    return builder->path_begun && add_point1( builder, x, y );
  }


  /* close the current contour */
  static
  void  close_contour( T1_Builder*  builder )
  {
    FT_Outline*  outline = &builder->current;

    if ( outline->n_contours > 0 )
      outline->contours[outline->n_contours-1] = outline->n_points-1;
  }


/*********************************************************************
 *
 * <Function>
 *    lookup_glyph_by_stdcharcode
 *
 * <Description>
 *    Lookup a given glyph by its StandardEncoding charcode. Used
 *    to implement the SEAC Type 1 operator.
 *
 * <Input>
 *    face     :: current face object         
 *    charcode :: charcode to look for
 *
 * <Return>
 *    glyph index in font face. Returns -1 if the corresponding
 *    glyph wasn't found.
 *
 *********************************************************************/

  static
  T1_Int    lookup_glyph_by_stdcharcode( T1_Face  face,
                                         T1_Int   charcode )
  {
    T1_Int            n;
    const T1_String*  glyph_name;
    
    /* check range of standard char code */
    if (charcode < 0 || charcode > 255)
      return -1;
      
    glyph_name = t1_standard_strings[t1_standard_encoding[charcode]];
    
    for ( n = 0; n < face->type1.num_glyphs; n++ )
    {
      T1_String*  name = (T1_String*)face->type1.glyph_names[n];
      
      if ( name && name[0] == glyph_name[0] && strcmp(name,glyph_name) == 0 )
        return n;
    }

    return -1;
  }



/*********************************************************************
 *
 * <Function>
 *    t1operator_seac
 *
 * <Description>
 *    Implements the "seac" Type 1 operator for a Type 1 decoder
 *
 * <Input>
 *    decoder  :: current Type 1 decoder
 *    asb      :: accent's side bearing
 *    adx      :: horizontal position of accent
 *    ady      :: vertical position of accent
 *    bchar    :: base character's StandardEncoding charcode
 *    achar    :: accent character's StandardEncoding charcode
 *
 * <Return>
 *    Error code. 0 means success.                               
 *
 *********************************************************************/

  static
  T1_Error  t1operator_seac( T1_Decoder*  decoder,
                             T1_Pos       asb,
                             T1_Pos       adx,
                             T1_Pos       ady,
                             T1_Int       bchar,
                             T1_Int       achar )
  {
    T1_Error     error;
    T1_Face      face = decoder->builder.face;
    T1_Int       bchar_index, achar_index, n_base_points;
    FT_Outline*  cur  = &decoder->builder.current;
    FT_Outline*  base = &decoder->builder.base;
    T1_Vector    left_bearing, advance;
    T1_Font*     type1 = &face->type1;
    
    bchar_index = lookup_glyph_by_stdcharcode( face, bchar );
    achar_index = lookup_glyph_by_stdcharcode( face, achar );
    
    if (bchar_index < 0 || achar_index < 0)
    {
      FT_ERROR(( "T1.Parse_Seac : invalid seac character code arguments\n" ));
      return T1_Err_Syntax_Error;
    }

    /* First load "bchar" in builder */
    /* now load the unscaled outline */
    cur->n_points   = 0;
    cur->n_contours = 0;
    cur->points     = base->points   + base->n_points;
    cur->flags      = base->flags    + base->n_points;
    cur->contours   = base->contours + base->n_contours;

    error = T1_Parse_CharStrings( decoder,
                                  type1->charstrings    [bchar_index],
                                  type1->charstrings_len[bchar_index],
                                  type1->num_subrs,
                                  type1->subrs,
                                  type1->subrs_len );
    if (error) return error;

    n_base_points   = cur->n_points;

    /* save the left bearing and width of the base character */
    /* as they will be erase by the next load..              */
    left_bearing = decoder->builder.left_bearing;
    advance      = decoder->builder.advance;

    decoder->builder.left_bearing.x = 0;
    decoder->builder.left_bearing.y = 0;    

    /* Now load "achar" on top of */
    /* the base outline           */
    /*                            */ 
    cur->n_points   = 0;
    cur->n_contours = 0;
    cur->points     = base->points   + base->n_points;
    cur->flags      = base->flags    + base->n_points;
    cur->contours   = base->contours + base->n_contours;

    error = T1_Parse_CharStrings( decoder,
                                  type1->charstrings    [achar_index],
                                  type1->charstrings_len[achar_index],
                                  type1->num_subrs,
                                  type1->subrs,
                                  type1->subrs_len );
    if (error) return error;

    /* adjust contours in accented character outline */
    if (decoder->builder.load_points)
    {
      T1_Int  n;

      for ( n = 0; n < cur->n_contours; n++ )
        cur->contours[n] += n_base_points;
    }

    /* restore the left side bearing and   */
    /* advance width of the base character */
    decoder->builder.left_bearing = left_bearing;
    decoder->builder.advance      = advance;

    /* Finally, move the accent */
    if (decoder->builder.load_points)
      FT_Translate_Outline( cur, adx - asb, ady );
    
    (void)asb;           /* ignore this parameter */
    return T1_Err_Ok;
  }


/*********************************************************************
 *
 * <Function>
 *    T1_Parse_CharStrings
 *
 * <Description>
 *    Parses a given Type 1 charstrings program
 *
 * <Input>
 *    decoder          :: current Type 1 decoder
 *    charstring_base  :: base of the charstring stream
 *    charstring_len   :: length in bytes of the charstring stream
 *    num_subrs        :: number of sub-routines
 *    subrs_base       :: array of sub-routines addresses
 *    subrs_len        :: array of sub-routines lengths
 *
 * <Return>
 *    Error code. 0 means success.                               
 *
 *********************************************************************/

#define USE_ARGS(n)  top -= n; if (top < decoder->stack) goto Stack_Underflow

  EXPORT_FUNC
  T1_Error   T1_Parse_CharStrings( T1_Decoder*  decoder,
                                   T1_Byte*     charstring_base,
                                   T1_Int       charstring_len,
                                   T1_Int       num_subrs,
                                   T1_Byte**    subrs_base,
                                   T1_Int*      subrs_len )
  {
    T1_Error            error;
    T1_Decoder_Zone*    zone;
    T1_Byte*            ip;
    T1_Byte*            limit;
    T1_Builder*         builder = &decoder->builder;
    FT_Outline*         outline;
    T1_Pos              x, y;

    /* First of all, initialise the decoder */
    decoder->top  = decoder->stack;
    decoder->zone = decoder->zones;
    zone          = decoder->zones;

    builder->path_begun  = 0;

    zone->base           = charstring_base;
    limit = zone->limit  = charstring_base + charstring_len;
    ip    = zone->cursor = zone->base;

    error   = T1_Err_Ok;
    outline = &builder->current;
    
    x = builder->pos_x;
    y = builder->pos_y;

    /* now, execute loop */
    while ( ip < limit )
    {
      T1_Int*      top      = decoder->top;

      /* First of all, decompress operator or value */
      switch (*ip++)
      {
        case 1:   /* hstem */
        case 3:   /* vstem */
          {
   Clear_Stack:
            top = decoder->stack;
            break;
          }

        case 4:   /* vmoveto */
          {
            USE_ARGS(1);
            y += top[0];
            builder->path_begun = 1;
            goto Clear_Stack;
          }
          
        case 5:   /* rlineto */
          {
            if ( start_point( builder, x, y ) ) goto Memory_Error;
              
            USE_ARGS(2);
            x += top[0];
            y += top[1];
   Add_Line:
            if (add_point1( builder, top[0], top[1] )) goto Memory_Error;
            goto Clear_Stack;
          }
          
        case 6:   /* hlineto */
          {
            if ( start_point( builder, x, y ) ) goto Memory_Error;
              
            USE_ARGS(1);
            x += top[0];
            goto Add_Line;
          }
          
        case 7:   /* vlineto */
          {
            if ( start_point( builder, x, y ) ) goto Memory_Error;
              
            USE_ARGS(1);
            y += top[0];
            goto Add_Line;
          }
          
        case 8:   /* rrcurveto */
          {
            if ( start_point( builder, x, y ) ||
                 check_points( builder, 3 )   ) goto Memory_Error;
              
            USE_ARGS(6);
            x += top[0];
            y += top[1];
            add_point( builder, x, y, 0 );
            
            x += top[2];
            y += top[3];
            add_point( builder, x, y, 0 );

            x += top[4];
            y += top[5];
            add_point( builder, x, y, 1 );
            goto Clear_Stack;
          }    
          
        case 9:   /* closepath */
          {
            close_contour( builder );
            builder->path_begun = 0;
          }
          break;
          
        case 10:  /* callsubr  */
          {
            T1_Int  index;

            USE_ARGS(1);
            
            index = top[0];
            if ( index < 0 || index >= num_subrs )
            {
              FT_ERROR(( "T1.Parse_CharStrings : invalid subrs index\n" ));
              goto Syntax_Error;
            }

            if ( zone - decoder->zones >= T1_MAX_SUBRS_CALLS )
            {
              FT_ERROR(( "T1.Parse_CharStrings : too many nested subrs\n" ));
              goto Syntax_Error;
            }

            zone->cursor = ip;  /* save current instruction pointer */

            zone++;
            zone->base    = subrs_base[index];
            zone->limit   = zone->base + subrs_len[index];
            zone->cursor  = zone->base;

            if (!zone->base)
            {
              FT_ERROR(( "T1.Parse_CharStrings : invoking empty subrs !!\n" ));
              goto Syntax_Error;
            }

            decoder->zone = zone;
            ip            = zone->base;
            limit         = zone->limit;
            
            /* do not clear stack */
          }
          break;
          
        case 11:  /* return */
          {
            if ( zone <= decoder->zones )
            {
              FT_ERROR(( "T1.Parse_CharStrings : unexpected return\n" ));
              goto Syntax_Error;
            }

            zone--;
            ip            = zone->cursor;
            limit         = zone->limit;
            decoder->zone = zone;
          }
          break;
          
        case 13:  /* hsbw */
          {
            USE_ARGS(2);
            builder->left_bearing.x += top[0];
            builder->advance.x       = top[1];
            builder->advance.y       = 0;
        
            builder->last.x = x = top[0];
            builder->last.y = y = 0;
            
            /* the "metrics_only" indicates that we only want to compute */
            /* the glyph's metrics (lsb + advance width), not load the   */
            /* rest of it.. so exit immediately                          */
            if (builder->metrics_only)
              return T1_Err_Ok;
              
            goto Clear_Stack;
          }

        case 14:  /* endchar */
          {
            close_contour( builder );  

            /* add current outline to the glyph slot */
            builder->base.n_points   += builder->current.n_points;
            builder->base.n_contours += builder->current.n_contours;
            
            /* return now !! */
            return T1_Err_Ok;
          }
          
        case 21:  /* rmoveto */
          {
            USE_ARGS(2);
            x += top[0];
            y += top[1];
            goto Clear_Stack;
          }
          
        case 22:  /* hmoveto */
          {
            USE_ARGS(1);
            x += top[0];
            goto Clear_Stack;
          }
          
        case 30:  /* vhcurveto */
          {
            if ( start_point( builder, x, y ) ||
                 check_points( builder, 3 )   ) goto Memory_Error;
                 
            USE_ARGS(4);
            y += top[0];
            add_point( builder, x, y, 0 );
            x += top[1];
            y += top[2];
            add_point( builder, x, y, 0 );
            x += top[3];
            add_point( builder, x, y, 1 );
            goto Clear_Stack;
          }
          
        case 31:  /* hvcurveto */
          {
            if ( start_point( builder, x, y ) ||
                 check_points( builder, 3 )   ) goto Memory_Error;
                 
            USE_ARGS(4);
            x += top[0];
            add_point( builder, x, y, 0 );
            x += top[1];
            y += top[2];
            add_point( builder, x, y, 0 );
            y += top[3];
            add_point( builder, x, y, 1 );
            goto Clear_Stack;
          }
          

        case 12:
          {
            if (ip > limit)
            {
              FT_ERROR(( "T1.Parse_CharStrings : invalid escape (12+EOF)\n" ));
              goto Syntax_Error;
            }

            switch (*ip++)
            {
              case 0:  /* dotsection */
              case 1:  /* vstem3 */
              case 2:  /* hstem3 */
                  goto Clear_Stack;
                  
              case 6:  /* seac */
                {
                  USE_ARGS(5);
                  
                  /* return immediately to implement an accented character */
                  return t1operator_seac( decoder,
                                          top[0], top[1], top[3],
                                          top[4], top[5] );
                }
                  
              case 7:  /* sbw */
                {
                  USE_ARGS(4);
                  builder->left_bearing.x += top[0];
                  builder->left_bearing.y += top[1];
                  builder->advance.x       = top[2];
                  builder->advance.y       = top[3];
              
                  builder->last.x = x = top[0];
                  builder->last.y = y = top[1];
                  
                  /* the "metrics_only" indicates that we only want to compute */
                  /* the glyph's metrics (lsb + advance width), not load the   */
                  /* rest of it.. so exit immediately                          */
                  if (builder->metrics_only)
                    return T1_Err_Ok;
              
                  goto Clear_Stack;
                }
                  
              case 12: /* div */
                {
                  USE_ARGS(2);
                  top[0] /= top[1];
                  top++;
                }
                break;
                  
              case 16: /* callothersubr */
                {
                  USE_ARGS(1);
                  switch (top[0])
                  {
                    case 1: /* start flex feature ---------------------- */
                      {
                        decoder->flex_state        = 1;
                        decoder->num_flex_vectors  = 0;
                        if ( start_point(builder, x, y) ||
                             check_points(builder,6) ) goto Memory_Error;
                      }
                      break;

                    case 2: /* add flex vectors ------------------------ */
                      {
                        T1_Int      index;
                        
                        /* note that we should not add a point for index 0 */
                        /* this will move our current position to the flex */
                        /* point without adding any point to the outline   */
                        index = decoder->num_flex_vectors++;
                        if (index > 0 && index < 7)
                          add_point( builder,
                                     x,
                                     y,
                                     (T1_Byte)( index==3 || index==6 ) );
                      }
                      break;
                      
                    case 0: /* end flex feature ------------------------- */
                      {
                        USE_ARGS(3);  /* ignore parameters */
                        
                        if ( decoder->flex_state       == 0 ||
                             decoder->num_flex_vectors != 7 )
                        {
                          FT_ERROR(( "T1.Parse_CharStrings: unexpected flex end\n" ));
                          goto Syntax_Error;
                        }
                        
                        /* now consume the remaining "pop pop setcurpoint" */
                        if ( ip+6 > limit ||
                             ip[0] != 12  || ip[1] != 17 || /* pop */
                             ip[2] != 12  || ip[3] != 17 || /* pop */
                             ip[4] != 12  || ip[5] != 33 )  /* setcurpoint */
                        {
                          FT_ERROR(( "T1.Parse_CharStrings: invalid flex charstring\n" ));
                          goto Syntax_Error;
                        }
                        
                        ip += 6;
                        decoder->flex_state = 0;
                        decoder->top        = top;
                        
                        goto Clear_Stack;
                      }
                      
                    case 3:  /* change hints ---------------------------- */
                      {
                        /* eat the following "pop" */
                        if (ip+2 > limit)
                        {
                          FT_ERROR(( "T1.Parse_CharStrings: invalid escape (12+%d)\n",
                                   ip[-1] ));
                          goto Syntax_Error;
                        }
          
                        if (ip[0] != 12 || ip[1] != 17)
                        {
                          FT_ERROR(( "T1.Parse_CharStrings: 'pop' expected, found (%d %d)\n",
                                   ip[0], ip[1] ));
                          goto Syntax_Error;
                        }
                        ip += 2;
                        goto Clear_Stack;
                      }
                      
                    default:
                      FT_ERROR(( "T1.Parse_CharStrings: invalid othersubr %d !!\n",
                                 top[0] ));
                      goto Syntax_Error;
                  }
                }

              case 17:  /* pop - should not happen !! */
                {
                  FT_ERROR(( "T1.Parse_CharStrings : 'pop' should not happen !!\n" ));
                  goto Syntax_Error;
                }
                
              case 33: /* setcurrentpoint */
                {
                  FT_ERROR(( "T1.Parse_CharStrings : 'setcurrentpoint' should not happen !!\n" ));
                  goto Syntax_Error;
                }  

              default:
                FT_ERROR(( "T1.Parse_CharStrings : invalid escape (12+%d)\n",
                         ip[-1] ));
                goto Syntax_Error;
            }
          }
          break;  /* escape - 12 */

        case 255:    /* four bytes integer */
          {
            if (ip+4 > limit)
            {
              FT_ERROR(( "T1.Parse_CharStrings : unexpected EOF in integer\n" ));
              goto Syntax_Error;
            }

            *top++ = ((long)ip[0] << 24) |
                     ((long)ip[1] << 16) |
                     ((long)ip[2] << 8)  |
                            ip[3];
            ip += 4;
          }
          break;

        default:
          {
            T1_Long  v, v2;
            
            v = ip[-1];
            if (v < 32)
            {
              FT_ERROR(( "T1.Parse_CharStrings : invalid byte (%d)\n",
                         ip[-1] ));
              goto Syntax_Error;
            }
            
            /* compute value ----  */
            /*                     */
            if (v < 247)   /* 1-byte value */
              v -= 139;
            else
            {
              if (++ip > limit)  /* 2-bytes value, check limits */
              {
                FT_ERROR(( "T1.Parse_CharStrings : unexpected EOF in integer\n" ));
                goto Syntax_Error;
              }
              
              v2 = ip[-1] + 108;
              if (v < 251)
                v = ((v-247) << 8) + v2;
              else
                v = -(((v-251) << 8) + v2);
            }
            
            /* store value - is there enough room  ?*/
            if ( top >= decoder->stack + T1_MAX_CHARSTRINGS_OPERANDS )
            {
              FT_ERROR(( "T1.Parse_CharStrings : Stack overflow !!\n" ));
              goto Syntax_Error;
            }
            
            *top++ = v;
            decoder->top = top;
          }
      } /* big switch */
      
    } /* while ip < limit */
    return error;

  Syntax_Error:
    return T1_Err_Syntax_Error;

  Stack_Underflow:
    return T1_Err_Stack_Underflow;
    
  Memory_Error:
    return builder->error;
  }



  /**********************************************************************/
  /**********************************************************************/
  /**********************************************************************/
  /**********                                                   *********/
  /**********                                                   *********/
  /**********           COMPUTE THE MAXIMUM ADVANCE WIDTH       *********/
  /**********                                                   *********/
  /**********   The following code is in charge of computing    *********/
  /**********   the maximum advance width of the font. It       *********/
  /**********   quickly process each glyph charstring to        *********/
  /**********   extract the value from either a "sbw" or "seac" *********/
  /**********   operator.                                       *********/
  /**********                                                   *********/
  /**********************************************************************/
  /**********************************************************************/
  /**********************************************************************/

  LOCAL_FUNC
  T1_Error  T1_Compute_Max_Advance( T1_Face  face,
                                    T1_Int  *max_advance )
  {
    T1_Error    error;
    T1_Decoder  decoder;
    T1_Int      glyph_index;
    T1_Font*    type1 = &face->type1;

    *max_advance = 0;

    /* Initialise load decoder */
    T1_Init_Decoder( &decoder );
    T1_Init_Builder( &decoder.builder, face, 0, 0 );

    decoder.builder.metrics_only = 1;
    decoder.builder.load_points  = 0;

    /* For each glyph, parse the glyph charstring and extract */
    /* the advance width..                                    */
    for ( glyph_index = 0; glyph_index < type1->num_glyphs; glyph_index++ )
    {
      /* now get load the unscaled outline */
      error = T1_Parse_CharStrings( &decoder,
                                    type1->charstrings    [glyph_index],
                                    type1->charstrings_len[glyph_index],
                                    type1->num_subrs,
                                    type1->subrs,
                                    type1->subrs_len );
      /* ignore the error if one occured - skip to next glyph */
      (void)error;
    }

    *max_advance = decoder.builder.advance.x;
    return T1_Err_Ok;
  }


  /**********************************************************************/
  /**********************************************************************/
  /**********************************************************************/
  /**********                                                   *********/
  /**********                                                   *********/
  /**********              UNHINTED GLYPH LOADER                *********/
  /**********                                                   *********/
  /**********   The following code is in charge of loading a    *********/
  /**********   single outline. It completely ignores hinting   *********/
  /**********   and is used when FT_LOAD_NO_HINTING is set.     *********/
  /**********                                                   *********/
  /**********************************************************************/
  /**********************************************************************/
  /**********************************************************************/


  LOCAL_FUNC
  T1_Error  T1_Load_Glyph( T1_GlyphSlot  glyph,
                           T1_Size       size,
                           T1_Int        glyph_index,
                           T1_Int        load_flags )
  {
    T1_Error        error;
    T1_Decoder      decoder;
    T1_Face         face = (T1_Face)glyph->root.face;
    T1_Bool         hinting;
    T1_Font*        type1 = &face->type1;

    glyph->x_scale = size->root.metrics.x_scale;
    glyph->y_scale = size->root.metrics.y_scale;

    glyph->root.outline.n_points   = 0;
    glyph->root.outline.n_contours = 0;

    hinting = ( load_flags & FT_LOAD_NO_SCALE   ) == 0 &&
              ( load_flags & FT_LOAD_NO_HINTING ) == 0;

    glyph->root.format = ft_glyph_format_none;

    {
      T1_Init_Decoder( &decoder );
      T1_Init_Builder( &decoder.builder, face, size, glyph );
  
      /* now load the unscaled outline */
      error = T1_Parse_CharStrings( &decoder,
                                    type1->charstrings    [glyph_index],
                                    type1->charstrings_len[glyph_index],
                                    type1->num_subrs,
                                    type1->subrs,
                                    type1->subrs_len );
  
      /* save new glyph tables */
      T1_Done_Builder( &decoder.builder );
    }

    /* Now, set the metrics.. - this is rather simple, as : */
    /* the left side bearing is the xMin, and the top side  */
    /* bearing the yMax..                                   */
    if (!error)
    {
      FT_BBox           cbox;
      FT_Glyph_Metrics* metrics = &glyph->root.metrics;

      FT_Get_Outline_CBox( &glyph->root.outline, &cbox );

      /* grid fit the bounding box if necessary */
      if (hinting)
      {
        cbox.xMin &= -64;
        cbox.yMin &= -64;
        cbox.xMax = ( cbox.xMax+63 ) & -64;
        cbox.yMax = ( cbox.yMax+63 ) & -64;
      }

      metrics->width  = cbox.xMax - cbox.xMin;
      metrics->height = cbox.yMax - cbox.yMin;

      metrics->horiBearingX = cbox.xMin;
      metrics->horiBearingY = cbox.yMax;

      /* copy the _unscaled_ advance width */
      metrics->horiAdvance  = decoder.builder.advance.x;

      /* make up vertical metrics */
      metrics->vertBearingX = 0;
      metrics->vertBearingY = 0;
      metrics->vertAdvance  = 0;

      glyph->root.format = ft_glyph_format_outline;

      glyph->root.outline.second_pass    = TRUE;
      glyph->root.outline.high_precision = ( size->root.metrics.y_ppem < 24 );
      glyph->root.outline.dropout_mode   = 2;

      if ( (load_flags & FT_LOAD_NO_SCALE) == 0 )
      {
        /* scale the outline and the metrics */
        T1_Int       n;
        FT_Outline*  cur = &decoder.builder.base;
        T1_Vector*   vec = cur->points;
        T1_Fixed     x_scale = glyph->x_scale;
        T1_Fixed     y_scale = glyph->y_scale;

        /* First of all, scale the points */
        for ( n = cur->n_points; n > 0; n--, vec++ )
        {
          vec->x = FT_MulFix( vec->x, x_scale );
          vec->y = FT_MulFix( vec->y, y_scale );
        }

        /* Then scale the metrics */
        metrics->width  = FT_MulFix( metrics->width,  x_scale );
        metrics->height = FT_MulFix( metrics->height, y_scale );

        metrics->horiBearingX = FT_MulFix( metrics->horiBearingX, x_scale );
        metrics->horiBearingY = FT_MulFix( metrics->horiBearingY, y_scale );
        metrics->horiAdvance  = FT_MulFix( metrics->horiAdvance,  x_scale );

        metrics->vertBearingX = FT_MulFix( metrics->vertBearingX, x_scale );
        metrics->vertBearingY = FT_MulFix( metrics->vertBearingY, y_scale );
        metrics->vertAdvance  = FT_MulFix( metrics->vertAdvance,  x_scale );
      }
    }
    return error;
  }

