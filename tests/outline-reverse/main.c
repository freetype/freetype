/* Regression tests for cubic contours with implicit on-curve points. */

#include <stdio.h>
#include <string.h>
#include <ft2build.h>
#include FT_OUTLINE_H
#include FT_BBOX_H


typedef struct  Segment_
{
  FT_Vector  points[4];

} Segment;

typedef struct  Recording_
{
  FT_Vector  current;
  Segment    segments[8];
  FT_UInt    count;

} Recording;


static int
move_to( const FT_Vector*  to,
         void*             user )
{
  ( (Recording*)user )->current = *to;
  return 0;
}


static int
conic_to( const FT_Vector*  control,
          const FT_Vector*  to,
          void*             user )
{
  (void)control;
  (void)to;
  (void)user;
  return 1;  /* These contours must contain only cubic segments. */
}


static int
cubic_to( const FT_Vector*  control1,
          const FT_Vector*  control2,
          const FT_Vector*  to,
          void*             user )
{
  Recording*  recording = (Recording*)user;
  Segment*    segment;


  if ( recording->count >= 8 )
    return 1;

  segment = &recording->segments[recording->count++];

  segment->points[0] = recording->current;
  segment->points[1] = *control1;
  segment->points[2] = *control2;
  segment->points[3] = *to;

  recording->current = *to;
  return 0;
}


static int
test_contour( const FT_Vector*  source_points,
              const FT_Byte*    source_tags,
              FT_UShort         count )
{
  static const FT_Outline_Funcs  funcs =
  {
    move_to, move_to, conic_to, cubic_to, 0, 0
  };

  FT_Vector   points[7];
  FT_Byte     tags[7];
  FT_UShort   end = count - 1;
  FT_Outline  outline = { 1, count, points, tags, &end, 0 };
  Recording   before, after;
  FT_BBox     bbox_before, bbox_after;
  FT_UInt     i, j;


  memcpy( points, source_points, count * sizeof ( *points ) );
  memcpy( tags, source_tags, count * sizeof ( *tags ) );
  memset( &before, 0, sizeof ( before ) );
  memset( &after, 0, sizeof ( after ) );

  if ( FT_Outline_Decompose( &outline, &funcs, &before ) ||
       FT_Outline_Get_BBox( &outline, &bbox_before ) )
    return 1;

  FT_Outline_Reverse( &outline );

  if ( FT_Outline_Decompose( &outline, &funcs, &after ) ||
       FT_Outline_Get_BBox( &outline, &bbox_after )     ||
       before.count != after.count )
    return 1;

  for ( i = 0; i < before.count; i++ )
    for ( j = 0; j < 4; j++ )
    {
      FT_Vector  a = before.segments[i].points[j];
      FT_Vector  b = after.segments[before.count - 1 - i].points[3 - j];


      if ( a.x != b.x || a.y != b.y )
        return 1;
    }

  if ( bbox_before.xMin != bbox_after.xMin ||
       bbox_before.yMin != bbox_after.yMin ||
       bbox_before.xMax != bbox_after.xMax ||
       bbox_before.yMax != bbox_after.yMax )
    return 1;

  FT_Outline_Reverse( &outline );

  after.count = 0;
  if ( outline.flags != 0                               ||
       FT_Outline_Decompose( &outline, &funcs, &after ) ||
       before.count != after.count                      )
    return 1;

  for ( i = 0; i < before.count; i++ )
    for ( j = 0; j < 4; j++ )
      if ( before.segments[i].points[j].x != after.segments[i].points[j].x ||
           before.segments[i].points[j].y != after.segments[i].points[j].y )
        return 1;

  return 0;
}


int
main( void )
{
  static const FT_Vector  points[] =
  {
    { 0, 0 }, { 100, 0 }, { 100, 100 }, { 0, 100 },
    { -100, 100 }, { -100, 0 }, { 0, -100 }
  };
  static const FT_Byte  tags[][7] =
  {
    { 2, 2, 1 },             /* starts with two cubic controls */
    { 2, 2 },                /* no explicit on-curve point */
    { 2, 2, 2, 2 },
    { 2, 2, 2, 2, 2, 2 },
    { 2, 2, 1, 2, 2 },       /* control pairs on both sides of the start */
    { 1, 2, 2, 2, 2, 2, 2 }, /* implicit cubic endpoints */
    { 3, 2, 2, 2, 2, 2, 2 }  /* on-curve point also has cubic flag */
  };
  static const FT_UShort  counts[] = { 3, 2, 4, 6, 5, 7, 7 };

  FT_UInt  i;


  for ( i = 0; i < sizeof ( counts ) / sizeof ( counts[0] ); i++ )
    if ( test_contour( points, tags[i], counts[i] ) )
    {
      fprintf( stderr, "Cubic contour reversal failed: case %u\n", i );
      return 1;
    }

  return 0;
}
