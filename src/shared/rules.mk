#
# FreeType 2 shared files configuration rules
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used modified
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


ifndef SHARED_RULES
  SHARED_RULES := 1

  SHARED  := $(SRC_)shared
  SHARED_ := $(SHARED)$(SEP)

  SHARED_H   := $(wildcard $(SHARED_)*.h)
  SHARED_SRC := $(wildcard $(SHARED_)*.c) 

endif

# EOF
