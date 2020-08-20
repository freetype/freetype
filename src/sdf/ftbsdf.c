
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftmemory.h>
#include <freetype/fttrigon.h>

#include "ftsdf.h"
#include "ftsdferrs.h"
#include "ftsdfcommon.h"


  /**************************************************************************
   *
   * useful macros
   *
   */

#define ONE  65536 /* 1 in 16.16 */


  /**************************************************************************
   *
   * structs
   *
   */


  /**************************************************************************
   *
   * @Struct:
   *   BSDF_TRaster
   *
   * @Description:
   *   This struct is used in place of @FT_Raster and is stored within the
   *   internal FreeType renderer struct.  While rasterizing this is passed
   *   to the @FT_Raster_RenderFunc function, which then can be used however
   *   we want.
   *
   * @Fields:
   *   memory ::
   *     Used internally to allocate intermediate memory while raterizing.
   *
   */
  typedef struct  BSDF_TRaster_
  {
    FT_Memory  memory;

  } BSDF_TRaster;


  /**************************************************************************
   *
   * @Struct:
   *   ED
   *
   * @Description:
   *   Euclidean distance.  It gets used for Euclidean distance transforms;
   *   it can also be interpreted as an edge distance.
   *
   * @Fields:
   *   dist ::
   *     Vector length of the `near` parameter.  Can be squared or absolute
   *     depending on the `USE_SQUARED_DISTANCES` macro defined in file
   *     `ftsdfcommon.h`.
   *
   *   near ::
   *     Vector to the nearest edge.  Can also be interpreted as shortest
   *     distance of a point.
   *
   *   alpha ::
   *     Alpha value of the original bitmap from which we generate SDF.
   *     Needed for computing the gradient and determining the proper sign
   *     of a pixel.
   *
   */
  typedef struct  ED_
  {
    FT_16D16      dist;
    FT_16D16_Vec  near;
    FT_Byte       alpha;

  } ED;


  /**************************************************************************
   *
   * @Struct:
   *   BSDF_Worker
   *
   * @Description:
   *   A convenience struct that is passed to functions while generating
   *   SDF; most of those functions require the same parameters.
   *
   * @Fields:
   *   distance_map ::
   *     A one-dimensional array that gets interpreted as two-dimensional
   *     one.  It contains the Euclidean distances of all points of the
   *     bitmap.
   *
   *   width ::
   *     Width of the above `distance_map`.
   *
   *   rows ::
   *     Number of rows in the above `distance_map`.
   *
   *   params ::
   *     Internal parameters and properties required by the rasterizer.  See
   *     file `ftsdf.h` for more.
   *
   */
  typedef struct  BSDF_Worker_
  {
    ED*  distance_map;

    FT_Int  width;
    FT_Int  rows;

    SDF_Raster_Params  params;

  } BSDF_Worker;


  /**************************************************************************
   *
   * initializer
   *
   */

  static const ED  zero_ed = { 0, { 0, 0 }, 0 };


  /**************************************************************************
   *
   * rasterizer functions
   *
   */

  /**************************************************************************
   *
   * @Function:
   *   bsdf_is_edge
   *
   * @Description:
   *   Check whether a pixel is an edge pixel, i.e., whether it is
   *   surrounded by a completely black pixel (zero alpha), and the current
   *   pixel is not a completely black pixel.
   *
   * @Input:
   *   dm ::
   *     Array of distances.  The parameter must point to the current
   *     pixel, i.e., the pixel that is to be checked for being an edge.
   *
   *   x ::
   *     The x position of the current pixel.
   *
   *   y ::
   *     The y position of the current pixel.
   *
   *   w ::
   *     Width of the bitmap.
   *
   *   r ::
   *     Number of rows in the bitmap.
   *
   * @Return:
   *   1~if the current pixel is an edge pixel, 0~otherwise.
   *
   */

#ifdef CHECK_NEIGHBOR
#undef CHECK_NEIGHBOR
#endif

