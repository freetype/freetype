#ifndef __FT_XFREE86_H__
#define __FT_XFREE86_H__

#include <ft2build.h>
#include FT_FREETYPE_H

FT_BEGIN_HEADER

  /* this comment is intentionally disabled for now, to prevent this       */
  /* function from appearing in the API Reference.                         */

  /*@***********************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_X11_Font_Format                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns a string describing the format of a given face as a X11    */
  /*    FONT_PROPERTY. It should only be used by FreeType 2 font backend   */
  /*    of the XFree86 font server.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: input face handle.                                         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    font format string. NULL in case of error.                         */
  /*                                                                       */
  FT_EXPORT_DEF( const char* )
  FT_Get_X11_Font_Format( FT_Face    face );

 /* */

FT_END_HEADER

#endif /* __FT_XFREE86_H__ */
