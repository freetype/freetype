make_module_list: add_psnames_driver

add_psnames_driver:
	$(OPEN_DRIVER)psnames_driver_interface$(CLOSE_DRIVER)
	$(ECHO_DRIVER)psnames   $(ECHO_DRIVER_DESC)Postscript & Unicode Glyph name handling$(ECHO_DRIVER_DONE)

# EOF
