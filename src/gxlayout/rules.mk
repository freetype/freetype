#
# FreeType 2 AAT/TrueTypeGX driver configuration rules
#


# Copyright 2004 by RedHat K.K.
# This file is derived from cff/rules.mk.

# ----------------------------------------------------------------------
#
# FreeType 2 OpenType/CFF driver configuration rules
#

# Copyright 1996-2000, 2001, 2003 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.
# ----------------------------------------------------------------------

# Development of the code in rules.mk is support of
# Information-technology Promotion Agency, Japan.   

# AAT/TrueTypeGX driver directory
#
GX_DIR := $(SRC_DIR)/gxlayout


GX_COMPILE := $(FT_COMPILE) $I$(subst /,$(COMPILER_SEP),$(GX_DIR))


# GX driver sources (i.e., C files)
#
GX_DRV_SRC :=  $(GX_DIR)/gxobjs.c		\
               $(GX_DIR)/gxload.c		\
               $(GX_DIR)/gxdriver.c		\
               $(GX_DIR)/gxlookuptbl.c		\
               $(GX_DIR)/gxstatetbl.c		\
               $(GX_DIR)/gxutils.c		\
	       $(GX_DIR)/gxlayout.c		\
               $(GX_DIR)/gxfeatreg.c		\
	       $(GX_DIR)/gxlfeatreg.c		\
               $(GX_DIR)/gxaccess.c             \
               $(GX_DIR)/gxvm.c                 \
               $(GX_DIR)/gxdump.c              

# GX driver headers
#
GX_DRV_H := $(GX_DIR)/gxdriver.h		\
            $(GX_DIR)/gxload.h			\
	    $(GX_DIR)/gxlookuptbl.h		\
	    $(GX_DIR)/gxobjs.h			\
            $(GX_DIR)/gxstatetbl.h		\
            $(GX_DIR)/gxutils.h			\
	    $(GX_DIR)/gxfeatreg.h		\
	    $(GX_DIR)/gxlfeatreg.h		\
            $(GX_DIR)/gxaccess.h                \
	    $(GX_DIR)/gxerrors.h                \
            $(GX_DIR)/gxvm.h                    \
	    $(GX_DIR)/gxdump.h              

# GX driver object(s)
#
#   GX_DRV_OBJ_M is used during `multi' builds
#   GX_DRV_OBJ_S is used during `single' builds
#
GX_DRV_OBJ_M := $(GX_DRV_SRC:$(GX_DIR)/%.c=$(OBJ_DIR)/%.$O)
GX_DRV_OBJ_S := $(OBJ_DIR)/gx.$O

# GX driver source file for single build
#
GX_DRV_SRC_S := $(GX_DIR)/gx.c


# GX driver - single object
#
$(GX_DRV_OBJ_S): $(GX_DRV_SRC_S) $(GX_DRV_SRC) $(FREETYPE_H) $(GX_DRV_H)
	$(GX_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $(GX_DRV_SRC_S))


# GX driver - multiple objects
#
$(OBJ_DIR)/%.$O: $(GX_DIR)/%.c $(FREETYPE_H) $(GX_DRV_H)
	$(GX_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


# update main driver object lists
#
DRV_OBJS_S += $(GX_DRV_OBJ_S)
DRV_OBJS_M += $(GX_DRV_OBJ_M)


# EOF
