#****************************************************************************
#*                                                                          *
#*  shared files Makefile                                                   *
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

ifndef SHARED_RULES
SHARED_RULES := 1

SHARED  := $(SRC_)shared
SHARED_ := $(SHARED)$(SEP)

SHARED_H   := $(wildcard $(SHARED_)*.h)
SHARED_SRC := $(wildcard $(SHARED_)*.c) 

endif

# END
