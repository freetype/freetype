#
# FreeType 2 renderer module build rules
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# renderer driver directory
#
REND_DIR  := $(SRC_)renderer
REND_DIR_ := $(REND_DIR)$(SEP)


# additional include flags used when compiling the driver
#
REND_INCLUDE := $(REND_DIR)

# compilation flags for the driver
#
REND_CFLAGS  := $(REND_INCLUDE:%=$I%)
REND_COMPILE := $(FT_COMPILE) $(REND_CFLAGS)


# renderer driver sources (i.e., C files)
#
REND_DRV_SRC := $(REND_DIR_)ftraster.c \
                $(REND_DIR_)ftgrays.c  \
                $(REND_DIR_)renderer.c

# renderer driver headers
#
REND_DRV_H := $(REND_DRV_SRC:%c=%h)


# renderer driver object(s)
#
#   REND_DRV_OBJ_M is used during `multi' builds.
#   REND_DRV_OBJ_S is used during `single' builds.
#
REND_DRV_OBJ_M := $(REND_DRV_SRC:$(REND_DIR_)%.c=$(OBJ_)%.$O)
REND_DRV_OBJ_S := $(REND_DRV_OBJ_M)

# renderer driver source file for single build
#
#REND_DRV_SRC_S := $(REND_DIR_)renderer.c


# renderer driver - single object
#
#$(REND_DRV_OBJ_S): $(REND_DRV_SRC_S) $(REND_DRV_SRC) \
#                   $(FREETYPE_H) $(REND_DRV_H)
#	$(REND_COMPILE) $T$@ $(REND_DRV_SRC_S)


# renderer driver - multiple objects
#
$(OBJ_)%.$O: $(REND_DIR_)%.c $(FREETYPE_H) $(REND_DRV_H)
	$(REND_COMPILE) $T$@ $<


# update main driver object lists
#
DRV_OBJS_S += $(REND_DRV_OBJ_M)
DRV_OBJS_M += $(REND_DRV_OBJ_M)


# EOF