#define CHECK_NEIGHBOR( x_offset, y_offset )            \
          if ( x + x_offset >= 0 && x + x_offset < w && \
               y + y_offset >= 0 && y + y_offset < r )  \
          {                                             \
            num_neighbors++;                            \
                                                        \
            to_check = dm + y_offset * w + x_offset;    \
            if ( to_check->alpha == 0 )                 \
            {                                           \
              is_edge = 1;                              \
              goto Done;                                \
            }                                           \
          }

  static FT_Bool
  bsdf_is_edge( ED*     dm,   /* distance map              */
                FT_Int  x,    /* x index of point to check */
                FT_Int  y,    /* y index of point to check */
                FT_Int  w,    /* width                     */
                FT_Int  r )   /* rows                      */
  {
    FT_Bool  is_edge       = 0;
    ED*      to_check      = NULL;
    FT_Int   num_neighbors = 0;


    if ( dm->alpha == 0 )
      goto Done;

    if ( dm->alpha > 0 && dm->alpha < 255 )
    {
      is_edge = 1;
      goto Done;
    }

    /* up */
    CHECK_NEIGHBOR(  0, -1 );

    /* down */
    CHECK_NEIGHBOR(  0,  1 );

    /* left */
    CHECK_NEIGHBOR( -1,  0 );

    /* right */
    CHECK_NEIGHBOR(  1,  0 );

    /* up left */
    CHECK_NEIGHBOR( -1, -1 );

    /* up right */
    CHECK_NEIGHBOR(  1, -1 );

    /* down left */
    CHECK_NEIGHBOR( -1,  1 );

    /* down right */
    CHECK_NEIGHBOR(  1,  1 );

    if ( num_neighbors != 8 )
      is_edge = 1;

  Done:
    return is_edge;
  }

