include $(TOP)/builds/unix/unix-def.mk
include $(TOP)/builds/unix/unix-cc.mk

ifdef BUILD_PROJECT

  # Now include the main sub-makefile.  It contains all the rules used to
  # build the library with the previous variables defined.
  #
  include $(TOP)/builds/$(PROJECT).mk


  # The cleanup targets.
  #
  clean_project: clean_project_unix
  distclean_project: distclean_project_unix


  # This final rule is used to link all object files into a single library. 
  # It is part of the system-specific sub-Makefile because not all
  # librarians accept a simple syntax like
  #
  #   librarian library_file {list of object files} 
  #
  $(PROJECT_LIBRARY): $(OBJECTS_LIST)
ifdef CLEAN_LIBRARY  
	  -$(CLEAN_LIBRARY) $(NO_OUTPUT)
endif          
	  $(LINK_LIBRARY)

endif

include $(TOP)/builds/unix/install.mk

# EOF
