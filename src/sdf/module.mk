

FTMODULE_H_COMMANDS += SDF_RENDERER
FTMODULE_H_COMMANDS += BSDF_RENDERER

define SDF_RENDERER
$(OPEN_DRIVER) FT_Renderer_Class, ft_sdf_renderer_class $(CLOSE_DRIVER)
$(ECHO_DRIVER)sdf       $(ECHO_DRIVER_DESC)signed distance field renderer$(ECHO_DRIVER_DONE)
endef

define BSDF_RENDERER
$(OPEN_DRIVER) FT_Renderer_Class, ft_bitmap_sdf_renderer_class $(CLOSE_DRIVER)
$(ECHO_DRIVER)bsdf      $(ECHO_DRIVER_DESC)bitmap to signed distance field converter$(ECHO_DRIVER_DONE)
endef

#EOF
