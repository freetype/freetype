make_module_list: add_type1z_driver

add_type1z_driver:
	$(OPEN_DRIVER)t1z_driver_interface$(CLOSE_DRIVER)
	$(ECHO_DRIVER)type1z    $(ECHO_DRIVER_DESC)Postscript font files with extension *.pfa or *.pfb$(ECHO_DRIVER_DONE)

# EOF
