
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

  #define ONE 65536 /* 1 in 16.16 */

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
   *   This struct is used in place of `FT_Raster' and is stored within
   *   the internal freetype renderer struct. While rasterizing this is
   *   passed to the `FT_Raster_Render_Func' function, which then can be
   *   used however we want.
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
   *   Euclidean distance used for euclidean distance transform can also be
   *   interpreted as edge distance.
   *
   * @Fields:
   *   dist ::
   *     Vector length of the `near' parameter. Can be squared or absolute
   *     depending on the `USE_SQUARED_DISTANCES' parameter defined in
   *     `ftsdfcommon.h'.
   *
   *   near ::
   *     Vector to the nearest edge. Can also be interpreted as shortest
   *     distance of a point.
   *
   *   alpha ::
   *     Alpha value of the original bitmap from which we generate SDF.
   *     While computing the gradient and determining the proper sign
   *     of a pixel this field is used.
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
   *   Just a convenient struct which is passed to most of the functions
   *   while generating SDF. This makes it easier to pass parameters because
   *   most functions require the same parameters.
   *
   * @Fields:
   *   distance_map ::
   *     A 1D array which is interpreted as 2D array. This array contains
   *     the Euclidean distance of all the points of the bitmap.
   *
   *   width ::
   *     Width of the above `distance_map'.
   *     
   *   rows ::
   *     Number of rows in the above `distance_map'.
   *
   *   params ::
   *     Internal params and properties required by the rasterizer. See
   *     `ftsdf.h' for the fields of this struct.
   *
   */
  typedef struct  BSDF_Worker_
  {
    ED*                distance_map;

    FT_Int             width;
    FT_Int             rows;

    SDF_Raster_Params  params;

  } BSDF_Worker;

  /**************************************************************************
   *
   * initializer
   *
   */

  static
  const ED  zero_ed = { 0, { 0, 0 }, 0 };

  /**************************************************************************
   *
   * rasterizer functions
   *
   */

  #ifdef CHECK_NEIGHBOR
  #undef CHECK_NEIGHBOR
  #endif

  /* Use the macro only in `bsdf_is_edge' function. */
  #define CHECK_NEIGHBOR( x_offset, y_offset )     \
    if ( x + x_offset >= 0 && x + x_offset < w &&  \
         y + y_offset >= 0 && y + y_offset < r )   \
    {                                              \
      num_neighbour++;                             \
      to_check = dm + y_offset * w + x_offset;     \
      if ( to_check->alpha == 0 )                  \
      {                                            \
        is_edge = 1;                               \
        goto Done;                                 \
      }                                            \
    }

  /**************************************************************************
   *
   * @Function:
   *   bsdf_is_edge
   *
   * @Description:
   *   This function checks weather a pixel is an edge pixel. A pixel
   *   is edge bixel if it surrounded by a completely black pixel ( 0
   *   alpha ) and the current pixel is not a completely black pixel.
   *
   * @Input:
   *   dm ::
   *     Array of distances. The parameter must point to the current
   *     pixel i.e. the pixel that is to be checked for edge.
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
   *   FT_Bool ::
   *     1 if the current pixel is an edge pixel, 0 otherwise.
   *
   */
  static FT_Bool
  bsdf_is_edge( ED*       dm,   /* distance map              */
                FT_Int    x,    /* x index of point to check */
                FT_Int    y,    /* y index of point to check */
                FT_Int    w,    /* width                     */
                FT_Int    r )   /* rows                      */
  {
    FT_Bool   is_edge       = 0; 
    ED*       to_check      = NULL;
    FT_Int    num_neighbour = 0;


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

    if ( num_neighbour != 8 )
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
   *   Approximate the outline and compute the distance from `current'
   *   to the approximated outline.
   *
   * @Input:
   *   current ::
   *     Array of distances. This parameter is an array of Euclidean
   *     distances. The `current' must point to the position for which
   *     the distance is to be caculated. We treat this array as a 2D
   *     array mapped to a 1D array.
   *
   *   x ::
   *     The x coordinate of the `current' parameter in the array.
   *
   *   y ::
   *     The y coordinate of the `current' parameter in the array.
   *
   *   w ::
   *     The width of the distances array.
   *
   *   r ::
   *     Number of rows in the distances array.
   *
   * @Return:
   *   FT_16D16_Vec ::
   *     A vector pointing to the approximate edge distance.
   *
   * @Note:
   *   This is a computationally expensive function. Try to reduce the
   *   number of calls to this function. Moreover this must only be used
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
    /* This is the function which is based on the paper presented */
    /* by Stefan Gustavson and Robin Strand which is used to app- */
    /* roximate edge distance from anti-aliased bitmaps.          */
    /*                                                            */
    /* The algorithm is as follows:                               */
    /*                                                            */
    /* * In anti-aliased images, the pixel's alpha value is the   */
    /*   coverage of the pixel by the outline. For example if the */
    /*   alpha value is 0.5f then we can assume the the outline   */
    /*   passes through the center of the pixel.                  */
    /*                                                            */
    /* * So, we can use that alpha value to approximate the real  */
    /*   distance of the pixel to edge pretty accurately. A real  */
    /*   simple approximation is ( 0.5f - alpha ), assuming that  */
    /*   the outline is parallel to the x or y axis. But in this  */
    /*   algorithm we use a different approximation which is qui- */
    /*   te accurate even for non axis aligned edges.             */
    /*                                                            */
    /* * The only remaining piece of information that we cannot   */
    /*   approximate directly from the alpha is the direction of  */
    /*   the edge. This is where we use the Sobel's operator to   */
    /*   compute the gradient of the pixel. The gradient give us  */
    /*   a pretty good approximation of the edge direction.       */
    /*   We use a 3x3 kernel filter to compute the gradient.      */
    /*                                                            */
    /* * After the above two steps we have both the direction and */
    /*   the distance to the edge which is used to generate the   */
    /*   Signed Distance Field.                                   */
    /*                                                            */
    /* References:                                                */
    /* * Anti-Aliased Euclidean Distance Transform:               */
    /*   http://weber.itn.liu.se/~stegu/aadist/edtaa_preprint.pdf */
    /* * Sobel Operator:                                          */
    /*   https://en.wikipedia.org/wiki/Sobel_operator             */
    /*                                                            */
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
    alphas[1] = 256 * (FT_16D16)current[  -w  ].alpha;
    alphas[2] = 256 * (FT_16D16)current[-w + 1].alpha;
    alphas[3] = 256 * (FT_16D16)current[  -1  ].alpha;
    alphas[4] = 256 * (FT_16D16)current[   0  ].alpha;
    alphas[5] = 256 * (FT_16D16)current[   1  ].alpha;
    alphas[6] = 256 * (FT_16D16)current[ w - 1].alpha;
    alphas[7] = 256 * (FT_16D16)current[   w  ].alpha;
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
    /* [Note]: 92681 is nothing but root(2) in 16.16  */
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

    /* The gradient gives us the direction of the   */
    /* edge for the current pixel. Once we have the */
    /* approximate direction of the edge, we can    */
    /* approximate the edge distance much better.   */

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
        gx = gy;
        gy = temp;
      }

      a1 = FT_DivFix( gy, gx ) / 2;
      if ( current_alpha < a1 )
        dist = (( gx + gy ) / 2) -
               square_root( 2 * FT_MulFix( gx, 
               FT_MulFix( gy, current_alpha ) ) );
      else if ( current_alpha < ( ONE - a1 ) )
        dist = FT_MulFix( ONE / 2 - current_alpha, gx );
      else
        dist = -(( gx + gy ) / 2) +
               square_root( 2 * FT_MulFix( gx,
               FT_MulFix( gy, ONE - current_alpha ) ) );
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
   *   This is a handy function which loops through all the pixels, and
   *   calls `compute_edge_distance' function only for edge pixels. This
   *   maked the process a lot faster since `compute_edge_distance' uses
   *   some functions such as `FT_Vector_NormLen' which are quite slow.
   *
   * @Input:
   *   worker ::
   *     Contains the distance map as well as all the relevant parameters
   *     required by the function.
   *
   * @Return:
   *   FT_Error ::
   *     FreeType error, 0 means success.
   *
   * @Note:
   *   The function dosen't have any actual output, it do computation on
   *   the `distance_map' parameter of the `worker' and put the data in
   *   that distance map itself.
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
             i, j, worker->width, worker->rows  ) )
        {
          /* for edge pixels approximate the edge distance */
          ed[index].near = compute_edge_distance( ed + index, i, j,
                             worker->width, worker->rows );
          ed[index].dist = VECTOR_LENGTH_16D16( ed[index].near );
        }
        else
        {
          /* for non edge pixels assign far away distances */
          ed[index].dist   = 400 * ONE;
          ed[index].near.x = 200 * ONE;
          ed[index].near.y = 200 * ONE;
        }
      }
    }

  Exit:
    return error;
  }

  /**************************************************************************
   *
   * @Function:
   *   bsdf_init_distance_map
   *
   * @Description:
   *   This function initialize the distance map according to
   *   algorithm `8-point sequential Euclidean distance mapping' (8SED).
   *   Basically it copy the `source' bitmap alpha values to the
   *   `distance_map->alpha' parameter of the `worker'.
   *
   * @Input:
   *   source ::
   *     Source bitmap to copy the data from.
   *
   * @Return:
   *   worker ::
   *     Target distance map to copy the data to.
   *
   *   FT_Error ::
   *     FreeType error, 0 means success.
   *
   */
  static FT_Error
  bsdf_init_distance_map( const FT_Bitmap*  source,
                          BSDF_Worker*      worker )
  {
    FT_Error  error         = FT_Err_Ok;

    FT_Int    x_diff, y_diff;
    FT_Int    t_i, t_j, s_i, s_j;
    FT_Byte*  s;
    ED*       t;

    /* again check the parameters (probably unnecessary) */
    if ( !source || !worker )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* Because of the way we convert bitmap to SDF     */
    /* i.e. aligning the source to the center of the   */
    /* target, the target's width/rows must be checked */
    /* before copying.                                 */
    if ( worker->width < source->width ||
         worker->rows  < source->rows )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* check pixel mode */
    if ( source->pixel_mode == FT_PIXEL_MODE_NONE )
    {
      FT_ERROR(( "[bsdf] bsdf_copy_source_to_target: "
                 "Invalid pixel mode of source bitmap" ));
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

  #ifdef FT_DEBUG_LEVEL_TRACE
    if ( source->pixel_mode == FT_PIXEL_MODE_MONO )
    {
      FT_TRACE0(( "[bsdf] bsdf_copy_source_to_target:\n"
                  "The `bsdf' renderer can convert monochrome bitmap\n"
                  "to SDF, but the results are not perfect because there\n"
                  "is no way to approximate actual outline from monochrome\n"
                  "bitmap. Consider using anti-aliased bitmap instead.\n" ));
    }
  #endif

    /* Calculate the difference in width and rows */
    /* of the target and source.                  */
    x_diff = worker->width - source->width;
    y_diff = worker->rows - source->rows;

    x_diff /= 2;
    y_diff /= 2;

    t = (ED*)worker->distance_map;
    s = source->buffer;

    /* For now we only support pixel mode `FT_PIXEL_MODE_MONO'  */
    /* and `FT_PIXEL_MODE_GRAY'. More will be added later.      */
    /* [NOTE]: We can also use `FT_Bitmap_Convert' to convert   */
    /*         bitmap to 8bpp. To avoid extra allocation and    */
    /*         since the target bitmap can be 16bpp we manually */
    /*         convert the source bitmap to desired bpp.        */
    switch ( source->pixel_mode ) {
    case FT_PIXEL_MODE_MONO:
    {
      FT_Int  t_width = worker->width;
      FT_Int  t_rows  = worker->rows;
      FT_Int  s_width = source->width;
      FT_Int  s_rows  = source->rows;


      for ( t_j = 0; t_j < t_rows; t_j++ )
      {
        for ( t_i = 0; t_i < t_width; t_i++ )
        {
          FT_Int   t_index = t_j * t_width + t_i;
          FT_Int   s_index;
          FT_Int   div, mod;
          FT_Byte  pixel, byte;


          t[t_index] = zero_ed;

          s_i = t_i - x_diff;
          s_j = t_j - y_diff;

          /* Assign 0 to padding similar to */
          /* the source bitmap.             */
          if ( s_i < 0 || s_i >= s_width ||
               s_j < 0 || s_j >= s_rows )
            continue;

          if ( worker->params.flip_y )
            s_index = ( s_rows - s_j - 1 ) * source->pitch;
          else
            s_index = s_j * source->pitch;

          div = s_index + s_i / 8;
          mod = 7 - s_i % 8;

          pixel = s[div];
          byte = 1 << mod;

          t[t_index].alpha = pixel & byte ? 255 : 0;

          pixel = 0;
        }
      }
      break;
    }
    case FT_PIXEL_MODE_GRAY:
    {
      FT_Int  t_width = worker->width;
      FT_Int  t_rows  = worker->rows;
      FT_Int  s_width = source->width;
      FT_Int  s_rows  = source->rows;


      /* loop through all the pixels and */
      /* assign pixel values from source */
      for ( t_j = 0; t_j < t_rows; t_j++ )
      {
        for ( t_i = 0; t_i < t_width; t_i++ )
        {
          FT_Int    t_index = t_j * t_width + t_i;
          FT_Int    s_index;


          t[t_index] = zero_ed;

          s_i = t_i - x_diff;
          s_j = t_j - y_diff;

          /* Assign 0 to padding similar to */
          /* the source bitmap.             */
          if ( s_i < 0 || s_i >= s_width ||
               s_j < 0 || s_j >= s_rows )
            continue;

          if ( worker->params.flip_y )
            s_index = ( s_rows - s_j - 1 ) * s_width + s_i;
          else
            s_index = s_j * s_width + s_i;

          /* simply copy the alpha values */
          t[t_index].alpha = s[s_index];
        }
      }

      break;
    }
    default:
      FT_ERROR(( "[bsdf] bsdf_copy_source_to_target: "
                 "unsopported pixel mode of source bitmap\n" ));
      error = FT_THROW( Unimplemented_Feature );
      break;
    }

  Exit:
    return error;
  }


/* END */
