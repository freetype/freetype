#
#  Borland C++ on Win32 + debugging
#

SEP := /
include $(TOP)/builds/win32/win32-def.mk
include $(TOP)/builds/compiler/bcc-dev.mk

# include linking instructions
include $(TOP)/builds/link_dos.mk

# EOF
