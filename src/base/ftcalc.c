/***************************************************************************/
/*                                                                         */
/*  ftcalc.c                                                               */
/*                                                                         */
/*    Arithmetic computations (body).                                      */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* Support for 1-complement arithmetic has been totally dropped in this  */
  /* release.  You can still write your own code if you need it.           */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* Implementing basic computation routines.                              */
  /*                                                                       */
  /* FT_MulDiv() and FT_MulFix() are declared in freetype.h.               */
  /*                                                                       */
  /*************************************************************************/

#include <ftcalc.h>
#include <ftdebug.h>
#include <ftobjs.h>  /* for ABS() */


#ifdef FT_CONFIG_OPTION_OLD_CALCS

  static const FT_Long  ft_square_roots[63] =
  {
       1,    1,    2,     3,     4,     5,     8,    11,
      16,   22,   32,    45,    64,    90,   128,   181,
     256,  362,  512,   724,  1024,  1448,  2048,  2896,
    4096, 5892, 8192, 11585, 16384, 23170, 32768, 46340,

      65536,   92681,  131072,   185363,   262144,   370727,
     524288,  741455, 1048576,  1482910,  2097152,  2965820,
    4194304, 5931641, 8388608, 11863283, 16777216, 23726566,

      33554432,   47453132,   67108864,   94906265,
     134217728,  189812531,  268435456,  379625062,
     536870912,  759250125, 1073741824, 1518500250,
    2147483647
  };
