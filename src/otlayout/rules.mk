#
# FreeType 2 OpneType driver configuration rules
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

# OpenType driver directory
#
OT_DIR := $(SRC_DIR)/otlayout


OT_COMPILE := $(FT_COMPILE) $I$(subst /,$(COMPILER_SEP),$(OT_DIR))


# OT driver sources (i.e., C files)
#
OT_DRV_SRC :=  $(OT_DIR)/otobjs.c		\
	       $(OT_DIR)/otlayout.c             \
               $(OT_DIR)/otdriver.c		\
               $(OT_DIR)/ftxgdef.c              \
               $(OT_DIR)/ftxgpos.c   		\
               $(OT_DIR)/ftxopen.c		\
               $(OT_DIR)/ftxgsub.c		\
	       $(OT_DIR)/ot-info.c 		\
               $(OT_DIR)/ot-ruleset.c		\
	       $(OT_DIR)/ot-array.c             \
	       $(OT_DIR)/ot-unicode.c


# OT driver headers
#
OT_DRV_H := $(OT_DIR)/otdriver.h		\
	    $(OT_DIR)/fterrcompat.h		\
	    $(OT_DIR)/ftxgdef.h			\
            $(OT_DIR)/ftxgpos.h   		\
            $(OT_DIR)/ftxgsub.h			\
	    $(OT_DIR)/ftxopen.h  		\
	    $(OT_DIR)/ftxopenf.h  		\
            $(OT_DIR)/ot-info.h                 \
	    $(OT_DIR)/ot-ruleset.h              \
            $(OT_DIR)/ot-types.h                \
            $(OT_DIR)/ot-array.h                \
	    $(OT_DIR)/otltypes.h                \
	    $(OT_DIR)/oterrors.h                \
	    $(OT_DIR)/otobjs.h                  \
            $(OT_DIR)/ot-unicode.h

# OT driver object(s)
#
#   OT_DRV_OBJ_M is used during `multi' builds
#   OT_DRV_OBJ_S is used during `single' builds
#
OT_DRV_OBJ_M := $(OT_DRV_SRC:$(OT_DIR)/%.c=$(OBJ_DIR)/%.$O)
OT_DRV_OBJ_S := $(OBJ_DIR)/ot.$O

# OT driver source file for single build
#
OT_DRV_SRC_S := $(OT_DIR)/ot.c


# OT driver - single object
#
$(OT_DRV_OBJ_S): $(OT_DRV_SRC_S) $(OT_DRV_SRC) $(FREETYPE_H) $(OT_DRV_H)
	$(OT_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $(OT_DRV_SRC_S))


# OT driver - multiple objects
#
$(OBJ_DIR)/%.$O: $(OT_DIR)/%.c $(FREETYPE_H) $(OT_DRV_H)
	$(OT_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


# update main driver object lists
#
DRV_OBJS_S += $(OT_DRV_OBJ_S)
DRV_OBJS_M += $(OT_DRV_OBJ_M)


# EOF
