#****************************************************************************
#*                                                                          *
#*  Base layer Makefile                                                     *
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
#****************************************************************************


#****************************************************************************
#*                                                                          *
#*  IMPORTANT NOTE: This Makefile is intended for GNU Make!                 *
#*                  If you provide Makefiles for other make utilities,      *
#*                  please place them in `freetype/lib/arch/<system>.'      *
#*                                                                          *
#*                                                                          *
#*  This file is to be included by the root FreeType sub-Makefile, usually  *
#*  named `freetype/lib/Makefile.lib.'  Here is the list of the variables   *
#*  that must be defined to use it:                                         *
#*                                                                          *
#*     BASE_DIR:  The location of the base layer's directory.  This is      *
#*                usually `freetype/lib/base.'                              *
#*                                                                          *
#*     ARCH_DIR:  The location of the architecture-dependent directory.     *
#*                This is usually `freetype/lib/arch/<system>.'             *
#*                                                                          *
#*     OBJ_DIR:   The location where the compiled object(s) file will be    *
#*                placed.                                                   *
#*                                                                          *
#*     FT_CFLAGS: A set of flags used for compilation of object files.      *
#*                This contains at least the include paths of the `arch'    *
#*                and `base' directories + optimization + warnings +        *
#*                ANSI compliance.                                          *
#*                                                                          *
#*     FT_IFLAGS: The flag used to specify an include path on the compiler  *
#*                command line.  For example, with GCC, this is `-I', while *
#*                some other compilers use `/i=' or `-J', etc.              *
#*                                                                          *
#*     FT_OBJ:    The suffix of an object file for the platform.  Can be    *
#*                `o', `obj', `coff', `tco', etc.                           *
#*                                                                          *
#*     FT_CC:     The C compiler to use.                                    *
#*                                                                          *
#*                                                                          *
#*  It sets the following variables, which are used by the parent Makefile  *
#*  after the call:                                                         *
#*                                                                          *
#*     BASE_H:       The list of base layer header files on which the rest  *
#*                   of the library (i.e., drivers) rely.                   *
#*                                                                          *
#*     BASE_OBJ_S:   The single-object base layer.                          *
#*     BASE_OBJ_M:   A list of all objects for a multiple-objects build.    *
#*     BASE_EXT_OBJ: A list of base layer extensions, i.e., components      *
#*                   found in `freetype/lib/base' which are not compiled    *
#*                   within the base layer proper.                          *
#*                                                                          *
#****************************************************************************

INCLUDES += $(SRC_)base

# Base layer sources
#
BASE_SRC := $(BASE_)ftstream.c  \
            $(BASE_)ftcalc.c    \
            $(BASE_)ftobjs.c    \
            $(BASE_)ftextend.c  \
            $(BASE_)ftlist.c

# Base layer headers
#
BASE_H := $(BASE_)ftcalc.h    \
          $(BASE_)ftobjs.h    \
          $(BASE_)ftdriver.h  \
          $(BASE_)ftextend.h  \
          $(BASE_)ftlist.h


# Base layer `extensions' sources
#
# An extension is added to the library file (.a or .lib) as a separate
# object.  It will then be linked to the final executable only if one of
# its symbols is used by the application.
#
BASE_EXT_SRC := $(BASE_)ftbbox.c   \
                $(BASE_)ftraster.c \
                $(BASE_)ftoutln.c


# Base layer extensions headers
#
BASE_EXT_H := $(BASE_EXT_SRC:%c=%h)


# Base layer object(s)
#
#   BASE_OBJ_M is used during `multi' builds (each base source file
#   compiles to a single object file).
#
#   BASE_OBJ_S is used during `single' builds (the whole base layer
#   is compiled as a single object file using ftbase.c).
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


# END
