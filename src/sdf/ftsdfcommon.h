
  /****************************************************
   *
   * This file contain common function and properties
   * for both `sdf' and `bsdf' renderer.
   *
   */

#ifndef FTSDFCOMMON_H_
#define FTSDFCOMMON_H_

#include <ft2build.h>
#include FT_CONFIG_CONFIG_H
#include <freetype/freetype.h>

FT_BEGIN_HEADER

  /**************************************************************************
   *
   * default values (cannot be set individually for each renderer)
   *
   */

  /* default spread value */
  #define DEFAULT_SPREAD  8

  /* minimum spread supported by the renderer. */
  #define MIN_SPREAD      2

  /* maximum spread supported by the renderer. */
  #define MAX_SPREAD      32

  /**************************************************************************
   *
   * common definitions (cannot be set individually for each renderer)
   *
   */

  /* If it is defined to 1 then the rasterizer will use squared distances */
  /* for computation. It can greatly improve the performance but there is */
  /* a chance of overflow and artifacts. You can safely use it upto a     */
  /* pixel size of 128.                                                   */
  #ifndef USE_SQUARED_DISTANCES
  #  define USE_SQUARED_DISTANCES 0
  #endif

  /**************************************************************************
   *
   * common macros
   *
   */

  /* convert int to 26.6 fixed point   */
  #define FT_INT_26D6( x )   ( x * 64 )

  /* convert int to 16.16 fixed point  */
  #define FT_INT_16D16( x )  ( x * 65536 )

  /* convert 26.6 to 16.16 fixed point */
  #define FT_26D6_16D16( x ) ( x * 1024 )

  /* Convenient macro which calls the function */
  /* and returns if any error occurs.          */
  #define FT_CALL( x ) do                          \
                       {                           \
                         error = ( x );            \
                         if ( error != FT_Err_Ok ) \
                           goto Exit;              \
                       } while ( 0 )

  /**************************************************************************
   *
   * common typedefs
   *
   */

  typedef  FT_Vector FT_26D6_Vec;   /* with 26.6 fixed point components  */
  typedef  FT_Vector FT_16D16_Vec;  /* with 16.16 fixed point components */

  typedef  FT_Fixed  FT_16D16;      /* 16.16 fixed point representation  */
  typedef  FT_Fixed  FT_26D6;       /* 26.6 fixed point representation   */
  typedef  FT_Short  FT_6D10;       /* 6.10 fixed point representation   */

  typedef  FT_BBox   FT_CBox;       /* control box of a curve            */

  /**************************************************************************
   *
   * common functions
   *
   */

  /* Original Algorithm: https://github.com/chmike/fpsqrt */
  static FT_16D16
  square_root( FT_16D16  val )
  {
    FT_ULong t, q, b, r;


    r = val;
    b = 0x40000000;
    q = 0;
    while( b > 0x40 )
    {
      t = q + b;
      if( r >= t )
      {
        r -= t;
        q = t + b;
      }
      r <<= 1;
      b >>= 1;
    }
    q >>= 8;

    return q;
  }

FT_END_HEADER

#endif /* FTSDFCOMMON_H_ */

/* END */
