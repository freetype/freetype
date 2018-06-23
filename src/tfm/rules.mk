#
# FreeType 2 TFM driver configuration rules
#


# Copyright 1996-2018 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# tfm driver directory
#
TFM_DIR := $(SRC_DIR)/tfm


TFM_COMPILE := $(CC) $(ANSIFLAGS)                            \
                     $I$(subst /,$(COMPILER_SEP),$(TFM_DIR)) \
                     $(INCLUDE_FLAGS)                        \
                     $(FT_CFLAGS)


# tfm driver sources (i.e., C files)
#
TFM_DRV_SRC :=  $(TFM_DIR)/tfmlib.c \
               $(TFM_DIR)/tfmdrivr.c


# tfm driver headers
#
TFM_DRV_H :=  $(TFM_DIR)/tfm.h \
             $(TFM_DIR)/tfmdrivr.h \
             $(TFM_DIR)/tfmerror.h

# tfm driver object(s)
#
#   TFM_DRV_OBJ_M is used during `multi' builds
#   TFM_DRV_OBJ_S is used during `single' builds
#
TFM_DRV_OBJ_M := $(TFM_DRV_SRC:$(TFM_DIR)/%.c=$(OBJ_DIR)/%.$O)
TFM_DRV_OBJ_S := $(OBJ_DIR)/tfm.$O

# tfm driver source file for single build
#
TFM_DRV_SRC_S := $(TFM_DIR)/tfm.c


# tfm driver - single object
#
$(TFM_DRV_OBJ_S): $(TFM_DRV_SRC_S) $(TFM_DRV_SRC) $(FREETYPE_H) $(TFM_DRV_H)
	$(TFM_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $(TFM_DRV_SRC_S))


# tfm driver - multiple objects
#
$(OBJ_DIR)/%.$O: $(TFM_DIR)/%.c $(FREETYPE_H) $(TFM_DRV_H)
	$(TFM_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


# update main driver object lists
#
DRV_OBJS_S += $(TFM_DRV_OBJ_S)
DRV_OBJS_M += $(TFM_DRV_OBJ_M)


# EOF
