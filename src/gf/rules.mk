#
# FreeType 2 GF driver configuration rules
#


# Copyright 1996-2018 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# gf driver directory
#
GF_DIR := $(SRC_DIR)/gf


GF_COMPILE := $(CC) $(ANSIFLAGS)                            \
                     $I$(subst /,$(COMPILER_SEP),$(GF_DIR)) \
                     $(INCLUDE_FLAGS)                        \
                     $(FT_CFLAGS)


# gf driver sources (i.e., C files)
#
GF_DRV_SRC :=  $(GF_DIR)/gflib.c \
               $(GF_DIR)/gfdrivr.c


# gf driver headers
#
GF_DRV_H :=  $(GF_DIR)/gf.h \
             $(GF_DIR)/gfdrivr.h \
             $(GF_DIR)/gferror.h

# gf driver object(s)
#
#   GF_DRV_OBJ_M is used during `multi' builds
#   GF_DRV_OBJ_S is used during `single' builds
#
GF_DRV_OBJ_M := $(GF_DRV_SRC:$(GF_DIR)/%.c=$(OBJ_DIR)/%.$O)
GF_DRV_OBJ_S := $(OBJ_DIR)/gf.$O

# gf driver source file for single build
#
GF_DRV_SRC_S := $(GF_DIR)/gf.c


# gf driver - single object
#
$(GF_DRV_OBJ_S): $(GF_DRV_SRC_S) $(GF_DRV_SRC) $(FREETYPE_H) $(GF_DRV_H)
	$(GF_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $(GF_DRV_SRC_S))


# gf driver - multiple objects
#
$(OBJ_DIR)/%.$O: $(GF_DIR)/%.c $(FREETYPE_H) $(GF_DRV_H)
	$(GF_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


# update main driver object lists
#
DRV_OBJS_S += $(GF_DRV_OBJ_S)
DRV_OBJS_M += $(GF_DRV_OBJ_M)


# EOF
