/***************************************************************************/
/*                                                                         */
/*  ttdriver.h                                                             */
/*                                                                         */
/*    High-level TrueType driver interface (specification).                */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef TTDRIVER_H
#define TTDRIVER_H

#include <freetype.h>
#include <ftdriver.h>
#include <ttobjs.h>
#include <tterrors.h>
#include <ttnameid.h>


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TTDriver_getFontData                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns either a single font table or the whole font file into     */
  /*    caller's memory.  This function mimics the GetFontData() API       */
  /*    function found in Windows.                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the source TrueType face object.             */
  /*                                                                       */
  /*    tag    :: A 32-bit integer used to name the table you want to      */
  /*              read.  Use the macro MAKE_TT_TAG (defined in freetype.h) */
  /*              to create one.  Use the value 0 if you want to access    */
  /*              the whole file instead.                                  */
  /*                                                                       */
  /*    offset :: The offset from the start of the table or file from      */
  /*              which you want to read bytes.                            */
  /*                                                                       */
  /*    buffer :: The address of the target/read buffer where data will be */
  /*              copied.                                                  */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    length :: The length in bytes of the data to read.  If it is set   */
  /*              to 0 when this function is called, it will return        */
  /*              immediately, setting the value of `length' to the        */
  /*              requested table's size (or the whole font file if the    */
  /*              tag is 0).  It is thus possible to allocate and read an  */
  /*              arbitrary table in two successive calls.                 */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  typedef TT_Error  (*TTDriver_getFontData)( TT_Face   face,
                                             TT_ULong  tag,
                                             TT_ULong  offset,
                                             void*     buffer,
                                             TT_Long*  length );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TTDriver_getFaceWidths                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns the widths and/or heights of a given range of glyph from   */
  /*    a face.                                                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: A handle to the source FreeType face object.        */
  /*                                                                       */
  /*    first_glyph :: The first glyph in the range.                       */
  /*                                                                       */
  /*    last_glyph  :: The last glyph in the range.                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    widths      :: The address of the table receiving the widths       */
  /*                   expressed in font units (UShorts).  Set this        */
  /*                   parameter to NULL if you're not interested in these */
  /*                   values.                                             */
  /*                                                                       */
  /*    heights     :: The address of the table receiving the heights      */
  /*                   expressed in font units (UShorts).  Set this        */
  /*                   parameter to NULL if you're not interested in these */
  /*                   values.                                             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0 means success.                                      */
  /*                                                                       */
  typedef TT_Error  (*TTDriver_getFaceWidths)( TT_Face     face,
                                               TT_UShort   first_glyph,
                                               TT_UShort   last_glyph,
                                               TT_UShort*  widths,
                                               TT_UShort*  heights );



  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_DriverInterface                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The TrueType-specific interface of this driver.  Note that some of */
  /*    the methods defined here are optional, as they're only used for    */
  /*    for specific tasks of the driver.                                  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    get_font_data   :: See the declaration of TTDriver_getFontData().  */
  /*    get_face_widths :: See the declaration of                          */
  /*                       TTDriver_getFaceWidths().                       */
  /*                                                                       */
  typedef struct  TT_DriverInterface_
  {
    TTDriver_getFontData    get_font_data;
    TTDriver_getFaceWidths  get_face_widths;

  } TT_DriverInterface;


  EXPORT_DEF
  const FT_DriverInterface  tt_driver_interface;


  EXPORT_DEF
  const TT_DriverInterface  tt_format_interface;



/*************************************************************************
 *
 *  Here is a template of the code that should appear in each
 *  font driver's _interface_ file (the one included by "ftinit.c").
 *
 *  It is used to build, at compile time, a simple linked list of
 *  the interfaces of the drivers which have been #included in 
 *  "ftinit.c". See the source code of the latter file for details
 *
 *  (Note that this is only required when you want your driver included
 *   in the set of default drivers loaded by FT_Init_FreeType. Other
 *   drivers can still be added manually at runtime with FT_Add_Driver.
 *
 * {
 *   #ifdef FTINIT_DRIVER_CHAIN
 *
 *   static
 *   const FT_DriverChain  ftinit_<FORMAT>_driver_chain =
 *   {
 *     FT_INIT_LAST_DRIVER_CHAIN,
 *     &<FORMAT>_driver_interface
 *   };
 * 
 *   #undef  FT_INIT_LAST_DRIVER_CHAIN
 *   #define FT_INIT_LAST_DRIVER_CHAIN   &ftinit_<FORMAT>_driver_chain
 *
 *   #endif 
 * }
 *
 *  replace <FORMAT> with your driver's prefix
 *
 *************************************************************************/


#ifdef FTINIT_DRIVER_CHAIN

  static
  const FT_DriverChain  ftinit_tt_driver_chain =
  {
    FT_INIT_LAST_DRIVER_CHAIN,
    &tt_driver_interface
  };

#undef  FT_INIT_LAST_DRIVER_CHAIN
#define FT_INIT_LAST_DRIVER_CHAIN   &ftinit_tt_driver_chain

#endif /* FTINIT_DRIVER_CHAIN */ 



#endif /* TTDRIVER_H */


/* END */
