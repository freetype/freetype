/***************************************************************************
 *
 * t1afm.h  - support for reading Type 1 AFM files
 *
 *
 ***************************************************************************/

#ifndef T1AFM_H
#define T1AFM_H

#include <freetype/internal/ftobjs.h>

/* In this version, we only read the kerning table from the */
/* AFM file. We may add support for ligatures a bit later.. */

typedef struct T1_Kern_Pair_
{
  FT_UInt   glyph1;
  FT_UInt   glyph2;
  FT_Vector kerning;

} T1_Kern_Pair;


typedef struct T1_AFM_
{
  FT_Int        num_pairs;
  T1_Kern_Pair* kern_pairs;

} T1_AFM;

#if 0

LOCAL_DEF
FT_Error  CID_Read_AFM( FT_Face   face,
                        FT_Stream stream );

LOCAL_DEF
void      CID_Done_AFM( FT_Memory  memory,
                        T1_AFM*    afm );

LOCAL_DEF
void  CID_Get_Kerning( T1_AFM*     afm,
                       FT_UInt     glyph1,
                       FT_UInt     glyph2,
                       FT_Vector*  kerning );
#endif

#endif /* T1AFM_H */
