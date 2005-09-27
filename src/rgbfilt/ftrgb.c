#include <ft2build.h>
#include FT_RGB_FILTER_H
#include FT_INTERNAL_OBJECTS_H

typedef struct FT_RgbFilterRec_
{
  FT_Fixed   factors[9];
  FT_Memory  memory;

} FT_RgbFilterRec;

typedef struct FT_RgbFilterinRec_
{
  FT_Fixed       factors[9];
  FT_Byte*       in_line;
  FT_Long        in_pitch;
  FT_Byte*       out_line;
  FT_Long        out_pitch;
  FT_Int         width;
  FT_Int         height;

} FT_RgbFilteringRec, *FT_RgbFiltering;


/* these values come from libXft */
static const FT_RgbFilterRec  ft_rgbfilter_default =
{
  { 65538*9/13, 65538*3/13, 65538*1/13,
    65538*1/6,  65538*4/6,  65538*1/6,
    65538*1/13, 65538*3/13, 65538*9/13 },
  NULL
};

static void
ft_rgbfilter_apply_argb_rgb( FT_RgbFiltering  oper )
{
#define  HMUL   3
#define  VMUL   1
#define  OFF_R  0
#define  OFF_G  1
#define  OFF_B  2
#include "ftrgbgen.h"
}


static void
ft_rgbfilter_apply_argb_bgr( FT_RgbFiltering  oper )
{
#define  HMUL   3
#define  VMUL   1
#define  OFF_R  2
#define  OFF_G  1
#define  OFF_B  0
#include "ftrgbgen.h"
}


static void
ft_rgbfilter_apply_argb_vrgb( FT_RgbFiltering  oper )
{
#define  HMUL   1
#define  VMUL   3
#define  OFF_R  0
#define  OFF_G  1
#define  OFF_B  2
#include "ftrgbgen.h"
}


static void
ft_rgbfilter_apply_argb_vbgr( FT_RgbFiltering  oper )
{
#define  HMUL   1
#define  VMUL   3
#define  OFF_R  2
#define  OFF_G  1
#define  OFF_B  0
#include "ftrgbgen.h"
}



static void
ft_rgbfilter_apply_inplace_rgb( FT_RgbFiltering  oper )
{
#define  HMUL   3
#define  VMUL   1
#define  OFF_R  0
#define  OFF_G  1
#define  OFF_B  2
#include "ftrgbgn2.h"
}


static void
ft_rgbfilter_apply_inplace_bgr( FT_RgbFiltering  oper )
{
#define  HMUL   3
#define  VMUL   1
#define  OFF_R  2
#define  OFF_G  1
#define  OFF_B  0
#include "ftrgbgn2.h"
}


static void
ft_rgbfilter_apply_inplace_vrgb( FT_RgbFiltering  oper )
{
#define  HMUL   1
#define  VMUL   3
#define  OFF_R  0
#define  OFF_G  1
#define  OFF_B  2
#include "ftrgbgn2.h"
}


static void
ft_rgbfilter_apply_inplace_vbgr( FT_RgbFiltering  oper )
{
#define  HMUL   1
#define  VMUL   3
#define  OFF_R  2
#define  OFF_G  1
#define  OFF_B  0
#include "ftrgbgn2.h"
}



