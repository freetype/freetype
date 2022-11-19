#
# FreeType 2 smooth renderer module definition
#


# Copyright (C) 1996-2021 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


FTMODULE_H_COMMANDS += DENSE_RENDERER

define DENSE_RENDERER
$(OPEN_DRIVER) FT_Renderer_Class, ft_dense_renderer_class $(CLOSE_DRIVER)
$(ECHO_DRIVER)dense     $(ECHO_DRIVER_DESC)anti-aliased dense renderer$(ECHO_DRIVER_DONE)
endef

# EOF
