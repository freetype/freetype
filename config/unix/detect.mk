#
# This file is used to detect which Makefile to use based on the
# value of the CC environment variable.
#
# Unix
#
#
# This will _much_ probably change in the future if we're going to use
# Automake/Autoconf..
#

ifeq ($(PLATFORM),ansi)
has_inittab := $(strip $(wildcard /etc/inittab))
ifneq ($(has_inittab),)

PLATFORM := unix
COPY     := cp
DELETE   := rm -f

# if `devel' is the requested target, use the development Makefile
#
ifneq ($(findstring devel,$(MAKECMDGOALS)),)
CONFIG_RULES := $(BUILD)$(SEP)unix-dev.mk
devel: setup;
endif

# test wether we're using gcc ? If it is, we selected the
# 'unix-gcc.mk' configuration file. Otherwise, the standard
# 'unix.mk' which simply calls "cc -c" with no extra arguments
#
# Feel free to add support for other platform specific compilers
# in this directory (e.g. solaris.mk + changes here to detect the
# platform)
#
ifeq ($(CC),gcc)
is_gcc := 1
else
ifneq ($(findstring gcc,$(shell $(CC) --version)),)
is_gcc := 1
endif

ifdef is_gcc
CONFIG_RULES := $(BUILD)$(SEP)unix-gcc.mk
else
CONFIG_RULES := $(BUILD)$(SEP)unix.mk
endif

setup: std_setup

endif # test Unix
endif # test PLATFORM






