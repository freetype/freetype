#
# FreeType 2 configuration file for CygWin host platform.
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


include $(TOP)/builds/cygwin/cygwin-def.mk
include $(TOP)/builds/cygwin/cygwin-cc.mk

ifdef BUILD_PROJECT

  # Now include the main sub-makefile.  It contains all the rules used to
  # build the library with the previous variables defined.
  #
  include $(TOP)/builds/$(PROJECT).mk


  # The cleanup targets.
  #
  clean_project: clean_project_cygwin
  distclean_project: distclean_project_cygwin


  # This final rule is used to link all object files into a single library.
  # this is compiler-specific
  #
  $(PROJECT_LIBRARY): $(OBJECTS_LIST)
  ifdef CLEAN_LIBRARY
	  -$(CLEAN_LIBRARY) $(NO_OUTPUT)
  endif
	  $(LINK_LIBRARY)

endif

include $(TOP)/builds/cygwin/install.mk

# EOF
