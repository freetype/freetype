/***************************************************************************/
/*                                                                         */
/*  gxtypes.h                                                              */
/*                                                                         */
/*    AAT/TrueTypeGX lower level type definitions                          */
/*    (specification).                                                     */
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/

#ifndef __GXTYPES_H__
#define __GXTYPES_H__ 

#include <ft2build.h>
#include FT_TYPES_H
#include FT_INTERNAL_TRUETYPE_TYPES_H
#include FT_INTERNAL_FTL_TYPES_H

FT_BEGIN_HEADER

/***************************************************************************/
/* Forward Declarations                                                    */
/***************************************************************************/
typedef TT_Face GX_Face;
typedef struct  GXL_FontRec_ * GXL_Font;
typedef struct  GX_TableRec_ * GX_Table;

typedef struct GX_MetamorphosisContextualPerGlyphRec_  *GX_MetamorphosisContextualPerGlyph;
typedef struct GX_MetamorphosisInsertionPerGlyphRec_   *GX_MetamorphosisInsertionPerGlyph;
typedef struct GX_XMetamorphosisContextualPerGlyphRec_ *GX_XMetamorphosisContextualPerGlyph;
typedef union  GX_OpticalBoundsDataRec_               *GX_OpticalBoundsData;
typedef struct GX_LigCaretClassEntryRec_              *GX_LigCaretClassEntry;
typedef struct GX_LigCaretSegmentRec_                 *GX_LigCaretSegment;

/***************************************************************************/
/* BinSrchHeader                                                           */
/***************************************************************************/
  typedef struct GX_BinSrchHeaderRec_
  {
    FT_UShort unitSize;
    FT_UShort nUnits;
    FT_UShort searchRange;
    FT_UShort entrySelector;
    FT_UShort rangeShift;
  } GX_BinSrchHeaderRec, *GX_BinSrchHeader;

/***************************************************************************/
/* LookupTable                                                             */
/***************************************************************************/
  typedef union GX_LookupValueExtraDesc_
  {
    GX_OpticalBoundsData  opbd_data;
    GX_LigCaretClassEntry lcar_class_entry;
    GX_LigCaretSegment    lcar_segment;
    FT_UShort            *word;
    FT_Pointer            any;
  } GX_LookupValueExtraDesc, *GX_LookupValueExtra;
  typedef union   GX_LookupValueRawDesc_
  {
    FT_UShort u;
    FT_Short  s;
  } GX_LookupValueRawDesc, *GX_LookupValueRaw;

  typedef struct  GX_LookupValueRec_
  {
    GX_LookupValueRawDesc   raw;
    GX_LookupValueExtraDesc extra;
  } GX_LookupValueRec, *GX_LookupValue;

  typedef enum
  {
    GX_LOOKUPTABLE_SIMPLE_ARRAY   = 0,
    GX_LOOKUPTABLE_SEGMENT_SINGLE = 2,
    GX_LOOKUPTABLE_SEGMENT_ARRAY  = 4,
    GX_LOOKUPTABLE_SINGLE_TABLE   = 6,
    GX_LOOKUPTABLE_TRIMMED_ARRAY  = 8
  } GX_LookupTable_Format;

  typedef struct GX_LookupSegmentRec_
  {
    FT_UShort  lastGlyph;
    FT_UShort  firstGlyph;
    GX_LookupValueRec value;
  } GX_LookupSegmentRec, *GX_LookupSegment;

  typedef struct GX_LookupSingleRec_
  {
    FT_UShort  glyph;
    GX_LookupValueRec value;
  } GX_LookupSingleRec, *GX_LookupSingle;

  typedef struct GX_LookupTable_BinSrchRec_
  {
    GX_BinSrchHeaderRec binSrchHeader;
    FT_Pointer dummy;
  } GX_LookupTable_BinSrchRec, *GX_LookupTable_BinSrch;

  typedef struct GX_LookupTable_SegmentRec_
  {
    GX_BinSrchHeaderRec binSrchHeader;
    GX_LookupSegment segments;
  } GX_LookupTable_SegmentRec, *GX_LookupTable_Segment;

  typedef struct GX_LookupTable_Single_TableRec_
  {
    GX_BinSrchHeaderRec binSrchHeader;
    GX_LookupSingle entries;
  } GX_LookupTable_Single_TableRec, *GX_LookupTable_Single_Table;

  typedef struct GX_LookupTable_Trimmed_ArrayRec_
  {
    FT_UShort firstGlyph;
    FT_UShort glyphCount;
    GX_LookupValue valueArray;
  } GX_LookupTable_Trimmed_ArrayRec, *GX_LookupTable_Trimmed_Array;

  typedef union  GX_LookupFormatSpecificDesc_
  {
    GX_LookupValue               simple_array;
    GX_LookupTable_BinSrch       bin_srch;
    GX_LookupTable_Segment       segment_generic; 
    GX_LookupTable_Segment       segment_single; 
    GX_LookupTable_Segment       segment_array; 
    GX_LookupTable_Single_Table  single_table;
    GX_LookupTable_Trimmed_Array trimmed_array;
    FT_Pointer                   any; 
  } GX_LookupFormatSpecificDesc, *GX_LookupFormatSpecific;

  typedef struct GX_LookupTableRec_
  {
    FT_ULong  position; /* The file position of lookup table. 
			   not in the spec but necessary to access format 4 datum. 
			   Not only for debug. */
    FT_Long num_glyphs;	/* not in the spec but necessary to access format 0 */
    FT_UShort  format;
    GX_LookupFormatSpecificDesc fsHeader;    
  } GX_LookupTableRec, *GX_LookupTable;


/***************************************************************************/
/* StateTable                                                               */
/***************************************************************************/
#define GX_STATE_HEADER_ADVANCE 8
  typedef struct GX_StateHeaderRec_
  {
    FT_ULong  position; /* The file position of state table. (not in the spec) */

    FT_UShort stateSize;
    FT_UShort classTable;
    FT_UShort stateArray;
    FT_UShort entryTable;
  } GX_StateHeaderRec, *GX_StateHeader;

