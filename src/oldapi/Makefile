#
#  The OldApi component had not been migrated to the new design yet
#  please wait a few days for some updates. This file should be
#  completely ignored during a BUILD !!
#
#
include drivers/shared/sfnt/Makefile

OLDAPI_DIR := $(TOP_DIR)/oldapi

OLDAPI_INCLUDES := $(FT_IFLAG)$(SFNT_DIR)    \
                   $(FT_IFLAG)$(OLDAPI_DIR)  \
                   $(FT_IFLAG)$(TT_DIR)

OLDAPI_CFLAGS   := $(FT_CFLAGS) $(OLDAPI_INCLUDES)

OLDAPI_OBJ := $(OBJ_DIR)/ttapi.$(FT_OBJ)

# FreeType-1.1 compatible interface
#
$(OLDAPI_OBJ): $(OLDAPI_DIR)/ttapi.c $(OLDAPI_DIR)/truetype.h \
                    $(BASE_H) $(SFNT_H) $(TT_DRV_H)
	$(FT_CC) $(OLDAPI_CFLAGS) $< -o $@

# update main driver object lists
#
DRV_OBJS_S += $(OLDAPI_OBJ)
DRV_OBJS_M += $(OLDAPI_OBJ)

