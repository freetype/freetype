/***************************************************************************/
/*                                                                         */
/*  ftccmap.h                                                              */
/*                                                                         */
/*    FreeType charmap cache (specification).                              */
/*                                                                         */
/*  Copyright 2000-2001, 2003, 2005 by                                     */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTCCMAP_H__
#define __FTCCMAP_H__

#include <ft2build.h>
#include FT_CACHE_H


FT_BEGIN_HEADER

/* the FT 2.1.7 Charmap cache interface 
 *
 * unfortunately, it is not possible to implement it in later
 * versions, since some function signature changed too significantly
 * to do that.
 */

#if 0

  typedef enum  FTC_CMapType_
  {
    FTC_CMAP_BY_INDEX    = 0,
    FTC_CMAP_BY_ENCODING = 1,
    FTC_CMAP_BY_ID       = 2

  } FTC_CMapType;


  typedef struct  FTC_CMapIdRec_
  {
    FT_UInt  platform;
    FT_UInt  encoding;

  } FTC_CMapIdRec;


  typedef struct  FTC_CMapDescRec_
  {
    FTC_FaceID    face_id;
    FTC_CMapType  type;

    union
    {
      FT_UInt        index;
      FT_Encoding    encoding;
      FTC_CMapIdRec  id;

    } u;

  } FTC_CMapDescRec, *FTC_CMapDesc;


#if 0
  FT_EXPORT( FT_Error )
  FTC_CMapCache_New( FTC_Manager     manager,
                     FTC_CMapCache  *acache );


  FT_EXPORT( FT_UInt )
  FTC_CMapCache_Lookup( FTC_CMapCache  cache,
                        FTC_CMapDesc   cmap_desc,
                        FT_UInt32      char_code );
#endif
                        
#endif /* FT_CONFIG_OPTION_OLD_INTERNALS */

  /* */

FT_END_HEADER


#endif /* __FTCCMAP_H__ */


/* END */
