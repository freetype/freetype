
#ifndef FTDENSEREND_H_
#define FTDENSEREND_H_


#include <freetype/ftmodapi.h>
#include <freetype/ftrender.h>
#include <freetype/internal/ftobjs.h>

FT_BEGIN_HEADER

/**************************************************************************
 *
 * @renderer:
 *   ft_dense_renderer_class
 *
 * @description:
 *   Renderer to convert @FT_Outline to bitmaps.
 *
 */
FT_DECLARE_RENDERER( ft_dense_renderer_class )

FT_END_HEADER

#endif /* FTDENSEREND_H_ */

/* END */
