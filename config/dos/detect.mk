#
# This file is used to detect a DOS host platform.
#
# This configuration file to be used depends on the value of the CC
# environment variable.
#
#

# We test for the COMSPEC environment variable, then run the 'ver'
# command-line program to see if its output contains the word "Dos"
#
# If this is true, we're running a Dos-ish platform (or an emulation)
#

ifeq ($(PLATFORM),ansi)

ifdef COMSPEC

is_dos := $(findstring Dos,$(shell ver))

# We try to recognize a Dos session under OS/2. The "ver" command
# returns 'Operating System/2 ...' there so 'is_dos' should be empty
# there.
#
# To recognize a Dos session under OS/2, we check COMSPEC for the
# substring "MDOS\COMMAND"
#
ifeq ($(is_dos),)
is_dos := $(findstring MDOS\COMMAND,$(COMSPEC))
endif

ifneq ($(is_dos),)

PLATFORM := dos
DELETE   := del
COPY     := copy

#####################################################################
#
# Use gcc, i.e. DJGPP by default. Aren't we biased ;-)
#
#
CONFIG_FILE := Makefile.gcc
SEP         := /
ifndef CC
CC          := gcc
endif


ifneq ($(findstring turboc,$(MAKECMDGOALS)),)     # Turbo C
CONFIG_FILE := Makefile.tcc
SEP         := $(BACKSLASH)
CC          := tcc
.PHONY: turboc
endif

ifneq ($(findstring watcom,$(MAKECMDGOALS)),)     # Watcom C/C++
CONFIG_FILE := Makefile.wat
SEP         := $(BACKSLASH)
CC          := wcc386
.PHONY: watcom
endif

ifneq ($(findstring borlandc16,$(MAKECMDGOALS)),)   # Borland C/C++ 16 bits
CONFIG_FILE := Makefile.bcc
SEP         := $(BACKSLASH)
CC          := bcc
.PHONY: borlandc16
endif

ifneq ($(findstring borlandc,$(MAKECMDGOALS)),)   # Borland C/C++ 32 bits
CONFIG_FILE := Makefile.bcc
SEP         := $(BACKSLASH)
CC          := bcc32
.PHONY: borlandc
endif

CONFIG_RULES := $(TOP)\config\dos\$(CONFIG_FILE)

# use the Dos version of the "setup dump"
#
setup:  dos_setup

endif # test Dos
endif # test COMSPEC
endif # test PLATFORM

