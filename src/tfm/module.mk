#
# FreeType 2 TFM module definition
#


# Copyright 1996-2018 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


FTMODULE_H_COMMANDS += TFM_MODULE

define TFM_MODULE
$(OPEN_DRIVER) FT_Module_Class, tfm_module_class $(CLOSE_DRIVER)
$(ECHO_DRIVER)tfm       $(ECHO_DRIVER_DESC)TFM helper module$(ECHO_DRIVER_DONE)
endef

# EOF
