#
# FreeType 2 Cache configuration rules
#


# Copyright 2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


# Cache driver directory
#
Cache_DIR  := $(SRC_)cache
Cache_DIR_ := $(Cache_DIR)$(SEP)


# compilation flags for the driver
#
Cache_COMPILE := $(FT_COMPILE)


# Cache driver sources (i.e., C files)
#
Cache_DRV_SRC := $(Cache_DIR_)ftlru.c    \
                 $(Cache_DIR_)ftcmanag.c \
                 $(Cache_DIR_)ftcimage.c

# Cache driver headers
#
Cache_DRV_H := $(Cache_DRV_SRC:%c=%h)


# Cache driver object(s)
#
#   Cache_DRV_OBJ_M is used during `multi' builds.
#   Cache_DRV_OBJ_S is used during `single' builds.
#
Cache_DRV_OBJ_M := $(Cache_DRV_SRC:$(Cache_DIR_)%.c=$(OBJ_)%.$O)
Cache_DRV_OBJ_S := $(OBJ_)ftcache.$O

# Cache driver source file for single build
#
Cache_DRV_SRC_S := $(Cache_DIR_)ftcache.c


# Cache driver - single object
#
$(Cache_DRV_OBJ_S): $(Cache_DRV_SRC_S) $(Cache_DRV_SRC) \
                   $(FREETYPE_H) $(Cache_DRV_H)
	$(Cache_COMPILE) $T$@ $(Cache_DRV_SRC_S)


# Cache driver - multiple objects
#
$(OBJ_)%.$O: $(Cache_DIR_)%.c $(FREETYPE_H) $(Cache_DRV_H)
	$(Cache_COMPILE) $T$@ $<


# update main driver object lists
#
DRV_OBJS_S += $(Cache_DRV_OBJ_S)
DRV_OBJS_M += $(Cache_DRV_OBJ_M)


# EOF
