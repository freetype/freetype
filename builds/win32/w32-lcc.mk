#
# Configuration rules for Win32 + LCC
#


SEP := /
include $(TOP)/builds/win32/win32-def.mk
include $(TOP)/builds/compiler/win-lcc.mk

# include linking instructions
include $(TOP)/builds/link_dos.mk

# EOF

