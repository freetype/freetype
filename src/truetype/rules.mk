#
# FreeType 2 PSNames driver configuration rules
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used modified
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# Include the rules defined for the SFNT driver, which is heavily used
# by the TrueType one.
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
TT_COMPILE := $(FT_COMPILE) $(TT_CFLAGS) 


# driver sources (i.e., C files)
#
TT_DRV_SRC := $(TT_DIR_)ttobjs.c   \
              $(TT_DIR_)ttpload.c  \
              $(TT_DIR_)ttgload.c  \
              $(TT_DIR_)ttinterp.c \
              $(TT_DIR_)ttdriver.c

# driver headers
#
TT_DRV_H := $(SFNT_H)             \
            $(TT_DRV_SRC:%.c=%.h)


# default extensions sources
#
TT_EXT_SRC := $(TT_EXT_DIR_)ttxkern.c  \
              $(TT_EXT_DIR_)ttxgasp.c  \
              $(TT_EXT_DIR_)ttxpost.c  \
              $(TT_EXT_DIR_)ttxcmap.c  \
              $(TT_EXT_DIR_)ttxwidth.c

# default extensions headers
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


# driver source file(s)
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


# update main driver object lists
#
DRV_OBJS_S += $(TT_DRV_OBJ_S)
DRV_OBJS_M += $(TT_DRV_OBJ_M)

# EOF
