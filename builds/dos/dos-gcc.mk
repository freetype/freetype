#
# FreeType 2 configuration rules for the DJGPP compiler
#

SEP := /
CLEAN_LIBRARY := $(DELETE) $@
include $(TOP)/builds/dos/dos-def.mk
include $(TOP)/builds/compiler/gcc.mk

# EOF
