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
CID_DIR  := $(SRC_)cid
CID_DIR_ := $(CID_DIR)$(SEP)


# additional include flags used when compiling the driver
#
CID_INCLUDE := $(SHARED) $(CID_DIR)
CID_COMPILE := $(FT_COMPILE) $(CID_INCLUDE:%=$I%)


# Type1 driver sources (i.e., C files)
#
CID_DRV_SRC := $(CID_DIR_)cidparse.c  \
               $(CID_DIR_)cidload.c   \
               $(CID_DIR_)cidriver.c  \
               $(CID_DIR_)cidgload.c  \
               $(CID_DIR_)cidafm.c

# Type1 driver headers
#
CID_DRV_H := $(CID_DIR_)t1errors.h   \
             $(CID_DIR_)cidtokens.h  \
             $(T1SHARED_H)          \
             $(CID_DRV_SRC:%.c=%.h)


# driver object(s)
#
#   CID_DRV_OBJ_M is used during `debug' builds
#   CID_DRV_OBJ_S is used during `release' builds
#
CID_DRV_OBJ_M := $(CID_DRV_SRC:$(CID_DIR_)%.c=$(OBJ_)%.$O) \
                 $(T1SHARED:$(T1SHARED_DIR_)%.c=$(OBJ_)%.$O)
CID_DRV_OBJ_S := $(OBJ_)type1cid.$O


# driver source file(s)
#
CID_DRV_SRC_M := $(CID_DRV_SRC) $(T1SHARED_SRC)
CID_DRV_SRC_S := $(CID_DIR_)type1cid.c


# driver - single object
#
#  the driver is recompiled if any of the header or source files is changed
#
$(CID_DRV_OBJ_S): $(BASE_H) $(CID_DRV_H) $(CID_DRV_SRC) $(CID_DRV_SRC_S)
	$(CID_COMPILE) $T$@ $(CID_DRV_SRC_S)


# driver - multiple objects
#
#   All objects are recompiled if any of the header files is changed
#
$(OBJ_)t1%.$O: $(CID_DIR_)t1%.c $(BASE_H) $(CID_DRV_H)
	$(CID_COMPILE) $T$@ $<

$(OBJ_)t1%.$O: $(T1SHARED_DIR_)t1%.c $(BASE_H) $(T1SHARED_H)
	$(CID_COMPILE) $T$@ $<


# update main driver object lists
#
DRV_OBJS_S += $(CID_DRV_OBJ_S)
DRV_OBJS_M += $(CID_DRV_OBJ_M)

# EOF
