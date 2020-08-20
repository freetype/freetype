
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


/* END */