#else  
  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Sqrt32                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Computes the square root of an Int32 integer (which will be        */
  /*    as an unsigned long value).                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    x :: The value to compute the root for.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `sqrt(x)'.                                           */
  /*                                                                       */
  EXPORT_FUNC
  FT_Int32  FT_Sqrt32( FT_Int32 x )
  {
    FT_ULong  val, root, newroot, mask;


    root = 0;
    mask = 0x40000000;
    val  = (FT_ULong)x;

    do
    {
      newroot = root + mask;
      if ( newroot <= val )
      {
        val -= newroot;
        root = newroot + mask;
      }

      root >>= 1;
      mask >>= 2;
    }
    while ( mask != 0 );

    return root;
  }
#endif /* OLD_CALCS */

#ifdef LONG64


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_MulDiv                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation `(a*b)/c'   */
  /*    with maximum accuracy (it uses a 64-bit intermediate integer       */
  /*    whenever necessary).                                               */
  /*                                                                       */
  /*    This function isn't necessarily as fast as some processor specific */
  /*    operations, but is at least completely portable.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.                                        */
  /*    c :: The divisor.                                                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*b)/c'.  This function never traps when trying to */
  /*    divide by zero, it simply returns `MaxInt' or `MinInt' depending   */
  /*    on the signs of `a' and `b'.                                       */
  /*                                                                       */
  EXPORT_FUNC
  FT_Long  FT_MulDiv( FT_Long  a,
                      FT_Long  b,
                      FT_Long  c )
  {
    FT_Int s;


    s = 1;
    if ( a < 0 ) { a = -a; s = -s; }
    if ( b < 0 ) { b = -b; s = -s; }
    if ( c < 0 ) { c = -c; s = -s; }

    return s * ( ( (FT_Int64)a * b + ( c >> 1 ) ) / c );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_MulFix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation             */
  /*    `(a*b)/0x10000' with maximum accuracy.  Most of the time this is   */
  /*    used to multiply a given value by a 16.16 fixed float factor.      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.  Use a 16.16 factor here whenever      */
  /*         possible (see note below).                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*b)/0x10000'.                                     */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function has been optimized for the case where the absolute   */
  /*    value of `a' is less than 2048, and `b' is a 16.16 scaling factor. */
  /*    As this happens mainly when scaling from notional units to         */
  /*    fractional pixels in FreeType, it resulted in noticeable speed     */
  /*    improvements between versions 2.x and 1.x.                         */
  /*                                                                       */
  /*    As a conclusion, always try to place a 16.16 factor as the         */
  /*    _second_ argument of this function; this can make a great          */
  /*    difference.                                                        */
  /*                                                                       */
  EXPORT_FUNC
  FT_Long  FT_MulFix( FT_Long  a,
                      FT_Long  b )
  {
    FT_Int s;


    s = 1;
    if ( a < 0 ) { a = -a; s = -s; }
    if ( b < 0 ) { b = -b; s = -s; }

    return s * (FT_Long)( ( (FT_Int64)a * b + 0x8000 ) >> 16 );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_DivFix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation             */
  /*    `(a*0x10000)/b' with maximum accuracy.  Most of the time, this is  */
  /*    used to divide a given value by a 16.16 fixed float factor.        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.  Use a 16.16 factor here whenever      */
  /*         possible (see note below).                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*0x10000)/b'.                                     */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The optimization for FT_DivFix() is simple: If (a << 16) fits in   */
  /*    32 bits, then the division is computed directly.  Otherwise, we    */
  /*    use a specialized version of the old FT_MulDiv64().                */
  /*                                                                       */
  EXPORT_FUNC
  FT_Int32  FT_DivFix( FT_Long  a,
                       FT_Long  b )
  {
    FT_Int32   s;
    FT_Word32  q;


    s  = a; a = ABS(a);
    s ^= b; b = ABS(b);

    if ( b == 0 )
      /* check for divide by 0 */
      q = 0x7FFFFFFF;

    else
      /* compute result directly */
      q = ((FT_Int64)a << 16) / b;

    return (FT_Int32)( s < 0 ? -q : q );
  }


#ifdef FT_CONFIG_OPTION_OLD_CALCS
  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Sqrt64                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Computes the square root of a 64-bits value ! Yeah, that sounds    */
  /*    stupid, but it's needed to obtain maximum accuracy in the          */
  /*    TrueType bytecode interpreter..                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    l :: 64-bits integer                                               */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The 32-bit square-root.                                            */
  /*                                                                       */
  
     static
     int  ft_order64( FT_Int64  z )
     {
       int  j = 0;
   
       while ( z )
       {
         z = (unsigned INT64)z >> 1;
         j++;
       }
       return j - 1;
     }


  EXPORT_FUNC
  FT_Int32  FT_Sqrt64( FT_Int64  l )
  {
    FT_Int64  r, s;

    if ( l <= 0 ) return 0;
    if ( l == 1 ) return 1;

    r = ft_square_roots[ft_order64( l )];

    do
    {
      s = r;
      r = ( r + l/r ) >> 1;
    }
    while ( r > s || r*r > l );

    return r;
  }
#endif


#else /* LONG64 */


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_MulDiv                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation `(a*b)/c'   */
  /*    with maximum accuracy (it uses a 64-bit intermediate integer       */
  /*    whenever necessary).                                               */
  /*                                                                       */
  /*    This function isn't necessarily as fast as some processor specific */
  /*    operations, but is at least completely portable.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.                                        */
  /*    c :: The divisor.                                                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*b)/c'.  This function never traps when trying to */
  /*    divide by zero, it simply returns `MaxInt' or `MinInt' depending   */
  /*    on the signs of `a' and `b'.                                       */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The FT_MulDiv() function has been optimized thanks to ideas from   */
  /*    Graham Asher.  The trick is to optimize computation if everything  */
  /*    fits within 32 bits (a rather common case).                        */
  /*                                                                       */
  /*    We compute `a*b+c/2', then divide it by `c' (positive values).     */
  /*                                                                       */
  /*      46340 is FLOOR(SQRT(2^31-1)).                                    */
  /*                                                                       */
  /*      if ( a <= 46340 && b <= 46340 ) then ( a*b <= 0x7FFEA810 )       */
  /*                                                                       */
  /*      0x7FFFFFFF - 0x7FFEA810 = 0x157F0                                */
  /*                                                                       */
  /*      if ( c < 0x157F0*2 ) then ( a*b+c/2 <= 0x7FFFFFFF )              */
  /*                                                                       */
  /*      and 2*0x157F0 = 176096.                                          */
  /*                                                                       */
  EXPORT_FUNC
  FT_Long  FT_MulDiv( FT_Long  a,
                      FT_Long  b,
                      FT_Long  c )
  {
    long   s;


    if ( a == 0 || b == c )
      return a;

    s  = a; a = ABS( a );
    s ^= b; b = ABS( b );
    s ^= c; c = ABS( c );

    if ( a <= 46340 && b <= 46340 && c <= 176095L )
    {
      a = ( a*b + (c >> 1) ) / c;
    }
    else
    {
      FT_Int64  temp, temp2;


      FT_MulTo64( a, b, &temp );
      temp2.hi = (FT_Int32)(c >> 31);
      temp2.lo = (FT_Word32)(c / 2);
      FT_Add64( &temp, &temp2, &temp );
      a = FT_Div64by32( &temp, c );
    }

    return ( s < 0 ) ? -a : a;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_MulFix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation             */
  /*    `(a*b)/0x10000' with maximum accuracy.  Most of the time, this is  */
  /*    used to multiply a given value by a 16.16 fixed float factor.      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.  Use a 16.16 factor here whenever      */
  /*         possible (see note below).                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*b)/0x10000'.                                     */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The optimization for FT_MulFix() is different.  We could simply be */
  /*    happy by applying the same principles as with FT_MulDiv(), because */
  /*                                                                       */
  /*      c = 0x10000 < 176096                                             */
  /*                                                                       */
  /*    However, in most cases, we have a `b' with a value around 0x10000  */
  /*    which is greater than 46340.                                       */
  /*                                                                       */
  /*    According to some testing, most cases have `a' < 2048, so a good   */
  /*    idea is to use bounds like 2048 and 1048576 (=floor((2^31-1)/2048) */
  /*    for `a' and `b', respectively.                                     */
  /*                                                                       */
  EXPORT_FUNC
  FT_Long  FT_MulFix( FT_Long  a,
                      FT_Long  b )
  {
    FT_Long   s;
    FT_ULong  ua, ub;

    if ( a == 0 || b == 0x10000L )
      return a;

    s  = a; a = ABS(a);
    s ^= b; b = ABS(b);

    ua = (FT_ULong)a;
    ub = (FT_ULong)b;

    if ( ua <= 2048 && ub <= 1048576L )
    {
      ua = ( ua*ub + 0x8000 ) >> 16;
    }
    else
    {
      FT_ULong  al = ua & 0xFFFF;


      ua = (ua >> 16)*ub + al*(ub >> 16) + ( al*(ub & 0xFFFF) >> 16 );
    }

    return ( s < 0 ? -(FT_Long)ua : ua );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_DivFix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation             */
  /*    `(a*0x10000)/b' with maximum accuracy.  Most of the time, this is  */
  /*    used to divide  a given value by a 16.16 fixed float factor.       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.  Use a 16.16 factor here whenever      */
  /*         possible (see note below).                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*0x10000)/b'.                                     */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The optimization for FT_DivFix() is simple: If (a << 16) fits in   */
  /*    32 bits, then the division is computed directly.  Otherwise, we    */
  /*    use a specialized version of the old FT_MulDiv64().                */
  /*                                                                       */
  EXPORT_FUNC
  FT_Long  FT_DivFix( FT_Long  a,
                      FT_Long  b )
  {
    FT_Int32   s;
    FT_Word32  q;


    s  = a; a = ABS(a);
    s ^= b; b = ABS(b);

    if ( b == 0 )
      /* check for divide by 0 */
      q = 0x7FFFFFFF;

    else if ( (a >> 16) == 0 )
    {
      /* compute result directly */
      q = (FT_Word32)(a << 16) / (FT_Word32)b;
    }
    else
    {
      /* we need more bits, we'll have to do it by hand */
      FT_Word32  c;


      q  = ( a / b ) << 16;
      c  = a % b;

      /* we must compute C*0x10000/B; we simply shift C and B so */
      /* C becomes smaller than 16 bits                          */
      while ( c >> 16 )
      {
        c >>= 1;
        b <<= 1;
      }

      q += ( c << 16 ) / b;
    }

    return ( s < 0 ? -(FT_Int32)q : (FT_Int32)q );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Add64                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Add two Int64 values.                                              */
  /*                                                                       */
  /* <Input>                                                               */
  /*    x :: A pointer to the first value to be added.                     */
  /*    y :: A pointer to the second value to be added.                    */
  /*                                                                       */
  /* <Output>                                                              */
  /*    z :: A pointer to the result of `x + y'.                           */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Will be wrapped by the ADD_64() macro.                             */
  /*                                                                       */
  EXPORT_FUNC
  void  FT_Add64( FT_Int64*  x,
                  FT_Int64*  y,
                  FT_Int64*  z )
  {
    register FT_Word32  lo, hi;

    lo = x->lo + y->lo;
    hi = x->hi + y->hi + ( lo < x->lo );

    z->lo = lo;
    z->hi = hi;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_MulTo64                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Multiplies two Int32 integers.  Returns a Int64 integer.           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    x :: The first multiplier.                                         */
  /*    y :: The second multiplier.                                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    z :: A pointer to the result of `x * y'.                           */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Will be wrapped by the MUL_64() macro.                             */
  /*                                                                       */
  EXPORT_FUNC
  void  FT_MulTo64( FT_Int32   x,
                    FT_Int32   y,
                    FT_Int64*  z )
  {
    FT_Int32   s;


    s  = x; x = ABS( x );
    s ^= y; y = ABS( y );

    {
      FT_Word32  lo1, hi1, lo2, hi2, lo, hi, i1, i2;


      lo1 = x & 0x0000FFFF;  hi1 = x >> 16;
      lo2 = y & 0x0000FFFF;  hi2 = y >> 16;

      lo = lo1 * lo2;
      i1 = lo1 * hi2;
      i2 = lo2 * hi1;
      hi = hi1 * hi2;

      /* Check carry overflow of i1 + i2 */
      i1 += i2;
      if ( i1 < i2 )
        hi += 1L << 16;

      hi += i1 >> 16;
      i1  = i1 << 16;

      /* Check carry overflow of i1 + lo */
      lo += i1;
      hi += (lo < i1);

      z->lo = lo;
      z->hi = hi;
    }

    if ( s < 0 )
    {
      z->lo = (FT_Word32)-(FT_Int32)z->lo;
      z->hi = ~z->hi + !(z->lo);
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Div64by32                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Divides an Int64 value by an Int32 value.  Returns an Int32        */
  /*    integer.                                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    x :: A pointer to the dividend.                                    */
  /*    y :: The divisor.                                                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `x / y'.                                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Will be wrapped by the DIV_64() macro.                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Int32  FT_Div64by32( FT_Int64*  x,
                          FT_Int32   y )
  {
    FT_Int32   s;
    FT_Word32  q, r, i, lo;


    s  = x->hi;
    if ( s < 0 )
    {
      x->lo = (FT_Word32)-(FT_Int32)x->lo;
      x->hi = ~x->hi + !(x->lo);
    }
    s ^= y;  y = ABS( y );

    /* Shortcut */
    if ( x->hi == 0 )
    {
      q = x->lo / y;
      return ( s < 0 ) ? -(FT_Int32)q : (FT_Int32)q;
    }

    r  = x->hi;
    lo = x->lo;

    if ( r >= (FT_Word32)y ) /* we know y is to be treated as unsigned here */
      return ( s < 0 ) ? 0x80000001L : 0x7FFFFFFFL;
                            /* Return Max/Min Int32 if divide overflow. */
                            /* This includes division by zero!          */
    q = 0;
    for ( i = 0; i < 32; i++ )
    {
      r <<= 1;
      q <<= 1;
      r  |= lo >> 31;

      if ( r >= (FT_Word32)y )
      {
        r -= y;
        q |= 1;
      }
      lo <<= 1;
    }

    return ( s < 0 ) ? -(FT_Int32)q : (FT_Int32)q;
  }


#ifdef FT_CONFIG_OPTION_OLD_CALCS

  static
  void  FT_Sub64( FT_Int64*  x, FT_Int64*  y, FT_Int64*  z )
  {
    register FT_Word32  lo, hi;


    lo = x->lo - y->lo;
    hi = x->hi - y->hi - ( (FT_Int32)lo < 0 );

    z->lo = lo;
    z->hi = hi;
  }

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Sqrt64                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Computes the square root of a 64-bits value ! Yeah, that sounds    */
  /*    stupid, but it's needed to obtain maximum accuracy in the          */
  /*    TrueType bytecode interpreter..                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    z :: pointer to 64-bits integer                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The 32-bit square-root.                                            */
  /*                                                                       */
  
      static
      int  ft_order64( FT_Int64*  z )
      {
        FT_Word32  i;
        int        j;
    
        i = z->lo;
        j = 0;
        if ( z->hi )
        {
          i = z->hi;
          j = 32;
        }
    
        while ( i > 0 )
        {
          i >>= 1;
          j++;
        }
        return j-1;
      }

  EXPORT_FUNC
  FT_Int32  FT_Sqrt64( FT_Int64*  l )
  {
    FT_Int64  l2;
    FT_Int32  r, s;


    if ( (FT_Int32)l->hi < 0          ||
        (l->hi == 0 && l->lo == 0) )  return 0;

    s = ft_order64( l );
    if ( s == 0 ) return 1;

    r = ft_square_roots[s];
    do
    {
      s = r;
      r = ( r + FT_Div64by32(l,r) ) >> 1;
      FT_MulTo64( r, r,   &l2 );
      FT_Sub64  ( l, &l2, &l2 );
    }
    while ( r > s || (FT_Int32)l2.hi < 0 );

    return r;
  }

#endif /* FT_CONFIG_OPTION_OLD_CALCS */

#endif /* LONG64 */


/* END */
