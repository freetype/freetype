#
# FreeType 2 configuration file to detect a UNIX host platform.
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

  has_init := $(strip $(wildcard /sbin/init))
  ifneq ($(has_init),)

    PLATFORM := unix
    COPY     := cp
    DELETE   := rm -f

    # Test whether we are using gcc.  If so, we select the `unix-gcc.mk'
    # configuration file.  Otherwise, the configure script is called and
    # `unix.mk' is created.
    #
    # The use of the configure script can be forced by saying `make unix'.
    #
    # Feel free to add support for other platform specific compilers in this
    # directory (e.g. solaris.mk + changes here to detect the platform).
    #
    ifneq ($(findstring unix,$(MAKECMDGOALS)),)
      CONFIG_FILE := unix.mk
      setup: unix.mk
      unix: setup
    else
      ifeq ($(firstword $(CC)),gcc)
        is_gcc := 1
      else
        ifneq ($(findstring gcc,$(shell $(CC) -v 2>&1)),)
          is_gcc := 1
        endif
      endif

      ifdef is_gcc
        CONFIG_FILE := unix-gcc.mk
      else
        CONFIG_FILE := unix.mk
        setup: unix.mk
      endif

      # If `devel' is the requested target, use the development Makefile.
      #
      ifneq ($(findstring devel,$(MAKECMDGOALS)),)
        CONFIG_FILE := unix-dev.mk
        devel: setup
      endif
    endif

    setup: std_setup

    unix.mk: builds/unix/unix.in
	    cd builds/unix; ./configure

  endif # test Unix
endif   # test PLATFORM

# EOF
