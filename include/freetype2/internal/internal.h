/***************************************************************************/
/*                                                                         */
/*  internal.h                                                             */
/*                                                                         */
/*    Internal header files (specification only).                          */
/*                                                                         */
/*  Copyright 1996-2015 by                                                 */
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
  /* This file is automatically included by `ft2build.h'.                  */
  /* Do not include it manually!                                           */
  /*                                                                       */
  /*************************************************************************/


#define FT_INTERNAL_OBJECTS_H             <freetype2/internal/ftobjs.h>
#define FT_INTERNAL_PIC_H                 <freetype2/internal/ftpic.h>
#define FT_INTERNAL_STREAM_H              <freetype2/internal/ftstream.h>
#define FT_INTERNAL_MEMORY_H              <freetype2/internal/ftmemory.h>
#define FT_INTERNAL_DEBUG_H               <freetype2/internal/ftdebug.h>
#define FT_INTERNAL_CALC_H                <freetype2/internal/ftcalc.h>
#define FT_INTERNAL_DRIVER_H              <freetype2/internal/ftdriver.h>
#define FT_INTERNAL_TRACE_H               <freetype2/internal/fttrace.h>
#define FT_INTERNAL_GLYPH_LOADER_H        <freetype2/internal/ftgloadr.h>
#define FT_INTERNAL_SFNT_H                <freetype2/internal/sfnt.h>
#define FT_INTERNAL_SERVICE_H             <freetype2/internal/ftserv.h>
#define FT_INTERNAL_RFORK_H               <freetype2/internal/ftrfork.h>
#define FT_INTERNAL_VALIDATE_H            <freetype2/internal/ftvalid.h>

#define FT_INTERNAL_TRUETYPE_TYPES_H      <freetype2/internal/tttypes.h>
#define FT_INTERNAL_TYPE1_TYPES_H         <freetype2/internal/t1types.h>

#define FT_INTERNAL_POSTSCRIPT_AUX_H      <freetype2/internal/psaux.h>
#define FT_INTERNAL_POSTSCRIPT_HINTS_H    <freetype2/internal/pshints.h>
#define FT_INTERNAL_POSTSCRIPT_GLOBALS_H  <freetype2/internal/psglobal.h>

#define FT_INTERNAL_AUTOHINT_H            <freetype2/internal/autohint.h>


#if defined( _MSC_VER )      /* Visual C++ (and Intel C++) */

  /* We disable the warning `conditional expression is constant' here */
  /* in order to compile cleanly with the maximum level of warnings.  */
  /* In particular, the warning complains about stuff like `while(0)' */
  /* which is very useful in macro definitions.  There is no benefit  */
  /* in having it enabled.                                            */
#pragma warning( disable : 4127 )

#endif /* _MSC_VER */


/* END */
