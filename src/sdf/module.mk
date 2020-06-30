

FTMODULE_H_COMMANDS += SDF_RENDERER

define SDF_RENDERER
$(OPEN_DRIVER) FT_Renderer_Class, ft_sdf_renderer_class $(CLOSE_DRIVER)
$(ECHO_DRIVER)sdf       $(ECHO_DRIVER_DESC)signed distance field renderer$(ECHO_DRIVER_DONE)
endef

#EOF
