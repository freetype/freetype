make_module_list: add_renderer_module

# XXX: important, the standard renderer *MUST* be first on this list..
#
add_renderer_module:
	$(OPEN_DRIVER)ft_standard_renderer_class$(CLOSE_DRIVER)
	$(ECHO_DRIVER)standard  $(ECHO_DRIVER_DESC)standard outline renderer module$(ECHO_DRIVER_DONE)
	$(OPEN_DRIVER)ft_smooth_renderer_class$(CLOSE_DRIVER)
	$(ECHO_DRIVER)smooth    $(ECHO_DRIVER_DESC)smooth outline renderer module$(ECHO_DRIVER_DONE)

# EOF
