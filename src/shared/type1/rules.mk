#****************************************************************************
#*                                                                          *
#*  Shared/Type1 Makefile                                                   *
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
#*  the shared source code in `freetype2/lib/drivers/type1'.  This code     *
#*  contains type definitions as well as interface which are common to all  *
#*  supported Postscript font formats (i.e. Type1 and Type2).               *
#*                                                                          *
#*                                                                          *
#*  The purpose of this Makefile is to define make variables that are used  *
#*  directly by the parent Makefile.                                        *
#*                                                                          *
#****************************************************************************


# T1SHARED_DIR is the directory to the `shared/type1' sources
#
T1SHARED_DIR  := $(SRC)/shared/type1
T1SHARED_DIR_ := $(T1SHARED_DIR)$(SEP)


# T1SHARED_H is the list of all header files on which the client drivers
# depend
#
T1SHARED_H := $(T1SHARED_DIR_)t1types.h  \
              $(T1SHARED_DIR_)t1encode.h

# T1SHARED_SRC is the list of all shared source files that are included
# by any client driver
#
T1SHARED_SRC := $(T1SHARED_DIR_)t1encode.c


# END
