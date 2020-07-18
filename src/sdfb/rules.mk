

# sdf bitmap driver directory
#
SDFB_DIR := $(SRC_DIR)/sdfb


# compilation flags for the driver
#
SDFB_COMPILE := $(CC) $(ANSIFLAGS)                             \
                     $I$(subst /,$(COMPILER_SEP),$(SDFB_DIR))  \
                     $(INCLUDE_FLAGS)                          \
                     $(FT_CFLAGS)


# sdf bitmap driver sources (i.e., C files)
#
SDFB_DRV_SRC := $(SDFB_DIR)/ftsdfbrend.c  \
                $(SDFB_DIR)/ftsdfb.c


# sdf bitmap driver headers
#
SDFB_DRV_H := $(SDFB_DRV_SRC:%.c=%.h)  \
              $(SDFB_DIR)/ftsdfberrs.h


# sdf bitmap driver object(s)
#
#   SDFB_DRV_OBJ_M is used during `multi' builds.
#   SDFB_DRV_OBJ_S is used during `single' builds.
#
SDFB_DRV_OBJ_M := $(SDFB_DRV_SRC:$(SDFB_DIR)/%.c=$(OBJ_DIR)/%.$O)
SDFB_DRV_OBJ_S := $(OBJ_DIR)/sdfb.$O


# sdf driver source file for single build
#
SDFB_DRV_SRC_S := $(SDFB_DIR)/sdfb.c


# sdf bitmap driver - single object
#
$(SDFB_DRV_OBJ_S): $(SDFB_DRV_SRC_S) $(SDFB_DRV_SRC) \
                  $(FREETYPE_H) $(SDFB_DRV_H)
	$(SDFB_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $(SDFB_DRV_SRC_S))


# sdf bitmap driver - multiple objects
#
$(OBJ_DIR)/%.$O: $(SDFB_DIR)/%.c $(FREETYPE_H) $(SDFB_DRV_H)
	$(SDFB_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


# update main driver list
#
DRV_OBJS_S += $(SDFB_DRV_OBJ_S)
DRV_OBJS_M += $(SDFB_DRV_OBJ_M)

# EOF
