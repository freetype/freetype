/***************************************************************************/
/*                                                                         */
/*  gxerrors.h                                                             */
/*                                                                         */
/*    OT error codes (specification only).                                 */
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This file is used to define the OT error enumeration constants.       */
  /*                                                                       */
  /*************************************************************************/

#ifndef __OTERRORS_H__
#define __OTERRORS_H__

#include FT_MODULE_ERRORS_H

#undef __FTERRORS_H__

#define FT_ERR_PREFIX  OT_Err_
#define FT_ERR_BASE    FT_Mod_Err_OT


#include FT_ERRORS_H

#endif /* __OTERRORS_H__ */


/* END */
