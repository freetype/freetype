#
# FreeType 2 configuration file to detect a CygWin host platform.
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


ifeq ($(PLATFORM),ansi)

  # Detecting Windows NT is easy, as the OS variable must be defined and
  # contains `Windows_NT'.  Untested with Windows 2K, but I guess it should
  # work...
  #
  ifeq ($(OS),Windows_NT)

    # Check if we are running on a CygWin system by checking the OSTYPE
    # variable.
    ifeq ($(OSTYPE),cygwin)
      is_cygwin := 1
    endif

  # We test for the COMSPEC environment variable, then run the `ver'
  # command-line program to see if its output contains the word `Windows'.
  #
  # If this is true, we are running a win32 platform (or an emulation).
  #
  else
    ifeq ($(OSTYPE),cygwin)
      ifdef COMSPEC
        is_cygwin := $(findstring Windows,$(strip $(shell ver)))
      endif
    endif  # test CygWin
  endif  # test NT

  ifdef is_cygwin

    PLATFORM := cygwin
    COPY     := cp
    DELETE   := rm -f

    # If `devel' is the requested target, we use a special configuration
    # file named `cygwin-dev.mk'.  It disables optimization and libtool.
    #
    ifneq ($(findstring devel,$(MAKECMDGOALS)),)
      CONFIG_FILE := cygwin-dev.mk
      CC          := gcc
      devel: setup
    else
      # If a CygWin platform is detected, the configure script is called and
      # `cygwin.mk' is created.
      #
      # Arguments to `configure' should be in the CFG variable.  Example:
      #
      #   make CFG="--prefix=/usr --disable-static"
      #
      # If you need to set CFLAGS or LDFLAGS, do it here also.
      #
      CONFIG_FILE := cygwin.mk
      setup: cygwin-def.mk
      cygwin: setup
    endif

    setup: std_setup

    cygwin-def.mk: builds/cygwin/cygwin-def.in
	    cd builds/cygwin; \
            $(USE_CFLAGS) CONFIG_SHELL=/bin/bash ./configure $(CFG)

  endif # test CygWin
endif   # test PLATFORM

# EOF
