#****************************************************************************
#*                                                                          *
#*  Type1z driver Makefile                                                  *
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
#*  The "Type1z" driver is an experimental replacement for the current      *
#*  Type 1 driver. It features a very different loading mechanism that      *
#*  is much faster than the one used by the "normal" driver, and also       *
#*  deals nicely with nearly broken Type 1 font files.. It is also          *
#*  much smaller..                                                          *
#*                                                                          *
#*  Note that it may become a permanent replacement of the current          *
#*  "src/type1" driver in the future..                                      *
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


# Type1z driver directory
#
T1Z_DIR  := $(SRC_)type1z
T1Z_DIR_ := $(T1Z_DIR)$(SEP)


# additional include flags used when compiling the driver
#
T1Z_INCLUDE := $(SHARED) $(T1Z_DIR)
T1Z_COMPILE := $(FT_COMPILE) $(T1Z_INCLUDE:%=$I%)


# Type1 driver sources (i.e., C files)
#
T1Z_DRV_SRC := $(T1Z_DIR_)t1parse.c   \
               $(T1Z_DIR_)t1load.c    \
               $(T1Z_DIR_)t1driver.c  \
               $(T1Z_DIR_)t1afm.c     \
               $(T1Z_DIR_)t1gload.c


# Type1 driver headers
#
T1Z_DRV_H := $(T1Z_DIR_)t1errors.h    \
             $(T1Z_DIR_)t1config.h    \
             $(T1SHARED_H)           \
             $(T1Z_DRV_SRC:%.c=%.h)


# driver object(s)
#
#   T1Z_DRV_OBJ_M is used during `debug' builds
#   T1Z_DRV_OBJ_S is used during `release' builds
#
T1Z_DRV_OBJ_M := $(T1Z_DRV_SRC:$(T1Z_DIR_)%.c=$(OBJ_)%.$O) \
                $(T1SHARED:$(T1SHARED_DIR_)%.c=$(OBJ_)%.$O)
T1Z_DRV_OBJ_S := $(OBJ_)type1z.$O



# driver root source file(s)
#
T1Z_DRV_SRC_M := $(T1Z_DRV_SRC) $(T1SHARED_SRC)
T1Z_DRV_SRC_S := $(T1Z_DIR_)type1z.c


# driver - single object
#
#  the driver is recompiled if any of the header or source files is changed
#
$(T1Z_DRV_OBJ_S): $(BASE_H) $(T1Z_DRV_H) $(T1Z_DRV_SRC) $(T1Z_DRV_SRC_S)
	$(T1Z_COMPILE) $T$@ $(T1Z_DRV_SRC_S)



# driver - multiple objects
#
#   All objects are recompiled if any of the header files is changed
#
$(OBJ_)t1%.$O: $(T1Z_DIR_)t1%.c $(BASE_H) $(T1Z_DRV_H)
	$(T1Z_COMPILE) $T$@ $<

$(OBJ_)t1%.$O: $(T1SHARED_DIR_)t1%.c $(BASE_H) $(T1SHARED_H)
	$(T1Z_COMPILE) $T$@ $<


# update main driver object lists
#
DRV_OBJS_S += $(T1Z_DRV_OBJ_S)
DRV_OBJS_M += $(T1Z_DRV_OBJ_M)

# END
