#
# FreeType 2 Configuration rules for cross building by Watcom C compiler
# for 16bit platforms.
#
#   Development version without optimizations & libtool
#   and no installation.
#


# Copyright 1996-2017 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


include $(TOP_DIR)/builds/unix/unixddef.mk
include $(TOP_DIR)/builds/compiler/watcom16.mk
include $(TOP_DIR)/builds/link_std.mk


# EOF
