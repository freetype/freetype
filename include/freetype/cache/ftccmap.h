/***************************************************************************/
/*                                                                         */
/*  ftccmap.h                                                              */
/*                                                                         */
/*    FreeType charmap cache                                               */
/*                                                                         */
/*  Copyright 2000-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef __FT_CACHE_CHARMAP_H__
#define __FT_CACHE_CHARMAP_H__

#include <ft2build.h>
#include FT_CACHE_H

FT_BEGIN_HEADER

 /************************************************************************
  *
  * @type: FTC_CmapCache
  *
  * @description:
  *   opaque handle used to manager a charmap cache. This cache is used
  *   to hold character codes -> glyph indices mappings
  */
  typedef struct FTC_CMapCacheRec_*    FTC_CMapCache;


 /************************************************************************
  *
  * @type: FTC_CMapDesc
  *
  * @description:
  *   handle to a @FTC_CMapDescRec structure used to describe a given charmap
  *   in a charmap cache.
  *
  *   each @FTC_CMapDesc describes which charmap, of which @FTC_Face we
  *   want to use in @FTC_CMapCache_Lookup
  */
  typedef struct FTC_CMapDescRec_*         FTC_CMapDesc;


 /************************************************************************
  *
  * @enum: FTC_CMapType
  *
  * @description:
  *   the list of valid @FTC_CMap types, they indicate how we want to
  *   address a charmap within a @FTC_FaceID
  *
  * @values:
  *   FTC_CMAP_BY_INDEX ::
  *     used to indicate that we want to address a charmap by its index in
  *     the corresponding @FT_Face
  *
  *   FTC_CMAP_BY_ENCODING ::
  *     used to indicate that we want to use a @FT_Face charmap that
  *     corresponds to a given encoding
  *
  *   FTC_CMAP_BY_ID ::
  *     used to indicate that we want to use a @FT_Face charmap that
  *     corresponds to a given (platform,encoding) id. see @FTC_CMapIdRec
  */
  typedef enum
  {
    FTC_CMAP_BY_INDEX    = 0,
    FTC_CMAP_BY_ENCODING = 1,
    FTC_CMAP_BY_ID       = 2
  
  } FTC_CMapType;


 /************************************************************************
  *
  * @struct: FTC_CMapIdRec
  *
  * @description:
  *   a short structure used to identify a charmap by a (platform,encoding)
  *   pair of values
  *
  * @fields:
  *   platform :: platform ID
  *   encoding :: encoding ID
  */
  typedef struct FTC_CMapIdRec_
  {
    FT_UInt  platform;
    FT_UInt  encoding;
  
  } FTC_CMapIdRec;


 /************************************************************************
  *
  * @struct: FTC_CMapDescRec
  *
  * @description:
  *   a structure used to describe a given charmap to the @FTC_CMapCache
  *
  * @fields:
  *   face_id :: @FTC_FaceID of the face this charmap belongs to
  *   type    :: type of charmap, see @FTC_CMapType
  *
  *   u.index    :: for @FTC_CMAP_BY_INDEX types, this is the charmap index
  *                 (within a @FT_Face) we want to use.
  *
  *   u.encoding :: for @FTC_CMAP_BY_ENCODING types, this is the charmap
  *                 encoding we want to use. see @FT_Encoding
  *
  *   u.id       :: for @FTC_CMAP_BY_ID types, this is the (platform,encoding)
  *                 pair we want to use. see @FTC_CMapIdRec and
  *                 @FT_CharMapRec
  */
  typedef struct FTC_CMapDescRec_
  {
    FTC_FaceID    face_id;
    FTC_CMapType  type;

    union
    {
      FT_UInt        index;
      FT_Encoding    encoding;
      FTC_CMapIdRec  id;
      
    } u;
     
  } FTC_CMapDescRec;



 /************************************************************************
  *
  * @function: FTC_CMapCache_New
  *
  * @description:
  *   create a new charmap cache
  *
  * @input:
  *   manager :: handle to cache manager
  *
  * @output:
  *   acache  :: new cache handle. NULL in case of error
  *
  * @return:
  *   FreeType error code. 0 means success
  *
  * @note:
  *   like all other caches, this one will be destroyed with the
  *   cache manager
  */
  FT_EXPORT( FT_Error )
  FTC_CMapCache_New( FTC_Manager     manager,
                     FTC_CMapCache  *acache );



 /************************************************************************
  *
  * @function: FTC_CMapCache_Lookup
  *
  * @description:
  *   translate a character code into a glyph index, using the charmap
  *   cache.
  *
  * @input:
  *   cache     :: charmap cache handle
  *   cmap_desc :: charmap descriptor handle
  *   char_code :: character code (in the corresponding charmap)
  *
  * @return:
  *   glyph index. 0 means "no glyph" !!
  *
  * @note:
  *   this function doesn't return @FTC_Node handles, since there is
  *   no real use for them with typical uses of charmaps
  */
  FT_EXPORT( FT_UInt )
  FTC_CMapCache_Lookup( FTC_CMapCache  cache,
                        FTC_CMapDesc   cmap_desc,
                        FT_UInt32      char_code );


 /* */

FT_END_HEADER

#endif /* __FT_CACHE_CHARMAP_H__ */
