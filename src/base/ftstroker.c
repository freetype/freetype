#include <ft2build.h>
#include FT_STROKER_H
#include FT_TRIGONOMETRY_H

 /***************************************************************************/
 /***************************************************************************/
 /*****                                                                 *****/
 /*****                       STROKE BORDERS                            *****/
 /*****                                                                 *****/
 /***************************************************************************/
 /***************************************************************************/

  typedef enum
  {
    FT_STROKE_TAG_ON    = 1,   /* on-curve point  */
    FT_STROKE_TAG_CUBIC = 2,   /* cubic off-point */
    FT_STROKE_TAG_BEGIN = 4,   /* sub-path start  */
    FT_STROKE_TAG_END   = 8    /* sub-path end    */
  
  } FT_StrokeTags;


  typedef struct FT_StrokeBorderRec_
  {
    FT_UInt     num_points;
    FT_UInt     max_points;
    FT_Vector*  points;
    FT_Byte*    tags;
    FT_Bool     movable;
    FT_Int      start;    /* index of current sub-path start point */
    FT_Memory   memory;
  
  } FT_StrokeBorderRec, *FT_StrokeBorder;


  static FT_Error
  ft_stroke_border_grow( FT_StrokeBorder  border,
                         FT_UInt          new_points )
  {
    FT_UInt   old_max = border->max_points;
    FT_UInt   new_max = border->num_points + new_points;
    FT_Error  error   = 0;
    
    if ( new_max > old_max )
    {
      FT_UInt    cur_max = old_max;
      FT_Memory  memory  = 
      
      while ( cur_max < new_max )
        cur_max += (cur_max >> 1) + 16;

      if ( FT_RENEW_ARRAY( border->points, old_max, cur_max ) ||
           FT_RENEW_ARRAY( border->tags,   old_max, cur_max ) )
        goto Exit;
      
      border->max_points = cur_max;
    }
  Exit:
    return error;
  }

  static void
  ft_stroke_border_close( FT_StrokeBorder  border )
  {
    FT_ASSERT( border->start >= 0 );
    
    border->tags[ border->start        ] |= FT_STROKE_TAG_BEGIN;
    border->tags[ border->num_points-1 ] |= FT_STROKE_TAG_END;
    
    border->start = -1;
  }


  static FT_Error
  ft_stroke_border_lineto( FT_StrokeBorder  border,
                           FT_Vector*       to )
  {
    FT_Error  error;
    
    FT_ASSERT( border->start >= 0 );
    
    error = ft_stroker_border_grow( border, 1 );
    if (!error)
    {
      FT_Vector*  vec = border->points + border->num_points;
      FT_Byte*    tag = border->tags   + border->num_points;
      
      vec[0] = *to;
      tag[0] = FT_STROKE_TAG_ON;
      
      border->num_points += 1;
    }
    return error;
  }                           


  static FT_Error
  ft_stroke_border_conicto( FT_StrokeBorder  border,
                            FT_Vector*       control,
                            FT_Vector*       to )
  {
    FT_Error  error;
    
    FT_ASSERT( border->start >= 0 );
    
    error = ft_stroker_border_grow( border, 2 );
    if (!error)
    {
      FT_Vector*  vec = border->points + border->num_points;
      FT_Byte*    tag = border->tags   + border->num_points;
      
      vec[0] = *control;
      vec[1] = *to;

      tag[0] = 0;
      tag[1] = FT_STROKE_TAG_ON;
      
      border->num_points += 2;
    }
    return error;
  }                           


  static FT_Error
  ft_stroke_border_cubicto( FT_StrokeBorder  border,
                            FT_Vector*       control1,
                            FT_Vector*       control2,
                            FT_Vector*       to )
  {
    FT_Error  error;
    
    FT_ASSERT( border->start >= 0 );
    
    error = ft_stroker_border_grow( border, 3 );
    if (!error)
    {
      FT_Vector*  vec = border->points + border->num_points;
      FT_Byte*    tag = border->tags   + border->num_points;
      
      vec[0] = *control1;
      vec[1] = *control2;
      vec[2] = *to;
      
      tag[0] = FT_STROKE_TAG_CUBIC;
      tag[1] = FT_STROKE_TAG_CUBIC;
      tag[2] = FT_STROKE_TAG_ON;
      
      border->num_points += 3;
    }
    return error;
  }


  static FT_Error
  ft_stroke_border_moveto( FT_StrokeBorder  border,
                           FT_Vector*       to )
  {
    FT_Error  error;
    
    /* close current open path if any ? */
    if ( border->start >= 0 )
      ft_stroke_border_close( border );
    
    border->start = border->num_points;

    return ft_stroke_border_lineto( border, to );
  }
  
 
 static void
 ft_stroke_border_init( FT_StrokeBorder  border,
                        FT_Memory        memory )
 {
   border->memory = memory;
   border->points = NULL;
   border->tags   = NULL;
   
   border->num_points = 0;
   border->max_points = 0;
   border->start      = -1;
 }
 

 static void
 ft_stroke_border_reset( FT_StrokeBorder  border )
 {
   border->num_points = 0;
   border->start      = -1;
 }
 

 static void
 ft_stroke_border_done( FT_StrokeBorder  border )
 {
   memory = border->memory;
   
   FT_FREE( border->points );
   FT_FREE( border->tags );
   
   border->num_points = 0;
   border->max_points = 0;
   border->start      = -1;
 }


 /***************************************************************************/
 /***************************************************************************/
 /*****                                                                 *****/
 /*****                           STROKER                               *****/
 /*****                                                                 *****/
 /***************************************************************************/
 /***************************************************************************/

