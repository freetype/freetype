#ifndef __SVPSNAME_H__
#define __SVPSNAME_H__

FT_BEGIN_HEADER

#define  FT_SERVICE_ID_POSTSCRIPT_NAMES  "postscript-names"

 /*  Adobe glyph name to unicode value
  */
  typedef FT_UInt32
  (*PS_Unicode_ValueFunc)( const char*  glyph_name );

 /*  Unicode value to Adobe glyph name index. 0xFFFF if not found
  */
  typedef FT_UInt
  (*PS_Unicode_Index_Func)( FT_UInt       num_glyphs,
                            const char**  glyph_names,
                            FT_ULong      unicode );

 /* Macintosh name id to glyph name, NULL if invalid index
  */
  typedef const char*
  (*PS_Macintosh_Name_Func)( FT_UInt  name_index );

 /* Adobe standard string id to glyph name, NULL if invalid index
  */
  typedef const char*
  (*PS_Adobe_Std_Strings_Func)( FT_UInt  string_index );


 /* Simple unicode -> glyph index charmap built from font glyph names
  * table
  */
  typedef struct  PS_UniMap_
  {
    FT_UInt  unicode;
    FT_UInt  glyph_index;

  } PS_UniMap;


  typedef struct  PS_Unicodes_
  {
    FT_UInt     num_maps;
    PS_UniMap*  maps;

  } PS_Unicodes;


  typedef FT_Error
  (*PS_Unicodes_InitFunc)( FT_Memory     memory,
                           FT_UInt       num_glyphs,
                           const char**  glyph_names,
                           PS_Unicodes*  unicodes );

  typedef FT_UInt
  (*PS_Unicodes_CharIndexFunc)( PS_Unicodes*  unicodes,
                                FT_UInt       unicode );

  typedef FT_ULong
  (*PS_Unicodes_CharNextFunc)( PS_Unicodes*  unicodes,
                               FT_ULong      unicode );

  FT_DEFINE_SERVICE( PsNames )
  {
    PS_Unicode_ValueFunc       unicode_value;

    PS_Unicodes_InitFunc       unicodes_init;
    PS_Unicodes_CharIndexFunc  unicodes_char_index;
    PS_Unicodes_CharNextFunc   unicodes_char_next;

    PS_Macintosh_Name_Func     macintosh_name;
    PS_Adobe_Std_Strings_Func  adobe_std_strings;
    const unsigned short*      adobe_std_encoding;
    const unsigned short*      adobe_expert_encoding;
  };

 /* */

FT_END_HEADER

#endif /* __SVPSNAME_H__ */
