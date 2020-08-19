

# sdf driver directory
#
SDF_DIR := $(SRC_DIR)/sdf


# compilation flags for the driver
#
SDF_COMPILE := $(CC) $(ANSIFLAGS)                            \
                     $I$(subst /,$(COMPILER_SEP),$(SDF_DIR)) \
                     $(INCLUDE_FLAGS)                        \
                     $(FT_CFLAGS)


# sdf driver sources (i.e., C files)
#
SDF_DRV_SRC := $(SDF_DIR)/ftsdfrend.c \
               $(SDF_DIR)/ftsdf.c     \
               $(SDF_DIR)/ftbsdf.c


# sdf driver headers
#
SDF_DRV_H := $(SDF_DIR)/ftsdfrend.h   \
             $(SDF_DIR)/ftsdf.h       \
             $(SDF_DIR)/ftsdferrs.h   \
             $(SDF_DIR)/ftsdfcommon.h


# sdf driver object(s)
#
#   SDF_DRV_OBJ_M is used during `multi' builds.
#   SDF_DRV_OBJ_S is used during `single' builds.
#
SDF_DRV_OBJ_M := $(SDF_DRV_SRC:$(SDF_DIR)/%.c=$(OBJ_DIR)/%.$O)
SDF_DRV_OBJ_S := $(OBJ_DIR)/sdf.$O


# sdf driver source file for single build
#
SDF_DRV_SRC_S := $(SDF_DIR)/sdf.c


# sdf driver - single object
#
$(SDF_DRV_OBJ_S): $(SDF_DRV_SRC_S) $(SDF_DRV_SRC) \
                  $(FREETYPE_H) $(SDF_DRV_H)
	$(SDF_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $(SDF_DRV_SRC_S))


# sdf driver - multiple objects
#
$(OBJ_DIR)/%.$O: $(SDF_DIR)/%.c $(FREETYPE_H) $(SDF_DRV_H)
	$(SDF_COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


# update main driver list
#
DRV_OBJS_S += $(SDF_DRV_OBJ_S)
DRV_OBJS_M += $(SDF_DRV_OBJ_M)


# EOF
