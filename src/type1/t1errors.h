/***************************************************************************/
/*                                                                         */
/*  t1errors.h                                                             */
/*                                                                         */
/*    Type 1 error codes (specification only).                             */
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
  /* This file is used to define the Type 1 error enumeration constants.   */
  /*                                                                       */
  /*************************************************************************/

#ifndef __T1ERRORS_H__
#define __T1ERRORS_H__

#include FT_MODULE_ERRORS_H

#undef __FTERRORS_H__

#define FT_ERRORDEF_( e, v, s )   \
          FT_ERRORDEF( T1_Err_ ## e, v + FT_Mod_Err_Type1, s )
#define FT_NOERRORDEF_( e, v, s ) \
          FT_ERRORDEF( T1_Err_ ## e, v, s )

#define FT_ERROR_START_LIST       enum {
#define FT_ERROR_END_LIST         T1_Err_Max };

#include FT_ERRORS_H

#endif /* __T1ERRORS_H__ */


/* END */
