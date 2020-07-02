/****************************************************************************
 *
 * ftconfig.h
 *
 *   ANSI-specific configuration file (specification only).
 *
 * Copyright (C) 1996-2020 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


  /**************************************************************************
   *
   * This header file contains a number of macro definitions that are used by
   * the rest of the engine.  Most of the macros here are automatically
   * determined at compile time, and you should not need to change it to port
   * FreeType, except to compile the library with a non-ANSI compiler.
   *
   * Note however that if some specific modifications are needed, we advise
   * you to place a modified copy in your build directory.
   *
   * The build directory is usually `builds/<system>`, and contains
   * system-specific files that are always included first when building the
   * library.
   *
   * This ANSI version should stay in `include/config/`.
   *
   */

#ifndef FTCONFIG_H_
#define FTCONFIG_H_

#include <ft2build.h>
#include FT_CONFIG_OPTIONS_H
#include FT_CONFIG_STANDARD_LIBRARY_H

#include <freetype/config/integer-types.h>
#include <freetype/config/mac-support.h>

FT_BEGIN_HEADER

  /* `FT_UNUSED` indicates that a given parameter is not used --   */
  /* this is only used to get rid of unpleasant compiler warnings. */
#ifndef FT_UNUSED
#define FT_UNUSED( arg )  ( (arg) = (arg) )
#endif


  /**************************************************************************
   *
   *                    AUTOMATIC CONFIGURATION MACROS
   *
   * These macros are computed from the ones defined above.  Don't touch
   * their definition, unless you know precisely what you are doing.  No
   * porter should need to mess with them.
   *
   */


  /* Fix compiler warning with sgi compiler. */
#if defined( __sgi ) && !defined( __GNUC__ )
#if defined( _COMPILER_VERSION ) && ( _COMPILER_VERSION >= 730 )
#pragma set woff 3505
#endif
#endif

#ifdef _WIN64
  /* only 64bit Windows uses the LLP64 data model, i.e., */
  /* 32bit integers, 64bit pointers                      */
#define FT_UINT_TO_POINTER( x ) (void*)(unsigned __int64)(x)
#else
#define FT_UINT_TO_POINTER( x ) (void*)(unsigned long)(x)
#endif


  /**************************************************************************
   *
   * miscellaneous
   *
   */


#define FT_BEGIN_STMNT  do {
#define FT_END_STMNT    } while ( 0 )
#define FT_DUMMY_STMNT  FT_BEGIN_STMNT FT_END_STMNT


  /* `typeof` condition taken from gnulib's `intprops.h` header file */
#if ( ( defined( __GNUC__ ) && __GNUC__ >= 2 )                       || \
      ( defined( __IBMC__ ) && __IBMC__ >= 1210 &&                      \
        defined( __IBM__TYPEOF__ ) )                                 || \
      ( defined( __SUNPRO_C ) && __SUNPRO_C >= 0x5110 && !__STDC__ ) )
#define FT_TYPEOF( type )  ( __typeof__ ( type ) )
#else
#define FT_TYPEOF( type )  /* empty */
#endif


  /* Use `FT_LOCAL` and `FT_LOCAL_DEF` to declare and define,            */
  /* respectively, a function that gets used only within the scope of a  */
  /* module.  Normally, both the header and source code files for such a */
  /* function are within a single module directory.                      */
  /*                                                                     */
  /* Intra-module arrays should be tagged with `FT_LOCAL_ARRAY` and      */
  /* `FT_LOCAL_ARRAY_DEF`.                                               */
  /*                                                                     */
#ifdef FT_MAKE_OPTION_SINGLE_OBJECT

#define FT_LOCAL( x )      static  x
#define FT_LOCAL_DEF( x )  static  x

#else

#ifdef __cplusplus
#define FT_LOCAL( x )      extern "C"  x
#define FT_LOCAL_DEF( x )  extern "C"  x
#else
#define FT_LOCAL( x )      extern  x
#define FT_LOCAL_DEF( x )  x
#endif

#endif /* FT_MAKE_OPTION_SINGLE_OBJECT */

#define FT_LOCAL_ARRAY( x )      extern const  x
#define FT_LOCAL_ARRAY_DEF( x )  const  x


  /* Use `FT_BASE` and `FT_BASE_DEF` to declare and define, respectively, */
  /* functions that are used in more than a single module.  In the        */
  /* current setup this implies that the declaration is in a header file  */
  /* in the `include/freetype/internal` directory, and the function body  */
  /* is in a file in `src/base`.                                          */
  /*                                                                      */
#ifndef FT_BASE

#ifdef __cplusplus
#define FT_BASE( x )  extern "C"  x
#else
#define FT_BASE( x )  extern  x
#endif

#endif /* !FT_BASE */


#ifndef FT_BASE_DEF

#ifdef __cplusplus
#define FT_BASE_DEF( x )  x
#else
#define FT_BASE_DEF( x )  x
#endif

