make_module_list: add_psaux_module

add_psaux_module:
	$(OPEN_DRIVER)psaux_module_class$(CLOSE_DRIVER)
	$(ECHO_DRIVER)psaux     $(ECHO_DRIVER_DESC)Postscript Type 1 & Type 2 helper module$(ECHO_DRIVER_DONE)

# EOF
