#******************************************************************************
#*
#*  FreeType build system - top-level Makefile
#*
#*  This file is designed for GNU Make, do not use it with another Make tool.
#*  It works as follows :
#*
#*  - when invoked for the first time, this Makefile will include
#*    the rules found in `freetype/config/detect.mk'. They are in charge
#*    of detecting the current platform.
#*
#*    A summary of the detection will be displayed, and the file `config.mk'
#*    will be created in the current directory
#*
#*
#*  - when invoked later, this Makefile will include the rules found in
#*    `config.mk'. This sub-Makefile will define some system-specific
#*    variables (like compiler, compilation flags, object suffix, etc..),
#*    then include the rules found in `freetype/config/freetype.mk',
#*    used to build the library.
#*
#*  See the comments in `config/detect.mk' and `config/freetype.mk' for
#*  more details on host platform detection and library builds..
#*
#******************************************************************************

.PHONY: setup

#
# The variable TOP holds the path to the topmost directory in the FreeType
# engine source hierarchy. If it is not defined, default it to '.'
#
ifndef TOP
TOP := .
endif

CONFIG_MK := config.mk

#############################################################################
#
# If no configuration sub-makefile is present, or if "setup" is the target
# to be built, run the auto-detection rules to figure out which configuration
# rules file to use.. 
#
# Note that the configuration file is put in the current directory, which is
# not necessarily TOP.
#

# if `config.mk' is not present, set "check_platform" and "first_time"
#
ifeq ($(wildcard $(CONFIG_MK)),)
check_platform := 1
first_time     := 1
endif

# if `setup' is one of the targets requested, set "check_platform"
#
ifneq ($(findstring setup,$(MAKECMDGOALS)),)
check_platform := 1
endif


#########################################################################
#
# include the automatic host platform detection rules when we need to
# check the platform.
#
#
ifdef check_platform

all: setup

include $(TOP)/config/detect.mk

# "setup" must be defined by the host platform detection rules

else

########################################################################
#
# A configuration sub-Makefile is present, simply run it..
#
#
all: build_freetype

BUILD_FREETYPE := yes
include $(CONFIG_MK)

endif #test check_platform