#define GX_DELETED_GLYPH_INDEX 0xFFFF
  typedef enum
  {
    GX_CLASS_END_OF_TEXT   = 0,
    GX_CLASS_OUT_OF_BOUNDS = 1,
    GX_CLASS_DELETED_GLYPH = 2,
    GX_CLASS_END_OF_LINE   = 3,
    GX_CLASS_YOURS_START   = 4
  } GX_Predefined_ClassCode;

  typedef struct GX_ClassSubtableRec_
  {
    FT_UShort firstGlyph;
    FT_UShort nGlyphs;
    FT_Byte *classArray;  
  } GX_ClassSubtableRec, * GX_ClassSubtable;

  typedef union GX_EntrySubtablePerGlyphDesc_
  {
    GX_MetamorphosisContextualPerGlyph  contextual;
    GX_MetamorphosisInsertionPerGlyph   insertion;
    GX_XMetamorphosisContextualPerGlyph xcontextual;
    FT_UShort                           ligActionIndex; /* For morx::ligatureSubtable */
    FT_Pointer                          any;
  } GX_EntrySubtablePerGlyphDesc, * GX_EntrySubtablePerGlyph;

  typedef struct GX_EntrySubtableRec_
  {
    FT_UShort newState;
    FT_UShort flags;
    GX_EntrySubtablePerGlyphDesc glyphOffsets;
  } GX_EntrySubtableRec, *GX_EntrySubtable;


  typedef struct GX_StateTableRec_
  {
    GX_StateHeaderRec   header;
    FT_ULong            nStates;  /* Number of States.  (not in the spec) */
    FT_Byte             nEntries; /* Number of entries. (not in the spec) */
    GX_ClassSubtableRec class_subtable;
    FT_Byte *           state_array;
    GX_EntrySubtable    entry_subtable;
  } GX_StateTableRec, *GX_StateTable;

/***************************************************************************/
/* Extended StateTable                                                     */
/***************************************************************************/
#define GX_XSTATE_HEADER_ADVANCE 16
  typedef struct GX_XStateHeaderRec_
  {
    FT_ULong  position; /* The file position of X state table. (not in the spec) */

    FT_ULong nClasses;
    FT_ULong classTableOffset;
    FT_ULong stateArrayOffset;
    FT_ULong entryTableOffset;
  } GX_XStateHeaderRec, *GX_XStateHeader;

  typedef struct GX_XStateTableRec_
  {
    GX_XStateHeaderRec  header;
    FT_ULong            nStates;  /* Number of States.  (not in the spec) */
    FT_Long             nEntries; /* Number of entries. (not in the spec) */
    GX_LookupTableRec   class_subtable;
    FT_UShort *         state_array;
    GX_EntrySubtable    entry_subtable;
  } GX_XStateTableRec, *GX_XStateTable;


/***************************************************************************/
/* GX_Table                                                                */
/***************************************************************************/
  typedef void
  (* GX_Table_Done_Func) ( GX_Table table, FT_Memory memory );

  typedef struct GX_TableRec_
  {
    /* If the status of a tag is GX_LOAD_LAZY or
       GX_LOAD_SUCCESSFUL, table position and length
       should be available. The value might be useful
       to debug.*/
    GXL_Font      font;
    FT_ULong      position;
    FT_ULong      length;
    GX_Table_Done_Func done_table;
  } GX_TableRec; /* , *GX_Table; */

/***************************************************************************/
/* FEAT                                                                    */
/***************************************************************************/
  typedef enum
  {
    GX_FEAT_MASK_EXCLUSIVE_SETTINGS = 0x8000,
    GX_FEAT_MASK_DYNAMIC_DEFAULT    = 0x4000,
    GX_FEAT_MASK_UNUSED 	    = 0x3F00,
    GX_FEAT_MASK_DEFAULT_SETTING    = 0x00FF
  } GX_FeatureFlagsMask ;

  typedef struct GX_FeatureSettingNameRec_
  {
    FT_UShort setting;
    FT_Short  nameIndex;
  } GX_FeatureSettingNameRec, *GX_FeatureSettingName;

  typedef struct GX_FeatureNameRec_
  {
    FT_UShort  feature;
    FT_UShort  nSettings;
    FT_ULong   settingTable;
    FT_UShort  featureFlags;	            /* use with GX_FeatureFlagsMask */
    FT_Short  nameIndex;
    GX_FeatureSettingName settingName;
  } GX_FeatureNameRec, *GX_FeatureName;

  typedef struct GX_FeatRec_
  {
    GX_TableRec  root;
    FT_Fixed          version;
    FT_UShort         featureNameCount;
    FT_UShort         reserved1;
    FT_ULong          reserved2;	     
    GX_FeatureName    names;		     /* names[featureNameCount] */
  } GX_FeatRec, *GX_Feat;

/***************************************************************************/
/* TRAK                                                                    */
/***************************************************************************/
  typedef struct GX_TrackTableEntryRec_
  {
    FT_Fixed  track;			     /* MUST */
    FT_UShort nameIndex;		     /* MUST */
    FT_UShort offset;			     /* USED DURING LOADING */
    FT_FWord *tracking_value;		     /* tracking_value[nSizes], MUST
					      * This field name, `tracking_value' 
					      * is no  appeared on the specification. 
					      */
  } GX_TrackTableEntryRec, *GX_TrackTableEntry;

  typedef struct GX_TrackDataRec_
  {
    FT_UShort            nTracks;	     /* MUST */
    FT_UShort            nSizes;	     /* MUST */
    FT_ULong             sizeTableOffset;    /* USED DURING LOADING */
    GX_TrackTableEntry   trackTable;	     /* MUST */
    FT_Fixed            *sizeTable;	     /* MUST, ???fixed32 */
  
  } GX_TrackDataRec, *GX_TrackData;

  typedef struct GX_TrakRec_
  {
    GX_TableRec      root;
    FT_Fixed         version;		     /* ???fixed */
    FT_UShort        format;
    FT_UShort        horizOffset;	     /* USED DURING LOADING */
    FT_UShort        vertOffset;	     /* USED DURING LOADING */
    FT_UShort        reserved;		     /* ??? */
    GX_TrackDataRec  horizData;		     /* MUST */
    GX_TrackDataRec  vertData;		     /* MUST */
  } GX_TrakRec, *GX_Trak;

/***************************************************************************/
/* PROP                                                                    */
/***************************************************************************/
  typedef enum
  {
    GX_PROP_MASK_FLOATER  	       = 0x8000,
    GX_PROP_MASK_HANG_OFF_LEFT_TOP     = 0x4000,
    GX_PROP_MASK_HANG_OFF_RIGHT_BOTTOM = 0x2000,
    GX_PROP_MASK_USE_COMPLEMENTARY_BRACKET    = 0x1000,
    GX_PROP_MASK_COMPLEMENTARY_BRACKET_OFFSET = 0x0F00,
    GX_PROP_MASK_ATTACHING_TO_RIGHT    = 0x0080,
    GX_PROP_MASK_RESERVED 	       = 0x0060,
    GX_PROP_MASK_DIRECTIONALITY_CLASS  = 0x001F
  } GX_PropertyMask ;

  typedef struct GX_PropRec_
  {
    GX_TableRec  root;
    FT_Fixed          version;
    FT_UShort         format;
    FT_UShort         default_properties;
    GX_LookupTableRec lookup_data;
  } GX_PropRec, *GX_Prop;

