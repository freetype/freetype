#ifndef __FT_SERVICE_BDF_H__
#define __FT_SERVICE_BDF_H__

#include FT_INTERNAL_SERVICE_H

FT_BEGIN_HEADER

#define FT_SERVICE_ID_BDF  "bdf"

  typedef FT_Error  (*FT_BDF_GetCharsetIdFunc)
                            ( FT_Face       face,
                              const char*  *acharset_encoding,
                              const char*  *acharset_registry );

  typedef FT_Error  (*FT_BDF_GetPropertyFunc)
                            ( FT_Face           face,
                              const char*       prop_name,
                              BDF_PropertyRec  *aproperty );

  FT_DEFINE_SERVICE( BDF )
  {
    FT_BDF_GetCharsetIdFunc  get_charset_id;
    FT_BDF_GetPropertyFunc   get_property;
  };

 /* */
 
FT_END_HEADER

#endif /* __FT_SERVICE_BDF_H__ */
