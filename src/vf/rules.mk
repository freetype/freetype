#
# FreeType 2 VF driver configuration rules
#


# Copyright 1996-2018 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# vf driver directory
#
VF_DIR := $(SRC_DIR)/vf


VF_COMPILE := $(CC) $(ANSIFLAGS)                            \
                     $I$(subst /,$(COMPILER_SEP),$(VF_DIR)) \
                     $(INCLUDE_FLAGS)                        \
                     $(FT_CFLAGS)


# vf driver sources (i.e., C files)
#
VF_DRV_SRC :=  $(VF_DIR)/vflib.c \
               $(VF_DIR)/vfdrivr.c


# vf driver headers
#
VF_DRV_H :=  $(VF_DIR)/vf.h \
             $(VF_DIR)/vfdrivr.h \
             $(VF_DIR)/vferror.h

# vf driver object(s)
#
#   VF_DRV_OBJ_M is used during `multi' builds
#   VF_DRV_OBJ_S is used during `single' builds
#
VF_DRV_OBJ_M := $(VF_DRV_SRC:$(VF_DIR)/%.c=$(OBJ_DIR)/%.$O)
VF_DRV_OBJ_S := $(OBJ_DIR)/vf.$O

# vf driver source file for single build
#
VF_DRV_SRC_S := $(VF_DIR)/vf.c


# vf driver - single object
#
$(VF_DRV_OBJ_S): $(VF_DRV_SRC_S) $(VF_DRV_SRC) $(FREETYPE_H) $(VF_DRV_H)
	$(VF_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $(VF_DRV_SRC_S))


# vf driver - multiple objects
#
$(OBJ_DIR)/%.$O: $(VF_DIR)/%.c $(FREETYPE_H) $(VF_DRV_H)
	$(VF_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


# update main driver object lists
#
DRV_OBJS_S += $(VF_DRV_OBJ_S)
DRV_OBJS_M += $(VF_DRV_OBJ_M)


# EOF
