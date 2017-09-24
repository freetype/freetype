#ifndef CFFOTYPES_H_
#define CFFOTYPES_H_

#include <ft2build.h>
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_CFF_TYPES_H
#include FT_INTERNAL_TRUETYPE_TYPES_H
#include FT_SERVICE_POSTSCRIPT_CMAPS_H
#include FT_INTERNAL_POSTSCRIPT_HINTS_H


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    CFF_Driver                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an OpenType driver object.                             */
  /*                                                                       */
  typedef struct CFF_DriverRec_*  CFF_Driver;

  typedef TT_Face  CFF_Face;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    CFF_Size                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an OpenType size object.                               */
  /*                                                                       */
  typedef struct  CFF_SizeRec_
  {
    FT_SizeRec  root;
    FT_ULong    strike_index;    /* 0xFFFFFFFF to indicate invalid */

  } CFF_SizeRec, *CFF_Size;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    CFF_GlyphSlot                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an OpenType glyph slot object.                         */
  /*                                                                       */
  typedef struct  CFF_GlyphSlotRec_
  {
    FT_GlyphSlotRec  root;

    FT_Bool          hint;
    FT_Bool          scaled;

    FT_Fixed         x_scale;
    FT_Fixed         y_scale;

  } CFF_GlyphSlotRec, *CFF_GlyphSlot;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    CFF_Internal                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The interface to the `internal' field of `FT_Size'.                */
  /*                                                                       */
  typedef struct  CFF_InternalRec_
  {
    PSH_Globals  topfont;
    PSH_Globals  subfonts[CFF_MAX_CID_FONTS];

  } CFF_InternalRec, *CFF_Internal;


  /*************************************************************************/
  /*                                                                       */
  /* Subglyph transformation record.                                       */
  /*                                                                       */
  typedef struct  CFF_Transform_
  {
    FT_Fixed    xx, xy;     /* transformation matrix coefficients */
    FT_Fixed    yx, yy;
    FT_F26Dot6  ox, oy;     /* offsets        */

  } CFF_Transform;


  /***********************************************************************/
  /*                                                                     */
  /* CFF driver class.                                                   */
  /*                                                                     */
  typedef struct  CFF_DriverRec_
  {
    FT_DriverRec  root;

    FT_UInt   hinting_engine;
    FT_Bool   no_stem_darkening;
    FT_Int    darken_params[8];
    FT_Int32  random_seed;

  } CFF_DriverRec;


FT_END_HEADER


#endif
