#*******************************************************************
#*
#*  FreeType 2 Configuration rules for Win32 + Visual C++
#*
#*  Copyright 1996-2000 by
#*  David Turner, Robert Wilhelm, and Werner Lemberg.
#*
#*  This file is part of the FreeType project, and may only be used
#*  modified and distributed under the terms of the FreeType project
#*  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#*  this file you indicate that you have read the license and
#*  understand and accept it fully.
#*
#*  Please read the file "freetype/docs/config.txt" to understand
#*  what this file does..
#*
#*******************************************************************

DELETE   := del
SEP      := /
HOSTSEP  := $(strip \ )
BUILD    := $(TOP)$(SEP)config$(SEP)win32
PLATFORM := win32
CC       := cl

# the directory where all object files are placed
#
# Note that this is not $(TOP)/obj !!
# This lets you build the library in your own directory
# with something like :
#
#  set TOP=....../path/to/freetype2/top/dir...
#  mkdir obj
#  make -f %TOP%/Makefile setup  [options]
#  make -f %TOP%/Makefile
#
#
OBJ_DIR := obj


# the directory where all library files are placed
#
#  by default, this is the same as OBJ_DIR, however, this can be
#  changed to suit particular needs..
#
LIB_DIR := $(OBJ_DIR)


# the object file extension, this can be .o, .tco, .obj, etc..
# depending on the platform
#
O := obj

# the library file extension, this can be .a, .lib, etc..
# depending on the platform
#
A := lib


# The name of the final library file.
# Note that the DOS-specific Makefile uses a shorter (8.3) name
#
LIBRARY := freetype


# path inclusion flag.
#
#  Some compilers use a different flag than '-I' to specify an
#  additional include path. Examples are "/i=" or "-J", etc...
#
I := /I


# The link flag used to specify a given library file on link.
# Note that this is only used to compile the demo programs, not
# the library itself.
#
L := /Fl


# C flag used to define a macro before the compilation of a given
# source object. Usually is '-D' like in "-DDEBUG"
#
D := /D

# Target flag - LCC uses "-Fo" instead of "-o ", what a broken compiler
#
#
T := /Fo

# C flags
#
#   These should concern :
#
#     - debug output
#     - optimization
#     - warnings
#     - ansi compliance..
#
ifndef CFLAGS
CFLAGS := /nologo /c /Ox /G5 /W3 /WX
endif

# ANSI_FLAGS : put there the flags used to make your compiler ANSI-compliant
#              nothing (if it already is by default like LCC).
#
ANSI_FLAGS := /Za

ifdef BUILD_FREETYPE

include $(TOP)/config/freetype.mk

clean_freetype: clean_freetype_dos
distclean_freetype: distclean_freetype_dos

# This final rule is used to link all object files into a single
# library. It is part of the system-specific sub-Makefile because not
# all librarians accept a simple syntax like :
#
#    librarian library_file {list of object files} 
#
$(FT_LIBRARY): $(OBJECTS_LIST)
	lib /nologo /out:$@ $(OBJECTS_LIST)

endif


