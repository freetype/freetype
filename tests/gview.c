#include <nirvana.h>
#include NV_VIEWPORT_H
#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

/* include FreeType internals to debug hints */
#include <../src/pshinter/pshrec.h>
#include <../src/pshinter/pshfit.h>

#include <time.h>    /* for clock() */

/* SunOS 4.1.* does not define CLOCKS_PER_SEC, so include <sys/param.h> */
/* to get the HZ macro which is the equivalent.                         */
#if defined(__sun__) && !defined(SVR4) && !defined(__SVR4)
#include <sys/param.h>
#define CLOCKS_PER_SEC HZ
#endif

static int  depth = 0;

static NV_Renderer   renderer;
static NV_Painter    painter;
static NV_Pixmap     target;
static NV_Error      error;
static NV_Memory     memory;
static NVV_Display   display;
static NVV_Surface   surface;

static FT_Library    freetype;
static FT_Face       face;


static  NV_Pos        glyph_scale;
static  NV_Pos        glyph_org_x;
static  NV_Pos        glyph_org_y;
static  NV_Transform  glyph_transform;  /* font units -> device pixels */
static  NV_Transform  size_transform;   /* subpixels  -> device pixels */

static  NV_Scale      grid_scale = 1.0;

static  int   glyph_index;
static  int   pixel_size = 12;
static  int   option_show_axis   = 1;
static  int   option_show_dots   = 1;
static  int   option_show_stroke = 1;
static  int   option_show_glyph  = 1;
static  int   option_show_grid   = 1;
static  int   option_show_em     = 0;

static  int   option_show_ps_hints   = 1;
static  int   option_show_horz_hints = 1;
static  int   option_show_vert_hints = 1;


static  int   option_hinting = 1;

static  char  temp_message[1024];

PS_Hints      the_ps_hints = 0;
int           ps_debug_no_horz_hints = 0;
int           ps_debug_no_vert_hints = 0;
PSH_HintFunc  ps_debug_hint_func     = 0;

#define  AXIS_COLOR        0xFFFF0000
#define  GRID_COLOR        0xFFD0D0D0
#define  ON_COLOR          0xFFFF2000
#define  OFF_COLOR         0xFFFF0080
#define  BACKGROUND_COLOR  0xFFFFFFFF
#define  TEXT_COLOR        0xFF000000
#define  EM_COLOR          0x80008000

#define  GHOST_HINT_COLOR  0xE00000FF
#define  STEM_HINT_COLOR   0xE02020FF
#define  STEM_JOIN_COLOR   0xE020FF20

/* print message and abort program */
static void
Panic( const char*  message )
{
  fprintf( stderr, "PANIC: %s\n", message );
  exit(1);
}



static void
reset_scale( NV_Scale  scale )
{ 
 /* compute font units -> grid pixels scale factor */
  glyph_scale = target->width*0.75 / face->units_per_EM * scale;
 
 /* setup font units -> grid pixels transform */
  nv_transform_set_scale( &glyph_transform, glyph_scale, -glyph_scale );
  glyph_org_x = glyph_transform.delta.x = target->width*0.125;
  glyph_org_y = glyph_transform.delta.y = target->height*0.875;
 
 /* setup subpixels -> grid pixels transform */
  nv_transform_set_scale( &size_transform,
                            glyph_scale/nv_fromfixed(face->size->metrics.x_scale),
                          - glyph_scale/nv_fromfixed(face->size->metrics.y_scale) );

  size_transform.delta = glyph_transform.delta;
}
           
           
static void
reset_size( int  pixel_size, NV_Scale  scale )
{
  FT_Set_Pixel_Sizes( face, pixel_size, pixel_size );
  reset_scale( scale );
}


static void
clear_background( void )
{
  nv_pixmap_fill_rect( target, 0, 0, target->width, target->height,
                       BACKGROUND_COLOR );
}


