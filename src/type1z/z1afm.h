/***************************************************************************
 *
 * t1afm.h  - support for reading Type 1 AFM files
 *
 *
 ***************************************************************************/

#ifndef T1AFM_H
#define T1AFM_H

#include <freetype/internal/ftstream.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/t1types.h>
#include <freetype/internal/t1errors.h>

/* In this version, we only read the kerning table from the */
/* AFM file. We may add support for ligatures a bit later.. */

typedef struct Z1_Kern_Pair_
{
  FT_UInt   glyph1;
  FT_UInt   glyph2;
  FT_Vector kerning;

} Z1_Kern_Pair;


typedef struct Z1_AFM_
{
  FT_Int        num_pairs;
  Z1_Kern_Pair* kern_pairs;

} Z1_AFM;


LOCAL_DEF
FT_Error  Z1_Read_AFM( FT_Face    face,
                       FT_Stream  stream );

LOCAL_DEF
void  Z1_Done_AFM( FT_Memory  memory,
                   Z1_AFM*    afm );

LOCAL_DEF
void  Z1_Get_Kerning( Z1_AFM*     afm,
                      FT_UInt     glyph1,
                      FT_UInt     glyph2,
                      FT_Vector*  kerning );

#endif /* T1AFM_H */
