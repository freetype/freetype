/***************************************************************************/
/*                                                                         */
/*  tttypes.h                                                              */
/*                                                                         */
/*    Basic SFNT/TrueType type definitions and interface (specification    */
/*    only).                                                               */
/*                                                                         */
/*  This code is shared by all TrueType and OpenType drivers.              */
/*                                                                         */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef TTTYPES_H
#define TTTYPES_H


#include <freetype/tttables.h>


#ifdef __cplusplus
  extern "C" {
#endif


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                DEFINITIONS OF BASIC DATA TYPES                    ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* The REDEFINE() macro is used to convert a FreeType generic type into  */
  /* a TrueType-specific one.  It simply replaces the `FT_' prefix by      */
  /* `TT_' in order to define compatible types like TT_Long, TT_Error,     */
  /* TT_Outline, etc.                                                      */
  /*                                                                       */
#undef  REDEFINE
#define REDEFINE( type )  typedef FT_##type  TT_##type


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Bool                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef of unsigned char, used for simple booleans.              */
  /*                                                                       */
  REDEFINE( Bool );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_FWord                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 16-bit integer used to store a distance in original font  */
  /*    units.                                                             */
  /*                                                                       */
  REDEFINE( FWord );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_UFWord                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An unsigned 16-bit integer used to store a distance in original    */
  /*    font units.                                                        */
  /*                                                                       */
  REDEFINE( UFWord );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Char                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the _signed_ char type.                       */
  /*                                                                       */
  REDEFINE( Char );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Byte                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the _unsigned_ char type.                     */
  /*                                                                       */
  REDEFINE( Byte );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_String                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the char type, usually used for strings.      */
  /*                                                                       */
  REDEFINE( String );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Short                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for signed short.                                        */
  /*                                                                       */
  REDEFINE( Short );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_UShort                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for unsigned short.                                      */
  /*                                                                       */
  REDEFINE( UShort );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Int                                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for the int type.                                        */
  /*                                                                       */
  REDEFINE( Int );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_UInt                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for the unsigned int type.                               */
  /*                                                                       */
  REDEFINE( UInt );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Long                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for signed long.                                         */
  /*                                                                       */
  REDEFINE( Long );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_ULong                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for unsigned long.                                       */
  /*                                                                       */
  REDEFINE( ULong );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_F2Dot14                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 2.14 fixed float type used for unit vectors.              */
  /*                                                                       */
  REDEFINE( F2Dot14 );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_F26Dot6                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 26.6 fixed float type used for vectorial pixel            */
  /*    coordinates.                                                       */
  /*                                                                       */
  REDEFINE( F26Dot6 );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Fixed                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This type is used to store 16.16 fixed float values, like scales   */
  /*    or matrix coefficients.                                            */
  /*                                                                       */
  REDEFINE( Fixed );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Pos                                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The type FT_Pos is a 32-bit integer used to store vectorial        */
  /*    coordinates.  Depending on the context, these can represent        */
  /*    distances in integer font units, or 26.6 fixed float pixel         */
  /*    coordinates.                                                       */
  /*                                                                       */
  REDEFINE( Pos );


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Vector                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to store a 2d vector; coordinates are of   */
  /*    the TT_Pos type.                                                   */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    x :: The horizontal coordinate.                                    */
  /*    y :: The vertical coordinate.                                      */
  /*                                                                       */
  REDEFINE( Vector );


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_UnitVector                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to store a 2d vector unit vector.  Uses    */
  /*    TT_F2Dot14 types.                                                  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    x :: Horizontal coordinate.                                        */
  /*    y :: Vertical coordinate.                                          */
  /*                                                                       */
  REDEFINE( UnitVector );


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Matrix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to store a 2x2 matrix.  Coefficients are   */
  /*    in 16.16 fixed float format.  The computation performed is:        */
  /*                                                                       */
  /*       {                                                               */
  /*          x' = x*xx + y*xy                                             */
  /*          y' = x*yx + y*yy                                             */
  /*       }                                                               */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    xx :: Matrix coefficient.                                          */
  /*    xy :: Matrix coefficient.                                          */
  /*    yx :: Matrix coefficient.                                          */
  /*    yy :: Matrix coefficient.                                          */
  /*                                                                       */
  REDEFINE( Matrix );


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_BBox                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold an outline's bounding box, i.e., the      */
  /*    coordinates of its extrema in the horizontal and vertical          */
  /*    directions.                                                        */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    xMin :: The horizontal minimum (left-most).                        */
  /*    yMin :: The vertical minimum (bottom-most).                        */
  /*    xMax :: The horizontal maximum (right-most).                       */
  /*    yMax :: The vertical maximum (top-most).                           */
  /*                                                                       */
  REDEFINE( BBox );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Error                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The TrueType error code type.  A value of 0 is always interpreted  */
  /*    as a successful operation.                                         */
  /*                                                                       */
  REDEFINE( Error );


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***             REQUIRED TRUETYPE/OPENTYPE TABLES DEFINITIONS         ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TTC_Header                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    TrueType collection header.  This table contains the offsets of    */
  /*    the font headers of each distinct TrueType face in the file.       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    tag     :: Must be `ttc ' to indicate a TrueType collection.       */
  /*    version :: The version number.                                     */
  /*    count   :: The number of faces in the collection.  The             */
  /*               specification says this should be an unsigned long, but */
  /*               we use a signed long since we need the value -1 for     */
  /*               specific purposes.                                      */
  /*    offsets :: The offsets of the font headers, one per face.          */
  /*                                                                       */
  typedef struct  TTC_Header_
  {
    TT_ULong   Tag;
    TT_Fixed   version;
    TT_Long    DirCount;
    TT_ULong*  TableDirectory;

  } TTC_Header;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    SFNT_Header                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    SFNT file format header.                                           */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    format_tag     :: font format tag..                                */
  /*    num_tables     :: number of tables in file                         */
  /*    search_range   :: must be 16*(max power of 2 <= num_tables)        */
  /*    entry_selector :: log2 of search_range/16                          */
  /*    range_shift    :: must be num_tables*16 - search_range             */
  /*                                                                       */
  typedef struct SFNT_Header_
  {
    TT_ULong   format_tag;
    TT_UShort  num_tables;
    TT_UShort  search_range;
    TT_UShort  entry_selector;
    TT_UShort  range_shift;
  
  } SFNT_Header;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_TableDir                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This structure models a TrueType table directory.  It is used to   */
  /*    access the various tables of the font face.                        */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    version       :: The version number; starts with 0x00010000.       */
  /*    numTables     :: The number of tables.                             */
  /*                                                                       */
  /*    searchRange   :: Unused.                                           */
  /*    entrySelector :: Unused.                                           */
  /*    rangeShift    :: Unused.                                           */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This structure is only used during font opening.                   */
  /*                                                                       */
  typedef struct  TT_TableDir_
  {
    TT_Fixed   version;      /* should be 0x10000 */
    TT_UShort  numTables;    /* number of tables  */

    TT_UShort  searchRange;    /* These parameters are only used  */
    TT_UShort  entrySelector;  /* for a dichotomy search in the   */
    TT_UShort  rangeShift;     /* directory.  We ignore them.     */

  } TT_TableDir;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Table                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This structure describes a given table of a TrueType font.         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    Tag      :: A four-bytes tag describing the table.                 */
  /*    CheckSum :: The table checksum.  This value can be ignored.        */
  /*    Offset   :: The offset of the table from the start of the TrueType */
  /*                font in its resource.                                  */
  /*    Length   :: The table length (in bytes).                           */
  /*                                                                       */
  typedef struct  TT_Table_
  {
    TT_ULong  Tag;        /*        table type */
    TT_ULong  CheckSum;   /*    table checksum */
    TT_ULong  Offset;     /* table file offset */
    TT_ULong  Length;     /*      table length */

  } TT_Table;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_CMapDir                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This structure describes the directory of the `cmap' table,        */
  /*    containing the font's character mappings table.                    */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    tableVersionNumber :: The version number.                          */
  /*    numCMaps           :: The number of charmaps in the font.          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This structure is only used during font loading.                   */
  /*                                                                       */
  typedef struct  TT_CMapDir_
  {
    TT_UShort  tableVersionNumber;
    TT_UShort  numCMaps;

  } TT_CMapDir;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_CMapDirEntry                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This structure describes a charmap in a TrueType font.             */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    platformID :: An ID used to specify for which platform this        */
  /*                  charmap is defined (FreeType manages all platforms). */
  /*                                                                       */
  /*    encodingID :: A platform-specific ID used to indicate which source */
  /*                  encoding is used in this charmap.                    */
  /*                                                                       */
  /*    offset ::     The offset of the charmap relative to the start of   */
  /*                  the `cmap' table.                                    */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This structure is only used during font loading.                   */
  /*                                                                       */
  typedef struct  TT_CMapDirEntry_
  {
    TT_UShort  platformID;
    TT_UShort  platformEncodingID;
    TT_Long    offset;

  } TT_CMapDirEntry;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_LongMetrics                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure modeling the long metrics of the `hmtx' and `vmtx'     */
  /*    TrueType tables.  The values are expressed in font units.          */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    advance :: The advance width or height for the glyph.              */
  /*    bearing :: The left-side or top-side bearing for the glyph.        */
  /*                                                                       */
  typedef struct  TT_LongMetrics_
  {
    TT_UShort  advance;
    TT_Short   bearing;

  } TT_LongMetrics;


  /*************************************************************************/
  /*                                                                       */
  /* <Type> TT_ShortMetrics                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple type to model the short metrics of the `hmtx' and `vmtx'  */
  /*    tables.                                                            */
  /*                                                                       */
  typedef TT_Short  TT_ShortMetrics;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_NameRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure modeling TrueType name records.  Name records are used */
  /*    to store important strings like family name, style name,           */
  /*    copyright, etc. in _localized_ versions (i.e., language, encoding, */
  /*    etc).                                                              */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    platformID   :: The ID of the name's encoding platform.            */
  /*                                                                       */
  /*    encodingID   :: The platform-specific ID for the name's encoding.  */
  /*                                                                       */
  /*    languageID   :: The platform-specific ID for the name's language.  */
  /*                                                                       */
  /*    nameID       :: The ID specifying what kind of name this is.       */
  /*                                                                       */
  /*    stringLength :: The length of the string in bytes.                 */
  /*                                                                       */
  /*    stringOffset :: The offset to the string in the `name' table.      */
  /*                                                                       */
  /*    string       :: A pointer to the string's bytes.  Note that these  */
  /*                    are usually UTF-16 encoded characters.             */
  /*                                                                       */
  typedef struct  TT_NameRec_
  {
    TT_UShort  platformID;
    TT_UShort  encodingID;
    TT_UShort  languageID;
    TT_UShort  nameID;
    TT_UShort  stringLength;
    TT_UShort  stringOffset;

    /* this last field is not defined in the spec */
    /* but used by the FreeType engine            */

    TT_Byte*   string;

  } TT_NameRec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_NameTable                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure modeling the TrueType name table.                      */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    format         :: The format of the name table.                    */
  /*                                                                       */
  /*    numNameRecords :: The number of names in table.                    */
  /*                                                                       */
  /*    storageOffset  :: The offset of the name table in the `name'       */
  /*                      TrueType table.                                  */
  /*                                                                       */
  /*    names          :: An array of name records.                        */
  /*                                                                       */
  /*    storage        :: The names storage area.                          */
  /*                                                                       */
  typedef struct  TT_NameTable_
  {
    TT_UShort    format;
    TT_UShort    numNameRecords;
    TT_UShort    storageOffset;
    TT_NameRec*  names;
    TT_Byte*     storage;

  } TT_NameTable;


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***             OPTIONAL TRUETYPE/OPENTYPE TABLES DEFINITIONS         ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_GaspRange                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A tiny structure used to model a gasp range according to the       */
  /*    TrueType specification.                                            */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    maxPPEM  :: The maximum ppem value to which `gaspFlag' applies.    */
  /*                                                                       */
  /*    gaspFlag :: A flag describing the grid-fitting and anti-aliasing   */
  /*                modes to be used.                                      */
  /*                                                                       */
  typedef struct  TT_GaspRange_
  {
    TT_UShort  maxPPEM;
    TT_UShort  gaspFlag;

  } TT_GaspRange;


#define TT_GASP_GRIDFIT  0x01
#define TT_GASP_DOGRAY   0x02


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Gasp                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure modeling the TrueType `gasp' table used to specify     */
  /*    grid-fitting and anti-aliasing behaviour.                          */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    version    :: The version number.                                  */
  /*    numRanges  :: The number of gasp ranges in table.                  */
  /*    gaspRanges :: An array of gasp ranges.                             */
  /*                                                                       */
  typedef struct  TT_Gasp_
  {
    TT_UShort      version;
    TT_UShort      numRanges;
    TT_GaspRange*  gaspRanges;

  } TT_Gasp;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_HdmxRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A small structure used to model the pre-computed widths of a given */
  /*    size.  They're found in the `hdmx' table.                          */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    ppem      :: The pixels per EM value at which these metrics apply. */
  /*                                                                       */
  /*    max_width :: The maximum advance width for this metric.            */
  /*                                                                       */
  /*    widths    :: An array of widths.  Note: These are 8-bit bytes.     */
  /*                                                                       */
  typedef struct  TT_HdmxRec_
  {
    TT_Byte   ppem;
    TT_Byte   max_width;
    TT_Byte*  widths;

  } TT_HdmxRec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_HdmxRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model the `hdmx' table, which contains         */
  /*    pre-computed widths for a set of given sizes/dimensions.           */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    version     :: The version number.                                 */
  /*    num_records :: The number of hdmx records.                         */
  /*    records     :: An array of hdmx records.                           */
  /*                                                                       */
  typedef struct  TT_Hdmx_
  {
    TT_UShort    version;
    TT_Short     num_records;
    TT_HdmxRec*  records;

  } TT_Hdmx;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Kern_0_Pair                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model a kerning pair for the kerning table     */
  /*    format 0.  The engine now loads this table if it finds one in the  */
  /*    font file.                                                         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    left  :: The index of the left glyph in pair.                      */
  /*    right :: The index of the right glyph in pair.                     */
  /*    value :: The kerning distance.  A positive value spaces the        */
  /*             glyphs, a negative one makes them closer.                 */
  /*                                                                       */
  typedef struct  TT_Kern_0_Pair_
  {
    TT_UShort  left;   /* index of left  glyph in pair */
    TT_UShort  right;  /* index of right glyph in pair */
    TT_FWord   value;  /* kerning value                */

  } TT_Kern_0_Pair;

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                  EMBEDDED BITMAPS SUPPORT                         ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_Metrics                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold the big metrics of a given glyph bitmap   */
  /*    in a TrueType or OpenType font.  These are usually found in the    */
  /*    `EBDT' (Microsoft) or `bdat' (Apple) table.                        */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    height       :: The glyph height in pixels.                        */
  /*                                                                       */
  /*    width        :: The glyph width in pixels.                         */
  /*                                                                       */
  /*    horiBearingX :: The horizontal left bearing.                       */
  /*                                                                       */
  /*    horiBearingY :: The horizontal top bearing.                        */
  /*                                                                       */
  /*    horiAdvance  :: The horizontal advance.                            */
  /*                                                                       */
  /*    vertBearingX :: The vertical left bearing.                         */
  /*                                                                       */
  /*    vertBearingY :: The vertical top bearing.                          */
  /*                                                                       */
  /*    vertAdvance  :: The vertical advance.                              */
  /*                                                                       */
  typedef struct  TT_SBit_Metrics_
  {
    TT_Byte  height;
    TT_Byte  width;

    TT_Char  horiBearingX;
    TT_Char  horiBearingY;
    TT_Byte  horiAdvance;

    TT_Char  vertBearingX;
    TT_Char  vertBearingY;
    TT_Byte  vertAdvance;

  } TT_SBit_Metrics;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_Small_Metrics                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold the small metrics of a given glyph bitmap */
  /*    in a TrueType or OpenType font.  These are usually found in the    */
  /*    `EBDT' (Microsoft) or the `bdat' (Apple) table.                    */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    height    :: The glyph height in pixels.                           */
  /*                                                                       */
  /*    width     :: The glyph width in pixels.                            */
  /*                                                                       */
  /*    bearingX  :: The left-side bearing.                                */
  /*                                                                       */
  /*    bearingY  :: The top-side bearing.                                 */
  /*                                                                       */
  /*    advance   :: The advance width or height.                          */
  /*                                                                       */
  typedef struct  TT_SBit_Small_Metrics_
  {
    TT_Byte  height;
    TT_Byte  width;

    TT_Char  bearingX;
    TT_Char  bearingY;
    TT_Byte  advance;

  } TT_SBit_Small_Metrics;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_Line_Metrics                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to describe the text line metrics of a given      */
  /*    bitmap strike, for either a horizontal or vertical layout.         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    ascender                :: The ascender in pixels.                 */
  /*                                                                       */
  /*    descender               :: The descender in pixels.                */
  /*                                                                       */
  /*    max_width               :: The maximum glyph width in pixels.      */
  /*                                                                       */
  /*    caret_slope_enumerator  :: Rise of the caret slope, typically set  */
  /*                               to 1 for non-italic fonts.              */
  /*                                                                       */
  /*    caret_slope_denominator :: Rise of the caret slope, typically set  */
  /*                               to 0 for non-italic fonts.              */
  /*                                                                       */
  /*    caret_offset            :: Offset in pixels to move the caret for  */
  /*                               proper positioning.                     */
  /*                                                                       */
  /*    min_origin_SB           :: Minimum of horiBearingX (resp.          */
  /*                               vertBearingY).                          */
  /*    min_advance_SB          :: Minimum of                              */
  /*                                                                       */
  /*                                 horizontal advance -                  */
  /*                                   ( horiBearingX + width )            */
  /*                                                                       */
  /*                               resp.                                   */
  /*                                                                       */
  /*                                 vertical advance -                    */
  /*                                   ( vertBearingY + height )           */
  /*                                                                       */
  /*    max_before_BL           :: Maximum of horiBearingY (resp.          */
  /*                               vertBearingY).                          */
  /*                                                                       */
  /*    min_after_BL            :: Minimum of                              */
  /*                                                                       */
  /*                                 horiBearingY - height                 */
  /*                                                                       */
  /*                               resp.                                   */
  /*                                                                       */
  /*                                 vertBearingX - width                  */
  /*                                                                       */
  typedef struct  TT_SBit_Line_Metrics_
  {
    TT_Char  ascender;
    TT_Char  descender;
    TT_Byte  max_width;
    TT_Char  caret_slope_numerator;
    TT_Char  caret_slope_denominator;
    TT_Char  caret_offset;
    TT_Char  min_origin_SB;
    TT_Char  min_advance_SB;
    TT_Char  max_before_BL;
    TT_Char  min_after_BL;
    TT_Char  pads[2];

  } TT_SBit_Line_Metrics;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_Range                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A TrueType/OpenType subIndexTable as defined in the `EBLC'         */
  /*    (Microsoft) or `bloc' (Apple) tables.                              */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    first_glyph   :: The first glyph index in the range.               */
  /*                                                                       */
  /*    last_glyph    :: The last glyph index in the range.                */
  /*                                                                       */
  /*    index_format  :: The format of index table.  Valid values are 1    */
  /*                     to 5.                                             */
  /*                                                                       */
  /*    image_format  :: The format of `EBDT' image data.                  */
  /*                                                                       */
  /*    image_offset  :: The offset to image data in `EBDT'.               */
  /*                                                                       */
  /*    image_size    :: For index formats 2 and 5.  This is the size in   */
  /*                     bytes of each glyph bitmap.                       */
  /*                                                                       */
  /*    big_metrics   :: For index formats 2 and 5.  This is the big       */
  /*                     metrics for each glyph bitmap.                    */
  /*                                                                       */
  /*    num_glyphs    :: For index formats 4 and 5.  This is the number of */
  /*                     glyphs in the code array.                         */
  /*                                                                       */
  /*    glyph_offsets :: For index formats 1 and 3.                        */
  /*                                                                       */
  /*    glyph_codes   :: For index formats 4 and 5.                        */
  /*                                                                       */
  /*    table_offset  :: The offset of the index table in the `EBLC'       */
  /*                     table.  Only used during strike loading.          */
  /*                                                                       */
  typedef struct  TT_SBit_Range
  {
    TT_UShort        first_glyph;
    TT_UShort        last_glyph;

    TT_UShort        index_format;
    TT_UShort        image_format;
    TT_ULong         image_offset;

    TT_ULong         image_size;
    TT_SBit_Metrics  metrics;
    TT_ULong         num_glyphs;

    TT_ULong*        glyph_offsets;
    TT_UShort*       glyph_codes;

    TT_ULong         table_offset;

  } TT_SBit_Range;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_Strike                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used describe a given bitmap strike in the `EBLC'      */
  /*    (Microsoft) or `bloc' (Apple) tables.                              */
  /*                                                                       */
  /* <Fields>                                                              */
  /*   num_index_ranges :: The number of index ranges.                     */
  /*                                                                       */
  /*   index_ranges     :: An array of glyph index ranges.                 */
  /*                                                                       */
  /*   color_ref        :: Unused.  A color reference?                     */
  /*                                                                       */
  /*   hori             :: The line metrics for horizontal layouts.        */
  /*                                                                       */
  /*   vert             :: The line metrics for vertical layouts.          */
  /*                                                                       */
  /*   start_glyph      :: The lowest glyph index for this strike.         */
  /*                                                                       */
  /*   end_glyph        :: The highest glyph index for this strike.        */
  /*                                                                       */
  /*   x_ppem           :: The number of horizontal pixels per EM.         */
  /*                                                                       */
  /*   y_ppem           :: The number of vertical pixels per EM.           */
  /*                                                                       */
  /*   bit_depth        :: The bit depth.  Valid values are 1, 2, 4,       */
  /*                       and 8.                                          */
  /*                                                                       */
  /*   flags            :: Is this a vertical or horizontal strike?        */
  /*                                                                       */
  typedef struct  TT_SBit_Strike_
  {
    TT_Int                 num_ranges;
    TT_SBit_Range*         sbit_ranges;
    TT_ULong               ranges_offset;

    TT_ULong               color_ref;

    TT_SBit_Line_Metrics   hori;
    TT_SBit_Line_Metrics   vert;

    TT_UShort              start_glyph;
    TT_UShort              end_glyph;

    TT_Byte                x_ppem;
    TT_Byte                y_ppem;

    TT_Byte                bit_depth;
    TT_Char                flags;

  } TT_SBit_Strike;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_Component                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure to describe a compound sbit element.            */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    glyph_code :: The element's glyph index.                           */
  /*    x_offset   :: The element's left bearing.                          */
  /*    y_offset   :: The element's top bearing.                           */
  /*                                                                       */
  typedef struct  TT_SBit_Component_
  {
    TT_UShort  glyph_code;

    TT_Char    x_offset;
    TT_Char    y_offset;

  } TT_SBit_Component;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_Scale                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used describe a given bitmap scaling table, as defined */
  /*    in the `EBSC' table.                                               */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    hori              :: The horizontal line metrics.                  */
  /*                                                                       */
  /*    vert              :: The vertical line metrics.                    */
  /*                                                                       */
  /*    x_ppem            :: The number of horizontal pixels per EM.       */
  /*                                                                       */
  /*    y_ppem            :: The number of vertical pixels per EM.         */
  /*                                                                       */
  /*    x_ppem_substitute :: Substitution x_ppem value.                    */
  /*                                                                       */
  /*    y_ppem_substitute :: Substitution y_ppem value.                    */
  /*                                                                       */
  typedef struct  TT_SBit_Scale_
  {
    TT_SBit_Line_Metrics  hori;
    TT_SBit_Line_Metrics  vert;

    TT_Byte               x_ppem;
    TT_Byte               y_ppem;

    TT_Byte               x_ppem_substitute;
    TT_Byte               y_ppem_substitute;

  } TT_SBit_Scale;

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                  POSTSCRIPT GLYPH NAMES SUPPORT                   ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Post_20                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Postscript names sub-table, format 2.0.  Stores the PS name of     */
  /*    each glyph in the font face.                                       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_glyphs    :: The number of named glyphs in the table.          */
  /*                                                                       */
  /*    num_names     :: The number of PS names stored in the table.       */
  /*                                                                       */
  /*    glyph_indices :: The indices of the glyphs in the names arrays.    */
  /*                                                                       */
  /*    glyph_names   :: The PS names not in Mac Encoding.                 */
  /*                                                                       */
  typedef struct  TT_Post_20_
  {
    TT_UShort   num_glyphs;
    TT_UShort   num_names;
    TT_UShort*  glyph_indices;
    TT_Char**   glyph_names;

  } TT_Post_20;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Post_25                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Postscript names sub-table, format 2.5.  Stores the PS name of     */
  /*    each glyph in the font face.                                       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_glyphs :: The number of glyphs in the table.                   */
  /*                                                                       */
  /*    offsets    :: An array of signed offsets in a normal Mac           */
  /*                  Postscript name encoding.                            */
  /*                                                                       */
  typedef struct  TT_Post_25_
  {
    TT_UShort  num_glyphs;
    TT_Char*   offsets;

  } TT_Post_25;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Post_Names                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Postscript names table, either format 2.0 or 2.5.                  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    loaded    :: A flag to indicate whether the PS names are loaded.   */
  /*                                                                       */
  /*    format_20 :: The sub-table used for format 2.0.                    */
  /*                                                                       */
  /*    format_25 :: The sub-table used for format 2.5.                    */
  /*                                                                       */
  typedef struct  TT_Post_Names_
  {
    TT_Bool       loaded;

    union
    {
      TT_Post_20  format_20;
      TT_Post_25  format_25;

    } names;

  } TT_Post_Names;

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                  TRUETYPE CHARMAPS SUPPORT                        ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /* format 0 */
  typedef struct  TT_CMap0_
  {
    TT_Byte*  glyphIdArray;

  } TT_CMap0;


  /* format 2 */
  typedef struct  TT_CMap2SubHeader_
  {
    TT_UShort  firstCode;      /* first valid low byte         */
    TT_UShort  entryCount;     /* number of valid low bytes    */
    TT_Short   idDelta;        /* delta value to glyphIndex    */
    TT_UShort  idRangeOffset;  /* offset from here to 1st code */

  } TT_CMap2SubHeader;


  typedef struct  TT_CMap2_
  {
    TT_UShort*  subHeaderKeys;
    /* high byte mapping table            */
    /* value = subHeader index * 8        */

    TT_CMap2SubHeader*  subHeaders;
    TT_UShort*          glyphIdArray;
    TT_UShort           numGlyphId;   /* control value */

  } TT_CMap2;


  /* format 4 */
  typedef struct  TT_CMap4Segment_
  {
    TT_UShort  endCount;
    TT_UShort  startCount;
    TT_Short   idDelta;
    TT_UShort  idRangeOffset;

  } TT_CMap4Segment;


  typedef struct  TT_CMap4_
  {
    TT_UShort  segCountX2;     /* number of segments * 2       */
    TT_UShort  searchRange;    /* these parameters can be used */
    TT_UShort  entrySelector;  /* for a binary search          */
    TT_UShort  rangeShift;

    TT_CMap4Segment*  segments;
    TT_UShort*        glyphIdArray;
    TT_UShort         numGlyphId;   /* control value */
    
    TT_CMap4Segment*  last_segment;  /* last used segment, this is a small  */
                                     /* cache to potentially increase speed */
  } TT_CMap4;


  /* format 6 */
  typedef struct  TT_CMap6_
  {
    TT_UShort   firstCode;      /* first character code of subrange      */
    TT_UShort   entryCount;     /* number of character codes in subrange */

    TT_UShort*  glyphIdArray;

  } TT_CMap6;

  typedef struct TT_CMapTable_  TT_CMapTable;

  typedef
  TT_UInt  (*TT_CharMap_Func)( TT_CMapTable*  charmap,
                               TT_ULong       char_code );

  /* charmap table */
  struct  TT_CMapTable_
  {
    TT_UShort  platformID;
    TT_UShort  platformEncodingID;
    TT_UShort  format;
    TT_UShort  length;
    TT_UShort  version;

    TT_Bool    loaded;
    TT_ULong   offset;

    union
    {
      TT_CMap0  cmap0;
      TT_CMap2  cmap2;
      TT_CMap4  cmap4;
      TT_CMap6  cmap6;
    } c;

    TT_CharMap_Func  get_index;
  };


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_CharMapRec                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*   The TrueType character map object type.                             */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    root :: The parent character map structure.                        */
  /*    cmap :: The used character map.                                    */
  /*                                                                       */
  typedef struct  TT_CharMapRec_
  {
    FT_CharMapRec   root;
    TT_CMapTable    cmap;

  } TT_CharMapRec;


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                  ORIGINAL TT_FACE CLASS DEFINITION                ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This structure/class is defined here because it is common to the      */
  /* following formats: TTF, OpenType-TT, and OpenType-CFF.                */
  /*                                                                       */
  /* Note however that the classes TT_Size, TT_GlyphSlot, and TT_CharMap   */
  /* are not shared between font drivers, and are thus defined normally in */
  /* `drivers/truetype/ttobjs.h'.                                          */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Face                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a TrueType face/font object.  A TT_Face encapsulates   */
  /*    the resolution and scaling independent parts of a TrueType font    */
  /*    resource.                                                          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The TT_Face structure is also used as a `parent class' for the     */
  /*    OpenType-CFF class (T2_Face).                                      */
  /*                                                                       */
  typedef struct TT_FaceRec_*   TT_Face;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_CharMap                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a TrueType character mapping object.                   */
  /*                                                                       */
  typedef struct TT_CharMapRec_*  TT_CharMap;


  /* a function type used for the truetype bytecode interpreter hooks */
  typedef TT_Error  (*TT_Interpreter)( void*  exec_context );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Goto_Table_Func                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Seeks a stream to the start of a given TrueType table.             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face      :: A handle to the target face object.                   */
  /*    tag       :: a 4-byte tag used to name the table                   */
  /*    stream    :: The input stream.                                     */
  /*                                                                       */
  /* <Output>                                                              */
  /*    length    :: length of table in bytes. Set to 0 when not needed    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The stream cursor must be at the font file's origin                */
  /*                                                                       */
  typedef
  TT_Error  (*TT_Goto_Table_Func)( TT_Face    face,
                                   TT_ULong   tag,
                                   FT_Stream  stream,
                                   TT_ULong  *length );

  /*************************************************************************/
  /*                                                                       */
  /*                         TrueType Face Type                            */
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Face                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The TrueType face class.  These objects model the resolution and   */
  /*    point-size independent data found in a TrueType font file.         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    root              :: The base FT_Face structure, managed by the    */
  /*                         base layer.                                   */
  /*                                                                       */
  /*    ttcHeader         :: The TrueType collection header, used when the */
  /*                         file is a `ttc' rather than a `ttf'.  For     */
  /*                         ordinary font files, the field                */
  /*                         `ttcHeader.DirCount' is set to 0.             */
  /*                                                                       */
  /*    num_tables        :: The number of TrueType tables in this font    */
  /*                         file.                                         */
  /*                                                                       */
  /*    dir_tables        :: The directory of TrueType tables for this     */
  /*                         font file.                                    */
  /*                                                                       */
  /*    header            :: The font's font header (`head' table).  Read  */
  /*                         on font opening.                              */
  /*                                                                       */
  /*    horizontal        :: The font's horizontal header (`hhea' table).  */
  /*                         This field also contains the associated       */
  /*                         horizontal metrics table (`hmtx').            */
  /*                                                                       */
  /*    max_profile       :: The font's maximum profile table.  Read on    */
  /*                         font opening.  Note that some maximum values  */
  /*                         cannot be taken directly from this table.  We */
  /*                         thus define additional fields below to hold   */
  /*                         the computed maxima.                          */
  /*                                                                       */
  /*    max_components    :: The maximum number of glyph components        */
  /*                         required to load any composite glyph from     */
  /*                         this font.  Used to size the load stack.      */
  /*                                                                       */
  /*    vertical_info     :: A boolean which is set when the font file     */
  /*                         contains vertical metrics.  If not, the value */
  /*                         of the `vertical' field is undefined.         */
  /*                                                                       */
  /*    vertical          :: The font's vertical header (`vhea' table).    */
  /*                         This field also contains the associated       */
  /*                         vertical metrics table (`vmtx'), if found.    */
  /*                         IMPORTANT: The contents of this field is      */
  /*                         undefined if the `verticalInfo' field is      */
  /*                         unset.                                        */
  /*                                                                       */
  /*    num_names         :: The number of name records within this        */
  /*                         TrueType font.                                */
  /*                                                                       */
  /*    name_table        :: The table of name records (`name').           */
  /*                                                                       */
  /*    os2               :: The font's OS/2 table (`OS/2').               */
  /*                                                                       */
  /*    postscript        :: The font's PostScript table (`post' table).   */
  /*                         The PostScript glyph names are not loaded by  */
  /*                         the driver on face opening.  See the `ttpost' */
  /*                         module for more details.                      */
  /*                                                                       */
  /*    num_charmaps      :: The number of character mappings in the font. */
  /*                                                                       */
  /*    charmaps          :: The array of charmap objects for this font    */
  /*                         file.  Note that this field is a typeless     */
  /*                         pointer.  The Reason is that the format of    */
  /*                         charmaps varies with the underlying font      */
  /*                         format and cannot be determined here.         */
  /*                                                                       */
  /*    goto_face         :: a function called by each TrueType table      */
  /*                         loader to position a stream's cursor to the   */
  /*                         start of a given table according to its tag.  */
  /*                         it defaults to TT_Goto_Face but can be        */
  /*                         different for strange formats (e.g. Type 42)  */
  /*                                                                       */
  /*    sfnt              :: a pointer to the SFNT `driver' interface.     */
  /*                                                                       */
  /*    hdmx              :: The face's horizontal device metrics (`hdmx'  */
  /*                         table).  This table is optional in            */
  /*                         TrueType/OpenType fonts.                      */
  /*                                                                       */
  /*    gasp              :: The grid-fitting and scaling properties table */
  /*                         (`gasp').  This table is optional in          */
  /*                         TrueType/OpenType fonts.                      */
  /*                                                                       */
  /*    num_sbit_strikes  :: The number of sbit strikes, i.e., bitmap      */
  /*                         sizes, embedded in this font.                 */
  /*                                                                       */
  /*    sbit_strikes      :: An array of sbit strikes embedded in this     */
  /*                         font.  This table is optional in a            */
  /*                         TrueType/OpenType font.                       */
  /*                                                                       */
  /*    num_sbit_scales   :: The number of sbit scales for this font.      */
  /*                                                                       */
  /*    sbit_scales       :: Array of sbit scales embedded in this font.   */
  /*                         This table is optional in a TrueType/OpenType */
  /*                         font.                                         */
  /*                                                                       */
  /*    postscript_names  :: A table used to store the Postscript names of */
  /*                         the glyphs for this font.  See the file       */
  /*                         `ttconfig.h' for comments on the              */
  /*                         TT_CONFIG_OPTION_POSTSCRIPT_NAMES option.     */
  /*                                                                       */
  /*    num_locations     :: The number of glyph locations in this         */
  /*                         TrueType file.  This should be identical to   */
  /*                         the number of glyphs.  Ignored for Type 2     */
  /*                         fonts.                                        */
  /*                                                                       */
  /*    glyph_locations   :: An array of longs.  These are offsets to      */
  /*                         glyph data within the `glyf' table.  Ignored  */
  /*                         for Type 2 font faces.                        */
  /*                                                                       */
  /*    font_program_size :: Size in bytecodes of the face's font program. */
  /*                         0 if none defined.  Ignored for Type 2 fonts. */
  /*                                                                       */
  /*    font_program      :: The face's font program (bytecode stream)     */
  /*                         executed at load time, also used during glyph */
  /*                         rendering.  Comes from the `fpgm' table.      */
  /*                         Ignored for Type 2 font fonts.                */
  /*                                                                       */
  /*    cvt_program_size  :: Size in bytecodes of the face's cvt program.  */
  /*                         Ignored for Type 2 fonts.                     */
  /*                                                                       */
  /*    cvt_program       :: The face's cvt program (bytecode stream)      */
  /*                         executed each time an instance/size is        */
  /*                         changed/reset.  Comes from the `prep' table.  */
  /*                         Ignored for Type 2 fonts.                     */
  /*                                                                       */
  /*    cvt_size          :: Size of the control value table (in entries). */
  /*                         Ignored for Type 2 fonts.                     */
  /*                                                                       */
  /*    cvt               :: The face's original control value table.      */
  /*                         Coordinates are expressed in unscaled font    */
  /*                         units.  Comes from the `cvt ` table.  Ignored */
  /*                         for Type 2 fonts.                             */
  /*                                                                       */
  /*    num_kern_pairs    :: The number of kerning pairs present in the    */
  /*                         font file.  The engine only loads the first   */
  /*                         horizontal format 0 kern table it finds in    */
  /*                         the font file.  You should use the `ttxkern'  */
  /*                         structures if you want to access other        */
  /*                         kerning tables.  Ignored for Type 2 fonts.    */
  /*                                                                       */
  /*    kern_table_index  :: The index of the kerning table in the font    */
  /*                         kerning directory.  Only used by the ttxkern  */
  /*                         extension to avoid data duplication.  Ignored */
  /*                         for Type 2 fonts.                             */
  /*                                                                       */
  /*    kern_pairs        :: Array of kerning pairs, if any.  Ignored for  */
  /*                         Type 2 fonts.                                 */
  /*                                                                       */
  /*    interpreter       :: Pointer to the TrueType bytecode interpreter  */
  /*                         this field is also used to hook the debugger  */
  /*                         in `ttdebug'                                  */
  /*                                                                       */
  typedef struct  TT_FaceRec_
  {
    FT_FaceRec         root;

    TTC_Header         ttc_header;

    FT_ULong           format_tag;
    TT_UShort          num_tables;
    TT_Table*          dir_tables;

    TT_Header          header;       /* TrueType header table          */
    TT_HoriHeader      horizontal;   /* TrueType horizontal header     */

    TT_MaxProfile      max_profile;
    TT_ULong           max_components;

    TT_Bool            vertical_info;
    TT_VertHeader      vertical;     /* TT Vertical header, if present */

    TT_Int             num_names;    /* number of name records  */
    TT_NameTable       name_table;   /* name table              */

    TT_OS2             os2;          /* TrueType OS/2 table            */
    TT_Postscript      postscript;   /* TrueType Postscript table      */

    TT_Int             num_charmaps;
    TT_CharMap         charmaps;     /* array of TT_CharMapRec */

    /* a pointer to the function used to seek a stream to the start of */
    /* a given TrueType table. This should default to the function     */
    /* TT_Goto_Table defined in `ttload.h', but some font drivers      */
    /* might need something different, e.g. Type 42 fonts              */
    TT_Goto_Table_Func       goto_table;

    /* a typeless pointer to the SFNT_Interface table used to load     */
    /* the basic TrueType tables in the face object                    */
    void*              sfnt;

    /* a typeless pointer to the PSNames_Interface table used to       */
    /* handle glyph names <-> unicode & Mac values                     */
    void*              psnames;

    /***********************************************************************/
    /*                                                                     */
    /* Optional TrueType/OpenType tables                                   */
    /*                                                                     */
    /***********************************************************************/

    /* horizontal device metrics */
    TT_Hdmx            hdmx;

    /* grid-fitting and scaling table */
    TT_Gasp            gasp;                 /* the `gasp' table */

    /* PCL 5 table */
	TT_PCLT            pclt;

    /* embedded bitmaps support */
    TT_Int             num_sbit_strikes;
    TT_SBit_Strike*    sbit_strikes;

    TT_Int             num_sbit_scales;
    TT_SBit_Scale*     sbit_scales;

    /* postscript names table */
    TT_Post_Names      postscript_names;

    /***********************************************************************/
    /*                                                                     */
    /* TrueType-specific fields (ignored by the OTF-Type2 driver)          */
    /*                                                                     */
    /***********************************************************************/

    /* the glyph locations */
    TT_UShort          num_locations;
    TT_Long*           glyph_locations;

    /* the font program, if any */
    TT_ULong           font_program_size;
    TT_Byte*           font_program;

    /* the cvt program, if any */
    TT_ULong           cvt_program_size;
    TT_Byte*           cvt_program;

    /* the original, unscaled, control value table */
    TT_ULong           cvt_size;
    TT_Short*          cvt;

    /* the format 0 kerning table, if any */
    TT_Int             num_kern_pairs;
    TT_Int             kern_table_index;
    TT_Kern_0_Pair*    kern_pairs;

    /* a pointer to the bytecode interpreter to use. This is also */
    /* used to hook the debugger for the `ttdebug' utility..      */
    TT_Interpreter     interpreter;

    /***********************************************************************/
    /*                                                                     */
    /* Other tables or fields. This is used by derivative formats like     */
    /* OpenType.                                                           */
    /*                                                                     */
    /***********************************************************************/

    void*              other;

  } TT_FaceRec;


#ifdef __cplusplus
  }
#endif


#endif /* TTTYPES_H */


/* END */