/***************************************************************************/
/* OPBD                                                                    */
/***************************************************************************/
#define GX_OPBD_NO_OPTICAL_EDGE -1
  typedef enum
  {
    GX_OPBD_DISTANCE = 0,
    GX_OPBD_CONTROL_POINTS = 1
  } GX_OpticalBoundsFormat ;
 
  typedef struct GX_OpbdRec_
  {
    GX_TableRec root;
    FT_Fixed         version;
    FT_UShort        format;
    GX_LookupTableRec lookup_data;
  } GX_OpbdRec, *GX_Opbd;

  typedef union GX_OpticalBoundsDataRec_
  {
    struct {
      FT_FWord  left_side;
      FT_FWord  top_side;
      FT_FWord  right_side;
      FT_FWord  bottom_side;
    } distance;
    struct {
      FT_Short  left_side;
      FT_Short  top_side;
      FT_Short  right_side;
      FT_Short  bottom_side;
    } control_points;
  } GX_OpticalBoundsDataRec;/*, *GX_OpticalBoundsData; */

/***************************************************************************/
/* LCAR                                                                    */
/***************************************************************************/
  typedef enum
  {
    GX_LCAR_DISTANCE = 0,
    GX_LCAR_CONTROL_POINTS = 1
  } GX_LigCaretFormat ;

  typedef struct GX_LcarRec_
  {
    GX_TableRec  root;
    FT_Fixed          version;
    FT_UShort         format;
    GX_LookupTableRec lookup;
  } GX_LcarRec, *GX_Lcar;

  /* Normally each value->extra in lookup points to GX_LigCaretClassEntry.
     If lcar->format is GX_LOOKUPTABLE_SEGMENT_ARRAY, each value->extra
     points to GX_LigCaretSegment data. */
  typedef struct GX_LigCaretClassEntryRec_
  {
    FT_UShort count;
    FT_Short *partials;
  } GX_LigCaretClassEntryRec; /* , *GX_LigCaretClassEntry; */
    
  typedef struct GX_LigCaretSegmentRec_
  {
    FT_UShort offset;
    GX_LigCaretClassEntry class_entry;
  } GX_LigCaretSegmentRec; /* , *GX_LigCaretSegment; */

/***************************************************************************/
/* BSLN                                                                    */
/***************************************************************************/
#define GX_BSLN_VALUE_COUNT 32
#define GX_BSLN_VALUE_EMPTY 0xFFFF

  typedef enum
  {
    GX_BSLN_VALUE_ROMAN_BASELINE 		= 0,
    GX_BSLN_VALUE_IDEOGRAPHIC_CENTERED_BASELINE = 1,
    GX_BSLN_VALUE_IDEOGRAPHIC_LOW_BASELINE 	= 2,
    GX_BSLN_VALUE_HANGING_BASELINE 	   	= 3,
    GX_BSLN_VALUE_MATH_BASELINE 	   	= 4
  } GX_BaselinePredefinedValue;
      
  typedef enum
  {
    GX_BSLN_FMT_DISTANCE_NO_MAPPING 	   = 0,
    GX_BSLN_FMT_DISTANCE_WITH_MAPPING      = 1,
    GX_BSLN_FMT_CONTROL_POINT_NO_MAPPING   = 2,
    GX_BSLN_FMT_CONTROL_POINT_WITH_MAPPING = 3
  } GX_BaselineFormat;

  typedef struct GX_BaselineFormat0PartRec_
  {
    FT_UShort deltas[GX_BSLN_VALUE_COUNT];
  } GX_BaselineFormat0PartRec, *GX_BaselineFormat0Part;

  typedef struct GX_BaselineFormat1PartRec_
  {
    FT_UShort deltas[GX_BSLN_VALUE_COUNT];
    GX_LookupTableRec mappingData;
  } GX_BaselineFormat1PartRec, *GX_BaselineFormat1Part;

  typedef struct GX_BaselineFormat2PartRec_
  {
    FT_UShort stdGlyph;
    FT_UShort ctlPoints[GX_BSLN_VALUE_COUNT];
  } GX_BaselineFormat2PartRec, *GX_BaselineFormat2Part;

  typedef struct GX_BaselineFormat3PartRec_
  {
    FT_UShort stdGlyph;
    FT_UShort ctlPoints[GX_BSLN_VALUE_COUNT];
    GX_LookupTableRec mappingData;
  } GX_BaselineFormat3PartRec, *GX_BaselineFormat3Part;

  typedef union GX_BaselinePartsDesc_
  {
    GX_BaselineFormat0Part fmt0;
    GX_BaselineFormat1Part fmt1;
    GX_BaselineFormat2Part fmt2;
    GX_BaselineFormat3Part fmt3;
    FT_Pointer             any;
  } GX_BaselinePartsDesc, *GX_BaselineParts;

  typedef struct GX_BslnRec_
  {
    GX_TableRec  root;
    FT_Fixed version;
    FT_UShort format;
    FT_UShort defaultBaseline;
    GX_BaselinePartsDesc parts;    
  } GX_BslnRec, *GX_Bsln;

/***************************************************************************/
/* MORT                                                                    */
/***************************************************************************/

  typedef enum
  {
    GX_MORT_COVERAGE_HORIZONTAL_OR_VERTICAL_TEXT     = 0x8000,
    GX_MORT_COVERAGE_ORDER_OF_PROCESSING_GLYPH_ARRAY = 0x4000,
    GX_MORT_COVERAGE_ORIENTATION_INDEPENDENT 	     = 0x2000,
    GX_MORT_COVERAGE_RESERVED 			     = 0x1FF8,
    GX_MORT_COVERAGE_SUBTABLE_TYPE  		     = 0x0007
  } GX_MetamorphosisCoverageMask;

  typedef enum
  {
    GX_MORT_REARRANGEMENT_SUBTABLE = 0,
    GX_MORT_CONTEXTUAL_SUBTABLE    = 1,
    GX_MORT_LIGATURE_SUBTABLE 	   = 2,
    GX_MORT_RESERVED_SUBTABLE 	   = 3,
    GX_MORT_NONCONTEXTUAL_SUBTABLE = 4,
    GX_MORT_INSERTION_SUBTABLE	   = 5
  } GX_MetamorphosisSubtableType;

  typedef enum
  {
    GX_MORT_LIGATURE_FLAGS_SET_COMPONENT = 0x8000,
    GX_MORT_LIGATURE_FLAGS_DONT_ADVANCE  = 0x4000,
    GX_MORT_LIGATURE_FLAGS_OFFSET  	 = 0x3FFF
  } GX_MetamorphosisLigatureFlagsMask;

