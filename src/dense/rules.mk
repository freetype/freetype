#
# FreeType 2 DENSE renderer module build rules
#


# Copyright (C) 1996-2021 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# DENSE driver directory
#
DENSE_DIR := $(SRC_DIR)/dense


# compilation flags for the driver
#
DENSE_COMPILE := $(CC) $(ANSIFLAGS)                               \
                        $I$(subst /,$(COMPILER_SEP),$(DENSE_DIR)) \
                        $(INCLUDE_FLAGS)                           \
                        $(FT_CFLAGS)


# DENSE driver sources (i.e., C files)
#
DENSE_DRV_SRC := $(DENSE_DIR)/ftdense.c  \
                  $(DENSE_DIR)/ftdenserend.c


# DENSE driver headers
#
DENSE_DRV_H := $(DENSE_DRV_SRC:%c=%h)  \
                $(DENSE_DIR)/ftdenseerrs.h


# DENSE driver object(s)
#
#   DENSE_DRV_OBJ_M is used during `multi' builds.
#   DENSE_DRV_OBJ_S is used during `single' builds.
#
DENSE_DRV_OBJ_M := $(DENSE_DRV_SRC:$(DENSE_DIR)/%.c=$(OBJ_DIR)/%.$O)
DENSE_DRV_OBJ_S := $(OBJ_DIR)/dense.$O

# DENSE driver source file for single build
#
DENSE_DRV_SRC_S := $(DENSE_DIR)/dense.c


# DENSE driver - single object
#
$(DENSE_DRV_OBJ_S): $(DENSE_DRV_SRC_S) $(DENSE_DRV_SRC) \
                     $(FREETYPE_H) $(DENSE_DRV_H)
	$(DENSE_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $(DENSE_DRV_SRC_S))


# DENSE driver - multiple objects
#
$(OBJ_DIR)/%.$O: $(DENSE_DIR)/%.c $(FREETYPE_H) $(DENSE_DRV_H)
	$(DENSE_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


# update main driver object lists
#
DRV_OBJS_S += $(DENSE_DRV_OBJ_S)
DRV_OBJS_M += $(DENSE_DRV_OBJ_M)


# EOF
