#ifndef FTSYNTH_H
#define FTSYNTH_H

#include <freetype/freetype.h>

  /* this code is completely experimental - use with care   */
  /* it will probably be completely rewritten in the future */
  /* or even integrated within the library...               */
  FT_EXPORT_DEF(FT_Error)  FT_Embolden_Outline( FT_Face      original,
                                                FT_Outline*  outline,
                                                FT_Pos*      advance );

  FT_EXPORT_DEF(FT_Error)  FT_Oblique_Outline( FT_Face      original,
                                               FT_Outline*  outline,
                                               FT_Pos*      advance );


#endif /* FTEMBOLD_H */
