/***************************************************************************/
/*                                                                         */
/*  gxload.h                                                               */
/*                                                                         */
/*    Functions load AAT/TrueTypeGX tables(specification)                  */
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

#ifndef __GXLOAD_H__
#define __GXLOAD_H__ 

#include <ft2build.h>
#include FT_FREETYPE_H
#include "gxtypes.h"
#include "gxltypes.h"

FT_BEGIN_HEADER

#define gx_table_load(face,stream,tag,FONT)				\
  (error = gx_face_load_##tag (face, stream, FONT->tag),		\
   FONT->tag->root.font = (error? NULL: FONT),				\
   FONT->tag->root.done_table = (error? NULL: FONT->tag->root.done_table), \
   error)

  FT_LOCAL ( FT_Error )
  gx_table_done ( GX_Table table, FT_Memory  memory );

  FT_LOCAL ( FT_Error )
  gx_face_load_feat( GX_Face   face,
		     FT_Stream stream,
		     GX_Feat   feat );

  FT_LOCAL ( FT_Error )
  gx_face_load_trak( GX_Face   face,
		     FT_Stream stream,
		     GX_Trak   trak );

  FT_LOCAL ( FT_Error )
  gx_face_load_kern( GX_Face   face,
		     FT_Stream stream,
		     GX_Kern   kern );

  FT_LOCAL ( FT_Error )
  gx_face_load_prop( GX_Face   face,
		     FT_Stream stream,
		     GX_Prop   prop );

  FT_LOCAL ( FT_Error )
  gx_face_load_opbd( GX_Face   face,
		     FT_Stream stream,
		     GX_Opbd   opbd );

  FT_LOCAL ( FT_Error )
  gx_face_load_lcar( GX_Face   face,
		     FT_Stream stream,
		     GX_Lcar   lcar );

  FT_LOCAL ( FT_Error )
  gx_face_load_bsln( GX_Face   face,
		     FT_Stream stream,
		     GX_Bsln   bsln );

  FT_LOCAL ( FT_Error )
  gx_face_load_mort( GX_Face   face,
		     FT_Stream stream,
		     GX_Mort   mort );
  
  FT_LOCAL ( FT_Error )
  gx_face_load_morx( GX_Face   face,
		     FT_Stream stream,
		     GX_Morx   morx );
  
  FT_LOCAL ( FT_Error )
  gx_face_load_fmtx( GX_Face   face,
		     FT_Stream stream,
		     GX_Fmtx   fmtx );
  
  FT_LOCAL ( FT_Error )
  gx_face_load_fdsc( GX_Face   face,
		     FT_Stream stream,
		     GX_Fdsc   fdsc );
  
  FT_LOCAL ( FT_Error )
  gx_face_load_just( GX_Face   face,
		     FT_Stream stream,
		     GX_Just   just );

FT_END_HEADER

#endif /* Not def: __GXLOAD_H__ */


/* END */
