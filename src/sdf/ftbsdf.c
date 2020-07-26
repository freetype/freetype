
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftmemory.h>
#include <freetype/fttrigon.h>

#include "ftsdf.h"
#include "ftsdferrs.h"

  /**************************************************************************
   *
   * useful macros
   *
   */

  /* Convenient macro which calls the function */
  /* and returns if any error occurs.          */
  #define FT_CALL( x ) do                          \
                       {                           \
                         error = ( x );            \
                         if ( error != FT_Err_Ok ) \
                           goto Exit;              \
                       } while ( 0 )

  #define ONE 65536 /* 1 in 16.16 */

  /**************************************************************************
   *
   * typedefs
   *
   */

  typedef  FT_Vector FT_16D16_Vec;  /* with 16.16 fixed point components */

  typedef  FT_Short FT_6D10;        /* 6.10 fixed point representation   */
  typedef  FT_Fixed FT_16D16;       /* 16.16 fixed point representation  */

  /**************************************************************************
   *
   * structs
   *
   */

  typedef struct  BSDF_TRaster_
  {
    FT_Memory  memory; /* used internally to allocate memory */

  } BSDF_TRaster;

  /* euclidean distance used for euclidean distance transform */
  typedef struct  ED_
  {
    FT_16D16      dist; /* distance at `near' */
    FT_16D16_Vec  near; /* nearest point      */
    FT_Char       sign; /* outside or inside  */

  } ED;

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

  /**************************************************************************
   *
   * @Function:
   *   bsdf_init_distance_map
   *
   * @Description:
   *   This function initialize the distance map according to
   *   algorithm '8-point sequential Euclidean distance mapping' (8SED).
   *
   * @Input:
   *   [TODO]
   *
   * @Return:
   *   [TODO]
   */
  static FT_Error
  bsdf_init_distance_map( const FT_Bitmap*  source,
                          BSDF_Worker*      worker )
  {
    FT_Error  error         = FT_Err_Ok;
    FT_Bool   is_monochrome = 0;

    FT_Int    x_diff, y_diff;
    FT_Int    num_channels;
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
    case FT_PIXEL_MODE_GRAY2:
    case FT_PIXEL_MODE_GRAY4:
    case FT_PIXEL_MODE_GRAY16:
    case FT_PIXEL_MODE_LCD:
    case FT_PIXEL_MODE_LCD_V:
    case FT_PIXEL_MODE_MONO:
      /* [TODO] */
      FT_ERROR(( "[bsdf] bsdf_copy_source_to_target: "
                 "support for pixel mode not yet added\n" ));
      error = FT_THROW( Unimplemented_Feature );
      break;
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
          FT_Byte   pixel_value;


          t[t_index] = zero_ed;

          s_i = t_i - x_diff;
          s_j = t_j - y_diff;

          /* Assign 0 to padding similar to */
          /* the source bitmap.             */
          if ( s_i < 0 || s_i >= s_width ||
               s_j < 0 || s_j >= s_rows )
          {
            t[t_index].near.x = FT_INT_MAX;
            t[t_index].near.y = FT_INT_MAX;
            t[t_index].dist   = 128 * ONE;
            continue;
          }

          if ( worker->params.flip_y )
            s_index = ( s_rows - s_j - 1 ) * s_width + s_i;
          else
            s_index = s_j * s_width + s_i;

          pixel_value = s[s_index];

  #if 0

          /* to make the fractional value 1 */
          /* for completely filled pixels   */
          if ( pixel_value == 255 )
            pixel_value = 256;

          /* Assume that 256 is fractional value with  */
          /* 0.8 representation, to make it 16.16 left */
          /* shift the value by 8.                     */
          pixel_value <<= 8;

  #else
          if ( pixel_value )
            t[t_index].dist = 0;
          else
          {
            t[t_index].near.x = FT_INT_MAX;
            t[t_index].near.y = FT_INT_MAX;
            t[t_index].dist   = 128 * ONE;
          }

  #endif
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
   *   first_pass
   *
   * @Description:
   *   [TODO]
   *
   * @Input:
   *   [TODO]
   *
   * @Return:
   *   [TODO]
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
      FT_16D16      dist;
      FT_16D16_Vec  dist_vec;
      ED*           to_check;
      ED*           current;


      /* Forward pass of rows (left -> right), leave, the */
      /* first column, will be covered in backward pass.  */
      for ( i = 1; i < w; i++ )
      {
        index = j * w + i;
        current = dm + index;

        /* While checking for nearest point we first     */
        /* approximate the dist of the `current' point   */
        /* by adding the deviation ( which will be root  */
        /* 2 max ). And if the new value is lesser than  */
        /* the current value then only we calculate the  */
        /* actual distances using `FT_Vector_Length'.    */
        /* Of course this will be eliminated while using */
        /* squared distances.                            */

        /* left-up */
        to_check = current - w - 1;
        dist = to_check->dist + 2 * ONE; /* will be root(2) but 2 is fine */
        if ( dist < current->dist )
        {
          dist_vec = to_check->near;
          dist_vec.x -= ONE;
          dist_vec.y -= ONE;
          dist = FT_Vector_Length( &dist_vec );
          if ( dist < current->dist )
          {
            current->dist = dist;
            current->near = dist_vec;
          }
        }

        /* up */
        to_check = current - w;
        dist = to_check->dist + 1 * ONE; /* can only be 1 */
        if ( dist < current->dist )
        {
          dist_vec = to_check->near;
          dist_vec.y -= ONE;
          dist = FT_Vector_Length( &dist_vec );
          if ( dist < current->dist )
          {
            current->dist = dist;
            current->near = dist_vec;
          }
        }

        /* up-right */
        to_check = current - w + 1;
        dist = to_check->dist + 2 * ONE;
        if ( dist < current->dist )
        {
          dist_vec = to_check->near;
          dist_vec.x += ONE;
          dist_vec.y -= ONE;
          dist = FT_Vector_Length( &dist_vec );
          if ( dist < current->dist )
          {
            current->dist = dist;
            current->near = dist_vec;
          }
        }

        /* left */
        to_check = current - 1;
        dist = to_check->dist + 1 * ONE;
        if ( dist < current->dist )
        {
          dist_vec = to_check->near;
          dist_vec.x -= ONE;
          dist = FT_Vector_Length( &dist_vec );
          if ( dist < current->dist )
          {
            current->dist = dist;
            current->near = dist_vec;
          }
        }
      }

      /* Backward pass of rows (right -> left), leave, the */
      /* last column, already covered in the forward pass. */
      for ( i = w - 2; i >= 0; i-- )
      {
        index = j * w + i;
        current = dm + index;

        /* right */
        to_check = current + 1;
        dist = to_check->dist + 1 * ONE;
        if ( dist < current->dist )
        {
          dist_vec = to_check->near;
          dist_vec.x += ONE;
          dist = FT_Vector_Length( &dist_vec );
          if ( dist < current->dist )
          {
            current->dist = dist;
            current->near = dist_vec;
          }
        }
      }
    }
  }

  /**************************************************************************
   *
   * @Function:
   *   edt8
   *
   * @Description:
   *   [TODO]
   *
   * @Input:
   *   [TODO]
   *
   * @Return:
   *   [TODO]
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
    /* [TODO] */

  Exit:
    return error;
  }

  /**************************************************************************
   *
   * @Function:
   *   finalize_sdf
   *
   * @Description:
   *   [TODO]
   *
   * @Input:
   *   [TODO]
   *
   * @Return:
   *   [TODO]
   */
  static FT_Error
  finalize_sdf( BSDF_Worker*  worker,
                FT_Bitmap*    target )
  {
    FT_Error   error = FT_Err_Ok;
    FT_Int     w, r;
    FT_Int     i, j;
    FT_6D10*   t_buffer;

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

    for ( j = 0; j < r; j++ )
    {
      for ( i = 0; i < w; i++ )
      {
        FT_Int    index;
        FT_16D16  dist;
        FT_6D10   final_dist;


        index = j * w + i;
        dist = worker->distance_map[index].dist;
        /* convert from 16.16 to 6.10 */
        dist /= 64;
        final_dist = (FT_6D10)(dist & 0x0000FFFF);

        /* [TODO]: Assing the sign properly. */

        if ( final_dist > worker->params.spread * 1024 )
          final_dist = worker->params.spread * 1024;

        t_buffer[index] = final_dist;
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

  static FT_Error
  bsdf_raster_render( FT_Raster                raster,
                      const FT_Raster_Params*  params )
  {
    FT_Error       error       = FT_Err_Ok;
    FT_Bitmap*     source      = NULL;
    FT_Bitmap*     target      = NULL;
    FT_Memory      memory      = NULL;
    BSDF_TRaster*  bsdf_raster = (BSDF_TRaster*)raster;
    BSDF_Worker    worker;

    const SDF_Raster_Params*  sdf_params = (const SDF_Raster_Params*)params;


    FT_UNUSED( raster );

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
    FT_CALL( edt8( &worker ) );
    FT_CALL( finalize_sdf( &worker, target ) );

  Exit:
    return error;
  }

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