FT_EXPORT_DEF( FT_Error )
FT_RgbFilter_ApplyARGB( FT_RgbFilter   filter,
                        FT_Bool        inverted,
                        FT_Pixel_Mode  in_mode,
                        FT_Byte*       in_bytes,
                        FT_Long        in_pitch,
                        FT_Int         out_width,
                        FT_Int         out_height,
                        FT_UInt32*     out_bytes,
                        FT_Long        out_pitch )
{
  FT_RgbFilteringRec  oper;
  int                 in_width  = out_width;
  int                 in_height = out_height;

  switch ( in_mode )
  {
    case FT_PIXEL_MODE_LCD:
      in_width = 3*out_width;
      break;

    case FT_PIXEL_MODE_LCD_V:
      in_height = 3*out_height;
      break;

    default:
      return FT_Err_Invalid_Argument;
  }

  if ( FT_ABS(in_pitch)  < in_width    ||
       FT_ABS(out_pitch) < 4*out_width )
    return FT_Err_Invalid_Argument;

  oper.width  = out_width;
  oper.height = out_height;

  oper.in_line   = in_bytes;
  oper.in_pitch  = in_pitch;
  if ( in_pitch < 0 )
    oper.in_line  -= in_pitch*(in_height-1);

  oper.out_line  = (FT_Byte*) out_bytes;
  oper.out_pitch = out_pitch;
  if ( out_pitch < 0 )
    oper.out_line  -= out_pitch*(out_height-1);

  if ( filter == NULL )
    filter = (FT_RgbFilter)&ft_rgbfilter_default;

  FT_ARRAY_COPY( oper.factors, filter->factors, 9 );

  switch ( in_mode )
  {
    case FT_PIXEL_MODE_LCD:
      if ( inverted )
        ft_rgbfilter_apply_argb_bgr( &oper );
      else
        ft_rgbfilter_apply_argb_rgb( &oper );
      break;

    case FT_PIXEL_MODE_LCD_V:
      if ( inverted )
        ft_rgbfilter_apply_argb_vbgr( &oper );
      else
        ft_rgbfilter_apply_argb_vrgb( &oper );
      break;

    default:
      ;
  }
  return 0;
}


FT_EXPORT_DEF( FT_Error )
FT_RgbFilter_ApplyInPlace( FT_RgbFilter   filter,
                           FT_Bool        inverted,
                           FT_Pixel_Mode  in_mode,
                           FT_Byte*       in_bytes,
                           FT_Long        in_pitch,
                           FT_Int         org_width,
                           FT_Int         org_height )
{
  FT_RgbFilteringRec  oper;
  int                 in_width  = org_width;
  int                 in_height = org_height;

  switch ( in_mode )
  {
    case FT_PIXEL_MODE_LCD:
      in_width = 3*org_width;
      break;

    case FT_PIXEL_MODE_LCD_V:
      in_height = 3*org_height;
      break;

    default:
      return FT_Err_Invalid_Argument;
  }

  if ( FT_ABS(in_pitch)  < in_width )
    return FT_Err_Invalid_Argument;

  oper.width  = org_width;
  oper.height = org_height;

  oper.in_line   = in_bytes;
  oper.in_pitch  = in_pitch;
  if ( in_pitch < 0 )
    oper.in_line  -= in_pitch*(in_height-1);

  if ( filter == NULL )
    filter = (FT_RgbFilter)&ft_rgbfilter_default;

  FT_ARRAY_COPY( oper.factors, filter->factors, 9 );

  switch ( in_mode )
  {
    case FT_PIXEL_MODE_LCD:
      if ( inverted )
        ft_rgbfilter_apply_inplace_bgr( &oper );
      else
        ft_rgbfilter_apply_inplace_rgb( &oper );
      break;

    case FT_PIXEL_MODE_LCD_V:
      if ( inverted )
        ft_rgbfilter_apply_inplace_vbgr( &oper );
      else
        ft_rgbfilter_apply_inplace_vrgb( &oper );
      break;

    default:
      ;
  }
  return 0;
}




FT_EXPORT_DEF( FT_Error )
FT_RgbFilter_New( FT_Library     library,
                  FT_Fixed*      values_9,
                  FT_RgbFilter  *pfilter )
{
  FT_Error      error  = 0;
  FT_RgbFilter  filter = NULL;

  if ( !library || !values_9 )
    error = FT_Err_Invalid_Argument;
  else
  {
    FT_Memory  memory = library->memory;

    if ( !FT_NEW( filter ) )
    {
      FT_ARRAY_COPY( filter->factors, values_9, 9 );
      filter->memory = memory;
    }
  }
  *pfilter = filter;
  return error;
}


FT_EXPORT_DEF( void )
FT_RgbFilter_Done( FT_RgbFilter  filter )
{
  if ( filter )
  {
    FT_Memory  memory = filter->memory;

    filter->memory = NULL;
    if ( memory )
      FT_FREE( filter );
  }
}


FT_EXPORT_DEF( void )
FT_RgbFilter_Reset( FT_RgbFilter  filter,
                    FT_Fixed*     values_9 )
{
  if ( filter && values_9 )
    FT_ARRAY_COPY( filter->factors, values_9, 9 );
}
