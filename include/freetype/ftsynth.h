/***************************************************************************/
/*                                                                         */
/*  ftsynth.h                                                              */
/*                                                                         */
/*    FreeType synthesizing code for emboldening and slanting              */
/*    (specification).                                                     */
/*                                                                         */
/*  Copyright 2000 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef FTSYNTH_H
#define FTSYNTH_H

#include <freetype/freetype.h>


#ifdef __cplusplus
  extern "C" {
#endif


  /* This code is completely experimental - use with care!  */
  /* It will probably be completely rewritten in the future */
  /* or even integrated within the library...               */
  FT_EXPORT_DEF( FT_Error )  FT_Embolden_Outline( FT_Face      original,
                                                  FT_Outline*  outline,
                                                  FT_Pos*      advance );

  FT_EXPORT_DEF( FT_Error )  FT_Oblique_Outline( FT_Face      original,
                                                 FT_Outline*  outline,
                                                 FT_Pos*      advance );


#ifdef __cplusplus
  }
#endif


#endif /* FTSYNTH_H */


/* END */
