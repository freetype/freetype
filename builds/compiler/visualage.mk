#
# Visual Age C++ specific definitions
#

# command line compiler name
#
CC       := icc

# The object file extension (for standard and static libraries).  This can be
# .o, .tco, .obj, etc., depending on the platform.
#
O  := obj
SO := obj

# The library file extension (for standard and static libraries).  This can
# be .a, .lib, etc., depending on the platform.
#
A  := lib
SA := lib

# Path inclusion flag.  Some compilers use a different flag than `-I' to
# specify an additional include path.  Examples are `/i=' or `-J'.
#
I := /I


# C flag used to define a macro before the compilation of a given source
# object.  Usually is `-D' like in `-DDEBUG'.
#
D := /D


# The link flag used to specify a given library file on link.  Note that
# this is only used to compile the demo programs, not the library itself.
#
L := /Fl


# Target flag.
#
T := /Fo


# C flags
#
#   These should concern: debug output, optimization & warnings.
#
ifndef CFLAGS
  CFLAGS := /Q- /Gd+ /O2 /G5 /W3 /C
endif

# ANSIFLAGS: Put there the flags used to make your compiler ANSI-compliant.
#
ANSI_FLAGS := /Sa


# Library linking
#
#CLEAN_LIBRARY :=
LINK_LIBRARY   = lib /nologo /out:$@ $(OBJECTS_LIST)

# EOF
