#
# This file is used to detect an OS/2 host, and set the build variables
# accordingly..
#
# which Makefile to use based on the value of the CC environment variable.
#
# OS/2
#
#

ifeq ($(PLATFORM),ansi)
ifdef OS2_SHELL

PLATFORM := os2
COPY     := copy
DELETE   := del

CONFIG_FILE := Makefile.emx   # gcc-emx by default
SEP         := /

ifneq ($(findstring visualage,$(MAKECMDGOALS)),)     # Visual Age C++
CONFIG_FILE := Makefile.icc
SEP         := $(BACKSLASH)
CC          := icc
.PHONY: visualage
endif

ifneq ($(findstring watcom,$(MAKECMDGOALS)),)        # Watcom C/C++
CONFIG_FILE := Makefile.wat
SEP         := $(BACKSLASH)
CC          := wcc386
.PHONY: watcom
endif

ifneq ($(findstring borlandc,$(MAKECMDGOALS)),)      # Borland C++ 32 bits
CONFIG_FILE := Makefile.bcc
SEP         := $(BACKSLASH)
CC          := bcc32
.PHONY: borlandc
endif

CONFIG_RULES := $(TOP)\config\os2\$(CONFIG_FILE)

setup: dos_setup

endif #test OS2_SHELL
endif #test PLATFORM
