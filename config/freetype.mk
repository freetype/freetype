#****************************************************************************
#*                                                                          *
#*  FreeType library sub-Makefile                                           *
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
#*                                                                          *
#*  DO NOT INVOKE THIS MAKEFILE DIRECTLY. IT IS MEANT TO BE INCLUDED BY     *
#*  OTHER MAKEFILES.                                                        *
#*                                                                          *
#****************************************************************************


# include the 'modules' rules file
#
include $(TOP)/config/modules.mk

# The targets `objects', `library' and `multiple' are defined
# at the end of this Makefile when all rules have been included..
#
.PHONY: build_freetype objects library

# default target - build objects and library
#
build_freetype: objects library

# `multi' target - build multiple objects and library
#
multi: objects library


# The FreeType sources directory.
#
SRC := $(TOP)$(SEP)src


# The directory where the base layer components are placed.
# By default, this is 'freetype/src/base'
#
BASE_DIR := $(SRC)$(SEP)base


# A Few short-cuts in order to avoid typing $(SEP) all the time for
# the directory separator
#
# For example:  SRC_ equals to './src/' where '.' is $(TOP)
#
#
SRC_    := $(SRC)$(SEP)
BASE_   := $(BASE_DIR)$(SEP)
OBJ_    := $(OBJ_DIR)$(SEP)
LIB_    := $(LIB_DIR)$(SEP)
PUBLIC_ := $(TOP)$(SEP)include$(SEP)

# The name of the final library file.
#
FT_LIBRARY := $(LIB_DIR)$(SEP)$(LIBRARY).$A


# include paths
#
# IMPORTANT NOTE: The architecture-dependent directory must ALWAYS be placed
#                 in front of the include list.  Porters are then able to put
#                 their own version of some of the FreeType components in
#                 the 'freetype/arch/<system>' directory, as these files
#                 will override the default sources.
#
INCLUDES := $(BUILD) $(TOP)$(SEP)include $(INCLUDES)

INCLUDE_FLAGS = $(INCLUDES:%=$I%)


# C flags used for the compilation of an object file.  This must include at
# least the paths for the 'base' and 'config/<system>' directories,
# debug/optimization/warning flags + ansi compliance if needed.
#
FT_CFLAGS  = $(CFLAGS) $(INCLUDE_FLAGS)
FT_CC      = $(CC) $(FT_CFLAGS)
FT_COMPILE = $(CC) $(ANSI_FLAGS) $(FT_CFLAGS)


#
# Free the lists of driver objects
#
COMPONENTS_LIST :=
DRIVERS_LIST    :=
OBJECTS_LIST    :=

# System-specific component - this must be defined in this Makefile
# for easy updates
#
# BASE_H is defined in src/base/rules.mk and contains the list of all
# base layer header files.
#
FTSYS_SRC = $(BUILD)$(SEP)ftsystem.c
FTSYS_OBJ = $(OBJ_DIR)$(SEP)ftsystem.$O

OBJECTS_LIST += $(FTSYS_OBJ)

$(FTSYS_OBJ): $(FTSYS_SRC) $(BASE_H)
	$(FT_COMPILE) $T$@ $<


# ftdebug component
#
#
#

FTDEBUG_SRC = $(BASE_)ftdebug.c
FTDEBUG_OBJ = $(OBJ_)ftdebug.$O

OBJECTS_LIST += $(FTDEBUG_OBJ)

$(FTDEBUG_OBJ): $(FTDEBUG_SRC) $(BASE_H)
	$(FT_COMPILE) $T$@ $<



# Define PUBLIC_H as the list of all public header files located in
# `$(TOP)/include'
#
PUBLIC_H := $(wildcard $(PUBLIC_)*.h)


# Include all rule files from FreeType components
#
#
include $(wildcard $(SRC)/*/rules.mk)

# FTInit file:
#
#   The C source 'ftinit.c' contains the FreeType initialisation routines.
#   It is able to automatically register one or more drivers when the API
#   function FT_Init_FreeType() is called.
#
#   The set of initial drivers is determined by the driver Makefiles
#   includes above.  Each driver Makefile updates the FTINIT_xxxx lists
#   which contain additional include paths and macros used to compile the
#   single 'ftapi.c' source.
#
FTINIT_SRC := $(BASE_DIR)$(SEP)ftinit.c
FTINIT_OBJ := $(OBJ_)ftinit.$O

$(FTINIT_OBJ): $(FTINIT_SRC) $(BASE_H) $(FTINIT_DRIVER_H) $(FT_MODULE_LIST)
	$(FT_COMPILE) $(FTINIT_DRIVER_PATHS:%=$I%) \
                      $(FTINIT_DRIVER_MACROS:%=$D%) $T$@ $<


# All FreeType library objects
#
#   By default, we include the base layer extensions.  These could be
#   ommitted on builds which do not want them.
#
OBJ_M = $(BASE_OBJ_M) $(BASE_EXT_OBJ) $(DRV_OBJS_M) \
        $(FTINIT_OBJ)

OBJ_S = $(BASE_OBJ_S) $(BASE_EXT_OBJ) $(DRV_OBJS_S) \
        $(FTINIT_OBJ)

ifneq ($(findstring multi,$(MAKECMDGOALS)),)
OBJECTS_LIST += $(OBJ_M)
else
OBJECTS_LIST += $(OBJ_S)
endif

objects: $(OBJECTS_LIST)

library: $(FT_LIBRARY)

.c.$O:
	$(FT_COMPILE) $T$@ $<


# Standard cleaning and distclean rules. These are not accepted
# on all systems though..
#
clean_freetype_std:
	-$(DELETE) $(subst $(SEP),$(HOSTSEP),$(OBJ_S) $(OBJ_M))

distclean_freetype_std: clean_freetype_std
	-$(DELETE) $(subst $(SEP),$(HOSTSEP),$(FT_LIBRARY))
	-$(DELETE) *.orig *~ core *.core

# The Dos command shell does not support very long list of arguments
# so we're stuck with wildcards
#
#SYSOBJ_ := $(subst $(SEP),\,$(OBJ_))

clean_freetype_dos:
	-del $(subst $(SEP),$(HOSTSEP),$(OBJ_))*.$O 2> nul

distclean_freetype_dos: clean_freetype_dos
	-del $(subst $(SEP),$(HOSTSEP),$(FT_LIBRARY)) 2> nul

remove_config_mk:
	-$(DELETE) $(subst $(SEP),$(HOSTSEP),$(CONFIG_MK)) 2> nul

# the "config.mk" must define 'clean_freetype' and 'distclean_freetype'
# implementations may use to relay these to either the 'std' or 'dos'
# versions, or simply provide their own implementation..
#
clean: clean_freetype
distclean: distclean_freetype remove_config_mk

# END
