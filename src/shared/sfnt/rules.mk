#****************************************************************************
#*                                                                          *
#*  SFNT/TrueType Makefile                                                  *
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
#*  This file is to be included by the Makefiles of each driver that uses   *
#*  the shared source code in `freetype2/lib/drivers/sfnt'.  This code      *
#*  contains type definitions as well as interface which are common to all  *
#*  `sfnt' font formats (i.e., TrueType, OpenType-TTF, and OpenType-CFF).   *
#*                                                                          *
#*                                                                          *
#*  The purpose of this Makefile is to define two make variables that are   *
#*  used directly by the parent Makefile.                                   *
#*                                                                          *
#****************************************************************************


# SFNT_DIR is the directory to the `sfnt' sources
#
SFNT_DIR  := $(SRC)$(SEP)shared$(SEP)sfnt
SFNT_DIR_ := $(SFNT_DIR)$(SEP)


# SFNT_H is the list of all header files on which the client drivers depend
#
SFNT_H := $(SFNT_DIR_)tttypes.h  \
          $(SFNT_DIR_)ttload.h   \
          $(SFNT_DIR_)ttsbit.h   \
          $(SFNT_DIR_)ttpost.h   \
          $(SFNT_DIR_)sfnt.h

# SFNT_SRC is the list of all shared source files that are included by any
# client driver
#
SFNT_SRC := $(SFNT_DIR_)ttload.c  \
            $(SFNT_DIR_)ttsbit.c  \
            $(SFNT_DIR_)ttpost.c


# END
