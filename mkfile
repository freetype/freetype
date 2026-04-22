# mkfile for Plan 9 from Bell Labs
#
# See builds/plan9/README for usage.
# 
# Copyright (C) 2026 by Yaroslav Kolomiiets.
# 
# This file is part of the FreeType project, and may only be used,
# modified, and distributed under the terms of the FreeType project
# license, LICENSE.TXT.  By continuing to use, modify, or distribute
# this file you indicate that you have read the license and understand
# and accept it fully.
#
</$objtype/mkfile

LIB=libfreetype.a$O

OFILES=\
	src/base/ftbase.$O \
		ftsystem.$O \
		ftinit.$O \
		ftdebug.$O \
		ftbitmap.$O \
		ftglyph.$O \
		ftmm.$O \
	src/autofit/autofit.$O \
	src/bdf/bdf.$O \
	src/cache/ftcache.$O \
	src/cff/cff.$O \
	src/cid/type1cid.$O \
	src/gzip/ftgzip.$O \
	src/hvf/hvf.$O \
	src/lzw/ftlzw.$O \
	src/pcf/pcf.$O \
	src/pfr/pfr.$O \
	src/psaux/psaux.$O \
	src/pshinter/pshinter.$O \
	src/psnames/psnames.$O \
	src/raster/raster.$O \
	src/sdf/sdf.$O \
	src/sfnt/sfnt.$O \
	src/smooth/smooth.$O \
	src/svg/svg.$O \
	src/truetype/truetype.$O \
	src/type1/type1.$O \
	src/type42/type42.$O \
	src/winfonts/winfnt.$O \

HFILES=\
	builds/plan9/p9ftopt.h \
	builds/plan9/p9lib.h \

CFLAGS=-p -Ibuilds/plan9 -Iinclude \
	-D'FT_CONFIG_OPTIONS_H=<p9ftopt.h>' \
	-D'FT_CONFIG_STANDARD_LIBRARY_H=<p9lib.h>' \
	-DFT2_BUILD_LIBRARY \

</sys/src/cmd/mklib

%.$O: %.c
	$CC $CFLAGS -o $stem.$O $stem.c
%.$O:	src/base/%.c
	$CC $CFLAGS src/base/$stem.c

builds/plan9/p9ftopt.h:D: include/freetype/config/ftoption.h
	sed '
		/#define FT_CONFIG_OPTION_USE_LZW/ s!^!// !
		/#define FT_CONFIG_OPTION_USE_ZLIB/ s!^!// !
	' $prereq > $target

clean:V:
	rm -f *.[$OS] src/*/*.[$OS] $CLEANFILES

nuke:V:
	rm -f *.[$OS] src/*/*.[$OS] $CLEANFILES
	rm -f libfreetype.a[$OS] builds/plan9/p9ftopt.h
