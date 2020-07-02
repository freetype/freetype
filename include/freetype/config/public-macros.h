/****************************************************************************
 *
 * config/public-macros.h
 *
 *   Define a set of compiler macros used in public FreeType headers.
 *
 * Copyright (C) 2020 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

/* The definitions in this file are used by the public FreeType headers,
 * and thus should be considered part of the public API.
 *
 * Other compiler-specific macro definitions that are not exposed by the
 * FreeType API should go into include/freetype/internal/compiler-macros.h
 * instead.
 */
#ifndef FREETYPE_CONFIG_PUBLIC_MACROS_H_
#define FREETYPE_CONFIG_PUBLIC_MACROS_H_

/* FT_BEGIN_HEADER and FT_END_HEADER might have already been defined by
 * <freetype/config/ftheader.h>, but we don't want to include this header
 * here, so redefine the macros here only when needed. Their definition is
 * very stable, so keeping them in sync with the ones in the header should
 * not be a maintenance issue.
 */
#ifndef FT_BEGIN_HEADER
#  ifdef __cplusplus
#    define FT_BEGIN_HEADER extern "C" {
#  else
#    define FT_BEGIN_HEADER  /* nothing */
#  endif
#endif  /* FT_END_HEADER */

#ifndef FT_END_HEADER
#  ifdef __cplusplus
#    define FT_END_HEADER  }
#  else
#    define FT_END_HEADER  /* nothing */
#  endif
#endif  /* FT_END_HEADER */

FT_BEGIN_HEADER

/* Define a public FreeType API function. This ensures it is properly exported
 * or imported at build time.
 */
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

#else  /* !FT2_BUILD_LIBRARY */

#if defined( _WIN32 ) && defined( DLL_IMPORT )
#define FT_EXPORT( x )  __declspec( dllimport )  x
#elif defined( __cplusplus )
#define FT_EXPORT( x )  extern "C"  x
#else
#define FT_EXPORT( x )  extern  x
#endif

#endif  /* !FT2_BUILD_LIBRARY */

#endif /* !FT_EXPORT */

FT_END_HEADER

#endif  /* FREETYPE_CONFIG_PUBLIC_MACROS_H_ */
