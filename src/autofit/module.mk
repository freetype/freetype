#
# FreeType 2 auto-fitter module definition
#


make_module_list: add_autofit_module

add_autofit_module:
	$(OPEN_DRIVER)autofit_module_class$(CLOSE_DRIVER)
	$(ECHO_DRIVER)autofit  $(ECHO_DRIVER_DESC)automatic hinting module$(ECHO_DRIVER_DONE)

# EOF
