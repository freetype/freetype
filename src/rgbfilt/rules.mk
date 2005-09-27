#
# FreeType 2 RGB filter configuration rules
#

# rgb driver directory
#
RGBFILT_DIR := $(SRC_DIR)/rgbfilt


RGBFILT_COMPILE := $(FT_COMPILE) $I$(subst /,$(COMPILER_SEP),$(RGBFILT_DIR))


# RGB filter sources (i.e., C files)
#
RGBFILT_DRV_SRC := $(RGBFILT_DIR)/ftrgb.c

# RGB filter driver headers
#
RGBFILT_DRV_H := $(RGBFILT_DIR)/ftrgbgen.h \
                 $(RGBFILT_DIR)/ftrgbgn2.h

# RGB filter driver object(s)
#
RGBFILT_DRV_OBJ := $(RGBFILT_DRV_SRC:$(RGBFILT_DIR)/%.c=$(OBJ_DIR)/%.$O)

$(RGBFILT_DRV_OBJ): $(RGBFILT_DRV_SRC) $(FREETYPE_H) $(RGBFILT_DRV_H)
	$(RGBFILT_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $(RGBFILT_DRV_SRC))

# update main driver object lists
#
DRV_OBJS_S += $(RGBFILT_DRV_OBJ)
DRV_OBJS_M += $(RGBFILT_DRV_OBJ)


# EOF