#endif /* !FT_BASE_DEF */


  /* When compiling FreeType as a DLL or DSO with hidden visibility    */
  /* some systems/compilers need a special attribute in front OR after */
  /* the return type of function declarations.                         */
  /*                                                                   */
  /* Two macros are used within the FreeType source code to define     */
  /* exported library functions: `FT_EXPORT` and `FT_EXPORT_DEF`.      */
  /*                                                                   */
  /* - `FT_EXPORT( return_type )`                                      */
  /*                                                                   */
  /*   is used in a function declaration, as in                        */
  /*                                                                   */
  /*   ```                                                             */
  /*     FT_EXPORT( FT_Error )                                         */
  /*     FT_Init_FreeType( FT_Library*  alibrary );                    */
  /*   ```                                                             */
  /*                                                                   */
  /* - `FT_EXPORT_DEF( return_type )`                                  */
  /*                                                                   */
  /*   is used in a function definition, as in                         */
  /*                                                                   */
  /*   ```                                                             */
  /*     FT_EXPORT_DEF( FT_Error )                                     */
  /*     FT_Init_FreeType( FT_Library*  alibrary )                     */
  /*     {                                                             */
  /*       ... some code ...                                           */
  /*       return FT_Err_Ok;                                           */
  /*     }                                                             */
  /*   ```                                                             */
  /*                                                                   */
  /* You can provide your own implementation of `FT_EXPORT` and        */
  /* `FT_EXPORT_DEF` here if you want.                                 */
  /*                                                                   */
  /* To export a variable, use `FT_EXPORT_VAR`.                        */
  /*                                                                   */
#ifndef FT_EXPORT

#ifdef FT2_BUILD_LIBRARY

#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define FT_EXPORT( x )  __declspec( dllexport )  x
#elif defined( __GNUC__ ) && __GNUC__ >= 4
#define FT_EXPORT( x )  __attribute__(( visibility( "default" ) ))  x
#elif defined( __SUNPRO_C ) && __SUNPRO_C >= 0x550
#define FT_EXPORT( x )  __global  x
#elif defined( __cplusplus )
#define FT_EXPORT( x )  extern "C"  x
#else
#define FT_EXPORT( x )  extern  x
#endif

#else

#if defined( _WIN32 ) && defined( DLL_IMPORT )
#define FT_EXPORT( x )  __declspec( dllimport )  x
#elif defined( __cplusplus )
#define FT_EXPORT( x )  extern "C"  x
#else
#define FT_EXPORT( x )  extern  x
#endif

#endif

#endif /* !FT_EXPORT */


#ifndef FT_EXPORT_DEF

#ifdef __cplusplus
#define FT_EXPORT_DEF( x )  extern "C"  x
#else
#define FT_EXPORT_DEF( x )  extern  x
#endif

#endif /* !FT_EXPORT_DEF */


#ifndef FT_EXPORT_VAR

#ifdef __cplusplus
#define FT_EXPORT_VAR( x )  extern "C"  x
#else
#define FT_EXPORT_VAR( x )  extern  x
#endif

#endif /* !FT_EXPORT_VAR */


  /* The following macros are needed to compile the library with a   */
  /* C++ compiler and with 16bit compilers.                          */
  /*                                                                 */

  /* This is special.  Within C++, you must specify `extern "C"` for */
  /* functions which are used via function pointers, and you also    */
  /* must do that for structures which contain function pointers to  */
  /* assure C linkage -- it's not possible to have (local) anonymous */
  /* functions which are accessed by (global) function pointers.     */
  /*                                                                 */
  /*                                                                 */
  /* FT_CALLBACK_DEF is used to _define_ a callback function,        */
  /* located in the same source code file as the structure that uses */
  /* it.                                                             */
  /*                                                                 */
  /* FT_BASE_CALLBACK and FT_BASE_CALLBACK_DEF are used to declare   */
  /* and define a callback function, respectively, in a similar way  */
  /* as FT_BASE and FT_BASE_DEF work.                                */
  /*                                                                 */
  /* FT_CALLBACK_TABLE is used to _declare_ a constant variable that */
  /* contains pointers to callback functions.                        */
  /*                                                                 */
  /* FT_CALLBACK_TABLE_DEF is used to _define_ a constant variable   */
  /* that contains pointers to callback functions.                   */
  /*                                                                 */
  /*                                                                 */
  /* Some 16bit compilers have to redefine these macros to insert    */
  /* the infamous `_cdecl` or `__fastcall` declarations.             */
  /*                                                                 */
#ifndef FT_CALLBACK_DEF
#ifdef __cplusplus
#define FT_CALLBACK_DEF( x )  extern "C"  x
#else
#define FT_CALLBACK_DEF( x )  static  x
#endif
#endif /* FT_CALLBACK_DEF */

#ifndef FT_BASE_CALLBACK
#ifdef __cplusplus
#define FT_BASE_CALLBACK( x )      extern "C"  x
#define FT_BASE_CALLBACK_DEF( x )  extern "C"  x
#else
#define FT_BASE_CALLBACK( x )      extern  x
#define FT_BASE_CALLBACK_DEF( x )  x
#endif
#endif /* FT_BASE_CALLBACK */

#ifndef FT_CALLBACK_TABLE
#ifdef __cplusplus
#define FT_CALLBACK_TABLE      extern "C"
#define FT_CALLBACK_TABLE_DEF  extern "C"
#else
#define FT_CALLBACK_TABLE      extern
#define FT_CALLBACK_TABLE_DEF  /* nothing */
#endif
#endif /* FT_CALLBACK_TABLE */


FT_END_HEADER


#endif /* FTCONFIG_H_ */


/* END */
