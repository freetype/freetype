/****************************************************************************
 *
 * wofftypes.h
 *
 *   Basic WOFF/WOFF2 type definitions and interface (specification
 *   only).
 *
 * Copyright (C) 1996-2019 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef WOFFTYPES_H_
#define WOFFTYPES_H_


#include <ft2build.h>
#include FT_TRUETYPE_TABLES_H
#include FT_INTERNAL_OBJECTS_H


FT_BEGIN_HEADER


  /**************************************************************************
   *
   * @struct:
   *   WOFF_HeaderRec
   *
   * @description:
   *   WOFF file format header.
   *
   * @fields:
   *   See
   *
   *     https://www.w3.org/TR/WOFF/#WOFFHeader
   */
  typedef struct  WOFF_HeaderRec_
  {
    FT_ULong   signature;
    FT_ULong   flavor;
    FT_ULong   length;
    FT_UShort  num_tables;
    FT_UShort  reserved;
    FT_ULong   totalSfntSize;
    FT_UShort  majorVersion;
    FT_UShort  minorVersion;
    FT_ULong   metaOffset;
    FT_ULong   metaLength;
    FT_ULong   metaOrigLength;
    FT_ULong   privOffset;
    FT_ULong   privLength;

  } WOFF_HeaderRec, *WOFF_Header;


  /**************************************************************************
   *
   * @struct:
   *   WOFF_TableRec
   *
   * @description:
   *   This structure describes a given table of a WOFF font.
   *
   * @fields:
   *   Tag ::
   *     A four-bytes tag describing the table.
   *
   *   Offset ::
   *     The offset of the table from the start of the WOFF font in its
   *     resource.
   *
   *   CompLength ::
   *     Compressed table length (in bytes).
   *
   *   OrigLength ::
   *     Uncompressed table length (in bytes).
   *
   *   CheckSum ::
   *     The table checksum.  This value can be ignored.
   *
   *   OrigOffset ::
   *     The uncompressed table file offset.  This value gets computed while
   *     constructing the (uncompressed) SFNT header.  It is not contained in
   *     the WOFF file.
   */
  typedef struct  WOFF_TableRec_
  {
    FT_ULong  Tag;           /* table ID                  */
    FT_ULong  Offset;        /* table file offset         */
    FT_ULong  CompLength;    /* compressed table length   */
    FT_ULong  OrigLength;    /* uncompressed table length */
    FT_ULong  CheckSum;      /* uncompressed checksum     */

    FT_ULong  OrigOffset;    /* uncompressed table file offset */
                             /* (not in the WOFF file)         */
  } WOFF_TableRec, *WOFF_Table;


  /**************************************************************************
   *
   * @struct:
   *   WOFF2_TtcFontRec
   *
   * @description:
   *   Metadata for a TTC font entry in WOFF2.
   *
   * @fields:
   *   flavor ::
   *     TTC font flavor.
   *
   *   num_tables ::
   *     Number of tables in TTC, indicating number of elements in
   *     `table_indices`.
   *
   *   dst_offset ::
   *     Uncompressed table offset.
   *
   *   header_checksum ::
   *     Checksum for font header.
   *
   *   table_indices ::
   *     Array of table indices for each TTC font.
   */
  typedef struct  WOFF2_TtcFontRec_
  {
    FT_ULong    flavor;
    FT_UShort   num_tables;
    FT_ULong    dst_offset;
    FT_ULong    header_checksum;
    FT_UShort*  table_indices;

  } WOFF2_TtcFontRec, *WOFF2_TtcFont;


  /**************************************************************************
   *
   * @struct:
   *   WOFF2_HeaderRec
   *
   * @description:
   *   WOFF2 file format header.
   *
   * @fields:
   *   See
   *
   *     https://www.w3.org/TR/WOFF2/#woff20Header
   *
   * @note:
   *   We don't care about the fields `reserved`, `majorVersion` and
   *   `minorVersion`, so they are not included.  The `totalSfntSize` field
   *   does not necessarily represent the actual size of the uncompressed
   *   SFNT font stream, so that is not included either.
   */
  typedef struct  WOFF2_HeaderRec_
  {
    FT_ULong   signature;
    FT_ULong   flavor;
    FT_ULong   length;
    FT_UShort  num_tables;
    FT_ULong   totalCompressedSize;
    FT_ULong   metaOffset;
    FT_ULong   metaLength;
    FT_ULong   metaOrigLength;
    FT_ULong   privOffset;
    FT_ULong   privLength;

    FT_ULong   uncompressed_size;
    FT_UInt64  compressed_offset;
    FT_ULong   header_version;
    FT_UShort  num_fonts;

    WOFF2_TtcFont  ttc_fonts;

  } WOFF2_HeaderRec, *WOFF2_Header;


  /**************************************************************************
   *
   * @struct:
   *   WOFF2_TableRec
   *
   * @description:
   *   This structure describes a given table of a WOFF2 font.
   *
   * @fields:
   *   See
   *
   *     https://www.w3.org/TR/WOFF2/#table_dir_format
   */
  typedef struct  WOFF2_TableRec_
  {
    FT_Byte   FlagByte;           /* table type and flags      */
    FT_ULong  Tag;                /* table file offset         */
    FT_ULong  OrigLength;         /* uncompressed table length */
    FT_ULong  TransformLength;    /* transformed length        */

    FT_ULong  flags;              /* calculated flags          */
    FT_ULong  src_offset;         /* compressed table offset   */
    FT_ULong  src_length;         /* compressed table length   */

    FT_ULong  dst_offset;         /* uncompressed table offset */

  } WOFF2_TableRec, *WOFF2_Table;


FT_END_HEADER

#endif /* WOFFTYPES_H_ */


/* END */
