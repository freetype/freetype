/***************************************************************************/
/*                                                                         */
/*  gxobjs.h                                                               */
/*                                                                         */
/*    AAT/TrueTypeGX objects manager (specification).                      */
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


#ifndef __GXOBJS_H__
#define __GXOBJS_H__

#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_TRUETYPE_TYPES_H

FT_BEGIN_HEADER

  typedef struct GX_DriverRec_
  {
    FT_DriverRec    root;
    TT_ExecContext  context;
    TT_GlyphZoneRec zone;
    void*           extension_component;
  } GX_DriverRec, *GX_Driver;

  /*************************************************************************/
  /*                                                                       */
  /* Driver functions                                                      */
  /*                                                                       */
  FT_LOCAL( FT_Error )
  gx_driver_init( GX_Driver  driver );

FT_END_HEADER

#endif /* __GXOBJS_H__ */


/* END */
