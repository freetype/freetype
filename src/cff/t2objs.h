/***************************************************************************/
/*                                                                         */
/*  t2objs.h                                                               */
/*                                                                         */
/*    Objects manager (specification).                                     */
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


#ifndef T2OBJS_H
#define T2OBJS_H


#include <freetype/internal/ftobjs.h>
#include <freetype/internal/t2types.h>
#include <freetype/internal/t2errors.h>


#ifdef __cplusplus
  extern "C" {
#endif


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    T2_Driver                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an OpenType driver object.                             */
  /*                                                                       */
  typedef struct T2_DriverRec_*  T2_Driver;

  typedef TT_Face  T2_Face;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    T2_Size                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an OpenType size object.                               */
  /*                                                                       */
  typedef FT_Size  T2_Size;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    T2_GlyphSlot                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an OpenType glyph slot object.                         */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This is a direct typedef of FT_GlyphSlot, as there is nothing      */
  /*    specific about the OpenType glyph slot.                            */
  /*                                                                       */
  typedef FT_GlyphSlot  T2_GlyphSlot;



  /*************************************************************************/
  /*                                                                       */
  /* Subglyph transformation record.                                       */
  /*                                                                       */
  typedef struct  T2_Transform_
  {
    FT_Fixed    xx, xy;     /* transformation matrix coefficients */
    FT_Fixed    yx, yy;
    FT_F26Dot6  ox, oy;     /* offsets        */

  } T2_Transform;


  /***********************************************************************/
  /*                                                                     */
  /* TrueType driver class.                                              */
  /*                                                                     */
  typedef struct  T2_DriverRec_
  {
    FT_DriverRec    root;
    FT_GlyphZone    zone;     /* glyph loader points zone */

    void*           extension_component;

  } T2_DriverRec;


 /*************************************************************************/
 /*  Face Funcs                                                           */

  LOCAL_DEF FT_Error  T2_Init_Face( FT_Stream      stream,
                                    T2_Face        face,
                                    TT_Int         face_index,
                                    TT_Int         num_params,
                                    FT_Parameter*  params );

  LOCAL_DEF void      T2_Done_Face( T2_Face  face );


 /*************************************************************************/
 /*  Size funcs                                                           */

  LOCAL_DEF FT_Error  T2_Init_Size ( T2_Size  size );
  LOCAL_DEF void      T2_Done_Size ( T2_Size  size );
  LOCAL_DEF FT_Error  T2_Reset_Size( T2_Size  size );


 /*************************************************************************/
 /*  GlyphSlot funcs                                                      */

  LOCAL_DEF FT_Error  T2_Init_GlyphSlot( T2_GlyphSlot  slot );
  LOCAL_DEF void      T2_Done_GlyphSlot( T2_GlyphSlot  slot );


 /*************************************************************************/
 /*  Driver funcs                                                         */

  LOCAL_DEF  FT_Error  T2_Init_Driver( T2_Driver  driver );
  LOCAL_DEF  void      T2_Done_Driver( T2_Driver  driver );


#ifdef __cplusplus
  }
#endif


#endif /* T2OBJS_H */


/* END */
