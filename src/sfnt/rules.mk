#
# FreeType 2 DFNT driver configuration rules
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used modified
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


ifndef SFNT_INCLUDE
  SFNT_INCLUDED := 1

  include $(SRC_)shared/rules.mk

  # SFNT driver directory
  #
  SFNT_DIR  := $(SRC_)sfnt
  SFNT_DIR_ := $(SFNT_DIR)$(SEP)

  # additional include flags used when compiling the driver
  #
  SFNT_INCLUDE := $(SHARED) $(SFNT_DIR)


  # compilation flags for the driver
  #
  SFNT_CFLAGS  := $(SFNT_INCLUDE:%=$I%)
  SFNT_COMPILE := $(FT_COMPILE) $(SFNT_CFLAGS) 


  # driver sources (i.e., C files)
  #
  SFNT_DRV_SRC := $(SFNT_DIR_)ttload.c   \
                  $(SFNT_DIR_)ttcmap.c   \
                  $(SFNT_DIR_)ttsbit.c   \
                  $(SFNT_DIR_)ttpost.c   \
                  $(SFNT_DIR_)sfdriver.c


  # driver headers
  #
  SFNT_DRV_H := $(SHARED_H)            \
                $(SFNT_DIR_)ttload.h   \
                $(SFNT_DIR_)ttsbit.h   \
                $(SFNT_DIR_)ttcmap.h   \
                $(SFNT_DIR_)ttpost.h


  # driver object(s)
  #
  #   SFNT_DRV_OBJ_M is used during `debug' builds
  #   SFNT_DRV_OBJ_S is used during `release' builds
  #
  SFNT_DRV_OBJ_M := $(SFNT_DRV_SRC:$(SFNT_DIR_)%.c=$(OBJ_)%.$O)
  SFNT_DRV_OBJ_S := $(OBJ_)sfnt.$O


  # driver source file(s)
  #
  SFNT_DRV_SRC_M := $(SFNT_DRV_SRC)
  SFNT_DRV_SRC_S := $(SFNT_DIR_)sfnt.c


  # driver - single object
  #
  #  the driver is recompiled if any of the header or source files is
  #  changed as well as any of the shared source files found in
  #  `shared'
  #
  $(SFNT_DRV_OBJ_S): $(BASE_H) $(SHARED_H) $(SFNT_DRV_H) \
                     $(SFNT_DRV_SRC) $(SFNT_DRV_SRC_S)
	  $(SFNT_COMPILE) $T$@ $(SFNT_DRV_SRC_S)


  # driver - multiple objects
  #
  #   All objects are recompiled if any of the header files is changed
  #
  $(OBJ_)tt%.$O: $(SFNT_DIR_)tt%.c $(BASE_H) $(SHARED_H) $(SFNT_DRV_H)
	  $(SFNT_COMPILE) $T$@ $<

  $(OBJ_)sf%.$O: $(SFNT_DIR_)sf%.c $(BASE_H) $(SHARED_H) $(SFNT_DRV_H)
	  $(SFNT_COMPILE) $T$@ $<


  # update main driver object lists
  #
  DRV_OBJS_S += $(SFNT_DRV_OBJ_S)
  DRV_OBJS_M += $(SFNT_DRV_OBJ_M)

endif

# EOF
