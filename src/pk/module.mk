#
# FreeType 2 PK Font module definition
#


# Copyright 1996-2018 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.

FTMODULE_H_COMMANDS += PK_DRIVER

define GF_DRIVER
$(OPEN_DRIVER) FT_Driver_ClassRec, pk_driver_class $(CLOSE_DRIVER)
$(ECHO_DRIVER)pk        $(ECHO_DRIVER_DESC)METAFONT bitmap fonts$(ECHO_DRIVER_DONE)
endef

# EOF
