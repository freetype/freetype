make_module_list: add_sfnt_driver

add_sfnt_driver:
	$(OPEN_DRIVER)sfnt_driver_interface$(CLOSE_DRIVER)
	$(ECHO_DRIVER)sfnt      $(ECHO_DRIVER_DESC)pseudo-driver for TrueType & OpenType formats$(ECHO_DRIVER_DONE)

# EOF
