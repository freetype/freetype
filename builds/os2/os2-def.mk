#
# FreeType 2 OS/2 specific definitions
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


DELETE   := del
HOSTSEP  := $(strip \ )
BUILD    := $(TOP)$(SEP)builds$(SEP)os2
PLATFORM := os2

# except for GCC+emx on OS/2
ifndef SEP
  SEP    := $(HOSTSEP)
endif


# The directory where all object files are placed.
#
# This lets you build the library in your own directory with something like
#
#   set TOP=.../path/to/freetype2/top/dir...
#   set OBJ_DIR=.../path/to/obj/dir
#   make -f %TOP%/Makefile setup [options]
#   make -f %TOP%/Makefile
#
ifndef OBJ_DIR
  OBJ_DIR := $(TOP)$(SEP)obj
endif


# The directory where all library files are placed.
#
# By default, this is the same as $(OBJ_DIR); however, this can be changed
# to suit particular needs.
#
LIB_DIR := $(OBJ_DIR)

# The name of the final library file.  Note that the DOS-specific Makefile
# uses a shorter (8.3) name.
#
LIBRARY := $(PROJECT)


# The NO_OUTPUT macro is used to ignore the output of commands.
#
NO_OUTPUT = 2> nul


ifdef BUILD_LIBRARY

  # Now include the main sub-makefile.  It contains all the rules used to
  # build the library with the previous variables defined.
  #
  include $(TOP)/builds/$(PROJECT).mk

  # The cleanup targets.
  #
  clean_project: clean_project_dos
  distclean_project: distclean_project_dos

  # This final rule is used to link all object files into a single library.
  # It is part of the system-specific sub-Makefile because not all
  # librarians accept a simple syntax like
  #
  #   librarian library_file {list of object files}
  #
  $(PROJECT_LIBRARY): $(OBJECTS_LIST)
	  -$(CLEAN_LIBRARY) $(NO_OUTPUT)
	  $(LINK_LIBRARY)

endif

# EOF
