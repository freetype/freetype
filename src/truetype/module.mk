make_module_list: add_truetype_driver

add_truetype_driver:
	$(OPEN_DRIVER)tt_driver_interface$(CLOSE_DRIVER)
	$(ECHO_DRIVER)truetype  $(ECHO_DRIVER_DESC)Windows/Mac font files with extension *.ttf or *.ttc$(ECHO_DRIVER_DONE)

# EOF