#if 0
  typedef enum
  {
    GX_MORT_LIGATURE_ACTION_LAST   = 0x80000000,
    GX_MORT_LIGATURE_ACTION_STORE  = 0x40000000,
    GX_MORT_LIGATURE_ACTION_OFFSET = 0x3FFFFFFF
  } GX_MetamorphosisLigatureActionMask;
#else 
  typedef FT_ULong GX_MetamorphosisLigatureActionMask;
#define GX_MORT_LIGATURE_ACTION_LAST   0x80000000
#define GX_MORT_LIGATURE_ACTION_STORE  0x40000000
#define GX_MORT_LIGATURE_ACTION_OFFSET 0x3FFFFFFF 
#endif /* 0 */

  typedef enum
  {
    GX_MORT_CONTEXTUAL_FLAGS_SET_MARK     = 0x8000,
    GX_MORT_CONTEXTUAL_FLAGS_DONT_ADVANCE = 0x4000,
    GX_MORT_CONTEXTUAL_FLAGS_RESERVED 	  = 0x3FFF
  } GX_MetamorphosisContextualFlagsMask;

  typedef enum
  {
    GX_MORT_REARRANGEMENT_FLAGS_MARK_FIRST   = 0x8000,
    GX_MORT_REARRANGEMENT_FLAGS_DONT_ADVANCE = 0x4000,
    GX_MORT_REARRANGEMENT_FLAGS_MARK_LAST    = 0x2000,
    GX_MORT_REARRANGEMENT_FLAGS_RESERVED     = 0x1FF0,
    GX_MORT_REARRANGEMENT_FLAGS_VERB         = 0x000F
  } GX_MetamorphosisRearrangementFlagsMask;

  typedef enum
  {
    GX_MORT_REARRANGEMENT_VERB_NO_CHANGE   = 0,
    GX_MORT_REARRANGEMENT_VERB_Ax2xA       = 1,
    GX_MORT_REARRANGEMENT_VERB_xD2Dx 	   = 2,
    GX_MORT_REARRANGEMENT_VERB_AxD2DxA 	   = 3,
    GX_MORT_REARRANGEMENT_VERB_ABx2xAB 	   = 4,
    GX_MORT_REARRANGEMENT_VERB_ABx2xBA 	   = 5,
    GX_MORT_REARRANGEMENT_VERB_xCD2CDx     = 6,
    GX_MORT_REARRANGEMENT_VERB_xCD2DCx     = 7,
    GX_MORT_REARRANGEMENT_VERB_AxCD2CDxA   = 8,
    GX_MORT_REARRANGEMENT_VERB_AxCD2DCxA   = 9,
    GX_MORT_REARRANGEMENT_VERB_ABxD2DxAB   = 10,
    GX_MORT_REARRANGEMENT_VERB_ABxD2DxBA   = 11,
    GX_MORT_REARRANGEMENT_VERB_ABxCD2CDxAB = 12,
    GX_MORT_REARRANGEMENT_VERB_ABxCD2CDxBA = 13,
    GX_MORT_REARRANGEMENT_VERB_ABxCD2DCxAB = 14,
    GX_MORT_REARRANGEMENT_VERB_ABxCD2DCxBA = 15
  } GX_MetamorphosisRearrangementVerb;

  typedef enum
  {
    GX_MORT_INSERTION_FLAGS_SET_MARK 	            = 0x8000,
    GX_MORT_INSERTION_FLAGS_DONT_ADVANCE   	    = 0x4000,
    GX_MORT_INSERTION_FLAGS_CURRENT_IS_KASHIDA_LIKE = 0x2000,
    GX_MORT_INSERTION_FLAGS_MARKED_IS_KASHIDA_LIKE  = 0x1000,
    GX_MORT_INSERTION_FLAGS_CURRENT_INSERT_BEFORE   = 0x0800,
    GX_MORT_INSERTION_FLAGS_MARKED_INSERT_BEFORE    = 0x0400,
    GX_MORT_INSERTION_FLAGS_CURRENT_INSERT_COUNT    = 0x03E0,
    GX_MORT_INSERTION_FLAGS_MARKED_INSERT_COUNT     = 0x001F
  } GX_MetamorphosisInsertionFlagsMask;

/* 
 * Rearrangement 
 */
  typedef struct GX_MetamorphosisRearrangementBodyRec_
  {
    GX_StateTableRec state_table;
  } GX_MetamorphosisRearrangementBodyRec, *GX_MetamorphosisRearrangementBody;

/* 
 * Contextual 
 */
  typedef struct GX_MetamorphosisContextualSubstitutionTableRec_
  {
    FT_UShort   offset;	        /* known as substitutionTable in the spec. */
    FT_UShort   nGlyphIndexes;	/* Not in the spec */
    FT_UShort * glyph_indexes;
  } GX_MetamorphosisContextualSubstitutionTableRec, *GX_MetamorphosisContextualSubstitutionTable;

  typedef struct GX_MetamorphosisContextualBodyRec_
  {
    GX_StateTableRec state_table;
    GX_MetamorphosisContextualSubstitutionTableRec substitutionTable;
  } GX_MetamorphosisContextualBodyRec, *GX_MetamorphosisContextualBody;

  typedef struct GX_MetamorphosisContextualPerGlyphRec_
  {
    /* The spec says that the type of next two variable should be FT_UShort,
       However the input gid [197 202] for /Library/Fonts/KufiStandarGK.ttf on MacOSX
       are not substituted well; the substitution for the marked glyph si failed
       because the `markOffset' is too large(65522). So I will change the type to
       FT_Short. You can walk through all codes related to this type changing by
    
       $ grep "Was:FT_UShort" *.c *.h 
    
       As far as my reading, ICU also does the same. */
    FT_Short markOffset;	/* Was:FT_UShort */
    FT_Short currentOffset;	/* Was:FT_UShort */
  } GX_MetamorphosisContextualPerGlyphRec;/*, *GX_MetamorphosisContextualPerGlyph; */

