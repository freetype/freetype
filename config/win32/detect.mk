#
# This file is used to detect a Win32 host platform.
#
# This configuration file to be used depends on the value of the CC
# environment variable.
#
#


ifeq ($(PLATFORM),ansi)

###################################################################
#
# Detecting Windows NT or Windows 9x
#

# Detecting Windows NT is easy, as the OS variable must be defined
# and contains "Windows_NT". Untested with Windows 2K, but I guess
# it should work ...
#
ifeq ($(OS),Windows_NT)
is_windows := 1

# We test for the COMSPEC environment variable, then run the 'ver'
# command-line program to see if its output contains the word "Windows"
#
# If this is true, we're running a win32 platform (or an emulation)
#
else
ifdef COMSPEC
is_windows := $(findstring Windows,$(strip $(shell ver)))
endif
endif  #test NT

####################################################################
#
# Rules for Win32
#

ifdef is_windows

PLATFORM := win32
DELETE   := del
COPY     := copy

CONFIG_FILE := w32-gcc.mk  # gcc Makefile by default - aren't we biased ;-)
SEP         := /
ifeq ($(CC),cc)
CC          := gcc
endif

ifneq ($(findstring visualc,$(MAKECMDGOALS)),)     # Visual C/C++
CONFIG_FILE := w32-vcc.mk
SEP         := $(BACKSLASH)
CC          := cl
visualc: setup
endif

ifneq ($(findstring watcom,$(MAKECMDGOALS)),)      # Watcom C/C++
CONFIG_FILE := w32-wat.mk
SEP         := $(BACKSLASH)
CC          := wcc386
watcom: setup
endif

ifneq ($(findstring visualage,$(MAKECMDGOALS)),)   # Visual Age C++
CONFIG_FILE := w32-icc.mk
SEP         := $(BACKSLASH)
CC          := icc
visualage: setup
endif

ifneq ($(findstring lcc,$(MAKECMDGOALS)),)         # LCC-Win32
CONFIG_FILE := w32-lcc.mk
SEP         := $(BACKSLASH)
CC          := lcc
lcc: setup
endif

ifneq ($(findstring devel,$(MAKECMDGOALS)),)
CONFIG_FILE := w32-dev.mk
CC          := gcc
SEP         := /
devel: setup
endif

CONFIG_RULES := $(TOP)\config\win32\$(CONFIG_FILE)

setup: dos_setup

endif #test is_windows
endif #test PLATFORM

