/***************************************************************************/
/*                                                                         */
/*  ft2build.h                                                             */
/*                                                                         */
/*    Build macros of the FreeType 2 library.                              */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This file corresponds to the default "ft2build.h" file for            */
  /* FreeType 2.  It uses the "freetype" include root.                     */
  /*                                                                       */
  /* Note that specific platforms might use a different configurations.    */
  /* For example, on Unix, the "freetype2" include root is used, with a    */
  /* specific "ft2build.h" used to take care of this.  The latter looks    */
  /* like the following:                                                   */
  /*                                                                       */
  /*     #ifndef __FT_BUILD_UNIX_H__                                       */
  /*     #define __FT_BUILD_UNIX_H__                                       */
  /*                                                                       */
  /*     #define FT2_ROOT  freetype2                                       */
  /*     #include <FT2_ROOT/config/ft2build.h>                             */
  /*                                                                       */
  /*     #endif // __FT_BUILD_UNIX_H__                                     */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FT2_BUILD_H__
#define __FT2_BUILD_H__


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT2_ROOT is used to define the root of all public header    */
  /* files for FreeType 2.  By default, it is set to "freetype", which     */
  /* means that all public files should be included with a line like:      */
  /*                                                                       */
  /*   #include <freetype/...>                                             */
  /*                                                                       */
  /* Redefine it to something different if necessary, depending where the  */
  /* library is installed on the particular system.                        */
  /*                                                                       */
#ifndef FT2_ROOT
#define FT2_ROOT  freetype
#endif


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT2_CONFIG_ROOT is used to define the root of all           */
  /* configuration header files for FreeType 2.  By default, it is set to  */
  /* "freetype/config", which means that all config files should be        */
  /* include with a line like:                                             */
  /*                                                                       */
  /*   #include <freetype/config/...>                                      */
  /*                                                                       */
  /* Redefine it to something different, depending where the library is    */
  /* installed on the particular system.                                   */
  /*                                                                       */
#ifndef FT2_CONFIG_ROOT
#define FT2_CONFIG_ROOT_( x )  x ## / ## config
#define FT2_CONFIG_ROOT        FT2_CONFIG_ROOT_(FT2_ROOT)
#endif


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT2_PUBLIC_FILE is used to include a FreeType 2 public file.*/
  /* Its parameter is the file pathname, relative to the public root of a  */
  /* given header file.                                                    */
  /*                                                                       */
#define FT2_PUBLIC_FILE_( x, y )  < ## x ## / ## y ## >
#define FT2_PUBLIC_FILE( x )      FT2_PUBLIC_FILE_(FT2_ROOT,x)


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT2_CONFIG_FILE is used to include a FreeType 2 config file.*/
  /* Its parameter is the file pathname, relative to the configuration     */
  /* root directory of a given header file.                                */
  /*                                                                       */
#define FT2_CONFIG_FILE_( x, y )  < ## x ## / ## y ## >
#define FT2_CONFIG_FILE( x )      FT2_CONFIG_FILE_(FT2_CONFIG_ROOT,x)


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT2_INTERNAL_FILE is used to include a FreeType 2 internal  */
  /* file.  Its parameter is the file pathname, relative to the            */
  /* configuration root directory of a given header file.                  */
  /*                                                                       */
#define FT2_INTERNAL_FILE_( x, y )  < ## x ## / ## internal ## / ## y ## >
#define FT2_INTERNAL_FILE( x )      FT2_INTERNAL_FILE_(FT2_ROOT,x)


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_SOURCE_FILE is used to include a given FreeType 2        */
  /* component source file (be it a header, a C source file, or an         */
  /* included file).                                                       */
  /*                                                                       */
  /* Its first argument is the component/module's directory according to   */
  /* the normal FreeType 2 source directory hierarchy, and the second one  */
  /* the file name.                                                        */
  /*                                                                       */
  /* Note that you can also put all library source files in a single       */
  /* directory and compile them normally by defining the macro             */
  /* FT_FLAT_COMPILATION.                                                  */
  /*                                                                       */
