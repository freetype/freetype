#ifndef __FT_SERVICE_POSTSCRIPT_NAME_H__
#define __FT_SERVICE_POSTSCRIPT_NAME_H__

#include FT_INTERNAL_SERVICE_H

FT_BEGIN_HEADER

 /*
  *  a trivial service used to retrieve the Postscript name of a given
  *  font when available. The "get_name" field should never be NULL
  *
  *  the correponding function can return NULL to indicate that the
  *  Postscript name is not available.
  *
  *  the name is owned by the face and will be destroyed with it
  *
  */

#define FT_SERVICE_ID_POSTSCRIPT_NAME   "postscript-name"

  typedef const char*  (*FT_PsName_GetFunc)( FT_Face  face );
  
  FT_DEFINE_SERVICE( PsName )
  {
    FT_PsName_GetFunc   get_ps_name;
  };

FT_END_HEADER

#endif /* __FT_SERVICE_POSTSCRIPT_NAME_H__ */
