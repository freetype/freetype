# Copyright 2000 David Turner
#
#  gcc-specific with NO OPTIMISATIONS + DEBUGGING
#

# Compiler command line name
CC := gcc

# The object file extension (for standard and static libraries).  This can be
# .o, .tco, .obj, etc., depending on the platform.
#
O  := o
SO := o

# The library file extension (for standard and static libraries).  This can
# be .a, .lib, etc., depending on the platform.
#
A  := a
SA := a


# Path inclusion flag.  Some compilers use a different flag than `-I' to
# specify an additional include path.  Examples are `/i=' or `-J'.
#
I := -I


# C flag used to define a macro before the compilation of a given source
# object.  Usually is `-D' like in `-DDEBUG'.
#
D := -D


# The link flag used to specify a given library file on link.  Note that
# this is only used to compile the demo programs, not the library itself.
#
L := -l


# Target flag.
#
T := -o # Don't remove this comment line!  We need the space after `-o'.


# C flags
#
#   These should concern: debug output, optimization & warnings.
#
#   Use the ANSIFLAGS variable to define the compiler flags used to enfore
#   ANSI compliance.
#
ifndef CFLAGS
  CFLAGS := -c -g -O0 -Wall
endif

# ANSIFLAGS: Put there the flags used to make your compiler ANSI-compliant.
#
ANSIFLAGS := -ansi -pedantic


# Library linking
#
ifndef
CLEAN_LIBRARY = $(DELETE) $(subst $(SEP),$(HOSTSEP),$(PROJECT_LIBRARY)) $(NO_OUTPUT)
endif
LINK_LIBRARY  = $(AR) -r $@ $(OBJECTS_LIST)

# EOF