#undef CHECK_NEIGHBOR


  /**************************************************************************
   *
   * @Function:
   *   compute_edge_distance
   *
   * @Description:
   *   Approximate the outline and compute the distance from `current`
   *   to the approximated outline.
   *
   * @Input:
   *   current ::
   *     Array of Euclidean distances.  `current` must point to the position
   *     for which the distance is to be caculated.  We treat this array as
   *     a two-dimensional array mapped to a one-dimensional array.
   *
   *   x ::
   *     The x coordinate of the `current` parameter in the array.
   *
   *   y ::
   *     The y coordinate of the `current` parameter in the array.
   *
   *   w ::
   *     The width of the distances array.
   *
   *   r ::
   *     Number of rows in the distances array.
   *
   * @Return:
   *   A vector pointing to the approximate edge distance.
   *
   * @Note:
   *   This is a computationally expensive function.  Try to reduce the
   *   number of calls to this function.  Moreover, this must only be used
   *   for edge pixel positions.
   *
   */
  static FT_16D16_Vec
  compute_edge_distance( ED*     current,
                         FT_Int  x,
                         FT_Int  y,
                         FT_Int  w,
                         FT_Int  r )
  {
    /*
     * This function, based on the paper presented by Stefan Gustavson and
     * Robin Strand, gets used to approximate edge distances from
     * anti-aliased bitmaps.
     *
     * The algorithm is as follows.
     *
     * (1) In anti-aliased images, the pixel's alpha value is the coverage
     *     of the pixel by the outline.  For example, if the alpha value is
     *     0.5f we can assume that the outline passes through the center of
     *     the pixel.
     *
     * (2) For this reason we can use that alpha value to approximate the real
     *     distance of the pixel to edge pretty accurately.  A simple
     *     approximation is `(0.5f - alpha)`, assuming that the outline is
     *     parallel to the x or y~axis.  However, in this algorithm we use a
     *     different approximation which is quite accurate even for
     *     non-axis-aligned edges.
     *
     * (3) The only remaining piece of information that we cannot
     *     approximate directly from the alpha is the direction of the edge. 
     *     This is where we use Sobel's operator to compute the gradient of
     *     the pixel.  The gradient give us a pretty good approximation of
     *     the edge direction.  We use a 3x3 kernel filter to compute the
     *     gradient.
     *
     * (4) After the above two steps we have both the direction and the
     *     distance to the edge which is used to generate the Signed
     *     Distance Field.
     *
     * References:
     *
     * - Anti-Aliased Euclidean Distance Transform:
     *     http://weber.itn.liu.se/~stegu/aadist/edtaa_preprint.pdf
     * - Sobel Operator:
     *     https://en.wikipedia.org/wiki/Sobel_operator
     */

    FT_16D16_Vec  g = { 0, 0 };
    FT_16D16      dist, current_alpha;
    FT_16D16      a1, temp;
    FT_16D16      gx, gy;
    FT_16D16      alphas[9];


    /* Since our spread cannot be 0, this condition */
    /* can never be true.                           */
    if ( x <= 0 || x >= w - 1 ||
         y <= 0 || y >= r - 1 )
      return g;

    /* initialize the alphas */
    alphas[0] = 256 * (FT_16D16)current[-w - 1].alpha;
    alphas[1] = 256 * (FT_16D16)current[-w    ].alpha;
    alphas[2] = 256 * (FT_16D16)current[-w + 1].alpha;
    alphas[3] = 256 * (FT_16D16)current[    -1].alpha;
    alphas[4] = 256 * (FT_16D16)current[     0].alpha;
    alphas[5] = 256 * (FT_16D16)current[     1].alpha;
    alphas[6] = 256 * (FT_16D16)current[ w - 1].alpha;
    alphas[7] = 256 * (FT_16D16)current[ w    ].alpha;
    alphas[8] = 256 * (FT_16D16)current[ w + 1].alpha;

    current_alpha = alphas[4];

    /* Compute the gradient using the Sobel operator. */
    /* In this case we use the following 3x3 filters: */
    /*                                                */
    /* For x: |   -1     0   -1    |                  */
    /*        | -root(2) 0 root(2) |                  */
    /*        |    -1    0    1    |                  */
    /*                                                */
    /* For y: |   -1 -root(2) -1   |                  */
    /*        |    0    0      0   |                  */
    /*        |    1  root(2)  1   |                  */
    /*                                                */
    /* [Note]: 92681 is root(2) in 16.16 format.      */
    g.x = -alphas[0] -
           FT_MulFix( alphas[3], 92681 ) -
           alphas[6] +
           alphas[2] +
           FT_MulFix( alphas[5], 92681 ) +
           alphas[8];

    g.y = -alphas[0] -
           FT_MulFix( alphas[1], 92681 ) -
           alphas[2] +
           alphas[6] +
           FT_MulFix( alphas[7], 92681 ) +
           alphas[8];

    FT_Vector_NormLen( &g );

    /* The gradient gives us the direction of the    */
    /* edge for the current pixel.  Once we have the */
    /* approximate direction of the edge, we can     */
    /* approximate the edge distance much better.    */

    if ( g.x == 0 || g.y == 0 )
      dist = ONE / 2 - alphas[4];
    else
    {
      gx = g.x;
      gy = g.y;

      gx = FT_ABS( gx );
      gy = FT_ABS( gy );

      if ( gx < gy )
      {
        temp = gx;
        gx   = gy;
        gy   = temp;
      }

      a1 = FT_DivFix( gy, gx ) / 2;

      if ( current_alpha < a1 )
        dist = ( gx + gy ) / 2 -
               square_root( 2 * FT_MulFix( gx,
                                           FT_MulFix( gy,
                                                      current_alpha ) ) );

      else if ( current_alpha < ( ONE - a1 ) )
        dist = FT_MulFix( ONE / 2 - current_alpha, gx );

      else
        dist = -( gx + gy ) / 2 +
               square_root( 2 * FT_MulFix( gx,
                                           FT_MulFix( gy,
                                                      ONE - current_alpha ) ) );
    }

    g.x = FT_MulFix( g.x, dist );
    g.y = FT_MulFix( g.y, dist );

    return g;
  }


  /**************************************************************************
   *
   * @Function:
   *   bsdf_approximate_edge
   *
   * @Description:
   *   Loops over all the pixels and call `compute_edge_distance` only for
   *   edge pixels.  This maked the process a lot faster since
   *   `compute_edge_distance` uses functions such as `FT_Vector_NormLen',
   *   which are quite slow.
   *
   * @InOut:
   *   worker ::
   *     Contains the distance map as well as all the relevant parameters
   *     required by the function.
   *
   * @Return:
   *   FreeType error, 0 means success.
   *
   * @Note:
   *   The function directly manipulates `worker->distance_map`.
   *
   */
  static FT_Error
  bsdf_approximate_edge( BSDF_Worker*  worker )
  {
    FT_Error  error = FT_Err_Ok;
    FT_Int    i, j;
    FT_Int    index;
    ED*       ed;


    if ( !worker || !worker->distance_map )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    ed = worker->distance_map;

    for ( j = 0; j < worker->rows; j++ )
    {
      for ( i = 0; i < worker->width; i++ )
      {
        index = j * worker->width + i;

        if ( bsdf_is_edge( worker->distance_map + index,
                           i, j,
                           worker->width,
                           worker->rows ) )
        {
          /* approximate the edge distance for edge pixels */
          ed[index].near = compute_edge_distance( ed + index,
                                                  i, j,
                                                  worker->width,
                                                  worker->rows );
          ed[index].dist = VECTOR_LENGTH_16D16( ed[index].near );
        }
        else
        {
          /* for non-edge pixels assign far away distances */
          ed[index].dist   = 400 * ONE;
          ed[index].near.x = 200 * ONE;
          ed[index].near.y = 200 * ONE;
        }
      }
    }

  Exit:
    return error;
  }

/* END */
