#
# FreeType 2 PSNames driver configuration rules
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used modified
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


ifndef PSNAMES_INCLUDE
  PSNAMES_INCLUDED := 1

  include $(SRC_)shared/rules.mk

  # PSNAMES driver directory
  #
  PSNAMES_DIR  := $(SRC_)psnames
  PSNAMES_DIR_ := $(PSNAMES_DIR)$(SEP)

  # additional include flags used when compiling the driver
  #
  PSNAMES_INCLUDE := $(SHARED) $(PSNAMES_DIR)


  # compilation flags for the driver
  #
  PSNAMES_CFLAGS  := $(PSNAMES_INCLUDE:%=$I%)
  PSNAMES_COMPILE := $(FT_COMPILE) $(PSNAMES_CFLAGS) 


  # driver sources (i.e., C files)
  #
  PSNAMES_DRV_SRC := $(PSNAMES_DIR_)psdriver.c


  # driver headers
  #
  PSNAMES_DRV_H := $(SHARED_H)               \
                   $(PSNAMES_DIR_)psdriver.h \
                   $(PSNAMES_DIR_)pstables.h


  # driver object(s)
  #
  #   PSNAMES_DRV_OBJ_M is used during `debug' builds
  #   PSNAMES_DRV_OBJ_S is used during `release' builds
  #
  PSNAMES_DRV_OBJ_M := $(PSNAMES_DRV_SRC:$(PSNAMES_DIR_)%.c=$(OBJ_)%.$O)
  PSNAMES_DRV_OBJ_S := $(OBJ_)psnames.$O


  # driver source file(s)
  #
  PSNAMES_DRV_SRC_M := $(PSNAMES_DRV_SRC)
  PSNAMES_DRV_SRC_S := $(PSNAMES_DIR_)psdriver.c


  # driver - single object
  #
  #  the driver is recompiled if any of the header or source files is
  #  changed as well as any of the shared source files found in
  #  `shared'
  #
  $(PSNAMES_DRV_OBJ_S): $(BASE_H) $(SHARED_H) $(PSNAMES_DRV_H) \
                        $(PSNAMES_DRV_SRC) $(PSNAMES_DRV_SRC_S)
	  $(PSNAMES_COMPILE) $T$@ $(PSNAMES_DRV_SRC_S)


  # driver - multiple objects
  #
  #   All objects are recompiled if any of the header files is changed.
  #
  $(OBJ_)ps%.$O: $(PSNAMES_DIR_)ps%.c $(BASE_H) $(SHARED_H) $(PSNAMES_DRV_H)
	  $(PSNAMES_COMPILE) $T$@ $<


  # update main driver object lists
  #
  DRV_OBJS_S += $(PSNAMES_DRV_OBJ_S)
  DRV_OBJS_M += $(PSNAMES_DRV_OBJ_M)

endif

# EOF
