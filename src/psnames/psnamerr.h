/***************************************************************************/
/*                                                                         */
/*  psnamerr.h                                                             */
/*                                                                         */
/*    PS names module error codes (specification only).                    */
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
  /* This file is used to define the PS names module error enumeration     */
  /* constants.                                                            */
  /*                                                                       */
  /*************************************************************************/

#ifndef __PSNAMERR_H__
#define __PSNAMERR_H__

#include FT_MODULE_ERRORS_H

#undef __FTERRORS_H__

#define FT_ERRORDEF( e, v, s )    PSnames_Err_ ## e = v + FT_Mod_Err_PSnames,
#define FT_NOERRORDEF( e, v, s )  PSnames_Err_ ## e = v,

#define FT_ERROR_START_LIST       enum {
#define FT_ERROR_END_LIST         PSnames_Err_Max };

#include FT_ERRORS_H

#endif /* __PSNAMERR_H__ */


/* END */