/*
 * Ligature
 */
  typedef struct GX_MetamorphosisLigatureActionTableRec_
  {
    FT_UShort  offset;		/* known as ligActionTable in the spec. */
    FT_UShort  nActions;
    FT_ULong  *body;
  } GX_MetamorphosisLigatureActionTableRec, *GX_MetamorphosisLigatureActionTable;

  typedef struct GX_MetamorphosisComponentTableRec_
  {
    FT_UShort  offset;		/* known as componentTable in the spec. */
    FT_UShort  nComponent;
    FT_UShort  *body;
  } GX_MetamorphosisComponentTableRec, *GX_MetamorphosisComponentTable;

  typedef struct GX_MetamorphosisLigatureTableRec_
  {
    FT_UShort  offset;		/* known as ligatureTable in the spec. */
    FT_UShort  nLigature;
    FT_UShort  *body;
  } GX_MetamorphosisLigatureTableRec, *GX_MetamorphosisLigatureTable;

  typedef struct GX_MetamorphosisLigatureBodyRec_
  {
    GX_StateTableRec state_table;
    GX_MetamorphosisLigatureActionTableRec ligActionTable;
    GX_MetamorphosisComponentTableRec componentTable;
    GX_MetamorphosisLigatureTableRec ligatureTable;
  } GX_MetamorphosisLigatureBodyRec, *GX_MetamorphosisLigatureBody;

/*
 * Noncontextual
 */
  typedef struct GX_MetamorphosisNoncontextualBodyRec_
  {
    GX_LookupTableRec lookup_table;
  } GX_MetamorphosisNoncontextualBodyRec, *GX_MetamorphosisNoncontextualBody;

/*
 * Insertion
 */
  typedef struct GX_MetamorphosisInsertionBodyRec_
  {
    GX_StateTableRec state_table;
  } GX_MetamorphosisInsertionBodyRec, *GX_MetamorphosisInsertionBody;

  typedef struct GX_MetamorphosisInsertionListRec_
  {
    FT_UShort offset;		/* known as currentInsertList, 
				   or makedInsertList in the sepc. */
    FT_UShort * glyphcodes;	/* Not in spec explicitly */
  } GX_MetamorphosisInsertionListRec, *GX_MetamorphosisInsertionList;

  typedef struct GX_MetamorphosisInsertionPerGlyphRec_
  {
    GX_MetamorphosisInsertionListRec currentInsertList;
    GX_MetamorphosisInsertionListRec markedInsertList;
  } GX_MetamorphosisInsertionPerGlyphRec;/*, *GX_MetamorphosisInsertionPerGlyph; */

/*
 * Generic
 */
  typedef struct GX_MetamorphosisSubtableHeaderRec_
  {
    FT_ULong  position;      	/* not in the spec, just for DEBUG */
    FT_UShort length;
    FT_UShort coverage;
    FT_ULong  subFeatureFlags;
  } GX_MetamorphosisSubtableHeaderRec, *GX_MetamorphosisSubtableHeader;

  typedef union  GX_MetamorphosisSubtableBodyDesc_
  {
    GX_MetamorphosisRearrangementBody  rearrangement;
    GX_MetamorphosisContextualBody     contextual;
    GX_MetamorphosisLigatureBody       ligature;
    GX_MetamorphosisNoncontextualBody  noncontextual;
    GX_MetamorphosisInsertionBody      insertion;
    FT_Pointer                         any;
  } GX_MetamorphosisSubtableBodyDesc, *GX_MetamorphosisSubtableBody;
  typedef struct GX_MetamorphosisSubtableRec_
  {
    GX_MetamorphosisSubtableHeaderRec header;
    GX_MetamorphosisSubtableBodyDesc  body;
  } GX_MetamorphosisSubtableRec, *GX_MetamorphosisSubtable;

  typedef struct GX_MetamorphosisFeatureTableRec_
  {
    FT_UShort featureType;
    FT_UShort featureSetting;
    FT_ULong  enableFlags;
    FT_ULong  disableFlags;
  } GX_MetamorphosisFeatureTableRec, *GX_MetamorphosisFeatureTable;

  typedef struct GX_MetamorphosisChainHeaderRec_
  {
    FT_ULong  defaultFlags;
    FT_ULong  chainLength;
    FT_UShort nFeatureEntries;
    FT_UShort nSubtables;
  } GX_MetamorphosisChainHeaderRec, *GX_MetamorphosisChainHeader;
  
  typedef struct GX_MetamorphosisChainRec_
  {
    GX_MetamorphosisChainHeaderRec header;
    GX_MetamorphosisFeatureTable   feat_Subtbl;
    GX_MetamorphosisSubtable       chain_Subtbl;
  } GX_MetamorphosisChainRec, *GX_MetamorphosisChain;

  typedef struct GX_MortRec_
  {
    GX_TableRec root;
    FT_Fixed version;
    FT_ULong nChains;
    GX_MetamorphosisChain chain;
  } GX_MortRec, *GX_Mort;


/***************************************************************************/
/* MORX                                                                    */
/***************************************************************************/

#define GX_MORX_REARRANGEMENT_SUBTABLE GX_MORT_REARRANGEMENT_SUBTABLE
#define GX_MORX_CONTEXTUAL_SUBTABLE    GX_MORT_CONTEXTUAL_SUBTABLE
#define GX_MORX_LIGATURE_SUBTABLE      GX_MORT_LIGATURE_SUBTABLE
#define GX_MORX_RESERVED_SUBTABLE      GX_MORT_RESERVED_SUBTABLE
#define GX_MORX_NONCONTEXTUAL_SUBTABLE GX_MORT_NONCONTEXTUAL_SUBTABLE
#define GX_MORX_INSERTION_SUBTABLE     GX_MORT_INSERTION_SUBTABLE	  

  typedef GX_MetamorphosisFeatureTable GX_XMetamorphosisFeatureTable;
  typedef GX_MetamorphosisFeatureTableRec GX_XMetamorphosisFeatureTableRec;
  typedef GX_MetamorphosisSubtableType GX_XMetamorphosisSubtableType;

#if 0
  typedef enum
  {
    GX_MORX_COVERAGE_HORIZONTAL_OR_VERTICAL_TEXT     = 0x80000000,
    GX_MORX_COVERAGE_ORDER_OF_PROCESSING_GLYPH_ARRAY = 0x40000000,
    GX_MORX_COVERAGE_ORIENTATION_INDEPENDENT 	     = 0x20000000,
    GX_MORX_COVERAGE_RESERVED 			     = 0x1FFFFF00,
    GX_MORX_COVERAGE_SUBTABLE_TYPE  		     = 0x000000FF
  } GX_XMetamorphosisCoverageMask;