static void
draw_grid( void )
{
  int  x = (int)glyph_org_x;
  int  y = (int)glyph_org_y;

  /* draw grid */
  if ( option_show_grid )
  {
    NV_Scale  min, max, x, step;
    
    /* draw vertical grid bars */
    step = 64. * size_transform.matrix.xx;
    if (step > 1.)
    {
      min  = max = glyph_org_x;
      while ( min - step >= 0 )             min -= step;
      while ( max + step < target->width )  max += step;
        
      for ( x = min; x <= max; x += step )
        nv_pixmap_fill_rect( target, (NV_Int)(x+.5), 0,
                             1, target->height, GRID_COLOR );
    }
                                
    /* draw horizontal grid bars */
    step = -64. * size_transform.matrix.yy;
    if (step > 1.)
    {
      min  = max = glyph_org_y;
      while ( min - step >= 0 )              min -= step;
      while ( max + step < target->height )  max += step;
        
      for ( x = min; x <= max; x += step )
        nv_pixmap_fill_rect( target, 0, (NV_Int)(x+.5),
                             target->width, 1, GRID_COLOR );
    }
  }  
  
  /* draw axis */
  if ( option_show_axis )
  {
    nv_pixmap_fill_rect( target, x, 0, 1, target->height, AXIS_COLOR );
    nv_pixmap_fill_rect( target, 0, y, target->width, 1, AXIS_COLOR );
  }
  
  if ( option_show_em )
  {
    NV_Path  path;
    NV_Path  stroke;
    NV_UInt  units = (NV_UInt)face->units_per_EM;
    
    nv_path_new_rectangle( renderer, 0, 0, units, units, 0, 0, &path );
    nv_path_transform( path, &glyph_transform );
    
    nv_path_stroke( path, 1.5, nv_path_linecap_butt, nv_path_linejoin_miter,
                    4.0, &stroke );

    nv_painter_set_color( painter, EM_COLOR, 256 );
    nv_painter_fill_path( painter, NULL, 0, stroke );

    nv_path_destroy( stroke );
    nv_path_destroy( path );
  }

}


static int pshint_cpos     = 0;
static int pshint_vertical = -1;

static void
draw_ps_hint( PSH_Hint   hint, FT_Bool  vertical )
{
  int        x1, x2;
  NV_Vector  v;
  
  
  if ( pshint_vertical != vertical )
  {
    if (vertical)
      pshint_cpos = 40;
    else
      pshint_cpos = 10;
      
    pshint_vertical = vertical;
  }
  
  if (vertical)
  {
    if ( !option_show_vert_hints )
      return;
      
    v.x = hint->cur_pos;
    v.y = 0;
    nv_vector_transform( &v, &size_transform );
    x1 = (int)(v.x + 0.5);

    v.x = hint->cur_pos + hint->cur_len;
    v.y = 0;
    nv_vector_transform( &v, &size_transform );
    x2 = (int)(v.x + 0.5);

    nv_pixmap_fill_rect( target, x1, 0, 1, target->height,
                         psh_hint_is_ghost(hint)
                         ? GHOST_HINT_COLOR : STEM_HINT_COLOR );

    if ( psh_hint_is_ghost(hint) )
    {
      x1 --;
      x2 = x1 + 2;
    }
    else
      nv_pixmap_fill_rect( target, x2, 0, 1, target->height,
                           psh_hint_is_ghost(hint)
                           ? GHOST_HINT_COLOR : STEM_HINT_COLOR );

    nv_pixmap_fill_rect( target, x1, pshint_cpos, x2+1-x1, 1,
                         STEM_JOIN_COLOR );
  }
  else
  {
    if (!option_show_horz_hints)
      return;
      
    v.y = hint->cur_pos;
    v.x = 0;
    nv_vector_transform( &v, &size_transform );
    x1 = (int)(v.y + 0.5);

    v.y = hint->cur_pos + hint->cur_len;
    v.x = 0;
    nv_vector_transform( &v, &size_transform );
    x2 = (int)(v.y + 0.5);

    nv_pixmap_fill_rect( target, 0, x1, target->width, 1,
                         psh_hint_is_ghost(hint)
                         ? GHOST_HINT_COLOR : STEM_HINT_COLOR );

    if ( psh_hint_is_ghost(hint) )
    {
      x1 --;
      x2 = x1 + 2;
    }
    else
      nv_pixmap_fill_rect( target, 0, x2, target->width, 1,
                           psh_hint_is_ghost(hint)
                           ? GHOST_HINT_COLOR : STEM_HINT_COLOR );

    nv_pixmap_fill_rect( target, pshint_cpos, x2, 1, x1+1-x2,
                         STEM_JOIN_COLOR );
  }

  printf( "[%7.3f %7.3f] %c\n", hint->cur_pos/64.0, (hint->cur_pos+hint->cur_len)/64.0, vertical ? 'v' : 'h' );
  
  pshint_cpos += 10;
}



