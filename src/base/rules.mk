#
# FreeType 2 base layer configuration rules
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used modified
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# It sets the following variables, which are used by the master Makefile
# after the call:
#
#    BASE_H:       The list of base layer header files on which the rest
#                  of the library (i.e., drivers) rely.
#
#    BASE_OBJ_S:   The single-object base layer.
#    BASE_OBJ_M:   A list of all objects for a multiple-objects build.
#    BASE_EXT_OBJ: A list of base layer extensions, i.e., components found
#                  in `freetype/lib/base' which are not compiled within the
#                  base layer proper.

INCLUDES += $(SRC_)base

# Base layer sources
#
BASE_SRC := $(BASE_)ftcalc.c    \
            $(BASE_)ftextend.c  \
            $(BASE_)ftlist.c    \
            $(BASE_)ftobjs.c    \
            $(BASE_)ftstream.c  \
            $(BASE_)ftoutln.c


# Base layer headers
#
BASE_H := $(BASE_)ftcalc.h    \
          $(BASE_)ftdebug.h   \
          $(BASE_)ftdriver.h  \
          $(BASE_)ftextend.h  \
          $(BASE_)ftlist.h    \
          $(BASE_)ftobjs.h    \
          $(BASE_)ftstream.h


# Base layer `extensions' sources
#
# An extension is added to the library file (.a or .lib) as a separate
# object.  It will then be linked to the final executable only if one of its
# symbols is used by the application.
#
BASE_EXT_SRC := $(BASE_)ftraster.c \
                $(BASE_)ftglyph.c  \
                $(BASE_)ftgrays.c

# Base layer extensions headers
#
BASE_EXT_H := $(BASE_EXT_SRC:%c=%h)


# Base layer object(s)
#
#   BASE_OBJ_M is used during `multi' builds (each base source file compiles
#   to a single object file).
#
#   BASE_OBJ_S is used during `single' builds (the whole base layer is
#   compiled as a single object file using ftbase.c).
#
BASE_OBJ_M := $(BASE_SRC:$(BASE_)%.c=$(OBJ_)%.$O)
BASE_OBJ_S := $(OBJ_)ftbase.$O


# Default extensions objects
#
BASE_EXT_OBJ := $(BASE_EXT_SRC:$(BASE_)%.c=$(OBJ_)%.$O)


# Base layer root source file(s)
#
BASE_SRC_M := $(BASE_SRC)
BASE_SRC_S := $(BASE_)ftbase.c


# Multiple objects build + extensions
#
$(OBJ_)ft%.$O: $(BASE_)ft%.c $(PUBLIC_H) $(BASE_H)
	$(FT_COMPILE) $T$@ $<


# Base layer - single object build
#
$(BASE_OBJ_S): $(PUBLIC_H) $(BASE_H) $(BASE_SRC_S) $(BASE_SRC)
	$(FT_COMPILE) $T$@ $(BASE_SRC_S)

# EOF
