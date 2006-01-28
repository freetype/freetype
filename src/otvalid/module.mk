#
# FreeType 2 otvalid module definition
#


# Copyright 2004, 2006 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


.PHONY: add_otvalid_module

make_module_list: add_otvalid_module

add_otvalid_module:
	$(OPEN_DRIVER)otvalid_module_class$(CLOSE_DRIVER)
	$(ECHO_DRIVER)otvalid     $(ECHO_DRIVER_DESC)OpenType validation module$(ECHO_DRIVER_DONE)

# EOF
