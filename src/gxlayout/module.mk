#
# FreeType 2 AAT/TrueTypeGX module definition
#

# Copyright 2004 by RedHat K.K.
# This file is derived from cff/module.mk.

# ----------------------------------------------------------------------
#
# FreeType 2 CFF module definition
#


# Copyright 1996-2000 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.
# ----------------------------------------------------------------------

# Development of the code in module.mk is support of
# Information-technology Promotion Agency, Japan.   

make_module_list: add_gx_driver

add_gx_driver:
	$(OPEN_DRIVER)gx_driver_class$(CLOSE_DRIVER)
	$(ECHO_DRIVER)gx       $(ECHO_DRIVER_DESC)AAT/TrueTypeGX fonts$(ECHO_DRIVER_DONE)

# EOF
