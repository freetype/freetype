/***************************************************************************/
/*                                                                         */
/*  psnames.h                                                              */
/*                                                                         */
/*    High-level interface for the "psnames" module (in charge of          */
/*    various functions related to Postscript glyph names conversion)      */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef PSNAMES_H
#define PSNAMES_H

#include <freetype.h>

 /**************************************************************************
  *
  *  <FuncType>
  *     PS_Unicode_Value_Func
  *
  *  <Description>
  *     A function used to return the Unicode index corresponding to a
  *     given glyph name.
  *
  *  <Input>
  *     glyph_name :: the glyph name
  *
  *  <Return>
  *     The Unicode character index. The non-Unicode value 0xFFFF if the
  *     glyph name has no known Unicode meaning..
  *
  *  <Note>
  *     This function is able to map several different glyph names to the
  *     same Unicode value, according to the rules defined in the Adobe
  *     Glyph List table.
  *
  *     This function will not be compiled if the configuration macro
  *     FT_CONFIG_OPTION_ADOBE_GLYPH_LIST is undefined.
  *
  **************************************************************************/

  typedef FT_ULong  (*PS_Unicode_Value_Func)( const char* glyph_name );


 /**************************************************************************
  *
  *  <FuncType>
  *     PS_Unicode_Index_Func
  *
  *  <Description>
  *     A function used to return the glyph index corresponding to
  *     a given unicode value.
  *
  *  <Input>
  *     num_glyphs  :: number of glyphs in face
  *     glyph_names :: array of glyph name pointers
  *     uncode      :: unicode value.
  *
  *  <Return>
  *     The glyph index. 0xFFFF is no glyph correspond to this Unicode   
  *     value..    
  *
  *  <Note>
  *     This function is able to recognize several glyph names per
  *     unicode values, according to the Adobe Glyph List.
  *
  *     This function will not be compiled if the configuration macro
  *     FT_CONFIG_OPTION_ADOBE_GLYPH_LIST is undefined.
  *
  **************************************************************************/

  typedef FT_UInt  (*PS_Unicode_Index_Func)( FT_UInt       num_glyphs,
                                             const char**  glyph_names,
                                             FT_ULong      unicode );

 /**************************************************************************
  *
  *  <FuncType>
  *     PS_Macintosh_Name_Func
  *
  *  <Description>
  *     A function used to return the glyph name corresponding to one   
  *     Apple glyph name index.
  *
  *  <Input>
  *     name_index :: index of the Mac name
  *
  *  <Return>
  *     The glyph name, or 0 if the index is incorrect.             
  *
  *  <Note>
  *     This function will not be compiled if the configuration macro
  *     FT_CONFIG_OPTION_POSTSCRIPT_NAMES is undefined
  *
  **************************************************************************/

  typedef const char*  (*PS_Macintosh_Name_Func)( FT_UInt  name_index );

  
  
  typedef const char*  (*PS_Adobe_Std_Strings_Func)( FT_UInt  string_index );

 /***************************************************************************
  *
  * <Struct>
  *    PS_Unicodes
  *
  * <Description>
  *    a simple table used to map Unicode values to glyph indices. It is
  *    built by the PS_Build_Unicodes table according to the glyphs present
  *    in a font file..
  *
  * <Fields>
  *    num_codes :: number of glyphs in the font that match a given Unicode
  *                 value..
  *
  *    unicodes  :: array of unicode values, sorted in increasing order
  *    gindex    :: array of glyph indices, corresponding to each unicode
  *
  * <Note>
  *    Use the function PS_Lookup_Unicode to retrieve the glyph index
  *    corresponding to a given Unicode character code.
  *
  ***************************************************************************/
  
  typedef struct PS_UniMap_
  {
    FT_UInt  unicode;
    FT_UInt  glyph_index;
    
  } PS_UniMap;
  
  typedef struct PS_Unicodes_
  {
    FT_UInt    num_maps;
    PS_UniMap* maps;
  
  } PS_Unicodes;


  typedef FT_Error  (*PS_Build_Unicodes_Func)( FT_Memory     memory,
                                               FT_UInt       num_glyphs,
                                               const char**  glyph_names,
                                               PS_Unicodes*  unicodes );
  
  typedef FT_UInt   (*PS_Lookup_Unicode_Func)( PS_Unicodes*  unicodes,
                                               FT_UInt       unicode );
  
 /*************************************************************************
  *                                                                       
  * <Struct>                                                              
  *    PSNames_Interface                                                  
  *                                                                       
  * <Description>                                                         
  *    this structure holds pointers to the functions used to load and    
  *    free the basic tables that are required in a `sfnt' font file.     
  *                                                                       
  * <Field>
  *    unicode_value   :: a function used to convert a glyph name into
  *                       a Unicode character code
  *
  *    unicode_index   :: a function used to return the glyph index
  *                       corresponding to a given Unicode character
  *
  *    macintosh_name  :: a function used to return the standard Apple
  *                       glyph Postscript name corresponding to a given
  *                       string index (used by the TrueType "post" table)
  *
  *    adobe_std_strings     :: a function that returns a pointer to a given
  *                             Adobe Standard Strings given a SID
  *
  *    adobe_std_encoding    :: a table of 256 unsigned shorts that maps
  *                             character codes in the Adobe Standard Encoding
  *                             to SIDs
  *
  *    adobe_expert_encoding :: a table of 256 unsigned shorts that maps
  *                             character codes in the Adobe Expert Encoding
  *                             to SIDs.
  *
  * <Note>
  *    The 'unicode_value' and 'unicode_index' will be set to 0 if the
  *    configuration macro FT_CONFIG_OPTION_ADOBE_GLYPH_LIST is undefined
  *
  *    The 'macintosh_name' will be set to 0 if the configuration macro
  *    FT_CONFIG_OPTION_POSTSCRIPT_NAMES is undefined
  *
  *************************************************************************/

  typedef struct  PSNames_Interface_
  {
    PS_Unicode_Value_Func      unicode_value;
    PS_Build_Unicodes_Func     build_unicodes;
    PS_Lookup_Unicode_Func     lookup_unicode;
    PS_Macintosh_Name_Func     macintosh_name;
    
    PS_Adobe_Std_Strings_Func  adobe_std_strings;
    const unsigned short*      adobe_std_encoding;
    const unsigned short*      adobe_expert_encoding;
  
  } PSNames_Interface;

#endif /* PSNAMES_H */



/* END */
