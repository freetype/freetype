#
# FreeType 2 GF Font module definition
#


# Copyright 1996-2018 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.

FTMODULE_H_COMMANDS += TFM_DRIVER

define TFM_DRIVER
$(OPEN_DRIVER) FT_Driver_ClassRec, tfm_driver_class $(CLOSE_DRIVER)
$(ECHO_DRIVER)tfm        $(ECHO_DRIVER_DESC)tfm TeX's bitmap fonts$(ECHO_DRIVER_DONE)
endef

# EOF
