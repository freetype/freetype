#ifndef __FT_TYPE1_CMAP_H__
#define __FT_TYPE1_CMAP_H__

FT_BEGIN_HEADER

 /***************************************************************************/
 /***************************************************************************/
 /*****                                                                 *****/
 /*****           TYPE1 STANDARD (AND EXPERT) ENCODING CMAPS            *****/
 /*****                                                                 *****/
 /***************************************************************************/
 /***************************************************************************/

  typedef struct T1_CMapStrRec_*       T1_CMapStd;

  typedef struct T1_CMapUnicodeRec_*   T1_CMapUnicode;

  
  typedef struct T1_CMapStdRec_
  {
    FT_CMapRec          cmap;

    const FT_UShort*    charcode_to_sid;
    const char* const*  adobe_sid_strings;

    FT_UInt             num_glyphs;
    const char**        glyph_names;
    
    
  } T1_CMapStdRec;


  FT_LOCAL( FT_CMap_Class )   t1_cmap_standard_class;

  FT_LOCAL( FT_CMap_Class )   t1_cmap_expert_class;

 /***************************************************************************/
 /***************************************************************************/
 /*****                                                                 *****/
 /*****                    TYPE1 CUSTOM ENCODING CMAP                   *****/
 /*****                                                                 *****/
 /***************************************************************************/
 /***************************************************************************/

  typedef struct T1_CMapCustomRec_*    T1_CMapCustom;
  
  typedef struct T1_CMapCustomRec_
  {
    FT_CMapRec    cmap;
    FT_UInt       first;
    FT_UInt       count;
    FT_UInt*      indices;
  
  } T1_CMapCustomRec;


  FT_LOCAL( FT_CMap_Class )   t1_cmap_custom_class;

 /***************************************************************************/
 /***************************************************************************/
 /*****                                                                 *****/
 /*****             TYPE1 SYNTHETIC UNICODE ENCODING CMAP               *****/
 /*****                                                                 *****/
 /***************************************************************************/
 /***************************************************************************/

  typedef struct T1_CMapUniPairRec_*   T1_CMapUniPair;
  
  typedef struct T1_CMapUniPairRec_
  {
    FT_UInt32  unicode;
    FT_UInt    gindex;
  
  } T1_CMapUniPairRec;


  typedef struct T1_CMapUnicodeRec_
  {
    FT_CMapRec      cmap;
    FT_UInt         num_pairs;
    T1_CMapUniPair  pairs;

  } T1_CMapUnicodeRec;


  FT_LOCAL( FT_CMap_Class )   t1_cmap_unicode_class;

 /* */
 
FT_END_HEADER

#endif /* __FT_TYPE1_CMAP_H__ */
