#****************************************************************************
#*                                                                          *
#*  SFNT driver Makefile                                                    *
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

ifndef SFNT_INCLUDE
SFNT_INCLUDED := 1

include $(SRC_)shared/rules.mk

# SFNT driver directory
#
SFNT_DIR  := $(SRC_)sfnt
SFNT_DIR_ := $(SFNT_DIR)$(SEP)

# additional include flags used when compiling the driver
#
SFNT_INCLUDE := $(SHARED) $(SFNT_DIR)


# compilation flags for the driver
#
SFNT_CFLAGS  := $(SFNT_INCLUDE:%=$I%)
SFNT_COMPILE := $(FT_COMPILE) $(SFNT_CFLAGS) 


# TrueType driver sources (i.e., C files)
#
SFNT_DRV_SRC := $(SFNT_DIR_)ttload.c    \
                $(SFNT_DIR_)ttcmap.c    \
                $(SFNT_DIR_)ttsbit.c    \
                $(SFNT_DIR_)ttpost.c    \
                $(SFNT_DIR_)sfdriver.c  \


# TrueType driver headers
#
SFNT_DRV_H := $(SHARED_H)               \
              $(SFNT_DIR_)sfconfig.h    \
              $(SFNT_DIR_)ttload.h      \
              $(SFNT_DIR_)ttsbit.h      \
              $(SFNT_DIR_)ttcmap.h      \
              $(SFNT_DIR_)ttpost.h


# driver object(s)
#
#   SFNT_DRV_OBJ_M is used during `debug' builds
#   SFNT_DRV_OBJ_S is used during `release' builds
#
SFNT_DRV_OBJ_M := $(SFNT_DRV_SRC:$(SFNT_DIR_)%.c=$(OBJ_)%.$O)
SFNT_DRV_OBJ_S := $(OBJ_)sfnt.$O


# driver root source file(s)
#
SFNT_DRV_SRC_M := $(SFNT_DRV_SRC)
SFNT_DRV_SRC_S := $(SFNT_DIR_)sfnt.c


# driver - single object
#
#  the driver is recompiled if any of the header or source files is changed
#  as well as any of the shared source files found in `shared/sfnt'
#
$(SFNT_DRV_OBJ_S): $(BASE_H) $(SHARED_H) $(SFNT_DRV_H) $(SFNT_DRV_SRC) $(SFNT_DRV_SRC_S)
	$(SFNT_COMPILE) $T$@ $(SFNT_DRV_SRC_S)



# driver - multiple objects
#
#   All objects are recompiled if any of the header files is changed
#
$(OBJ_)tt%.$O: $(SFNT_DIR_)tt%.c $(BASE_H) $(SHARED_H) $(SFNT_DRV_H)
	$(SFNT_COMPILE) $T$@ $<

$(OBJ_)sf%.$O: $(SFNT_DIR_)sf%.c $(BASE_H) $(SHARED_H) $(SFNT_DRV_H)
	$(SFNT_COMPILE) $T$@ $<

# update main driver object lists
#
DRV_OBJS_S += $(SFNT_DRV_OBJ_S)
DRV_OBJS_M += $(SFNT_DRV_OBJ_M)

endif
# END
