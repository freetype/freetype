#include <freetype/ftnames.h>
#include <freetype/internal/tttypes.h>

#ifdef FT_CONFIG_OPTION_GLYPH_NAMES
#endif /* FT_CONFIG_OPTION_GLYPH_NAMES */




#ifdef FT_CONFIG_OPTION_SFNT_NAMES
  FT_EXPORT_FUNC(FT_UInt)  FT_Get_Sfnt_Name_Count( FT_Face  face )
  {
    return  ( face && FT_IS_SFNT(face) ? ((TT_Face)face)->num_names : 0 );
  }
  
  
  FT_EXPORT_FUNC(FT_Error) FT_Get_Sfnt_Name( FT_Face       face,
                                             FT_UInt       index,
                                             FT_SfntName*  aname )
  {
    FT_Error  error = FT_Err_Invalid_Argument;
    
    if ( face && FT_IS_SFNT(face) )
    {
      TT_Face  ttface = (TT_Face)face;
      
      if (index < ttface->num_names)
      {
        TT_NameRec*  name = ttface->name_table.names + index;
        
        aname->platform_id = name->platformID;
        aname->encoding_id = name->encodingID;
        aname->language_id = name->languageID;
        aname->name_id     = name->nameID;
        aname->string      = (FT_Byte*)name->string;
        aname->string_len  = name->stringLength;
        
        error = FT_Err_Ok;
      }
    }
    
    return error;
  }                                             
#endif /* FT_CONFIG_OPTION_SFNT_NAMES */