static void
draw_glyph( int  glyph_index )
{
  NV_Path   path;

  pshint_vertical    = -1;
  ps_debug_hint_func = option_show_ps_hints ? draw_ps_hint : 0;

  error = FT_Load_Glyph( face, glyph_index, option_hinting
                                          ? FT_LOAD_NO_BITMAP
                                          : FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING );
  if (error) Panic( "could not load glyph" );
  
  if ( face->glyph->format != ft_glyph_format_outline )
    Panic( "could not load glyph outline" );
  
  error = nv_path_new_from_outline( renderer,
                                    (NV_Outline*)&face->glyph->outline,
                                    &size_transform,
                                    &path );
  if (error) Panic( "could not create glyph path" );
  
  /* tracé du glyphe plein */
  if ( option_show_glyph )
  {
    nv_painter_set_color ( painter, 0xFF404080, 128 );
    nv_painter_fill_path ( painter, 0, 0, path );
  }

  if ( option_show_stroke )
  {
    NV_Path   stroke;
    
    error = nv_path_stroke( path, 0.6,
                            nv_path_linecap_butt,
                            nv_path_linejoin_miter,
                            1.0, &stroke );
    if (error) Panic( "could not stroke glyph path" );
  
    nv_painter_set_color ( painter, 0xFF000040, 256 );
    nv_painter_fill_path ( painter, 0, 0, stroke );
  
    nv_path_destroy( stroke );
  }

  /* tracé des points de controle */
  if ( option_show_dots )
  {
    NV_Path     plot;
    NV_Outline  out;
    NV_Scale    r = 2;
    NV_Int      n, first, last;

    nv_path_new_circle( renderer, 0, 0, 2., &plot );
    nv_path_get_outline( path, NULL, memory, &out );
    
    first = 0;
    for ( n = 0; n < out.n_contours; n++ )
    {
      int            m;
      NV_Transform   trans;
      NV_Color       color;
      NV_SubVector*  vec;
      
      last  = out.contours[n];
      
      for ( m = first; m <= last; m++ )
      {
        color = (out.tags[m] & FT_Curve_Tag_On)
              ? ON_COLOR
              : OFF_COLOR;
              
        vec = out.points + m;

        nv_transform_set_translate( &trans, vec->x/64.0, vec->y/64.0 );        
        nv_painter_set_color( painter, color, 256 );
        nv_painter_fill_path( painter, &trans, 0, plot );
      }
      
      first = last + 1;
    }

    nv_path_destroy( plot );
  }

  nv_path_destroy( path );
  
  /* autre infos */
  {
    char  temp[1024];
    char  temp2[64];
    
    sprintf( temp, "font name : %s (%s)", face->family_name, face->style_name );
    nv_pixmap_cell_text( target, 0, 0, temp, TEXT_COLOR );
    
    FT_Get_Glyph_Name( face, glyph_index, temp2, 63 );
    temp2[63] = 0;
    
    sprintf( temp, "glyph %4d: %s", glyph_index, temp2 );
    nv_pixmap_cell_text( target, 0, 8, temp, TEXT_COLOR );
    
    if ( temp_message[0] )
    {
      nv_pixmap_cell_text( target, 0, 16, temp_message, TEXT_COLOR );
      temp_message[0] = 0;
    }
  }
}


