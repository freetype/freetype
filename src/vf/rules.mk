#
# FreeType 2 PK driver configuration rules
#


# Copyright 1996-2018 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# pk driver directory
#
PK_DIR := $(SRC_DIR)/pk


PK_COMPILE := $(CC) $(ANSIFLAGS)                            \
                     $I$(subst /,$(COMPILER_SEP),$(PK_DIR)) \
                     $(INCLUDE_FLAGS)                        \
                     $(FT_CFLAGS)


# pk driver sources (i.e., C files)
#
PK_DRV_SRC :=  $(PK_DIR)/pklib.c \
               $(PK_DIR)/pkdrivr.c


# pk driver headers
#
PK_DRV_H :=  $(PK_DIR)/pk.h \
             $(PK_DIR)/pkdrivr.h \
             $(PK_DIR)/pkerror.h

# pk driver object(s)
#
#   PK_DRV_OBJ_M is used during `multi' builds
#   PK_DRV_OBJ_S is used during `single' builds
#
PK_DRV_OBJ_M := $(PK_DRV_SRC:$(PK_DIR)/%.c=$(OBJ_DIR)/%.$O)
PK_DRV_OBJ_S := $(OBJ_DIR)/pk.$O

# pk driver source file for single build
#
PK_DRV_SRC_S := $(PK_DIR)/pk.c


# pk driver - single object
#
$(PK_DRV_OBJ_S): $(PK_DRV_SRC_S) $(PK_DRV_SRC) $(FREETYPE_H) $(PK_DRV_H)
	$(PK_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $(PK_DRV_SRC_S))


# pk driver - multiple objects
#
$(OBJ_DIR)/%.$O: $(PK_DIR)/%.c $(FREETYPE_H) $(PK_DRV_H)
	$(PK_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


# update main driver object lists
#
DRV_OBJS_S += $(PK_DRV_OBJ_S)
DRV_OBJS_M += $(PK_DRV_OBJ_M)


# EOF
