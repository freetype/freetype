/***************************************************************************/
/*                                                                         */
/*  ftsmerrs.h                                                             */
/*                                                                         */
/*    smooth renderer error codes (specification only).                    */
/*                                                                         */
/*  Copyright 2001 by                                                      */
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
  /* This file is used to define the smooth renderer error enumeration     */
  /* constants.                                                            */
  /*                                                                       */
  /*************************************************************************/

#ifndef __FTSMERRS_H__
#define __FTSMERRS_H__

#include FT_MODULE_ERRORS_H

#undef __FTERRORS_H__

#define FT_ERRORDEF_( e, v, s )   \
          FT_ERRORDEF( Smooth_Err_ ## e, v + FT_Mod_Err_Smooth, s )
#define FT_NOERRORDEF_( e, v, s ) \
          FT_ERRORDEF( Smooth_Err_ ## e, v, s )

#define FT_ERROR_START_LIST       enum {
#define FT_ERROR_END_LIST         Smooth_Err_Max };

#include FT_ERRORS_H

#endif /* __FTSMERRS_H__ */


/* END */
