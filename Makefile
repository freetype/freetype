#
# FreeType 2 build system -- top-level Makefile
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT. By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# This file is designed for GNU Make, do not use it with another Make tool!
#
# It works as follows:
#
# - When invoked for the first time, this Makefile will include the rules
#   found in `freetype/config/detect.mk'.  They are in charge of detecting
#   the current platform.
#
#   A summary of the detection will be displayed, and the file `config.mk'
#   will be created in the current directory.
#
# - When invoked later, this Makefile will include the rules found in
#   `config.mk'.  This sub-Makefile will define some system-specific
#   variables (like compiler, compilation flags, object suffix, etc.), then
#   include the rules found in `freetype/config/freetype.mk', used to build
#   the library.
#
# See the comments in `config/detect.mk' and `config/freetype.mk' for more
# details on host platform detection and library builds.


.PHONY: setup

# The variable TOP holds the path to the topmost directory in the FreeType
# engine source hierarchy.  If it is not defined, default it to `.'.
#
ifndef TOP
  TOP := .
endif

CONFIG_MK := config.mk

# If no configuration sub-makefile is present, or if `setup' is the target
# to be built, run the auto-detection rules to figure out which
# configuration rules file to use.
#
# Note that the configuration file is put in the current directory, which is
# not necessarily $(TOP).

# If `config.mk' is not present, set `check_platform'.
#
ifeq ($(wildcard $(CONFIG_MK)),)
  check_platform := 1
endif

# If `setup' is one of the targets requested, set `check_platform'.
#
ifneq ($(findstring setup,$(MAKECMDGOALS)),)
  check_platform := 1
endif

# Include the automatic host platform detection rules when we need to
# check the platform.
#
ifdef check_platform

  all: setup

  # If the module list $(FT_MODULE_LIST) file is not present, generate it.
  #
  modules: make_module_list setup

  include $(TOP)/builds/detect.mk
  include $(TOP)/builds/modules.mk

  ifeq ($(wildcard $(FT_MODULE_LIST)),)
    setup: make_module_list
  endif

  # IMPORTANT:
  #
  # `setup' must be defined by the host platform detection rules to create
  # the `config.mk' file in the current directory.

else

  # A configuration sub-Makefile is present -- simply run it.
  #
  all: single

  modules: make_module_list

  BUILD_FREETYPE := yes
  include $(CONFIG_MK)

endif # test check_platform

# EOF