#ifdef FT_FLAT_COMPILATION
#define FT_SOURCE_FILE( d, x )  #x
#else
#define FT_SOURCE_FILE( d, x )  < ## d ## / ## x ## >
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Macro>                                                               */
  /*    FT_BEGIN_HEADER                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This macro is used in association with @FT_END_HEADER in header    */
  /*    files to ensure that the declarations within are properly          */
  /*    encapsulated in an `extern "C" { .. }' block when included from a  */
  /*    C++ compiler.                                                      */
  /*                                                                       */
#ifdef __cplusplus
#define FT_BEGIN_HEADER  extern "C" {
#else
#define FT_BEGIN_HEADER  /* nothing */
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Macro>                                                               */
  /*    FT_END_HEADER                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This macro is used in association with @FT_BEGIN_HEADER in header  */
  /*    files to ensure that the declarations within are properly          */
  /*    encapsulated in an `extern "C" { .. }' block when included from a  */
  /*    C++ compiler.                                                      */
  /*                                                                       */
#ifdef __cplusplus
#define FT_END_HEADER  }
#else
#define FT_END_HEADER  /* nothing */
#endif


  /*************************************************************************/
  /*                                                                       */
  /* Aliases for the FreeType 2 public and configuration files.            */
  /*                                                                       */
  /*************************************************************************/

  /* don't add spaces around arguments to FT2_CONFIG_FILE! */

  /* configuration files */
#ifndef FT_CONFIG_CONFIG_H
#define FT_CONFIG_CONFIG_H     FT2_CONFIG_FILE(ftconfig.h)
#endif

#ifndef FT_CONFIG_OPTIONS_H
#define FT_CONFIG_OPTIONS_H    FT2_CONFIG_FILE(ftoption.h)
#endif

#ifndef FT_CONFIG_MODULES_H
#define FT_CONFIG_MODULES_H    FT2_CONFIG_FILE(ftmodule.h)
#endif

  /* public headers */
#define FT_ERRORS_H            FT2_PUBLIC_FILE(fterrors.h)
#define FT_SYSTEM_H            FT2_PUBLIC_FILE(ftsystem.h)
#define FT_IMAGE_H             FT2_PUBLIC_FILE(ftimage.h)

#define FT_TYPES_H             FT2_PUBLIC_FILE(fttypes.h)

#define FT_FREETYPE_H          FT2_PUBLIC_FILE(freetype.h)
#define FT_GLYPH_H             FT2_PUBLIC_FILE(ftglyph.h)
#define FT_BBOX_H              FT2_PUBLIC_FILE(ftbbox.h)
#define FT_CACHE_H             FT2_PUBLIC_FILE(ftcache.h)
#define FT_LIST_H              FT2_PUBLIC_FILE(ftlist.h)
#define FT_MAC_H               FT2_PUBLIC_FILE(ftmac.h)
#define FT_MULTIPLE_MASTERS_H  FT2_PUBLIC_FILE(ftmm.h)
#define FT_MODULE_H            FT2_PUBLIC_FILE(ftmodule.h)
#define FT_NAMES_H             FT2_PUBLIC_FILE(ftnames.h)
#define FT_OUTLINE_H           FT2_PUBLIC_FILE(ftoutln.h)
#define FT_RENDER_H            FT2_PUBLIC_FILE(ftrender.h)
#define FT_SYNTHESIS_H         FT2_PUBLIC_FILE(ftsynth.h)
#define FT_TYPE1_TABLES_H      FT2_PUBLIC_FILE(t1tables.h)
#define FT_TRUETYPE_NAMES_H    FT2_PUBLIC_FILE(ttnameid.h)
#define FT_TRUETYPE_TABLES_H   FT2_PUBLIC_FILE(tttables.h)
#define FT_TRUETYPE_TAGS_H     FT2_PUBLIC_FILE(tttags.h)


  /* now include internal headers definitions from <freetype/internal/...> */
#define FT2_INTERNAL_H  FT2_INTERNAL_FILE(internal.h)
#include FT2_INTERNAL_H


#endif /* __FT2_BUILD_H__ */


/* END */
