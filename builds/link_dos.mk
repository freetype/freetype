#
#  Link instructions for Dos-like systems (Dos, Win32, OS/2)
#

ifdef BUILD_PROJECT

  # Now include the main sub-makefile.  It contains all the rules used to
  # build the library with the previous variables defined.
  #
  include $(TOP)/builds/$(PROJECT).mk

  # The cleanup targets.
  #
  clean_project: clean_project_dos
  distclean_project: distclean_project_dos

  # This final rule is used to link all object files into a single library. 
  # this is compiler-specific
  #
  $(PROJECT_LIBRARY): $(OBJECTS_LIST)
ifdef CLEAN_LIBRARY  
	-$(CLEAN_LIBRARY) $(NO_OUTPUT)
endif          
	$(LINK_LIBRARY)

endif

# EOF
