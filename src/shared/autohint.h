/***************************************************************************/
/*                                                                         */
/*  autohint.h                                                             */
/*                                                                         */
/*    High-level `autohint" driver interface (specification)               */
/*                                                                         */
/*  Copyright 1996-2000 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/*                                                                         */
/*  The auto-hinter is used to load and automatically hint glyphs          */
/*  when a format-specific hinter isn't available..                        */
/*                                                                         */
/***************************************************************************/

#ifndef AUTOHINT_H
#define AUTOHINT_H

#include <freetype.h>

 /***********************************************************************
  *
  * <FuncType>
  *    FT_AutoHinter_Init_Func
  *
  * <Description>
  *    Each face can have its own auto-hinter object. This function
  *    is used to initialise it. Its role is to perform a few statistics
  *    on the face's content in order to compute global metrics like
  *    blue zones and standard width/heights.
  *
  * <Input>
  *    face  :: handle to the face.
  *
  * <Note>
  *    this function will call FT_Load_Glyph several times in order to
  *    compute various statistics on the face's glyphs. This means that
  *    the face handle must be valid when calling it.
  *
  *    This function set the "hinter" and "hinter_len" fields of 'face'.
  */
  typedef  FT_Error    (*FT_AutoHinter_Init_Func)( FT_Face  face );


 /***********************************************************************
  *
  * <FuncType>
  *    FT_AutoHinter_Done_Func
  *
  * <Description>
  *    Each face can have its own auto-hinter object. This function
  *    is used to finalise and destroy it.
  *
  * <Input>
  *    face  :: handle to the face.
  *
  * <Note>
  *    This function clears the "hinter" and "hinter_len" fields of
  *    "face". However, the face object is still valid and can be used
  *    to load un-hinted glyphs..
  *
  */
  typedef  void        (*FT_AutoHinter_Done_Func)( FT_Face  face );


 /***********************************************************************
  *
  * <FuncType>
  *    FT_AutoHinter_Load_Func
  *
  * <Description>
  *    This function is used to load, scale and automatically hint a glyph
  *    from a given face.
  *
  * <Input>
  *    face        :: handle to the face.
  *    glyph_index :: glyph index
  *    load_flags  :: load flags
  *
  * <Note>
  *    This function is capable of loading composite glyphs by hinting
  *    each sub-glyph independently (which improves quality).
  *
  *    It will call the font driver with FT_Load_Glyph, with FT_LOAD_NO_SCALE
  *    set..
  *
  */
  typedef  FT_Error    (*FT_AutoHinter_Load_Func)( FT_Face   face,
                                                   FT_UInt   glyph_index,
   					           FT_ULong  load_flags );

 /***********************************************************************
  *
  * <Struct>
  *    FT_AutoHinter_Interface
  *
  * <Description>
  *    The auto-hinter module's interface.
  *
  */
  typedef struct FT_AutoHinter_Interface
  {
    FT_AutoHinter_Init_Func   init_autohinter;
    FT_AutoHinter_Done_Func   done_autohinter;
    FT_AutoHinter_Load_Func   load_glyph;

  } FT_AutoHinter_Interface;					 

#endif /* AUTOHINT_H */


