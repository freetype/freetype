#
# FreeType 2 TrueType module definition
#


# Copyright (C) 1996-2023 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


FTMODULE_H_COMMANDS += PRELOAD_DRIVER

define PRELOAD_DRIVER
$(OPEN_DRIVER) FT_Driver_ClassRec, preload_driver_class $(CLOSE_DRIVER)
$(ECHO_DRIVER)preload  $(ECHO_DRIVER_DESC)Windows/Mac font files with extension *.ttf or *.ttc$(ECHO_DRIVER_DONE)
endef

# EOF