#else
  typedef FT_ULong GX_XMetamorphosisCoverageMask;
#define GX_MORX_COVERAGE_HORIZONTAL_OR_VERTICAL_TEXT     0x80000000
#define GX_MORX_COVERAGE_ORDER_OF_PROCESSING_GLYPH_ARRAY 0x40000000
#define GX_MORX_COVERAGE_ORIENTATION_INDEPENDENT         0x20000000
#define GX_MORX_COVERAGE_RESERVED                        0x1FFFFF00
#define GX_MORX_COVERAGE_SUBTABLE_TYPE                   0x000000FF
#endif	/* 0 */

/* 
 * Rearrangement 
 */
  typedef GX_MetamorphosisRearrangementVerb GX_XMetamorphosisRearrangementVerb;
  typedef enum
  {
    GX_MORX_REARRANGEMENT_FLAGS_MARK_FIRST   = GX_MORT_REARRANGEMENT_FLAGS_MARK_FIRST,
    GX_MORX_REARRANGEMENT_FLAGS_DONT_ADVANCE = GX_MORT_REARRANGEMENT_FLAGS_DONT_ADVANCE,
    GX_MORX_REARRANGEMENT_FLAGS_MARK_LAST    = GX_MORT_REARRANGEMENT_FLAGS_MARK_LAST,
    GX_MORX_REARRANGEMENT_FLAGS_RESERVED     = GX_MORT_REARRANGEMENT_FLAGS_RESERVED,
    GX_MORX_REARRANGEMENT_FLAGS_VERB 	     = GX_MORT_REARRANGEMENT_FLAGS_VERB
  } GX_XMetamorphosisRearrangementFlagsMask;

  typedef struct GX_XMetamorphosisRearrangementBodyRec_
  {
    GX_XStateTableRec state_table;
  } GX_XMetamorphosisRearrangementBodyRec, *GX_XMetamorphosisRearrangementBody;

/*
 * Contextual
 */
  typedef enum
  {
    GX_MORX_CONTEXTUAL_FLAGS_SET_MARK     = GX_MORT_CONTEXTUAL_FLAGS_SET_MARK,
    GX_MORX_CONTEXTUAL_FLAGS_DONT_ADVANCE = GX_MORT_CONTEXTUAL_FLAGS_DONT_ADVANCE,
    GX_MORX_CONTEXTUAL_FLAGS_RESERVED 	  = GX_MORT_CONTEXTUAL_FLAGS_RESERVED
  } GX_XMetamorphosisContextualFlagsMask;


  typedef struct GX_XMetamorphosisContextualSubstitutionTableRec_
  {
    FT_ULong         offset;
    FT_UShort        nTables;
    GX_LookupTable * lookupTables;
  } GX_XMetamorphosisContextualSubstitutionTableRec, *GX_XMetamorphosisContextualSubstitutionTable;

  typedef struct GX_XMetamorphosisContextualBodyRec_
  {
    GX_XStateTableRec state_table;
    GX_XMetamorphosisContextualSubstitutionTableRec substitutionTable;
  } GX_XMetamorphosisContextualBodyRec, *GX_XMetamorphosisContextualBody;

  typedef struct GX_XMetamorphosisContextualPerGlyphRec_
  {
    FT_UShort markIndex;
    FT_UShort currentIndex;
  } GX_XMetamorphosisContextualPerGlyphRec;/*, *GX_XMetamorphosisContextualPerGlyph; */

/*
 * Ligature
 */

#if 0
  typedef enum
  {
    GX_MORX_LIGATURE_ACTION_LAST   = GX_MORT_LIGATURE_ACTION_LAST,
    GX_MORX_LIGATURE_ACTION_STORE  = GX_MORT_LIGATURE_ACTION_STORE,
    GX_MORX_LIGATURE_ACTION_OFFSET = GX_MORT_LIGATURE_ACTION_OFFSET
  } GX_XMetamorphosisLigatureActionMask;
#else 
  typedef GX_MetamorphosisLigatureActionMask GX_XMetamorphosisLigatureActionMask;
#define GX_MORX_LIGATURE_ACTION_LAST   GX_MORT_LIGATURE_ACTION_LAST
#define GX_MORX_LIGATURE_ACTION_STORE  GX_MORT_LIGATURE_ACTION_STORE
#define GX_MORX_LIGATURE_ACTION_OFFSET GX_MORT_LIGATURE_ACTION_OFFSET
#endif /* 0 */

  typedef enum
  {
    GX_MORX_LIGATURE_FLAGS_SET_COMPONENT  = 0x8000,
    GX_MORX_LIGATURE_FLAGS_DONT_ADVANCE   = 0x4000,
    GX_MORX_LIGATURE_FLAGS_PERFORM_ACTION = 0x2000,
    GX_MORX_LIGATURE_FLAGS_RESERVED       = 0x3FFF
  } GX_XMetamorphosisLigatureFlagsMask;

  typedef struct GX_XMetamorphosisLigatureActionTableRec_
  {
    FT_ULong   offset;		/* known as ligActionTable in the spec. */
    FT_UShort  nActions;
    FT_ULong  *body;
  } GX_XMetamorphosisLigatureActionTableRec, *GX_XMetamorphosisLigatureActionTable;

  typedef struct GX_XMetamorphosisComponentTableRec_
  {
    FT_ULong   offset;		/* known as componentTable in the spec. */
    FT_UShort  nComponent;
    FT_UShort *body;
  } GX_XMetamorphosisComponentTableRec, *GX_XMetamorphosisComponentTable;

  typedef struct GX_XMetamorphosisLigatureTableRec_
  {
    FT_ULong   offset;		/* known as ligatureTable in the spec. */
    FT_UShort  nLigature;
    FT_UShort *body;
  } GX_XMetamorphosisLigatureTableRec, *GX_XMetamorphosisLigatureTable;

  typedef struct GX_XMetamorphosisLigatureBodyRec_
  {
    GX_XStateTableRec state_table;
    GX_XMetamorphosisLigatureActionTableRec ligActionTable;
    GX_XMetamorphosisComponentTableRec componentTable;
    GX_XMetamorphosisLigatureTableRec ligatureTable;
  } GX_XMetamorphosisLigatureBodyRec, *GX_XMetamorphosisLigatureBody;

/*
 * Noncontextual
 */
  typedef GX_MetamorphosisNoncontextualBody GX_XMetamorphosisNoncontextualBody;

