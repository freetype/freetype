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

CONFIG_FILE := os2-gcc.mk   # gcc-emx by default
SEP         := /

ifneq ($(findstring visualage,$(MAKECMDGOALS)),)     # Visual Age C++
CONFIG_FILE := os2-icc.mk
SEP         := $(BACKSLASH)
CC          := icc
.PHONY: visualage
endif

ifneq ($(findstring watcom,$(MAKECMDGOALS)),)        # Watcom C/C++
CONFIG_FILE := os2-wat.mk
SEP         := $(BACKSLASH)
CC          := wcc386
.PHONY: watcom
endif

ifneq ($(findstring borlandc,$(MAKECMDGOALS)),)      # Borland C++ 32 bits
CONFIG_FILE := os2-bcc.mk
SEP         := $(BACKSLASH)
CC          := bcc32
.PHONY: borlandc
endif

ifneq ($(findstring devel,$(MAKECMDGOALS)),)
CONFIG_FILE := os2-dev.mk
CC          := gcc
SEP         := /
devel: setup
endif

CONFIG_RULES := $(TOP)\config\os2\$(CONFIG_FILE)

setup: dos_setup

endif #test OS2_SHELL
endif #test PLATFORM
