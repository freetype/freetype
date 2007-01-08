#ifndef _FT_GASP_H_
#define _FT_GASP_H_

#include <ft2build.h>
#include FT_FREETYPE_H

/**
 * @enum: FT_GASP_XXX
 *
 * @description:
 *   a list of values and/or bit-flags returned by the
 *   @FT_Get_Gasp function.
 *
 * @values:
 *   FT_GASP_NO_TABLE ::
 *     this special value means that there is no GASP table
 *     in this face. It's up to the client to decide what to
 *     do
 *
 *   FT_GASP_DO_GRIDFIT ::
 *     indicates that grid-fitting/hinting should be
 *     performed at the specified ppem. This *really*
 *     means TrueType bytecode interpretation
 *
 *   FT_GASP_DO_GRAY ::
 *     indicates that anti-aliased rendering should be
 *     performed at the specified ppem
 *
 *   FT_GASP_SYMMETRIC_SMOOTHING ::
 *     indicates that smoothing along multiple axis
 *     must be used with ClearType.
 *
 *   FT_GASP_SYMMETRIC_GRIDFIT ::
 *     indicates that grid-fitting must be used with
 *     ClearType's symmetric smoothing
 */
#define  FT_GASP_NO_TABLE               -1
#define  FT_GASP_DO_GRIDFIT           0x01
#define  FT_GASP_DO_GRAY              0x02
#define  FT_GASP_SYMMETRIC_SMOOTHING  0x08
#define  FT_GASP_SYMMETRIC_GRIDFIT    0x10

 /**
  * @func: FT_Get_Gasp
  *
  * @description:
  *   read the GASP table from a TrueType or OpenType font file
  *   and return the entry corresponding to a given character
  *   pixel size
  *
  * @input:
  *   face :: source face handle
  *   ppem :: vertical character pixel size
  *
  * @return:
  *   bit flags, or @FT_GASP_NO_TABLE is there is no GASP table
  *   in the face.
  */
  FT_EXPORT( FT_Int )
  FT_Get_Gasp( FT_Face    face,
               FT_UInt    ppem );

/* */

#endif /* _FT_GASP_H_ */
