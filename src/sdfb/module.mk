

FTMODULE_H_COMMANDS += SDFB_RENDERER

define SDFB_RENDERER
$(OPEN_DRIVER) FT_Renderer_Class, ft_sdfb_renderer_class $(CLOSE_DRIVER)
$(ECHO_DRIVER)sdfb      $(ECHO_DRIVER_DESC)signed distance field converter$(ECHO_DRIVER_DONE)
endef

#EOF
