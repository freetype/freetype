/****************************************************************************/
/*                                                                          */
/*  The FreeType project -- a free and portable quality TrueType renderer.  */
/*                                                                          */
/*  Copyright 1996-1999 by                                                  */
/*  D. Turner, R.Wilhelm, and W. Lemberg                                    */
/*                                                                          */
/*  blitter.h: Support for blitting of bitmaps with various depth.          */
/*                                                                          */
/****************************************************************************/

#ifndef GRBLIT_H
#define GRBLIT_H

#include "grobjs.h"

  int  grBlitMono( grBitmap*  target,
                   grBitmap*  source,
                   int        x_offset,
                   int        y_offset,
                   grColor    color );


#endif /* GRBLIT_H */
/* End */
