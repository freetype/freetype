#
# FreeType 2 configuration file to detect a Win32 host platform.
#


# Copyright 1996-2000, 2003, 2004 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


.PHONY: setup


ifeq ($(PLATFORM),ansi)

  # Detecting Windows NT is easy, as the OS variable must be defined and
  # contains `Windows_NT'.  This also works with W2K, XP, and Windows 98.
  #
  ifeq ($(OS),Windows_NT)

    is_windows := 1

    # We have to use the shell for copying files to preserve the case of
    # file names.  Without this, we get a `CONFIG.MK' file which isn't
    # found later on by `make'.
    COPY := cmd.exe /c copy

  else
    # We test for the COMSPEC environment variable, then run the `ver'
    # command-line program to see if its output contains the word `Windows'.
    #
    # If this is true, we are running a win32 platform (or an emulation).
    #
    ifdef COMSPEC
      is_windows := $(findstring Windows,$(strip $(shell ver)))
      COPY := copy
    endif
  endif  # test NT

  ifdef is_windows

    PLATFORM := win32

  endif
endif # test PLATFORM ansi

ifeq ($(PLATFORM),win32)

  DELETE := del
  SEP    := $(BACKSLASH)

  # gcc Makefile by default
  CONFIG_FILE := w32-gcc.mk
  ifeq ($(firstword $(CC)),cc)
    CC        := gcc
  endif

  ifneq ($(findstring list,$(MAKECMDGOALS)),)  # test for the "list" target
    dump_target_list:
	    @echo ÿ
	    @echo $(PROJECT_TITLE) build system -- supported compilers
	    @echo ÿ
	    @echo Several command-line compilers are supported on Win32:
	    @echo ÿ
	    @echo ÿÿmake setupÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿgcc (with Mingw)
	    @echo ÿÿmake setup visualcÿÿÿÿÿÿÿÿÿÿÿÿÿMicrosoft Visual C++
	    @echo ÿÿmake setup bcc32ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿBorland C/C++
	    @echo ÿÿmake setup lccÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿWin32-LCC
	    @echo ÿÿmake setup intelcÿÿÿÿÿÿÿÿÿÿÿÿÿÿIntel C/C++
	    @echo ÿ

    setup: dump_target_list
    .PHONY: dump_target_list list
  else
    setup: dos_setup
  endif

  # additionally, we provide hooks for various other compilers
  #
  ifneq ($(findstring visualc,$(MAKECMDGOALS)),)     # Visual C/C++
    CONFIG_FILE := w32-vcc.mk
    CC          := cl
    visualc: setup
    .PHONY: visualc
  endif

  ifneq ($(findstring intelc,$(MAKECMDGOALS)),)      # Intel C/C++
    CONFIG_FILE := w32-intl.mk
    CC          := cl
    visualc: setup
    .PHONY: intelc
  endif

  ifneq ($(findstring watcom,$(MAKECMDGOALS)),)      # Watcom C/C++
    CONFIG_FILE := w32-wat.mk
    CC          := wcc386
    watcom: setup
    .PHONY: watcom
  endif

  ifneq ($(findstring visualage,$(MAKECMDGOALS)),)   # Visual Age C++
    CONFIG_FILE := w32-icc.mk
    CC          := icc
    visualage: setup
    .PHONY: visualage
  endif

  ifneq ($(findstring lcc,$(MAKECMDGOALS)),)         # LCC-Win32
    CONFIG_FILE := w32-lcc.mk
    CC          := lcc
    lcc: setup
    .PHONY: lcc
  endif

  ifneq ($(findstring mingw32,$(MAKECMDGOALS)),)     # mingw32
    CONFIG_FILE := w32-mingw32.mk
    CC          := gcc
    mingw32: setup
    .PHONY: mingw32
  endif

  ifneq ($(findstring bcc32,$(MAKECMDGOALS)),)       # Borland C++
    CONFIG_FILE := w32-bcc.mk
    CC          := bcc32
    bcc32: setup
    .PHONY: bcc32
  endif

  ifneq ($(findstring devel-bcc,$(MAKECMDGOALS)),)   # development target
    CONFIG_FILE := w32-bccd.mk
    CC          := bcc32
    devel-bcc: setup
    .PHONY: devel-bcc
  endif

  ifneq ($(findstring devel-gcc,$(MAKECMDGOALS)),)   # development target
    CONFIG_FILE := w32-dev.mk
    CC          := gcc
    devel-gcc: setup
    .PHONY: devel-gcc
  endif

endif   # test PLATFORM win32


# EOF
