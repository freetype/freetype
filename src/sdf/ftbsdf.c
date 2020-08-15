
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftmemory.h>
#include <freetype/fttrigon.h>

#include "ftsdf.h"
#include "ftsdferrs.h"
#include "ftsdfcommon.h"

  /**************************************************************************
   *
   * A brief technical overview of how the BSDF rasterizer works.
   * ------------------------------------------------------------
   * 
   * [Notes]:
   *   * SDF stands for Signed Distance Field everywhere.
   * 
   *   * BSDF stands for Bitmap to Signed Distance Field rasterizer.
   * 
   *   * This renderer convert rasterized bitmaps to SDF. There is another
   *     renderer `sdf' which generate SDF directly from outlines, see
   *     `ftsdf.c' for more details on the `sdf' rasterizer.
   * 
   *   * The idea of generating SDF from bitmaps is taken from two research 
   *     papers, where one is dependent on the other:
   *     
   *     - First paper:
   *         Euclidean Distance Mapping, PER-ERIK DANIELSSON.
   *         Link: http://webstaff.itn.liu.se/~stegu/JFA/Danielsson.pdf
   *         From this paper we use the he eight-point sequential Euclidean
   *         distance mapping (8SED). This is the heart of the process used
   *         in this rasterizer.
   *         The 8SED algorithm is the basic algorithm which generate SDF
   *         (distance map) from binary bitmaps.
   *         
   *     - Second paper:
   *         Anti-aliased Euclidean distance transform. Stefan Gustavson,
   *         Robin Strand.
   *         Link: http://weber.itn.liu.se/~stegu/aadist/edtaa_preprint.pdf
   *         The 8SED discards the pixel's alpha values which can contain
   *         information about the actual outline of the glyph. So, this
   *         paper takes advantage of those alpha values and approximate
   *         outline pretty accurately.
   * 
   *   * The two algorithms together generate pretty accurate SDF from only
   *     bitmaps.
   * 
   *   * This rasterizer will work for monochrome bitmaps but the result will
   *     not be as accurate since we don't have any way to approximate outli-
   *     nes from binary bitmaps.
   * 
   * ========================================================================
   * 
   *   Generating SDF from bitmap is done in several steps:
   * 
   *   1 - First, the only information we have is the bitmap itself. It can
   *       be monochrome or anti-aliased. If it is anti-aliased the pixel
   *       values are nothing but the coverage values. There coverage values
   *       can be used to extract information about the outline of the image.
   *       For example: If the pixel's alpha value is 0.5, then we can safely
   *       assume that the outline pass through the center of the pixel.
   *
   *   2 - Now we find the edge pixels in the bitmap (see `bsdf_is_edge' for
   *       more details about how we find edge pixels). For all edge pixels
   *       we use the Anti-aliased Euclidean distance transform algorithm and
   *       compute approximate edge distances (see `compute_edge_distance'
   *       and/or the second paper about how we compute approximate edge
   *       distances).
   *       
   *   3 - Now that we have computed approximate distance for edge pixels we
   *       use the 8SED algorithm to basically sweep the entire bitmap and
   *       compute distances for the rest of the pixels. (Since the algorithm
   *       is pretty large it is only explained briefly in the file, the
   *       function for which is `edt8'. To see the actual algorithm refer
   *       to the first paper).
   *
   *   4 - And finally we compute the sign for each pixel. This is done in
   *       the `finalize_sdf' function. The basic idea is that if the pixel's
   *       original alpha/coverage value is greater than 0.5 then it is
   *       'inside' otherwise it is 'outside'.
   *
   *   5 - This concludes the algorithm.
   *
   *   Pseudo Code:
   *
   *     b  = source bitmap;
   *     t  = target bitmap;
   *     dm = list of distances; // dimension equal to b
   *
   *     foreach grid_point (x, y) in b:
   *       if ( is_edge(x, y) ):
   *         dm = approximate_edge_distance(b, x, y);
   *     
   *     // do the 8SED on the distances
   *     edt8(dm);
   *
   *     // determine the signs
   *     determine_signs(dm):
   *
   *     // copy SDF data to the target bitmap
   *     copy(dm to t);
   *
   */

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

  /**************************************************************************
   *
   * @Function:
   *   compare_neighbor
   *
   * @Description:
   *   Handy function which compare the neighbor ( which is defined
   *   by the offset ) and updae the `current' distance if the new
   *   distance is shorter than the original.
   *
   * @Input:
   *   current ::
   *     Array of distances. This parameter must point to the position
   *     whose neighbor is to be checked. Also the array is treated as
   *     a 2D array.
   *
   *   x_offset ::
   *     X offset of the neighbor to be checked. The offset is releative
   *     to the `current' point.
   *
   *   y_offset ::
   *     Y offset of the neighbor to be checked. The offset is releative
   *     to the `current' point.
   *
   *   width ::
   *     Width of the `current' array, we need this since we treat the
   *     distance array as a 2D array.
   *
   * @Return:
   *   None. It just update the current distance.
   *
   */
  static void
  compare_neighbor( ED*     current,
                    FT_Int  x_offset,
                    FT_Int  y_offset,
                    FT_Int  width )
  {
    ED*           to_check;
    FT_16D16      dist;
    FT_16D16_Vec  dist_vec;


    to_check = current + ( y_offset * width ) + x_offset;

    /* While checking for nearest point we first     */
    /* approximate the dist of the `current' point   */
    /* by adding the deviation ( which will be root  */
    /* 2 max ). And if the new value is lesser than  */
    /* the current value then only we calculate the  */
    /* actual distances using `FT_Vector_Length'.    */
    /* Of course this will be eliminated while using */
    /* squared distances.                            */

    /* Approximate the distance, use 1 to avoid  */
    /* precision errors. We subtract because the */
    /* two directions can be opposite.           */
    dist = to_check->dist - ONE;

    if ( dist < current->dist )
    {
      dist_vec = to_check->near;
      dist_vec.x += x_offset * ONE;
      dist_vec.y += y_offset * ONE;
      dist = VECTOR_LENGTH_16D16( dist_vec );
      if ( dist < current->dist )
      {
        current->dist = dist;
        current->near = dist_vec;
      }
    }
  }

  /**************************************************************************
   *
   * @Function:
   *   first_pass
   *
   * @Description:
   *   First pass the 8SED algorithm. It loop the bitmap from top
   *   to bottom and scan each row left to right updating the distances
   *   in the distance map ( in the `worker' parameter ).
   *
   * @Input:
   *   worker::
   *     Contains all the relevant parameters.
   *
   * @Return:
   *   None. It update the distance map.
   *
   */
  static void
  first_pass( BSDF_Worker*  worker )
  {
    FT_Int    i, j; /* iterators    */
    FT_Int    w, r; /* width, rows  */
    ED*       dm;   /* distance map */


    dm = worker->distance_map;
    w  = worker->width;
    r  = worker->rows;

    /* Start scanning from top to bottom and sweep each   */
    /* row back and forth comparing the distances of the  */
    /* neighborhood. Leave the first row as it has no top */
    /* neighbor, it will be covered in the 2nd scan of    */
    /* the image, that is from bottom to top.             */
    for ( j = 1; j < r; j++ )
    {
      FT_Int        index;
      ED*           current;


      /* Forward pass of rows (left -> right), leave, the */
      /* first column, will be covered in backward pass.  */
      for ( i = 1; i < w; i++ )
      {
        index = j * w + i;
        current = dm + index;

        /* left-up */
        compare_neighbor( current, -1, -1, w );

        /* up */
        compare_neighbor( current,  0, -1, w );

        /* up-right */
        compare_neighbor( current,  1, -1, w );

        /* left */
        compare_neighbor( current, -1,  0, w );
      }

      /* Backward pass of rows (right -> left), leave, the */
      /* last column, already covered in the forward pass. */
      for ( i = w - 2; i >= 0; i-- )
      {
        index = j * w + i;
        current = dm + index;

        /* right */
        compare_neighbor( current,  1,  0, w );
      }
    }
  }

  /**************************************************************************
   *
   * @Function:
   *   second_pass
   *
   * @Description:
   *   Second pass the 8SED algorithm. It loop the bitmap from bottom
   *   to top and scan each row left to right updating the distances
   *   in the distance map ( in the `worker' parameter ).
   *
   * @Input:
   *   worker::
   *     Contains all the relevant parameters.
   *
   * @Return:
   *   None. It update the distance map.
   *
   */
  static void
  second_pass( BSDF_Worker*  worker )
  {
    FT_Int    i, j; /* iterators    */
    FT_Int    w, r; /* width, rows  */ 
    ED*       dm;   /* distance map */


    dm = worker->distance_map;
    w  = worker->width;
    r  = worker->rows;

    /* Start scanning from bottom to top and sweep each   */
    /* row back and forth comparing the distances of the  */
    /* neighborhood. Leave the last row as it has no down */
    /* neighbor, it is already covered in the 1stscan of  */
    /* the image, that is from top to bottom.             */
    for ( j = r - 2; j >= 0; j-- )
    {
      FT_Int        index;
      ED*           current;


      /* Forward pass of rows (left -> right), leave, the */
      /* first column, will be covered in backward pass.  */
      for ( i = 1; i < w; i++ )
      {
        index = j * w + i;
        current = dm + index;

        /* left-up */
        compare_neighbor( current, -1,  1, w );

        /* up */
        compare_neighbor( current,  0,  1, w );

        /* up-right */
        compare_neighbor( current,  1,  1, w );

        /* left */
        compare_neighbor( current, -1,  0, w );
      }

      /* Backward pass of rows (right -> left), leave, the */
      /* last column, already covered in the forward pass. */
      for ( i = w - 2; i >= 0; i-- )
      {
        index = j * w + i;
        current = dm + index;

        /* right */
        compare_neighbor( current,  1,  0, w );
      }
    }
  }


  /**************************************************************************
   *
   * @Function:
   *   edt8
   *
   * @Description:
   *   Function which compute the distance map of the a bitmap. It does
   *   both first and second pass of the 8SED algorithm.
   *
   * @Input:
   *   worker::
   *     Contains all the relevant parameters.
   *
   * @Return:
   *   FT_Error ::
   *     FreeType error, 0 means success.
   *
   */
  static FT_Error
  edt8( BSDF_Worker*  worker )
  {
    FT_Error  error = FT_Err_Ok;


    if ( !worker || !worker->distance_map )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* first scan of the image */
    first_pass( worker );

    /* second scan of the image */
    second_pass( worker );

  Exit:
    return error;
  }

  /**************************************************************************
   *
   * @Function:
   *   finalize_sdf
   *
   * @Description:
   *   This function copy the SDF data from `worker->distance_map' to the
   *   `target' bitmap. It aslo transforms the data to our output format,
   *    i.e. 6.10 fixed point format at the moment.
   *
   * @Input:
   *   worker ::
   *     Conaints source distance map and parameters/properties which contains
   *     SDF data.
   *
   * @Return:
   *   target ::
   *     Target bitmap to which the SDF data is copied to.
   *
   *   FT_Error ::
   *     FreeType error, 0 means success.
   *
   */
  static FT_Error
  finalize_sdf( BSDF_Worker*      worker,
                const FT_Bitmap*  target )
  {
    FT_Error   error = FT_Err_Ok;
    FT_Int     w, r;
    FT_Int     i, j;
    FT_6D10*   t_buffer;
    FT_16D16   spread;

    if ( !worker || !target )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    w = target->width;
    r = target->rows;
    t_buffer = (FT_6D10*)target->buffer;

    if ( w != worker->width ||
         r != worker->rows )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;      
    }

  #if USE_SQUARED_DISTANCES
    spread = FT_INT_16D16( worker->params.spread *
                           worker->params.spread );
  #else
    spread = FT_INT_16D16( worker->params.spread );
  #endif

    for ( j = 0; j < r; j++ )
    {
      for ( i = 0; i < w; i++ )
      {
        FT_Int    index;
        FT_16D16  dist;
        FT_6D10   final_dist;
        FT_Char   sign;


        index = j * w + i;
        dist = worker->distance_map[index].dist;

        if ( dist < 0 || dist > spread )
          dist = spread;

  #if USE_SQUARED_DISTANCES
          dist = square_root( dist );
  #endif

        /* convert from 16.16 to 6.10 */
        dist /= 64;
        final_dist = (FT_6D10)(dist & 0x0000FFFF);

        /* We assume that if the pixel is inside a contour */
        /* then it's coverage value must be > 127.         */
        sign = worker->distance_map[index].alpha < 127 ? -1 : 1;

        /* flip the sign according to the property */
        if ( worker->params.flip_sign )
          sign = -sign;

        t_buffer[index] = final_dist * sign;
      }
    }

  Exit:
    return error;
  }

  /**************************************************************************
   *
   * interface functions
   *
   */

  /* called when adding a new module through `FT_Add_Module' */
  static FT_Error
  bsdf_raster_new( FT_Memory   memory,
                   FT_Raster*  araster)
  {
    FT_Error       error  = FT_Err_Ok;
    BSDF_TRaster*  raster = NULL;


    *araster = 0;
    if ( !FT_ALLOC( raster, sizeof( BSDF_TRaster ) ) )
    {
      raster->memory = memory;
      *araster = (FT_Raster)raster;
    }

    return error;
  }

  /* unused */
  static void
  bsdf_raster_reset( FT_Raster       raster,
                     unsigned char*  pool_base,
                     unsigned long   pool_size )
  {
    /* no use of this function */
    FT_UNUSED( raster );
    FT_UNUSED( pool_base );
    FT_UNUSED( pool_size );
  }

  /* unused */
  static FT_Error
  bsdf_raster_set_mode( FT_Raster      raster,
                        unsigned long  mode,
                        void*          args )
  {
    FT_UNUSED( raster );
    FT_UNUSED( mode );
    FT_UNUSED( args );


    return FT_Err_Ok;
  }

  /* called while rendering through `FT_Render_Glyph' */
  static FT_Error
  bsdf_raster_render( FT_Raster                raster,
                      const FT_Raster_Params*  params )
  {
    FT_Error          error       = FT_Err_Ok;
    const FT_Bitmap*  source      = NULL;
    const FT_Bitmap*  target      = NULL;
    FT_Memory         memory      = NULL;
    BSDF_TRaster*     bsdf_raster = (BSDF_TRaster*)raster;
    BSDF_Worker       worker;

    const SDF_Raster_Params*  sdf_params = (const SDF_Raster_Params*)params;


    worker.distance_map = NULL;

    /* check for valid parameters */
    if ( !raster || !params )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* check if the flag is set */
    if ( sdf_params->root.flags != FT_RASTER_FLAG_SDF )
    {
      error = FT_THROW( Raster_Corrupted );
      goto Exit;
    }

    source = sdf_params->root.source;
    target = sdf_params->root.target;

    /* check the source and target bitmap */
    if ( !source || !target )
    {
      error = FT_THROW( Invalid_Argument );
      goto Exit; 
    }

    memory = bsdf_raster->memory;
    if ( !memory )
    {
      FT_TRACE0(( "[bsdf] bsdf_raster_render:\n"
                  "      Raster not setup properly, "
                  "unable to find memory handle.\n" ));
      error = FT_THROW( Invalid_Handle );
      goto Exit;
    }

    /* check if spread is set properly */
    if ( sdf_params->spread > MAX_SPREAD ||
         sdf_params->spread < MIN_SPREAD )
    {
      FT_TRACE0(( 
        "[bsdf] bsdf_raster_render:\n"
        "       The `spread' field of `SDF_Raster_Params' is invalid,\n"
        "       the value of this field must be within [%d, %d].\n"
        "       Also, you must pass `SDF_Raster_Params' instead of the\n"
        "       default `FT_Raster_Params' while calling this function\n"
        "       and set the fields properly.\n"
        , MIN_SPREAD, MAX_SPREAD ));
      error = FT_THROW( Invalid_Argument );
      goto Exit;
    }

    /* setup the worker */

    /* allocate the distance map */
    if ( FT_QALLOC_MULT( worker.distance_map, target->rows,
                         target->width * sizeof( *worker.distance_map ) ) )
      goto Exit;

    worker.width  = target->width;
    worker.rows   = target->rows;
    worker.params = *sdf_params;

    FT_CALL( bsdf_init_distance_map( source, &worker ) );
    FT_CALL( bsdf_approximate_edge( &worker ) );
    FT_CALL( edt8( &worker ) );
    FT_CALL( finalize_sdf( &worker, target ) );

    FT_TRACE0(( "[bsdf] bsdf_raster_render: "
            "Total memory used = %ld\n",
             worker.width * worker.rows * sizeof( *worker.distance_map ) ));

  Exit:
    if ( worker.distance_map )
      FT_FREE( worker.distance_map );

    return error;
  }

  /* called while deleting a `FT_Library' only if the module is added */
  static void
  bsdf_raster_done( FT_Raster  raster )
  {
    FT_Memory  memory = (FT_Memory)((BSDF_TRaster*)raster)->memory;


    FT_FREE( raster );
  }

  FT_DEFINE_RASTER_FUNCS(
    ft_bitmap_sdf_raster,

    FT_GLYPH_FORMAT_BITMAP,

    (FT_Raster_New_Func)      bsdf_raster_new,       /* raster_new      */
    (FT_Raster_Reset_Func)    bsdf_raster_reset,     /* raster_reset    */
    (FT_Raster_Set_Mode_Func) bsdf_raster_set_mode,  /* raster_set_mode */
    (FT_Raster_Render_Func)   bsdf_raster_render,    /* raster_render   */
    (FT_Raster_Done_Func)     bsdf_raster_done       /* raster_done     */
  )

/* END */