/*
 * Insertion
 */
#define GX_MORX_NO_INSERTION ((FT_UShort)-1)
#define GX_MORX_NO_SUBSTITUTION ((FT_UShort)-1)

  typedef GX_MetamorphosisInsertionPerGlyphRec GX_XMetamorphosisInsertionPerGlyphRec;
  typedef GX_MetamorphosisInsertionPerGlyph    GX_XMetamorphosisInsertionPerGlyph;
  typedef GX_MetamorphosisInsertionListRec     GX_XMetamorphosisInsertionListRec;
  typedef GX_MetamorphosisInsertionList        GX_XMetamorphosisInsertionList;

  typedef struct GX_XMetamorphosisInsertionBodyRec_
  {
    GX_XStateTableRec state_table;
    FT_ULong          insertion_glyph_table;
  } GX_XMetamorphosisInsertionBodyRec, * GX_XMetamorphosisInsertionBody;

  typedef enum
  {
    GX_MORX_INSERTION_FLAGS_SET_MARK 	            = GX_MORT_INSERTION_FLAGS_SET_MARK,
    GX_MORX_INSERTION_FLAGS_DONT_ADVANCE   	    = GX_MORT_INSERTION_FLAGS_DONT_ADVANCE,
    GX_MORX_INSERTION_FLAGS_CURRENT_IS_KASHIDA_LIKE = GX_MORT_INSERTION_FLAGS_CURRENT_IS_KASHIDA_LIKE,
    GX_MORX_INSERTION_FLAGS_MARKED_IS_KASHIDA_LIKE  = GX_MORT_INSERTION_FLAGS_MARKED_IS_KASHIDA_LIKE,
    GX_MORX_INSERTION_FLAGS_CURRENT_INSERT_BEFORE   = GX_MORT_INSERTION_FLAGS_CURRENT_INSERT_BEFORE,
    GX_MORX_INSERTION_FLAGS_MARKED_INSERT_BEFORE    = GX_MORT_INSERTION_FLAGS_MARKED_INSERT_BEFORE,
    GX_MORX_INSERTION_FLAGS_CURRENT_INSERT_COUNT    = GX_MORT_INSERTION_FLAGS_CURRENT_INSERT_COUNT,
    GX_MORX_INSERTION_FLAGS_MARKED_INSERT_COUNT     = GX_MORT_INSERTION_FLAGS_MARKED_INSERT_COUNT
  } GX_XMetamorphosisInsertionFlagsMask;

/*
 * Generic
 */

  typedef struct GX_XMetamorphosisSubtableHeaderRec_
  {
    FT_ULong position;		/* not in the spec, just for DEBUG */
    FT_ULong length;
    FT_ULong coverage;
    FT_ULong subFeatureFlags;
  } GX_XMetamorphosisSubtableHeaderRec, *GX_XMetamorphosisSubtableHeader;

  typedef union  GX_XMetamorphosisSubtableBodyDesc_
  {
    GX_XMetamorphosisRearrangementBody  rearrangement;
    GX_XMetamorphosisContextualBody     contextual;
    GX_XMetamorphosisLigatureBody       ligature;
    GX_XMetamorphosisNoncontextualBody  noncontextual;
    GX_XMetamorphosisInsertionBody      insertion; 
    FT_Pointer                          any;
  } GX_XMetamorphosisSubtableBodyDesc, *GX_XMetamorphosisSubtableBody;

  typedef struct GX_XMetamorphosisSubtableRec_
  {
    GX_XMetamorphosisSubtableHeaderRec header;
    GX_XMetamorphosisSubtableBodyDesc  body;
  } GX_XMetamorphosisSubtableRec, *GX_XMetamorphosisSubtable;

  typedef struct GX_XMetamorphosisChainHeaderRec_
  {
    FT_ULong  defaultFlags;
    FT_ULong  chainLength;
    FT_ULong  nFeatureEntries;
    FT_ULong  nSubtables;
  } GX_XMetamorphosisChainHeaderRec, *GX_XMetamorphosisChainHeader;

  typedef struct GX_XMetamorphosisChainRec_
  {
    GX_XMetamorphosisChainHeaderRec header;
    GX_XMetamorphosisFeatureTable   feat_Subtbl;
    GX_XMetamorphosisSubtable       chain_Subtbl;
  } GX_XMetamorphosisChainRec, *GX_XMetamorphosisChain;

  typedef struct GX_MorxRec_
  {
    GX_TableRec root;
    FT_Fixed version;
    FT_ULong nChains;
    GX_XMetamorphosisChain chain;
  } GX_MorxRec, *GX_Morx;


/***************************************************************************/
/* FMTX                                                                    */
/***************************************************************************/

  typedef struct GX_FmtxRec_
  {
    GX_TableRec root;
    FT_Fixed version;
    FT_ULong glyphIndex;
    FT_Byte horizontalBefore;
    FT_Byte horizontalAfter;
    FT_Byte horizontalCaretHead;
    FT_Byte horizontalCaretBase;
    FT_Byte verticalBefore;
    FT_Byte verticalAfter;
    FT_Byte verticalCaretHead;
    FT_Byte verticalCaretBase;
  } GX_FmtxRec, *GX_Fmtx;


/***************************************************************************/
/* FDSC                                                                    */
/***************************************************************************/


  typedef struct GX_FontDescriptorRec_
  {
    FT_ULong tag;
    FT_Fixed value;
  } GX_FontDescriptorRec, *GX_FontDescriptor;

  typedef struct GX_FdscRec_
  {
    GX_TableRec root;
    FT_Fixed version;
    FT_ULong descriptorCount;
    GX_FontDescriptor descriptor;
  } GX_FdscRec, *GX_Fdsc;

/***************************************************************************/
/* JUST                                                                    */
/***************************************************************************/

  typedef enum
  {
    GX_JUST_STATE_FLAGS_SET_MARK      = 0x8000,
    GX_JUST_STATE_FLAGS_DONT_ADVANCE  = 0x4000,
    GX_JUST_STATE_FLAGS_MARK_CLASS    = 0x3F80,
    GX_JUST_STATE_FLAGS_CURRENT_CLASS = 0x007F
  } GX_JustificationStateFlagsMask;

  typedef struct GX_JustificationClassStateTableRec_
  {
    GX_MetamorphosisSubtableHeaderRec morphHeader;
    GX_StateHeaderRec stHeader;
  } GX_JustificationClassStateTableRec, *GX_JustificationClassStateTable;

  typedef struct GX_JustificationHeaderRec_
  {
    FT_UShort justClassTableOffset;
    FT_UShort wdcTableOffset;
    FT_UShort pcTableOffset;
    GX_LookupTableRec lookup_table;
  } GX_JustificationHeaderRec, *GX_JustificationHeader;
  typedef struct GX_JustRec_
  {
    GX_TableRec root;
    FT_Fixed  version;
    FT_UShort format;
    FT_UShort horizOffset;
    FT_UShort vertOffset;
    
  } GX_JustRec, *GX_Just;

