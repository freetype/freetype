#
# FreeType 2 library sub-Makefile
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used modified
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# DO NOT INVOKE THIS MAKEFILE DIRECTLY!  IT IS MEANT TO BE INCLUDED BY
# OTHER MAKEFILES.


# The targets `objects' and `library' are defined at the end of this
# Makefile when all rules have been included.
#
.PHONY: build_freetype objects library

# default target -- build objects and library
#
build_freetype: objects library

# `multi' target -- build multiple objects and library
#
multi: objects library


# The FreeType source directory.
#
SRC := $(TOP)$(SEP)src


# The directory where the base layer components are placed.  By default,
# this is `freetype/src/base'.
#
BASE_DIR := $(SRC)$(SEP)base


# A few short-cuts in order to avoid typing $(SEP) all the time for the
# directory separator.
#
# For example:  SRC_ equals to `./src/' where `.' is $(TOP).
#
#
SRC_    := $(SRC)$(SEP)
BASE_   := $(BASE_DIR)$(SEP)
OBJ_    := $(OBJ_DIR)$(SEP)
LIB_    := $(LIB_DIR)$(SEP)
PUBLIC_ := $(TOP)$(SEP)include$(SEP)
CONFIG_ := $(TOP)$(SEP)config$(SEP)


# The name of the final library file.
#
FT_LIBRARY := $(LIB_)$(LIBRARY).$A


# include paths
#
# IMPORTANT NOTE: The architecture-dependent directory must ALWAYS be placed
#                 in front of the include list.  Porters are then able to
#                 put their own version of some of the FreeType components
#                 in the `freetype/config/<system>' directory, as these
#                 files will override the default sources.
#
INCLUDES := $(BUILD) $(TOP)$(SEP)config $(TOP)$(SEP)include $(INCLUDES)

INCLUDE_FLAGS = $(INCLUDES:%=$I%)


# C flags used for the compilation of an object file.  This must include at
# least the paths for the `base' and `config/<system>' directories,
# debug/optimization/warning flags + ansi compliance if needed.
#
FT_CFLAGS  = $(CFLAGS) $(INCLUDE_FLAGS)
FT_CC      = $(CC) $(FT_CFLAGS)
FT_COMPILE = $(CC) $(ANSIFLAGS) $(FT_CFLAGS)


# include the `modules' rules file
#
include $(CONFIG_)modules.mk


# Free the lists of driver objects.
#
COMPONENTS_LIST :=
DRIVERS_LIST    :=
OBJECTS_LIST    :=

# System-specific component -- this must be defined in this Makefile for
# easy updates.  The default ANSI ftsystem.c is located in
# `freetype/config/ftsystem.c'.  However, some system-specific configuration
# might define $(FTSYS_SRC) to fetch it in other places, like
# `freetype/config/<system>/ftsystem.c'.
#
# $(BASE_H) is defined in `src/base/rules.mk' and contains the list of all
# base layer header files.
#
ifndef FTSYS_SRC
  FTSYS_SRC = $(BASE_)ftsystem.c
endif
FTSYS_OBJ = $(OBJ_)ftsystem.$O

OBJECTS_LIST += $(FTSYS_OBJ)

$(FTSYS_OBJ): $(FTSYS_SRC) $(BASE_H)
	$(FT_COMPILE) $T$@ $<


# ftdebug component
#
# FTDebug contains code used to print traces and errors.  It is normally
# empty for a release build (see ftoption.h).
#
FTDEBUG_SRC = $(BASE_)ftdebug.c
FTDEBUG_OBJ = $(OBJ_)ftdebug.$O

OBJECTS_LIST += $(FTDEBUG_OBJ)

$(FTDEBUG_OBJ): $(FTDEBUG_SRC) $(BASE_H)
	$(FT_COMPILE) $T$@ $<


# Define $(PUBLIC_H) as the list of all public header files located in
# `$(TOP)/include'.
#
PUBLIC_H := $(wildcard $(PUBLIC_)*.h)


# Include all rule files from FreeType components.
#
include $(wildcard $(SRC)/*/rules.mk)


# FTInit file
#
#   The C source `ftinit.c' contains the FreeType initialization routines.
#   It is able to automatically register one or more drivers when the API
#   function FT_Init_FreeType() is called.
#
#   The set of initial drivers is determined by the driver Makefiles
#   includes above.  Each driver Makefile updates the FTINIT_xxxx lists
#   which contain additional include paths and macros used to compile the
#   single `ftinit.c' source.
#
FTINIT_SRC := $(BASE_)ftinit.c
FTINIT_OBJ := $(OBJ_)ftinit.$O

OBJECTS_LIST += $(FTINIT_OBJ)

$(FTINIT_OBJ): $(FTINIT_SRC) $(BASE_H) $(FT_MODULE_LIST)
	$(FT_COMPILE) $T$@ $<


# All FreeType library objects
#
#   By default, we include the base layer extensions.  These could be
#   omitted on builds which do not want them.
#
OBJ_M = $(BASE_OBJ_M) $(BASE_EXT_OBJ) $(DRV_OBJS_M)
OBJ_S = $(BASE_OBJ_S) $(BASE_EXT_OBJ) $(DRV_OBJS_S)


# The target `multi' on the Make command line indicates that we want to
# compile each source file independently.
#
# Otherwise, each module/driver is compiled in a single object file through
# source file inclusion (see `src/base/ftbase.c' or
# `src/truetype/truetype.c' for examples).
#
BASE_OBJECTS := $(OBJECTS_LIST)

ifneq ($(findstring multi,$(MAKECMDGOALS)),)
  OBJECTS_LIST += $(OBJ_M)
else
  OBJECTS_LIST += $(OBJ_S)
endif

objects: $(OBJECTS_LIST)

library: $(FT_LIBRARY)

.c.$O:
	$(FT_COMPILE) $T$@ $<


# Standard cleaning and distclean rules.  These are not accepted
# on all systems though.
#
clean_freetype_std:
	-$(DELETE) $(BASE_OBJECTS) $(OBJ_M) $(OBJ_S)

distclean_freetype_std: clean_freetype_std
	-$(DELETE) $(FT_LIBRARY)
	-$(DELETE) *.orig *~ core *.core

# The Dos command shell does not support very long list of arguments, so
# we are stuck with wildcards.
#
clean_freetype_dos:
	-$(DELETE) $(subst $(SEP),$(HOSTSEP),$(OBJ_))*.$O 2> nul

distclean_freetype_dos: clean_freetype_dos
	-$(DELETE) $(subst $(SEP),$(HOSTSEP),$(FT_LIBRARY)) 2> nul

# Remove configuration file (used for distclean).
#
remove_config_mk:
	-$(DELETE) $(subst $(SEP),$(HOSTSEP),$(CONFIG_MK))


# The `config.mk' file must define `clean_freetype' and
# `distclean_freetype'.  Implementations may use to relay these to either
# the `std' or `dos' versions, or simply provide their own implementation.
#
clean: clean_freetype
distclean: distclean_freetype remove_config_mk

# EOF
