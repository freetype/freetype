#
# FreeType 2 OpenType/CFF driver configuration rules
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


# OpenType driver directory
#
T2_DIR  := $(SRC_)cff
T2_DIR_ := $(T2_DIR)$(SEP)


# location of all extensions to the driver, if any
#
T2_EXT_DIR  := $(T2_DIR_)extend
T2_EXT_DIR_ := $(T2_EXT_DIR)$(SEP)

# additional include flags used when compiling the driver
#
T2_INCLUDE := $(SFNT_INCLUDE) $(T2_DIR) $(T2_EXT_DIR)


# compilation flags for the driver
#
T2_CFLAGS  := $(T2_INCLUDE:%=$I%)
T2_COMPILE := $(FT_COMPILE) $(T2_CFLAGS)


# driver sources (i.e., C files)
#
T2_DRV_SRC := $(T2_DIR_)t2objs.c   \
              $(T2_DIR_)t2load.c   \
              $(T2_DIR_)t2gload.c  \
              $(T2_DIR_)t2parse.c  \
              $(T2_DIR_)t2driver.c

# driver headers
#
T2_DRV_H := $(SFNT_H)             \
            $(T2_DRV_SRC:%.c=%.h)


# default extensions headers
#
T2_EXT_H := $(T2_EXT_SRC:.c=.h)


# driver object(s)
#
#   T2_DRV_OBJ_M is used during `debug' builds
#   T2_DRV_OBJ_S is used during `release' builds
#
T2_DRV_OBJ_M := $(T2_DRV_SRC:$(T2_DIR_)%.c=$(OBJ_)%.$O)
T2_DRV_OBJ_S := $(OBJ_)cff.$O


# default extensions objects
#
T2_EXT_OBJ := $(T2_EXT_SRC:$(T2_EXT_DIR_)%.c=$(OBJ_)%.$O)


# driver source file(s)
#
T2_DRV_SRC_M := $(T2_DRV_SRC) $(SFNT_SRC)
T2_DRV_SRC_S := $(T2_DIR_)cff.c


# driver - single object
#
#  the driver is recompiled if any of the header or source files is changed
#  as well as any of the shared source files found in `shared/sfnt'
#
$(T2_DRV_OBJ_S): $(BASE_H) $(T2_DRV_H) $(T2_DRV_SRC) $(T2_DRV_SRC_S)
	$(T2_COMPILE) $T$@ $(T2_DRV_SRC_S)



# driver - multiple objects
#
#   All objects are recompiled if any of the header files is changed
#
$(OBJ_)t2%.$O: $(T2_DIR_)t2%.c $(BASE_H) $(T2_DRV_H)
	$(T2_COMPILE) $T$@ $<

$(OBJ_)t2x%.$O: $(T2_EXT_DIR_)t2x%.c $(BASE_H) $(SFNT_H) $(T2_EXT_H)
	$(T2_COMPILE) $T$@ $<

$(OBJ_)t2%.$O: $(SFNT_DIR_)t2%.c $(BASE_H) $(SFNT_H)
	$(T2_COMPILE) $T$@ $<


# update main driver object lists
#
DRV_OBJS_S += $(T2_DRV_OBJ_S)
DRV_OBJS_M += $(T2_DRV_OBJ_M)

# EOF
