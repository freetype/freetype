#ifndef __FT_BDF_H__
#define __FT_BDF_H__

#include <ft2build.h>
#include FT_FREETYPE_H

FT_BEGIN_HEADER

  FT_EXPORT( FT_Error )
  FT_Get_BDF_Charset_ID( FT_Face       face,
                         const char*  *acharset_encoding,
                         const char*  *acharset_registry );

FT_END_HEADER

#endif /* __FT_BDF_H__ */
