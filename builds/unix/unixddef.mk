#
# FreeType 2 configuration rules templates for
# developement under Unix with no configure (gcc only)
#

# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.

ifndef TOP
  TOP := .
endif
TOP := $(shell cd $(TOP); pwd)

DELETE        := rm -f
SEP           := /
HOSTSEP       := $(SEP)
BUILD         := $(TOP)/builds/unix/devel # we use a special devel ftoption.h
PLATFORM      := unixdev # do not set it to 'unix', or libtool will trick you

# don't use `:=' here since the path stuff will be included after this file
#
FTSYS_SRC = @FTSYS_SRC@

# The directory where all object files are placed.
#
OBJ_DIR := obj


# The directory where all library files are placed.
#
# By default, this is the same as $(OBJ_DIR), however, this can be changed
# to suit particular needs.
#
LIB_DIR := $(OBJ_DIR)

#
NO_OUTPUT := 2> /dev/nul

# EOF