#if 0

static void
draw_ps_hints( void )
{
  if ( option_show_ps_hints && the_ps_hints )
  {
    PS_Dimension  dim;
    PS_Hint       hint;
    NV_UInt       count;
    NV_Int        cpos;
    NV_Vector     v;
    
    /* draw vertical stems */
    if ( option_show_vert_hints )
    {
      dim  = &the_ps_hints->dimension[1];
      hint = dim->hints.hints;
      cpos = 40;
      for ( count = dim->hints.num_hints; count > 0; count--, hint++ )
      {
        NV_Int   x1, x2;
  
        v.x = hint->pos;
        v.y = 0;
        nv_vector_transform( &v, &glyph_transform );
        x1  = (int)(v.x+0.5);
        x1  = glyph_org_x + hint->pos*glyph_scale;
        nv_pixmap_fill_rect( target, x1, 0, 1, target->height,
                                  ps_hint_is_ghost(hint)
                                  ? GHOST_HINT_COLOR : STEM_HINT_COLOR );
        
        if ( !ps_hint_is_ghost(hint) )
        {
          v.x = hint->pos + hint->len;
          v.y = 0;
          nv_vector_transform( &v, &glyph_transform );
          x2  = (int)(v.x+0.5);
          x2  = glyph_org_x + (hint->pos + hint->len)*glyph_scale;
          nv_pixmap_fill_rect( target, x2, 0, 1, target->height,
                                    STEM_HINT_COLOR );
        }
        else
        {
          x1 -= 1;
          x2  = x1 + 2;
        }
        
        nv_pixmap_fill_rect( target, x1, cpos, x2-x1+1, 1,
                                  STEM_JOIN_COLOR );
        cpos += 10;
      }
    }

    /* draw horizontal stems */
    if ( option_show_horz_hints )
    {
      dim  = &the_ps_hints->dimension[0];
      hint = dim->hints.hints;
      cpos = 10;
      for ( count = dim->hints.num_hints; count > 0; count--, hint++ )
      {
        NV_Int   y1, y2;
  
        v.x = 0;
        v.y = hint->pos;
        nv_vector_transform( &v, &glyph_transform );
        y1  = (int)(v.y+0.5);
        y1  = glyph_org_y - hint->pos*glyph_scale;
        nv_pixmap_fill_rect( target, 0, y1, target->width, 1,
                                  ps_hint_is_ghost(hint)
                                  ? GHOST_HINT_COLOR : STEM_HINT_COLOR );
        
        if ( !ps_hint_is_ghost(hint) )
        {
          v.x = 0;
          v.y = hint->pos + hint->len;
          nv_vector_transform( &v, &glyph_transform );
          y2  = (int)(v.y+0.5);
          y2  = glyph_org_y - (hint->pos + hint->len)*glyph_scale;
          nv_pixmap_fill_rect( target, 0, y2, target->width, 1,
                                    STEM_HINT_COLOR );
        }
        else
        {
          y1 -= 1;
          y2  = y1 + 2;
        }
        
        nv_pixmap_fill_rect( target, cpos, y2, 1, y1-y2+1,
                                  STEM_JOIN_COLOR );
        cpos += 10;
      }
    }
  }
}

#endif

