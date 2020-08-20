
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


/* END */
