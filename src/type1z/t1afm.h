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

typedef struct T1_Kern_Pair_
{
  T1_UInt   glyph1;
  T1_UInt   glyph2;
  T1_Vector kerning;

} T1_Kern_Pair;


typedef struct T1_AFM_
{
  T1_Int        num_pairs;
  T1_Kern_Pair* kern_pairs;

} T1_AFM;


LOCAL_DEF
T1_Error  T1_Read_AFM( FT_Face    face,
                       FT_Stream  stream );

LOCAL_DEF
void  T1_Done_AFM( FT_Memory  memory,
                   T1_AFM*    afm );

LOCAL_DEF
void  T1_Get_Kerning( T1_AFM*     afm,
                      T1_UInt     glyph1,
                      T1_UInt     glyph2,
                      T1_Vector*  kerning );

#endif /* T1AFM_H */
