#
# FreeType 2 Configuration rules for Win32 + GCC
#

# the separator must be set before including win32-def
# as it defaults to "\" on Win32
SEP := /

# include Win32-specific definitions
include $(TOP)/builds/win32/win32-def.mk

# include gcc-specific definitions
include $(TOP)/builds/compiler/gcc.mk

# include linking instructions
include $(TOP)/builds/link_dos.mk

# EOF
