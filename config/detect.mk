#****************************************************************************
#*                                                                          *
#*  FreeType host platform detection rules                                  *
#*                                                                          *
#*  Copyright 1996-1999 by                                                  *
#*  David Turner, Robert Wilhelm, and Werner Lemberg.                       *
#*                                                                          *
#*  This file is part of the FreeType project, and may only be used         *
#*  modified and distributed under the terms of the FreeType project        *
#*  license, LICENSE.TXT.  By continuing to use, modify, or distribute      *
#*  this file you indicate that you have read the license and               *
#*  understand and accept it fully.                                         *
#*                                                                          *
#*                                                                          *
#*  This sub-Makefile is in charge of detecting the current platform        *
#*  It sets some variables accordingly. Namely :                            *
#*                                                                          *
#*   PLATFORM     The detected platform. This will default to "ansi" if     *
#*                auto-detection fails.                                     *
#*                                                                          *
#*   BUILD        The configuration and system-specific directory. Usually  *
#*                'freetype/config/$(PLATFORM)' but can be different when   *
#*                a specific compiler has been requested on the             *
#*                command line..                                            *
#*                                                                          *
#*   CONFIG_RULES The Makefile to use. This usually depends on the compiler *
#*                defined in the 'CC' environment variable.                 *
#*                                                                          *
#*   DELETE       The shell command used to remove a given file             *
#*   COPY         The shell command used to copy one file                   *
#*                                                                          *
#*  You need to set the following variable(s) before calling it:            *
#*                                                                          *
#*    TOP         The top-most directory in the FreeType library source     *
#*                hierarchy. If not defined, it will default to '.'         *
#*                                                                          *
#****************************************************************************

# If TOP is not defined, default it to '.'
#
ifndef TOP
TOP := .
endif

#
# set auto-detection default to ANSI.
# Note that we delay the valuation of BUILD and RULES
#
PLATFORM := ansi
CONFIG    = $(TOP)$(SEP)config
DELETE   := $(RM)
COPY     := cp
SEP      := /

BUILD       = $(CONFIG)$(SEP)$(PLATFORM)
CONFIG_FILE = $(BUILD)/Makefile


###########################################################################
#
# Now, include each detection rules file found in a `config/<system>'
# directory..
#
#

# we define the BACKSLASH variable to hold a single back-slash character
# This is needed because a line like
#
# SEP := \
#
# does not work with GNU Make (the back-slash is interpreted as a line
# continuation). While a line like :
#
# SEP := \\
#
# really define $(SEP) as "\" on Unix, and "\\" on Dos and Windows !!
#
BACKSLASH := $(strip \ )

include $(wildcard $(CONFIG)/*/detect.mk)


# The following targets are equivalent, with the exception that they use
# slightly different syntaxes for the `echo' command. This is due to
#
# std_setup: is defined for most platforms
# dos_setup: is defined for Dos-ish platforms like Dos, Windows & OS/2
#

.PHONY: std_setup  dos_setup

std_setup:
	@echo ""
	@echo "FreeType build system - automatic system detection"
	@echo ""
	@echo "The following settings were detected :"
	@echo ""
	@echo "  platform                 :  $(PLATFORM)"
	@echo "  compiler                 :  $(CC)"
	@echo "  configuration directory  :  $(BUILD)"
	@echo "  configuration rules      :  $(CONFIG_RULES)"
	@echo ""
	@echo "If this does not correspond to your system or settings please remove the file"
	@echo "\`$(CONFIG_MK)' from this directory then read the INSTALL file for help."
	@echo ""
	@echo "Otherwise, simple type \`make' again to build the library"
	@echo ""
	@$(COPY) $(CONFIG_RULES) $(CONFIG_MK)

dos_setup:
	@echo ÿ
	@echo FreeType build system - automatic system detection
	@echo ÿ
	@echo The following settings were detected :
	@echo ÿ
	@echo ÿÿplatformÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ:  $(PLATFORM)
	@echo ÿÿcompilerÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ:  $(CC)
	@echo ÿÿconfiguration directoryÿÿ:  $(BUILD)
	@echo ÿÿconfiguration rulesÿÿÿÿÿÿ:  $(CONFIG_RULES)
	@echo ÿ
	@echo If this does not correspond to your system or settings please remove the file
	@echo '$(CONFIG_MK)' from this directory then read the INSTALL file for help.
	@echo ÿ
	@echo Otherwise, simple type 'make' again to build the library
	@echo ÿ
	@$(COPY) $(CONFIG_RULES) $(CONFIG_MK) > nul