static void
handle_event( NVV_EventRec*   ev )
{
  switch (ev->key)
  {
    case NVV_Key_Left:
      {
        if ( glyph_index > 0 )
          glyph_index--;
        break;
      }

    case NVV_Key_Right:
      {
        if ( glyph_index+1 < face->num_glyphs )
          glyph_index++;
        break;
      }

    case NVV_KEY('x'):
      {
        option_show_axis = !option_show_axis;
        break;
      }

    case NVV_KEY('s'):
      {
        option_show_stroke = !option_show_stroke;
        break;
      }
    
    case NVV_KEY('g'):
      {
        option_show_glyph = !option_show_glyph;
        break;
      }
      
    case NVV_KEY('d'):
      {
        option_show_dots = !option_show_dots;
        break;
      }
      
    case NVV_KEY('e'):
      {
        option_show_em = !option_show_em;
        break;
      }
    
    case NVV_KEY('+'):
      {
        grid_scale *= 1.2;
        reset_scale( grid_scale );
        break;
      }
      
    case NVV_KEY('-'):
      {
        if (grid_scale > 0.3)
        {
          grid_scale /= 1.2;
          reset_scale( grid_scale );
        }
        break;
      }

    case NVV_Key_Up:
      {
        pixel_size++;
        reset_size( pixel_size, grid_scale );
        sprintf( temp_message, "pixel size = %d", pixel_size );
        break;
      }

    case NVV_Key_Down:
      {
        if (pixel_size > 1)
        {
          pixel_size--;
          reset_size( pixel_size, grid_scale );
          sprintf( temp_message, "pixel size = %d", pixel_size );
        }
        break;
      }

    case NVV_KEY('z'):
      {
        ps_debug_no_vert_hints = !ps_debug_no_vert_hints;
        sprintf( temp_message, "vertical hints processing is now %s",
                 ps_debug_no_vert_hints ? "off" : "on" );
        break;
      }      

    case NVV_KEY('a'):
      {
        ps_debug_no_horz_hints = !ps_debug_no_horz_hints;
        sprintf( temp_message, "horizontal hints processing is now %s",
                 ps_debug_no_horz_hints ? "off" : "on" );
        break;
      }      

    case NVV_KEY('Z'):
      {
        option_show_vert_hints = !option_show_vert_hints;
        sprintf( temp_message, "vertical hints display is now %s",
                 option_show_vert_hints ? "off" : "on" );
        break;
      }      

    case NVV_KEY('A'):
      {
        option_show_horz_hints = !option_show_horz_hints;
        sprintf( temp_message, "horizontal hints display is now %s",
                 option_show_horz_hints ? "off" : "on" );
        break;
      }      


    case NVV_KEY('h'):
      {
        option_hinting = !option_hinting;
        sprintf( temp_message, "hinting is now %s",
                 option_hinting ? "off" : "on" );
        break;
      }      
  }
}


int  main( int  argc, char**  argv )
{
  /* create library */
  error = nv_renderer_new( 0, &renderer );
  if (error) Panic( "could not create Nirvana renderer" );
  
  memory = nv_renderer_get_memory( renderer );

  error = nvv_display_new( renderer, &display );
  if (error) Panic( "could not create display" );
  
  error = nvv_surface_new( display, 400, 400, nv_pixmap_type_argb, &surface );
  if (error) Panic( "could not create surface" );

  target = nvv_surface_get_pixmap( surface );

  error = nv_painter_new( renderer, &painter );
  if (error) Panic( "could not create painter" );
  
  nv_painter_set_target( painter, target );
  
  clear_background();

  error = FT_Init_FreeType( &freetype );
  if (error) Panic( "could not initialise FreeType" );
  
  error = FT_New_Face( freetype, "h:/fonts/cour.pfa", 0, &face );
  if (error) Panic( "could not open font face" );

  reset_size( pixel_size, grid_scale );
 

  nvv_surface_set_title( surface, "FreeType Glyph Viewer" );

  {
    NVV_EventRec  event;

    glyph_index = 0;    
    for ( ;; )
    {
      clear_background();
      draw_grid();

      the_ps_hints = 0;
      draw_glyph( glyph_index );
      /* draw_ps_hints(); */
      
      nvv_surface_refresh( surface, NULL );

      nvv_surface_listen( surface, 0, &event );
      if ( event.key == NVV_Key_Esc )
        break;
        
      handle_event( &event );
      switch (event.key)
      {
        case NVV_Key_Esc:
          goto Exit;
          
        default:
          ;
      }
    }
  }
 
 Exit:
  /* wait for escape */
      
  
  /* destroy display (and surface) */
  nvv_display_unref( display );
  nv_renderer_unref( renderer );

  return 0;
}
