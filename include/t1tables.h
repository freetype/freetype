/***************************************************************************/
/*                                                                         */
/*  t1tables.h                                                             */
/*                                                                         */
/*    Basic Type 1/Type 2 tables definitions and interface                 */
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

#ifndef T1TABLES_H
#define T1TABLES_H

#include <freetype.h>

 /* Note that we separate font data in T1_FontInfo and T1_Private structures */
 /* in order to later support multiple master fonts..                        */

 /*************************************************************************
  *
  * <Struct>
  *    T1_FontInfo
  *
  * <Description>
  *    A structure used to model a Type1/Type2 FontInfo dictionary
  *    Note that for multiple-master fonts, each instance has its own
  *    FontInfo.
  *
  */
  
  typedef struct T1_FontInfo
  {
    FT_String*     version;
    FT_String*     notice;
    FT_String*     full_name;
    FT_String*     family_name;
    FT_String*     weight;
    FT_Long        italic_angle;
    FT_Bool        is_fixed_pitch;
    FT_Short       underline_position;
    FT_UShort      underline_thickness;
  
  } T1_FontInfo;


 /*************************************************************************
  *
  * <Struct>
  *    T1_Private
  *
  * <Description>
  *    A structure used to model a Type1/Type2 FontInfo dictionary
  *    Note that for multiple-master fonts, each instance has its own
  *    Private dict.
  *
  */
  
  typedef struct T1_Private
  {

    FT_Int       unique_id;
    FT_Int       lenIV;

    FT_Byte      num_blues;
    FT_Byte      num_other_blues;
    FT_Byte      num_family_blues;
    FT_Byte      num_family_other_blues;

    FT_Short     blue_values[14];
    FT_Short     other_blues[10];

    FT_Short     family_blues      [14];
    FT_Short     family_other_blues[10];

    FT_Fixed     blue_scale;
    FT_Int       blue_shift;
    FT_Int       blue_fuzz;

    FT_UShort    standard_width;
    FT_UShort    standard_height;

    FT_Byte      num_snap_widths;
    FT_Byte      num_snap_heights;
    FT_Bool      force_bold;
    FT_Bool      round_stem_up;

    FT_Short     stem_snap_widths [13];  /* reserve one place for the std */
    FT_Short     stem_snap_heights[13];  /* reserve one place for the std */

    FT_Long      language_group;
    FT_Long      password;

    FT_Short     min_feature[2];
    
  } T1_Private;


  typedef struct CID_FontDict_
  {
    T1_FontInfo   font_info;
    T1_Private    private;

    FT_UInt       num_subrs;
    FT_ULong      subrmap_offset;
    FT_Int        sd_bytes;
  
  } CID_FontDict;

  
  typedef struct CID_Info_
  {
    FT_String*  cid_font_name;
    FT_Fixed    cid_version;
    FT_Int      cid_font_type;
  
    FT_String*  registry;
    FT_String*  ordering;
    FT_Int      supplement;

    FT_ULong    uid_base;

    FT_Int      num_xuid;
    FT_ULong    xuid[16];
    
    
    FT_ULong    cidmap_offset;
    FT_Int      fd_bytes;
    FT_Int      gd_bytes;
    FT_ULong    cid_count;
    
    FT_Int         num_font_dicts;
    CID_FontDict*  font_dicts;
  
  } CID_Info;


#endif /* T1TABLES_H */
