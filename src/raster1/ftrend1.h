#ifndef FTREND1_H
#define FTREND1_H

#include <freetype/ftrender.h>

  FT_EXPORT_VAR(const FT_Renderer_Class)   ft_raster1_renderer_class;

 /* this renderer is _NOT_ part of the default modules, you'll need */
 /* to register it by hand in your application. It should only be   */
 /* used for backwards-compatibility with FT 1.x anyway..           */
 /*                                                                 */
  FT_EXPORT_VAR(const FT_Renderer_Class)   ft_raster5_renderer_class;

#endif /* FTREND1_H */
