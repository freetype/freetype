/***************************************************************************/
/*                                                                         */
/*  ftheader.h                                                             */
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
#define FT2_ENCLOSE(x)  <x>

  /* this macro is used to join a path and a file name */
#define FT2_JOINPATH(d,x)  d/x

  /* this macro is used to format a path in "<d/x>" format easily */
#define FT2_PUBLIC_PATH(d,x)  FT2_ENCLOSE(d/x)



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

/*************************************************************************
 *
 * <Section> header_file_macros
 *
 * <Title> Header File Macros
 *
 * <Abstract>
 *    Macro definitions used to #include specific header files
 *
 * <Description>
 *    The following macros are defined to the name of specific FreeType 2
 *    header files. They can be used directly in #include statements as in:
 *
 *    {
 *      #include FT_FREETYPE_H
 *      #include FT_MULTIPLE_MASTERS_H
 *      #include FT_GLYPH_H
 *    }
 *
 *    there are several reasons why we're now using macros to name
 *    public header files. The first one is that the macros are not
 *    limited to the infamous 8.3 naming rule required by Dos
 *    (and FT_MULTIPLE_MASTERS_H is a lot more meaningful than "ftmm.h")
 *
 *    The second reason is that is allows for more flexibility in the way
 *    FreeType 2 is installed on a given system..
 *
 */

  /* configuration files */
/*************************************************************************
 *
 * @macro: FT_CONFIG_CONFIG_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    FreeType 2 configuration
 */
#ifndef FT_CONFIG_CONFIG_H
#define FT_CONFIG_CONFIG_H     <freetype/config/ftconfig.h>
#endif

/*************************************************************************
 *
 * @macro: FT_CONFIG_OPTIONS_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    FreeType 2 project-specific configuration options
 */
#ifndef FT_CONFIG_OPTIONS_H
#define FT_CONFIG_OPTIONS_H    <freetype/config/ftoption.h>
#endif

/*************************************************************************
 *
 * @macro: FT_CONFIG_MODULES_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the list of FreeType 2 modules that are statically linked to new
 *    library instances in @FT_Init_FreeType
 */
#ifndef FT_CONFIG_MODULES_H
#define FT_CONFIG_MODULES_H    <freetype/config/ftmodule.h>
#endif

  /* public headers */
/*************************************************************************
 *
 * @macro: FT_FREETYPE_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the base FreeType 2 API
 */
#define FT_FREETYPE_H          <freetype/freetype.h>


/*************************************************************************
 *
 * @macro: FT_ERRORS_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the list of FreeType 2 error codes (and messages).
 *
 *    it is included by @FT_FREETYPE_H
 */
#define FT_ERRORS_H            <freetype/fterrors.h>

/*************************************************************************
 *
 * @macro: FT_SYSTEM_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the FreeType 2 interface to low-level operations (i.e. memory management
 *    and stream i/o)
 *
 *    it is included by @FT_FREETYPE_H
 */
#define FT_SYSTEM_H            <freetype/ftsystem.h>

/*************************************************************************
 *
 * @macro: FT_IMAGE_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    types definitions related to glyph images (i.e. bitmaps, outlines,
 *    scan-converter parameters)
 *
 *    it is included by @FT_FREETYPE_H
 */
#define FT_IMAGE_H             <freetype/ftimage.h>

/*************************************************************************
 *
 * @macro: FT_TYPES_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the basic data types defined by FreeType 2
 *
 *    it is included by @FT_FREETYPE_H
 */
#define FT_TYPES_H             <freetype/fttypes.h>

/*************************************************************************
 *
 * @macro: FT_LIST_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the list management API of FreeType 2
 *
 *    (most applications will never need to include this file)
 */
#define FT_LIST_H              <freetype/ftlist.h>


/*************************************************************************
 *
 * @macro: FT_OUTLINE_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the scalable outline management API of FreeType 2
 */
#define FT_OUTLINE_H           <freetype/ftoutln.h>


/*************************************************************************
 *
 * @macro: FT_MODULE_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the module management API of FreeType 2
 */
#define FT_MODULE_H            <freetype/ftmodule.h>

/*************************************************************************
 *
 * @macro: FT_RENDER_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the renderer module management API of FreeType 2
 */
#define FT_RENDER_H            <freetype/ftrender.h>

/*************************************************************************
 *
 * @macro: FT_TYPE1_TABLES_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the types and API specific to the Type 1 format.
 */
#define FT_TYPE1_TABLES_H      <freetype/t1tables.h>

/*************************************************************************
 *
 * @macro: FT_TRUETYPE_NAMES_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the enumeration values used to identify name strings, languages,
 *    encodings, etc.. This file really contains a _large_ set of
 *    constant macro definitions, taken from the TrueType and OpenType
 *    specs..
 */
#define FT_TRUETYPE_NAMES_H    <freetype/ttnameid.h>

/*************************************************************************
 *
 * @macro: FT_TRUETYPE_TABLES_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the types and API specific to the TrueType (as well as OpenType) format.
 */
#define FT_TRUETYPE_TABLES_H   <freetype/tttables.h>

/*************************************************************************
 *
 * @macro: FT_TRUETYPE_TAGS_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the definitions of TrueType 4-byte "tags" used to identify blocks
 *    in SFNT-based font formats (i.e. TrueType and OpenType)
 */
#define FT_TRUETYPE_TAGS_H     <freetype/tttags.h>

/*************************************************************************
 *
 * @macro: FT_GLYPH_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the API of the optional glyph management component.
 */
#define FT_GLYPH_H             <freetype/ftglyph.h>

/*************************************************************************
 *
 * @macro: FT_BBOX_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the API of the optional exact bounding box computation routines
 */
#define FT_BBOX_H              <freetype/ftbbox.h>

/*************************************************************************
 *
 * @macro: FT_CACHE_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the API of the optional FreeType 2 cache sub-system.
 */
#define FT_CACHE_H             <freetype/ftcache.h>

/*************************************************************************
 *
 * @macro: FT_MAC_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the Macintosh-specific FreeType 2 API. The latter is used to 
 *    access fonts embedded in resource forks..
 *
 *    this header file must be explicitely included by client applications
 *    compiled on the Mac (note that the base API still works though)
 */
#define FT_MAC_H               <freetype/ftmac.h>

/*************************************************************************
 *
 * @macro: FT_MULTIPLE_MASTERS_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the optional multiple-masters management API of FreeType 2
 */
#define FT_MULTIPLE_MASTERS_H  <freetype/ftmm.h>

/*************************************************************************
 *
 * @macro: FT_NAMES_H
 *
 * @description:
 *    a macro used in #include statements to name the file containing
 *    the optional FreeType 2 API used to access embedded "name" strings
 *    in SFNT-based font formats (i.e. TrueType and OpenType)
 */
#define FT_NAMES_H             <freetype/ftnames.h>

 /* */
 
#define FT_SYNTHESIS_H         <freetype/ftsynth.h>


  /* now include internal headers definitions from <freetype/internal/...> */
#define  FT_INTERNAL_INTERNAL_H   <freetype/internal/internal.h>
#include FT_INTERNAL_INTERNAL_H


#endif /* __FT2_BUILD_H__ */


/* END */
