/***************************************************************************/
/*                                                                         */
/*  gxdump.h                                                               */
/*                                                                         */
/*    Debug functions for AAT/TrueTypeGX driver (specification).           */
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

#ifndef __GXDUMP_H__
#define __GXDUMP_H__ 

#include <ft2build.h>
#include FT_GXLAYOUT_H
#include <stdio.h>

FT_BEGIN_HEADER

  typedef enum {
    GX_DUMP_mort = 1 << 0,
    GX_DUMP_morx = 1 << 1,
    GX_DUMP_feat = 1 << 2,
    GX_DUMP_prop = 1 << 3,
    GX_DUMP_trak = 1 << 4,
    GX_DUMP_kern = 1 << 5,
    GX_DUMP_just = 1 << 6,
    GX_DUMP_lcar = 1 << 7,
    GX_DUMP_opbd = 1 << 8,
    GX_DUMP_bsln = 1 << 9,
    GX_DUMP_fmtx = 1 << 10,
    GX_DUMP_fdsc = 1 << 11,
    GX_DUMP_fvar = 1 << 12,
    GX_DUMP_ALL  = 0x7FFFFFFFUL	/* gcc warns if I set this to 0xFFFFFFFFUL. */
  } GXDumpFlag;

  FT_EXPORT( FT_Error )
  gx_face_dump( FT_Face face, FT_ULong tables, const char * fname );

/* If STREAM is NULL, stderr is used as output stream. */
  FT_EXPORT ( void )
  gxl_features_request_dump ( GXL_FeaturesRequest request, FILE * stream );


  FT_EXPORT ( void )
  gx_feature_registory_dump ( FILE * stream );

  FT_EXPORT ( void )
  gxl_feature_dump ( GXL_Feature feature, FILE * stream );

  FT_EXPORT ( void )
  gxl_setting_dump ( GXL_Setting setting, FILE * stream );

FT_END_HEADER

#endif /* Not def: __GXDUMP_H__ */


/* END */
