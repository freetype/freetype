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
  /* Note that specific platforms might use a different configuration.     */
  /* For example, on Unix, the "freetype2" include root could be used,     */
  /* with a specific "ft2build.h" to take care of this.  The latter then   */
  /* looks like the following:                                             */
  /*                                                                       */
  /*     #ifndef __FT2_BUILD_UNIX_H__                                      */
  /*     #define __FT2_BUILD_UNIX_H__                                      */
  /*                                                                       */
  /*     #define FT2_ROOT  freetype2                                       */
  /*     #include <freetype2/config/ft2build.h>                            */
  /*                                                                       */
  /*     #include FT2_CONFIG_FILE( ft2build.h )                            */
  /*                                                                       */
  /*     #endif                                                            */
  /*                                                                       */
  /* If necessary, the macro FT_SOURCE_FILE() can be redefined also if a   */
  /* different path separator is needed.                                   */
  /*                                                                       */
  /*************************************************************************/


#ifndef __FT2_BUILD_H__
#define __FT2_BUILD_H__


  /* this macro is used to enclose its argument in brackets */
#define  FT2_ENCLOSE(x)   <x>

  /* this macro is used to join a path and a file name */
#define  FT2_JOINPATH(d,x)   d/x

  /* this macro is used to format a path in "<d/x>" format easily */
#define  FT2_PUBLIC_PATH(d,x)   FT2_ENCLOSE(d/x)



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
#define FT2_CONFIG_ROOT    FT2_JOINPATH(FT2_ROOT,config)
#endif


#define FT2_INTERNAL_ROOT  FT2_JOINPATH(FT2_ROOT,internal)

  /*************************************************************************/
  /*                                                                       */
  /* The macro FT2_PUBLIC_FILE is used to include a FreeType 2 public file.*/
  /* Its parameter is the file pathname, relative to the public root of a  */
  /* given header file.                                                    */
  /*                                                                       */
#define FT2_PUBLIC_FILE( x )  FT2_PUBLIC_PATH(FT2_ROOT,x)


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT2_CONFIG_FILE is used to include a FreeType 2 config file.*/
  /* Its parameter is the file pathname, relative to the configuration     */
  /* root directory of a given header file.                                */
  /*                                                                       */
#define FT2_CONFIG_FILE( x )  FT2_PUBLIC_PATH(FT2_CONFIG_ROOT,x)


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT2_INTERNAL_FILE is used to include a FreeType 2 internal  */
  /* file.  Its parameter is the file pathname, relative to the            */
  /* configuration root directory of a given header file.                  */
  /*                                                                       */
#define FT2_INTERNAL_FILE( x )  FT2_ENCLOSE(FT2_ROOT/internal/x)


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
#ifndef FT_SOURCE_FILE

#ifdef FT_FLAT_COMPILATION
#define FT_SOURCE_FILE( d, x )  #x
#else
#define FT_SOURCE_FILE( d, x )  FT2_PUBLIC_PATH(d,x)
#endif

#endif /* !FT_SOURCE_FILE */


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

  /* configuration files */
#ifndef FT_CONFIG_CONFIG_H
#define FT_CONFIG_CONFIG_H     <freetype/config/ftconfig.h>
#endif

#ifndef FT_CONFIG_OPTIONS_H
#define FT_CONFIG_OPTIONS_H    <freetype/config/ftoption.h>
#endif

#ifndef FT_CONFIG_MODULES_H
#define FT_CONFIG_MODULES_H    <freetype/config/ftmodule.h>
#endif

  /* public headers */
#define FT_ERRORS_H            <freetype/fterrors.h>
#define FT_SYSTEM_H            <freetype/ftsystem.h>
#define FT_IMAGE_H             <freetype/ftimage.h>

#define FT_TYPES_H             <freetype/fttypes.h>

#define FT_FREETYPE_H          <freetype/freetype.h>
#define FT_GLYPH_H             <freetype/ftglyph.h>
#define FT_BBOX_H              <freetype/ftbbox.h>
#define FT_CACHE_H             <freetype/ftcache.h>
#define FT_LIST_H              <freetype/ftlist.h>
#define FT_MAC_H               <freetype/ftmac.h>
#define FT_MULTIPLE_MASTERS_H  <freetype/ftmm.h>
#define FT_MODULE_H            <freetype/ftmodule.h>
#define FT_NAMES_H             <freetype/ftnames.h>
#define FT_OUTLINE_H           <freetype/ftoutln.h>
#define FT_RENDER_H            <freetype/ftrender.h>
#define FT_SYNTHESIS_H         <freetype/ftsynth.h>
#define FT_TYPE1_TABLES_H      <freetype/t1tables.h>
#define FT_TRUETYPE_NAMES_H    <freetype/ttnameid.h>
#define FT_TRUETYPE_TABLES_H   <freetype/tttables.h>
#define FT_TRUETYPE_TAGS_H     <freetype/tttags.h>

  /* now include internal headers definitions from <freetype/internal/...> */
#define  FT_INTERNAL_INTERNAL_H   <freetype/internal/internal.h>
#include FT_INTERNAL_INTERNAL_H


#endif /* __FT2_BUILD_H__ */


/* END */
