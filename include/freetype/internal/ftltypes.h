/***************************************************************************/
/*                                                                         */
/*  ftltypes.h                                                             */
/*                                                                         */
/*    Types used in the layout engine stacked on ft2  (specification)      */ 
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

#ifndef __FTL_TYPES_H__
#define __FTL_TYPES_H__ 

#include <ft2build.h>
#include FT_LAYOUT_H

FT_BEGIN_HEADER

#define FTL_FONT( x ) ((FTL_Font)(x))
#define FTL_FONT_FACE( x ) (FTL_FONT( x )->face)
#define FTL_FEATURES_REQUEST( x ) ((FTL_FeaturesRequest)( x ))
#define FTL_FEATURES_REQUEST_FONT( x )(FTL_FEATURES_REQUEST( x )->font)

  typedef struct FTL_FontRec_            * FTL_Font;
  typedef struct FTL_FeaturesRequestRec_
  {
    FTL_Font      font;
    FTL_Direction direction;
  } FTL_FeaturesRequestRec;

  typedef struct FTL_FontRec_
  {
    FT_Face face;

    /* This one is used as active features request. */
    FTL_FeaturesRequest features_request; 
    FT_ListRec          features_requests_list;
  } FTL_FontRec;

  FT_EXPORT( FT_Error )
  FTL_Font_Init                          ( FTL_Font font,
					   FT_Face  face );
  FT_EXPORT( FT_Error )
  FTL_Font_Finalize                      ( FTL_Font font );

  FT_EXPORT( FT_Error )
  FTL_FeaturesRequest_Init               ( FT_Face face, 
					   FTL_FeaturesRequest request);
  FT_EXPORT( FT_Error )
  FTL_FeaturesRequest_Finalize           ( FTL_FeaturesRequest request );

  FT_EXPORT_DEF( FT_Error )
  FTL_FeaturesRequest_Copy               ( FTL_FeaturesRequest from,  
					   FTL_FeaturesRequest to );

  FT_EXPORT( FT_Error )
  FTL_Font_Get_Default_FeaturesRequest   ( FTL_Font font,
					   FTL_FeaturesRequest * request );

  FT_EXPORT( FT_Error )
  FTL_Font_Get_Current_FeaturesRequest   ( FTL_Font font,
					   FTL_FeaturesRequest * request );

  FT_EXPORT( FT_Error )
  FTL_Get_Font                           ( FT_Face face,
					   FTL_Font * font );

FT_END_HEADER

#endif /* Not def: __FTL_TYPES_H__ */


/* END */
