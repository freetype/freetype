#
# FreeType 2 driver configuration rules
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used modified
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


#****************************************************************************
#*                                                                          *
#*  The "Type1z" driver is an experimental replacement for the current      *
#*  Type 1 driver.  It features a very different loading mechanism that     *
#*  is much faster than the one used by the `normal' driver, and also       *
#*  deals nicely with nearly broken Type 1 font files. It is also           *
#*  much smaller...                                                         *
#*                                                                          *
#*  Note that it may become a permanent replacement of the current          *
#*  "src/type1" driver in the future..                                      *
#*                                                                          *
#****************************************************************************

# Type1z driver directory
#
T1Z_DIR  := $(SRC_)type1z
T1Z_DIR_ := $(T1Z_DIR)$(SEP)


# additional include flags used when compiling the driver
#
T1Z_INCLUDE := $(SHARED) $(T1Z_DIR)
T1Z_COMPILE := $(FT_COMPILE) $(T1Z_INCLUDE:%=$I%)


# Type1 driver sources (i.e., C files)
#
T1Z_DRV_SRC := $(T1Z_DIR_)t1parse.c  \
               $(T1Z_DIR_)t1load.c   \
               $(T1Z_DIR_)t1driver.c \
               $(T1Z_DIR_)t1afm.c    \
               $(T1Z_DIR_)t1gload.c

# Type1 driver headers
#
T1Z_DRV_H := $(T1Z_DIR_)t1errors.h  \
             $(T1SHARED_H)          \
             $(T1Z_DRV_SRC:%.c=%.h)


# driver object(s)
#
#   T1Z_DRV_OBJ_M is used during `debug' builds
#   T1Z_DRV_OBJ_S is used during `release' builds
#
T1Z_DRV_OBJ_M := $(T1Z_DRV_SRC:$(T1Z_DIR_)%.c=$(OBJ_)%.$O) \
                 $(T1SHARED:$(T1SHARED_DIR_)%.c=$(OBJ_)%.$O)
T1Z_DRV_OBJ_S := $(OBJ_)type1z.$O


# driver source file(s)
#
T1Z_DRV_SRC_M := $(T1Z_DRV_SRC) $(T1SHARED_SRC)
T1Z_DRV_SRC_S := $(T1Z_DIR_)type1z.c


# driver - single object
#
#  the driver is recompiled if any of the header or source files is changed
#
$(T1Z_DRV_OBJ_S): $(BASE_H) $(T1Z_DRV_H) $(T1Z_DRV_SRC) $(T1Z_DRV_SRC_S)
	$(T1Z_COMPILE) $T$@ $(T1Z_DRV_SRC_S)


# driver - multiple objects
#
#   All objects are recompiled if any of the header files is changed
#
$(OBJ_)t1%.$O: $(T1Z_DIR_)t1%.c $(BASE_H) $(T1Z_DRV_H)
	$(T1Z_COMPILE) $T$@ $<

$(OBJ_)t1%.$O: $(T1SHARED_DIR_)t1%.c $(BASE_H) $(T1SHARED_H)
	$(T1Z_COMPILE) $T$@ $<


# update main driver object lists
#
DRV_OBJS_S += $(T1Z_DRV_OBJ_S)
DRV_OBJS_M += $(T1Z_DRV_OBJ_M)

# EOF
