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

CONFIG_RULES := $(BUILD)$(SEP)Makefile

setup: std_setup

endif # test Unix
endif # test PLATFORM






