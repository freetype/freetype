/***************************************************************************/
/*                                                                         */
/*  winfnt.h                                                               */
/*                                                                         */
/*    FreeType font driver for Windows FNT/FON files                       */
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


#ifndef WINFNT_H
#define WINFNT_H

#include <freetype/internal/ftdriver.h>


#ifdef __cplusplus
  extern "C" {
#endif


  FT_EXPORT_VAR( const FT_Driver_Class )  winfnt_driver_class;


#ifdef __cplusplus
  }
#endif


#endif /* WINFNT_H */


/* END */
