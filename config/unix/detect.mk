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
CONFIG_RULES := $(BUILD)$(SEP)Makefile.devel
devel: ;
else
CONFIG_RULES := $(BUILD)$(SEP)Makefile
endif

setup: std_setup

endif # test Unix
endif # test PLATFORM






