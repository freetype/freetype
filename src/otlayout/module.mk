#
# FreeType 2 GX module definition
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


make_module_list: add_ot_driver

add_ot_driver:
	$(OPEN_DRIVER)ot_driver_class$(CLOSE_DRIVER)
	$(ECHO_DRIVER)ot       $(ECHO_DRIVER_DESC)OpenType fonts$(ECHO_DRIVER_DONE)

# EOF
