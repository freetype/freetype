#ifndef RENDERER_H
#define RENDERER_H

#include <freetype/ftrender.h>

#ifndef FT_CONFIG_OPTION_NO_STD_RASTER
  FT_EXPORT_VAR(const FT_Renderer_Class)   ft_std_renderer_class;
#endif

#ifndef FT_CONFIG_OPTION_NO_SMOOTH_RASTER
  FT_EXPORT_VAR(const FT_Renderer_Class)   ft_smooth_renderer_class;
#endif

#endif /* RENDERER_H */
