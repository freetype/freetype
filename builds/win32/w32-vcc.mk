#
#  Visual C++ on Win32
#

SEP := /
include $(TOP)/builds/win32/win32-def.mk
include $(TOP)/builds/compiler/visualc.mk

# include linking instructions
include $(TOP)/builds/link_dos.mk

# EOF