/***************************************************************************/
/* KERN                                                                    */
/***************************************************************************/
  typedef enum
  {
    GX_KERN_COVERAGE_VERTICAL 	  = 0x8000,
    GX_KERN_COVERAGE_CROSS_STREAM = 0x4000,
    GX_KERN_COVERAGE_VARIATION 	  = 0x2000,
    GX_KERN_COVERAGE_UNUSED_BITS  = 0x1F00,
    GX_KERN_COVERAGE_FORMAT_MASK  = 0x00FF
  } GX_KerningCoverageMask;

  typedef enum
  {
    GX_KERN_FMT_ORDERED_LIST_OF_KERNING_PAIRS 	    = 0,
    GX_KERN_FMT_STATE_TABLE_FOR_CONTEXTUAL_KERNING  = 1,
    GX_KERN_FMT_SIMPLE_NXM_ARRAY_OF_KERNING_VALUES  = 2,
    GX_KERN_FMT_SIMPLE_NXM_ARRAY_OF_KERNING_INDICES = 3
  } GX_KerningFormat;

  typedef enum
  {
    GX_KERN_ACTION_PUSH 	= 0x8000,
    GX_KERN_ACTION_DONT_ADVANCE = 0x4000,
    GX_KERN_ACTION_VALUE_OFFSET = 0x3FFF
  } GX_KerningFormat1Action;

  typedef enum
  {
    GX_KERN_VALUE_END_LIST 	     = 0x0001,
    GX_KERN_VALUE_RESET_CROSS_STREAM = 0x8000
  } GX_KerningFormat1ValueMask;
      
  typedef struct GX_KerningSubtableHeaderRec_
  {
    FT_ULong  position;		/* Not in the spec but needed in fmt2 */
    FT_ULong  length;
    FT_UShort coverage;
    FT_UShort tupleIndex;
  } GX_KerningSubtableHeaderRec, *GX_KerningSubtableHeader;

  typedef struct GX_KerningSubtableFormat0EntryRec_
  {
    FT_UShort left;
    FT_UShort right;
    FT_Short  value;
  } GX_KerningSubtableFormat0EntryRec, *GX_KerningSubtableFormat0Entry;

  typedef struct GX_KerningSubtableFormat0BodyRec_
  {
    FT_UShort nPairs;
    FT_UShort searchRange;
    FT_UShort entrySelector;
    FT_UShort rangeShift;
    GX_KerningSubtableFormat0Entry entries;
  } GX_KerningSubtableFormat0BodyRec, * GX_KerningSubtableFormat0Body;

  typedef struct GX_KerningSubtableFormat1BodyRec_
  {
    GX_StateTableRec     state_table; /* stHeader written in the spec are included 
					 in this field */
    FT_UShort            valueTable;
    FT_ULong             value_absolute_pos;
    FT_ULong             nValues;     /* in FT_FWord counts */
    FT_FWord            *values;
  } GX_KerningSubtableFormat1BodyRec, *GX_KerningSubtableFormat1Body;

  typedef struct GX_KerningSubtableFormat2ClassTableRec_
  {
    FT_UShort firstGlyph;
    FT_UShort nGlyphs;
    FT_Byte   max_class;	/* Not in the spec but useful */
    FT_Byte  *classes;
  } GX_KerningSubtableFormat2ClassTableRec, *GX_KerningSubtableFormat2ClassTable;

  typedef struct GX_KerningSubtableFormat2BodyRec_
  {
    FT_UShort rowWidth;
    FT_UShort leftClassTable;
    FT_UShort rightClassTable;
    FT_UShort array;
    GX_KerningSubtableFormat2ClassTableRec leftClass;
    GX_KerningSubtableFormat2ClassTableRec rightClass;
    FT_FWord *values;		/* The length is 
				   leftClass.max_class + rightClass.max_class */
  } GX_KerningSubtableFormat2BodyRec, *GX_KerningSubtableFormat2Body;

  typedef struct GX_KerningSubtableFormat3BodyRec_
  {
    FT_UShort glyphCount;
    FT_Byte   kernValueCount;
    FT_Byte   leftClassCount;
    FT_Byte   rightClassCount;
    FT_Byte   flags;
    FT_FWord  *kernValue;	/* [kernValueCount] */
    FT_Byte   *leftClass;	/* [glyphCount] */
    FT_Byte   *rightClass;	/* [glyphCount] */
    FT_Byte   *kernIndex;	/* [leftClassCount * rightClassCount] */
  } GX_KerningSubtableFormat3BodyRec, * GX_KerningSubtableFormat3Body;

  typedef union GX_KerningSubtableBodyDesc_
  {
    GX_KerningSubtableFormat0Body fmt0;
    GX_KerningSubtableFormat1Body fmt1;
    GX_KerningSubtableFormat2Body fmt2;
    GX_KerningSubtableFormat3Body fmt3;
    FT_Pointer any;
  } GX_KerningSubtableBodyDesc, *GX_KerningSubtableBody;

  typedef struct GX_KerningSubtableRec_
  {
    GX_KerningSubtableHeaderRec header;
    GX_KerningSubtableBodyDesc body;
  } GX_KerningSubtableRec, *GX_KerningSubtable;

  typedef struct GX_KernRec_
  {
    GX_TableRec root;
    FT_Fixed  version;
    FT_ULong  nTables;
    GX_KerningSubtable subtables;
  } GX_KernRec, *GX_Kern;

/***************************************************************************/
/* Generic                                                                 */
/***************************************************************************/
   typedef struct GXL_FontRec_
   {
     FTL_FontRec root;
     
     GX_Feat feat;     
     GX_Mort mort;
     GX_Morx morx;
     GX_Trak trak;
     GX_Kern kern;
     GX_Prop prop;
     GX_Lcar lcar;
     GX_Opbd opbd;
     GX_Bsln bsln;
     GX_Fmtx fmtx;
     GX_Fdsc fdsc;
     GX_Just just;

   } GXL_FontRec; /* *GX_Font; */

FT_END_HEADER

#endif /* Not def: __GXTYPES_H__ */


/* END */
