#
# This file is used to detect a Win32 host platform.
#
# This configuration file to be used depends on the value of the CC
# environment variable.
#
#

# We test for the COMSPEC environment variable, then run the 'ver'
# command-line program to see if its output contains the word "Windows"
#
# If this is true, we're running a win32 platform (or an emulation)
#

ifeq ($(PLATFORM),ansi)
ifdef COMSPEC

is_windows := $(findstring Windows,$(strip $(shell ver)))
ifneq ($(is_windows),)

PLATFORM := win32
DELETE   := del
COPY     := copy

CONFIG_FILE := Makefile.gcc  # gcc Makefile by default - aren't we biased ;-)
SEP         := /
ifeq ($(CC),cc)
CC          := gcc
endif

ifneq ($(findstring visualc,$(MAKECMDGOALS)),)     # Visual C/C++
CONFIG_FILE := Makefile.vcc
SEP         := $(BACKSLASH)
CC          := cl
visualc: setup
endif

ifneq ($(findstring watcom,$(MAKECMDGOALS)),)      # Watcom C/C++
CONFIG_FILE := Makefile.wat
SEP         := $(BACKSLASH)
CC          := wcc386
watcom: setup
endif

ifneq ($(findstring visualage,$(MAKECMDGOALS)),)   # Visual Age C++
CONFIG_FILE := Makefile.icc
SEP         := $(BACKSLASH)
CC          := icc
visualage: setup
endif

ifneq ($(findstring lcc,$(MAKECMDGOALS)),)         # LCC-Win32
CONFIG_FILE := Makefile.lcc
SEP         := $(BACKSLASH)
CC          := lcc
lcc: setup
endif

CONFIG_RULES := $(TOP)\config\win32\$(CONFIG_FILE)

setup: dos_setup

endif #test Windows
endif #test COMSPEC
endif #test PLATFORM