#define  FT_SIDE_TO_ROTATE(s)   (FT_PI2 - (s)*FT_PI)

  typedef struct FT_StrokerRec_
  {
    FT_Angle             angle_in;
    FT_Angle             angle_out;
    FT_Vector            center;
    FT_Bool              first_point;
    FT_Bool              subpath_open;
    FT_Angle             subpath_angle;
    FT_Vector            subpath_start;

    FT_Stroker_LineCap   line_cap;
    FT_Stroker_LineJoin  line_join;
    FT_Fixed             miter_limit;
    FT_Fixed             radius;

    FT_StrokeBorderRec   borders[2];
    FT_Memory            memory;
  
  } FT_StrokerRec;


  FT_EXPORT_DEF( FT_Error )
  FT_Stroker_New( FT_Memory    memory,
                  FT_Stroker  *astroker )
  {
    FT_Error    error;
    FT_Stroker  stroker;
    
    if ( !FT_NEW( stroker ) )
    {
      stroker->memory = memory;
      
      ft_stroke_border_init( &stroker->borders[0], memory );
      ft_stroke_border_init( &stroker->borders[1], memory );
    }
    *astroker = stroker;
    return error;
  }


  FT_EXPORT_DEF( void )
  FT_Stroker_Set( FT_Stroker           stroker,
                  FT_Fixed             radius,
                  FT_Stroker_LineCap   line_cap,
                  FT_Stroker_LineJoin  line_join,
                  FT_Fixed             miter_limit )
  {
    stroker->radius      = radius;
    stroker->line_cap    = line_cap;
    stroker->line_join   = line_join;
    stroker->miter_limit = miter_limit;
    
    ft_stroke_border_reset( &stroker->borders[0] );
    ft_stroke_border_reset( &stroker->borders[1] );
  }


  FT_EXPORT( void )
  FT_Stroker_Done( FT_Stroker  stroker )
  {
    if ( stroker )
    {
      FT_Memory  memory = stroker->memory;
      
      ft_stroke_border_done( &stroker->borders[0] );
      ft_stroke_border_done( &stroker->borders[1] );
      
      stroker->memory = NULL;
      FT_FREE( stroker );
    }
  }



  