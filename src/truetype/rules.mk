#****************************************************************************
#*                                                                          *
#*  TrueType driver Makefile                                                *
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
#*                  please place them in `freetype/lib/arch/<system>'.      *
#*                                                                          *
#*                                                                          *
#*  This file is to be included by the FreeType Makefile.lib, located in    *
#*  the `freetype/lib' directory.  Here is the list of the variables that   *
#*  must be defined to use it:                                              *
#*                                                                          *
#*                                                                          *
#*     BASE_DIR:    The location of the base layer's directory.  This is    *
#*                  usually `freetype/lib/base'.                            *
#*                                                                          *
#*     ARCH_DIR:    The location of the architecture-dependent directory.   *
#*                  This is usually `freetype/lib/arch/<system>'.           *
#*                                                                          *
#*     DRIVERS_DIR: The location of the font driver sub-dirs, usually       *
#*                  `freetype/lib/drivers'.                                 *
#*                                                                          *
#*     OBJ_DIR:     The location where the compiled object(s) file will be  *
#*                  placed.                                                 *
#*                                                                          *
#*     BASE_H:      A list of pathnames to the base layer's header files on *
#*                  which the driver depends.                               *
#*                                                                          *
#*     FT_CFLAGS:   A set of flags used for compilation of object files.    *
#*                  This contains at least the include paths of the arch    *
#*                  and base directories + optimization + warnings + ANSI   *
#*                  compliance.                                             *
#*                                                                          *
#*     FT_IFLAG:    The flag used to specify an include path on the         *
#*                  compiler command line.  For example, with GCC, this is  *
#*                  `-I', while some other compilers use `/i=' or `-J',     *
#*                  etc.                                                    *
#*                                                                          *
#*     FT_OBJ:      The suffix of an object file for the platform; can be   *
#*                  `o', `obj', `coff', `tco', etc. depending on the        *
#*                  platform.                                               *
#*                                                                          *
#*                                                                          *
#*  It also updates the following variables defined and used in the main    *
#*  Makefile:                                                               *
#*                                                                          *
#*     DRV_OBJ_S:            The list of driver object files in             *
#*                           single-object mode.                            *
#*                                                                          *
#*     DRV_OBJ_M:            The list of driver object files in             *
#*                           multiple-objects mode.                         *
#*                                                                          *
#*     FTINIT_DRIVER_PATHS:  The list of include paths used to compile the  *
#*                           `ftinit' component which registers all font    *
#*                            drivers in the FT_Init_FreeType() function.   *
#*                                                                          *
#*     FTINIT_DRIVER_H:      The list of header dependencies used to        *
#*                           compile the `ftinit' component.                *
#*                                                                          *
#*     FTINIT_DRIVER_MACROS: The list of macros to be defined when          *
#*                           compiling the `ftinit' component.              *
#*                                                                          *
#*  `Single-object compilation' means that each font driver is compiled     *
#*  into a single object file.  This is useful to get rid of all            *
#*  driver-specific entries.                                                *
#*                                                                          *
#****************************************************************************

# include the rules defined for the SFNT driver, which is heavily used
# by the TrueType one..
#
include $(SRC_)sfnt/rules.mk


# TrueType driver directory
#
TT_DIR  := $(SRC_)truetype
TT_DIR_ := $(TT_DIR)$(SEP)


# location of all extensions to the driver, if any
#
TT_EXT_DIR  := $(TT_DIR_)extend
TT_EXT_DIR_ := $(TT_EXT_DIR)$(SEP)

# additional include flags used when compiling the driver
#
TT_INCLUDE := $(SFNT_INCLUDE) $(TT_DIR) $(TT_EXT_DIR)


# compilation flags for the driver
#
TT_CFLAGS  := $(TT_INCLUDE:%=$I%)
TT_COMPILE := $(FT_CC) $(TT_CFLAGS) 


# TrueType driver sources (i.e., C files)
#
TT_DRV_SRC := $(TT_DIR_)ttobjs.c    \
              $(TT_DIR_)ttpload.c   \
              $(TT_DIR_)ttgload.c   \
              $(TT_DIR_)ttinterp.c  \
              $(TT_DIR_)ttdriver.c


# TrueType driver headers
#
TT_DRV_H := $(SFNT_H)               \
            $(TT_DIR_)ttconfig.h    \
            $(TT_DRV_SRC:%.c=%.h)


# default TrueType extensions sources
#
TT_EXT_SRC := $(TT_EXT_DIR_)ttxkern.c  \
              $(TT_EXT_DIR_)ttxgasp.c  \
              $(TT_EXT_DIR_)ttxpost.c  \
              $(TT_EXT_DIR_)ttxcmap.c  \
              $(TT_EXT_DIR_)ttxwidth.c


# default TrueType extensions headers
#
TT_EXT_H := $(TT_EXT_SRC:.c=.h)


# driver object(s)
#
#   TT_DRV_OBJ_M is used during `debug' builds
#   TT_DRV_OBJ_S is used during `release' builds
#
TT_DRV_OBJ_M := $(TT_DRV_SRC:$(TT_DIR_)%.c=$(OBJ_)%.$O)
TT_DRV_OBJ_S := $(OBJ_)truetype.$O


# default extensions objects
#
TT_EXT_OBJ := $(TT_EXT_SRC:$(TT_EXT_DIR_)%.c=$(OBJ_)%.$O)



# driver root source file(s)
#
TT_DRV_SRC_M := $(TT_DRV_SRC) $(SFNT_SRC)
TT_DRV_SRC_S := $(TT_DIR_)truetype.c


# driver - single object
#
#  the driver is recompiled if any of the header or source files is changed
#  as well as any of the shared source files found in `shared/sfnt'
#
$(TT_DRV_OBJ_S): $(BASE_H) $(TT_DRV_H) $(TT_DRV_SRC) $(TT_DRV_SRC_S)
	$(TT_COMPILE) $T$@ $(TT_DRV_SRC_S)



# driver - multiple objects
#
#   All objects are recompiled if any of the header files is changed
#
$(OBJ_)tt%.$O: $(TT_DIR_)tt%.c $(BASE_H) $(TT_DRV_H)
	$(TT_COMPILE) $T$@ $<

$(OBJ_)ttx%.$O: $(TT_EXT_DIR_)ttx%.c $(BASE_H) $(SFNT_H) $(TT_EXT_H)
	$(TT_COMPILE) $T$@ $<

$(OBJ_)tt%.$O: $(SFNT_DIR_)tt%.c $(BASE_H) $(SFNT_H)
	$(TT_COMPILE) $T$@ $<


# treat `ttpload' as a special case, as it includes C files
#
$(OBJ_)ttpload.$O: $(TT_DIR_)ttpload.c $(BASE_H) $(SFNT_SRC) $(TT_DRV_H)
	$(TT_COMPILE) $T$@ $<


# update main driver object lists
#
DRV_OBJS_S += $(TT_DRV_OBJ_S)
DRV_OBJS_M += $(TT_DRV_OBJ_M)

# END
