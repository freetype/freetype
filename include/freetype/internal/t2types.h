/***************************************************************************/
/*                                                                         */
/*  t2types.h                                                              */
/*                                                                         */
/*    Basic OpenType/CFF type definitions and interface (specification     */
/*    only).                                                               */
/*                                                                         */
/*  This code is used by the OpenType/CFF driver.                          */
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


#ifndef T2TYPES_H
#define T2TYPES_H


#include <freetype/internal/tttypes.h>


#ifdef __cplusplus
  extern "C" {
#endif

 /*************************************************************************
  *
  * <Struct>
  *    CFF_Index
  *
  * <Description>
  *   A structure used to model a CFF Index table
  *
  * <Fields>
  *   count        :: number of elements in index
  *   off_size     :: size in bytes of object offsets in index
  *   data_offset  :: position of first data byte in the index's bytes
  *
  *   total_size   :: total_size of index in bytes
  *   bytes        :: when the index is loaded in memory, its bytes
  *   file_offset  :: position of index in file. The offset table starts
  *                   at file_offset + 3
  */
  typedef struct CFF_Index_
  {
    FT_Stream  stream;
    FT_UInt    count;
    FT_Byte    off_size;
    FT_ULong   data_offset;

    FT_ULong*  offsets;
    FT_Byte*   bytes;

  } CFF_Index;


  typedef struct CFF_Top_Dict_
  {
    FT_UInt      version;
    FT_UInt      notice;
    FT_UInt      copyright;
    FT_UInt      full_name;
    FT_UInt      family_name;
    FT_UInt      weight;
    FT_Bool      is_fixed_pitch;
    FT_Fixed     italic_angle;
    FT_Pos       underline_position;
    FT_Pos       underline_thickness;
    FT_Int       paint_type;
    FT_Int       charstring_type;
    FT_Matrix    font_matrix;
    FT_ULong     unique_id;
    FT_BBox      font_bbox;
    FT_Pos       stroke_width;
    FT_ULong     charset_offset;
    FT_ULong     encoding_offset;
    FT_ULong     charstrings_offset;
    FT_ULong     private_offset;
    FT_ULong     private_size;
    FT_Long      synthetic_base;
    FT_UInt      embedded_postscript;
    FT_UInt      base_font_name;       
    FT_UInt      postscript;

    FT_UInt      cid_registry;
    FT_UInt      cid_ordering;
    FT_ULong     cid_supplement;

    FT_Long      cid_font_version;
    FT_Long      cid_font_revision;
    FT_Long      cid_font_type;
    FT_Long      cid_count;
    FT_ULong     cid_uid_base;
    FT_ULong     cid_fd_array_offset;
    FT_ULong     cid_fd_select_offset;
    FT_UInt      cid_font_name;

  } CFF_Top_Dict;
  
  typedef struct CFF_Private_
  {
    FT_Byte  num_blue_values;
    FT_Byte  num_other_blues;
    FT_Byte  num_family_blues;
    FT_Byte  num_family_other_blues;
    
    FT_Pos   blue_values[14];
    FT_Pos   other_blues[10];
    FT_Pos   family_blues[14];
    FT_Pos   family_other_blues[10];
    
    FT_Fixed blue_scale;
    FT_Pos   blue_shift;
    FT_Pos   blue_fuzz;
    FT_Pos   standard_width;
    FT_Pos   standard_height;
    
    FT_Byte  num_snap_widths;
    FT_Byte  num_snap_heights;
    FT_Pos   snap_widths[13];
    FT_Pos   snap_heights[13];
    FT_Bool  force_bold;
    FT_Fixed force_bold_threshold;
    FT_Int   lenIV;
    FT_Int   language_group;
    FT_Fixed expansion_factor;
    FT_Long  initial_random_seed;
    FT_ULong local_subrs_offset;
    FT_Pos   default_width;
    FT_Pos   nominal_width;
  
  } CFF_Private;
  
  typedef struct CFF_Font_
  {
    FT_Stream  stream;
    FT_Memory  memory;
    FT_UInt    num_faces;
    
    FT_Byte    version_major;
    FT_Byte    version_minor;
    FT_Byte    header_size;
    FT_Byte    absolute_offsize;

  
    CFF_Index  name_index;
    CFF_Index  top_dict_index;
    CFF_Index  string_index;
    CFF_Index  global_subrs_index;
  
    /* we don't load the Encoding and CharSet tables */
  
    CFF_Index  charstrings_index;
    CFF_Index  font_dict_index;
    CFF_Index  private_index;
    CFF_Index  local_subrs_index;

    FT_String*     font_name;
    CFF_Top_Dict   top_dict;
    CFF_Private    private_dict;

  } CFF_Font;

#ifdef __cplusplus
  }
#endif

#endif /* T2TYPES_H */
/* END */
