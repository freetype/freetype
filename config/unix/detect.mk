#
# FreeType 2 configuration file to detect a UNIX host platform.
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used modified
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# This will probably change a lost in the future if we are going to use
# Automake/Autoconf...


ifeq ($(PLATFORM),ansi)

# Some Unix systems like *BSD do not have a /etc/inittab so we commented
# the line.. (thanks to Yamano-uchi, Hidetoshi for pointing this out)..
#
# has_inittab := $(strip $(wildcard /etc/inittab))
  has_init := $(strip $(wildcard /sbin/init))
  ifneq ($(has_init),)

    PLATFORM := unix
    COPY     := cp
    DELETE   := rm -f

    # Test whether we're using gcc.  If so, we select the `unix-gcc.mk'
    # configuration file.  Otherwise, the standard `unix.mk' is used which
    # simply calls `cc -c' with no extra arguments.
    #
    # Feel free to add support for other platform specific compilers in this
    # directory (e.g. solaris.mk + changes here to detect the platform).
    #
    ifeq ($(CC),gcc)
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
    endif

    # If `devel' is the requested target, use the development Makefile.
    #
    ifneq ($(findstring devel,$(MAKECMDGOALS)),)
      CONFIG_FILE := unix-dev.mk
      devel: setup
    endif

    setup: std_setup

  endif # test Unix
endif   # test PLATFORM

# EOF
