/***************************************************************************/
/*                                                                         */
/*  otlayout.h                                                             */
/*                                                                         */
/*    OpenType layout type definitions (specification only).               */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef OTLAYOUT_H
#define OTLAYOUT_H

#include <tttypes.h>

#ifdef __cplusplus
extern "C" {
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_LangSys                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An OTL LangSys record.                                             */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    lang_tag            :: The language tag.                           */
  /*    lang_offset         :: The offset of the langsys data in the
  /*                           resource.                                   */
  /*                                                                       */
  /*    lookup_order        :: Always 0 for OTL 1.0.                       */
  /*    req_feature_index   :: The `required feature' index.               */
  /*    num_feature_indices :: The number of feature indices.              */
  /*    feature_indices     :: An array of feature indices.                */
  /*                                                                       */
  typedef struct  OTL_LangSys_
  {
    TT_ULong    lang_tag;
    TT_ULong    lang_offset;

    TT_UShort   lookup_order;           /* always 0 for TT Open 1.0  */
    TT_UShort   req_feature_index;      /* required FeatureIndex     */
    TT_UShort   num_feature_indices;    /* number of Feature indices */
    TT_UShort*  feature_indices;        /* array of Feature indices  */

  } OTL_LangSys;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_Script                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An OTL Script record.                                              */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    script_tag      :: The script tag.                                 */
  /*    script_offset   :: The offset of the script data in the resource.  */
  /*                                                                       */
  /*    num_langsys     :: The number of langsys records.                  */
  /*    langsys         :: An array of langsys records.                    */
  /*    langsys_default :: A pointer to the default langsys table for this */
  /*                       script.                                         */
  /*                                                                       */
  typedef struct  OTL_Script_
  {
    TT_ULong      script_tag;
    TT_ULong      script_offset;

    TT_UShort     num_langsys;
    OTL_LangSys*  langsys;
    OTL_LangSys*  langsys_default;

  } OTL_Script;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_Script_List                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An OTL Script List record.                                         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_scripts :: The number of scripts records.                      */
  /*    scripts     :: An array of script records.                         */
  /*                                                                       */
  typedef struct  OTL_Script_List_
  {
    TT_UShort    num_scripts;
    OTL_Script*  scripts;

  } OTL_Script_List;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_Feature                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An OTL Feature record.                                             */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    feature_tag    :: The feature tag.                                 */
  /*    feature_offset :: The offset of the feature data in the resource.  */
  /*                                                                       */
  /*    feature_params :: Always 0 for OpenType Layout 1.0.                */
  /*    num_lookups    :: The number of lookup indices.                    */
  /*    lookups        :: An array of lookup indices.                      */
  /*                                                                       */
  typedef struct  OTL_Feature_
  {
    TT_ULong    feature_tag;
    TT_ULong    feature_offset;

    TT_UShort   feature_params;    /* always 0 for OpenType Layout 1.0 */
    TT_UShort   num_lookups;
    TT_UShort*  lookups;

  } OTL_Feature;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_Feature_List                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An OTL Feature List record.                                        */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_features :: The number of features.                            */
  /*    features     :: An array of features.                              */
  /*                                                                       */
  typedef struct  OTL_Feature_List_
  {
    TT_UShort     num_features;
    OTL_Feature*  features;

  } OTL_Feature_List;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_Lookup                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An OTL Lookup record.                                              */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    lookup_offset    :: The offset of the lookup data in the resource. */
  /*    lookup_type      :: The lookup type.                               */
  /*    lookup_flag      :: The lookup bit flags.                          */
  /*                                                                       */
  /*    num_subtables    :: The number of subtables.                       */
  /*    subtable_offsets :: An array of offsets to the subtables.          */
  /*    subtables        :: An array of pointers to the subtable records.  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The format of each lookup sub-table is determined by the parent    */
  /*    OpenType table, and cannot be known here.                          */
  /*                                                                       */
  /*    The `subtable_offsets' array is filled when the lookup table is    */
  /*    loaded the first time.  It is up to OT table handlers to read the  */
  /*    corresponding sub-table records and store them in the `subtables'  */
  /*    array.                                                             */
  /*                                                                       */
  typedef struct  OTL_Lookup_
  {
    TT_ULong   lookup_offset;
    TT_UShort  lookup_type;
    TT_UShort  lookup_flag;

    TT_UShort  num_subtables;
    TT_ULong*  subtable_offsets;
    void**     subtables;

  } OTL_Lookup;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_Lookup_List                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An OTL Lookup List record.                                         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_lookups :: The number of lookups.                              */
  /*    lookups     :: An array of lookups.                                */
  /*                                                                       */
  typedef struct  OTL_Lookup_List_
  {
    TT_UShort    num_lookups;
    OTL_Lookup*  lookups;

  } OTL_Lookup_List;


#define OTL_LOOKUP_FLAG_RIGHT_TO_LEFT       1
#define OTL_LOOKUP_FLAG_IGNORE_BASE_GLYPHS  2
#define OTL_LOOKUP_FLAG_IGNORE_LIGATURES    4
#define OTL_LOOKUP_FLAG_IGNORE_MARKS        8


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_SubTable1                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The first generic OTL sub-table format.                            */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_indices :: The number of indices.                              */
  /*    indices     :: An array of indices.                                */
  /*    data        :: A generic value.                                    */
  /*                                                                       */
  typedef struct  OTL_SubTable1_
  {
    TT_UShort   num_indices;
    TT_UShort*  indices;
    TT_UShort   data;

  } OTL_SubTable1;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_SubTable2_Rec                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A record for the second generic OTL sub-table format.              */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    start :: The first element in the range.                           */
  /*    end   :: The last element in the range.                            */
  /*    data  :: A generic value.                                          */
  /*                                                                       */
  typedef struct  OTL_SubTable2_Rec_
  {
    TT_UShort  start;
    TT_UShort  end;
    TT_UShort  data;

  } OTL_SubTable2_Rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_SubTable2                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The second generic OTL sub-table format.                           */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_ranges :: The number of ranges.                                */
  /*    ranges     :: An array of ranges.                                  */
  /*                                                                       */
  typedef struct  OTL_SubTable2_
  {
    TT_UShort           num_ranges;
    OTL_SubTable2_Rec*  ranges;

  } OTL_SubTable2;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_SubTable                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A generic OTL sub-table which is a union of two possible formats   */
  /*    just defined.                                                      */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    format :: The format of the sub-table.                             */
  /*    set    :: A union of `format1' and `format2', representing         */
  /*              sub-tables.                                              */
  /*                                                                       */
  typedef struct  OTL_SubTable_
  {
    TT_UShort  format;

    union
    {
      OTL_SubTable1  format1;
      OTL_SubTable2  format2;

    } set;

  } OTL_SubTable;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_Coverage                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An OTL Coverage sub-table.                                         */
  /*                                                                       */
  typedef OTL_SubTable       OTL_Coverage;
  typedef OTL_SubTable1      OTL_Coverage1;
  typedef OTL_SubTable2      OTL_Coverage2;
  typedef OTL_SubTable2_Rec  OTL_Coverage2_Rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_Class_Def                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An OTL Class Definition sub-table.                                 */
  /*                                                                       */
  typedef OTL_SubTable       OTL_Class_Def;
  typedef OTL_SubTable1      OTL_Class_Def1;
  typedef OTL_SubTable2      OTL_Class_Def2;
  typedef OTL_SubTable2_Rec  OTL_Class_Def2_Rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    OTL_Device                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An OTL Device sub-table                                            */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    start_size   :: The smallest size to correct.                      */
  /*    end_size     :: The largest size to correct.                       */
  /*    delta_format :: The format of the `delta_values' array.            */
  /*    delta_values :: An array of compressed delta values.               */
  typedef struct  OTL_Device_
  {
    TT_UShort   start_size;
    TT_UShort   end_size;
    TT_UShort   delta_format;
    TT_UShort*  delta_values;

  } OTL_Device;


#ifdef __cplusplus
}
#endif

#endif /* OTLAYOUT_H */


/* END */
