# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.
#
# This file is used to compile FreeType on the GNU Hurd operating system
# the _only_ difference with builds/unix/detect.mk is that we look for
# a file named "/hurd/auth" instead of "/sbin/init"
#

.PHONY: devel lcc setup unix

ifeq ($(PLATFORM),ansi)

  is_hurd := $(strip $(wildcard /hurd/auth))
  ifneq ($(is_hurd),)

  PLATFORM := unix
  COPY     := cp
  DELETE   := rm -f


  # If `devel' is the requested target, we use a special configuration
  # file named `unix-dev.mk'.  It disables optimization and libtool.
  #
  ifneq ($(findstring devel,$(MAKECMDGOALS)),)
    CONFIG_FILE := unix-dev.mk
    CC          := gcc
    devel: setup
  else

    # If `lccl' is the requested target, we use a special configuration
    # file named `unix-lcc.mk'.  It disables libtool for LCC
    #
    ifneq ($(findstring lcc,$(MAKECMDGOALS)),)
      CONFIG_FILE := unix-lcc.mk
      CC          := lcc
      lcc: setup
    else
      # If a Unix platform is detected, the configure script is called and
      # `unix-def.mk' together with `unix-cc.mk' is created.
      #
      # Arguments to `configure' should be in the CFG variable.  Example:
      #
      #   make CFG="--prefix=/usr --disable-static"
      #
      # If you need to set CFLAGS or LDFLAGS, do it here also.
      #
      # Feel free to add support for other platform specific compilers in
      # this directory (e.g. solaris.mk + changes here to detect the
      # platform).
      #
       CONFIG_FILE := unix.mk
       setup: unix-def.mk
       unix: setup
     endif
   endif

   setup: std_setup

   unix-def.mk: $(TOP)/builds/unix/unix-def.in
           cd builds/unix; $(USE_CFLAGS) ./configure $(CFG)

  endif # test Unix
endif   # test PLATFORM

# EOF


