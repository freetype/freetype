#
# FreeType 2 PFR driver configuration rules
#


# Copyright 1996-2002 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# Pfr driver directory
#
PFR_DIR  := $(SRC_)pfr
PFR_DIR_ := $(PFR_DIR)$(SEP)


# compilation flags for the driver
#
PFR_COMPILE := $(FT_COMPILE) $I$(PFR_DIR)


# Pfr driver sources (i.e., C files)
#
PFR_DRV_SRC := $(PFR_DIR_)pfrload.c  \
               $(PFR_DIR_)pfrgload.c \
               $(PFR_DIR_)pfrcmap.c  \
               $(PFR_DIR_)pfrdrivr.c \
               $(PFR_DIR_)pfrobjs.c

# Pfr driver headers
#
PFR_DRV_H := $(PFR_DRV_SRC:%.c=%.h)


# Pfr driver object(s)
#
#   PFR_DRV_OBJ_M is used during `multi' builds
#   PFR_DRV_OBJ_S is used during `single' builds
#
PFR_DRV_OBJ_M := $(PFR_DRV_SRC:$(PFR_DIR_)%.c=$(OBJ_)%.$O)
PFR_DRV_OBJ_S := $(OBJ_)pfr.$O

# Pfr driver source file for single build
#
PFR_DRV_SRC_S := $(PFR_DIR_)pfr.c


# Pfr driver - single object
#
$(PFR_DRV_OBJ_S): $(PFR_DRV_SRC_S) $(PFR_DRV_SRC) $(FREETYPE_H) $(PFR_DRV_H)
	$(PFR_COMPILE) $T$@ $(PFR_DRV_SRC_S)


# Pfr driver - multiple objects
#
$(OBJ_)%.$O: $(PFR_DIR_)%.c $(FREETYPE_H) $(PFR_DRV_H)
	$(PFR_COMPILE) $T$@ $<


# update main driver object lists
#
DRV_OBJS_S += $(PFR_DRV_OBJ_S)
DRV_OBJS_M += $(PFR_DRV_OBJ_M)

# EOF
