#ifndef __PFR_GLYPH_LOAD_H__
#define __PFR_GLYPH_LOAD_H__

#include "pfrtypes.h"

FT_BEGIN_HEADER

  FT_LOCAL( void )
  pfr_glyph_init( PFR_Glyph       glyph,
                  FT_GlyphLoader  loader );

  FT_LOCAL( void )
  pfr_glyph_done( PFR_Glyph  glyph );


  FT_LOCAL( FT_Error )
  pfr_glyph_load( PFR_Glyph  glyph,
                  FT_Stream  stream,
                  FT_ULong   gps_offset,
                  FT_ULong   offset,
                  FT_ULong   size );


FT_END_HEADER


#endif /* __PFR_GLYPH_LOAD_H__ */
