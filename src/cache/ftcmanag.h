/***************************************************************************/
/*                                                                         */
/*  ftcmanag.h                                                             */
/*                                                                         */
/*    FreeType Cache Manager (specification).                              */
/*                                                                         */
/*  Copyright 2000 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef FTCMANAG_H
#define FTCMANAG_H

#include <freetype/ftcache.h>
#include <cache/ftlru.h>


#ifdef __cplusplus
  extern "C" {
#endif


#define  FTC_MAX_FACES  4
#define  FTC_MAX_SIZES  8


  typedef struct  FTC_ManagerRec_
  {
    FT_Library          library;
    FT_Lru              faces_lru;
    FT_Lru              sizes_lru;
    
    FT_Pointer          request_data;
    FTC_Face_Requester  request_face;
    
  } FTC_ManagerRec;



#ifdef __cplusplus
  }
#endif


#endif /* FTCMANAG_H */


/* END */
