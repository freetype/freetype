#ifndef __FT_INTERNAL_GLYPH_LOADER_H__
#define __FT_INTERNAL_GLYPH_LOADER_H__

#include <ft2build.h>
#include FT_FREETYPE_H

FT_BEGIN_HEADER

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                   G L Y P H   L O A D E R                       ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_GlyphLoader                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The glyph loader is an internal object used to load several glyphs */
  /*    together (for example, in the case of composites).                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The glyph loader implementation is not part of the high-level API, */
  /*    hence the forward structure declaration.                           */
  /*                                                                       */
  typedef struct FT_GlyphLoaderRec_*  FT_GlyphLoader ;


#define FT_SUBGLYPH_FLAG_ARGS_ARE_WORDS          1
#define FT_SUBGLYPH_FLAG_ARGS_ARE_XY_VALUES      2
#define FT_SUBGLYPH_FLAG_ROUND_XY_TO_GRID        4
#define FT_SUBGLYPH_FLAG_SCALE                   8
#define FT_SUBGLYPH_FLAG_XY_SCALE             0x40
#define FT_SUBGLYPH_FLAG_2X2                  0x80
#define FT_SUBGLYPH_FLAG_USE_MY_METRICS      0x200


  enum
  {
    FT_GLYPH_OWN_BITMAP = 1
  };


  struct  FT_SubGlyph_
  {
    FT_Int     index;
    FT_UShort  flags;
    FT_Int     arg1;
    FT_Int     arg2;
    FT_Matrix  transform;
  };


  typedef struct  FT_GlyphLoadRec_
  {
    FT_Outline    outline;       /* outline             */
    FT_Vector*    extra_points;  /* extra points table  */
    FT_UInt       num_subglyphs; /* number of subglyphs */
    FT_SubGlyph*  subglyphs;     /* subglyphs           */

  } FT_GlyphLoadRec, *FT_GlyphLoad;


  typedef struct  FT_GlyphLoaderRec_
  {
    FT_Memory        memory;
    FT_UInt          max_points;
    FT_UInt          max_contours;
    FT_UInt          max_subglyphs;
    FT_Bool          use_extra;

    FT_GlyphLoadRec  base;
    FT_GlyphLoadRec  current;

    void*            other;            /* for possible future extension? */

  } FT_GlyphLoaderRec;


 /* create new empty glyph loader */
  FT_BASE( FT_Error )
  FT_GlyphLoader_New( FT_Memory         memory,
                      FT_GlyphLoader   *aloader );

 /* add an extra points table to a glyph loader */
  FT_BASE( FT_Error )
  FT_GlyphLoader_Create_Extra( FT_GlyphLoader   loader );

 /* destroy a glyph loader */
  FT_BASE( void )
  FT_GlyphLoader_Done( FT_GlyphLoader   loader );

 /* reset a glyph loader (frees everything int it) */
  FT_BASE( void )
  FT_GlyphLoader_Reset( FT_GlyphLoader   loader );

 /* rewind a glyph loader */
  FT_BASE( void )
  FT_GlyphLoader_Rewind( FT_GlyphLoader   loader );

 /* check that there is enough room to add 'n_points' and 'n_contours' */
 /* to the glyph loader..                                              */
  FT_BASE( FT_Error )
  FT_GlyphLoader_Check_Points( FT_GlyphLoader   loader,
                               FT_UInt          n_points,
                               FT_UInt          n_contours );

 /* check that there is enough room to add 'n_subs' sub-glyphs to */
 /* a glyph loader                                                */
  FT_BASE( FT_Error )
  FT_GlyphLoader_Check_Subglyphs( FT_GlyphLoader   loader,
                                  FT_UInt          n_subs );

 /* prepare a glyph loader, i.e. empty the current glyph */
  FT_BASE( void )
  FT_GlyphLoader_Prepare( FT_GlyphLoader   loader );

 /* add the current glyph to the base glyph */
  FT_BASE( void )
  FT_GlyphLoader_Add( FT_GlyphLoader   loader );

 /* copy points from one glyph loader to another */
  FT_BASE( FT_Error )
  FT_GlyphLoader_Copy_Points( FT_GlyphLoader   target,
                              FT_GlyphLoader   source );

 /* */

FT_END_HEADER

#endif /* __FT_INTERNAL_GLYPH_LOADER_H__ */
