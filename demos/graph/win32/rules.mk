#**************************************************************************
#*
#*  Win32 specific rules file, used to compile the Win32 graphics driver
#*  to the graphics subsystem
#*
#**************************************************************************

ifeq ($(PLATFORM),win32)

# directory of the Win32 graphics driver
#
GR_WIN32  := $(GRAPH_)win32
GR_WIN32_ := $(GR_WIN32)$(SEP)

# Add the Win32 driver object file to the graphics library "graph.a"
#
GRAPH_OBJS += $(OBJ_)grwin32.$O

DEVICES         += WIN32
DEVICE_INCLUDES += $(GR_WIN32)

# the rule used to compile the graphics driver
#
$(OBJ_)grwin32.$O: $(GR_WIN32_)grwin32.c $(GR_WIN32_)grwin32.h
	$(CC) $(CFLAGS) $(GRAPH_INCLUDES:%=$I%) $I$(GR_WIN32) $T$@ $<

# Now update COMPILE_GRAPH_LIB according to the compiler used on Win32
#
ifeq ($(CC),gcc)   # test for GCC
LINK              = $(CC) $T$@ $< $(FTLIB)
COMMON_LINK       = $(LINK) $(COMMON_OBJ)
GRAPH_LINK        = $(COMMON_LINK) $(GRAPH_LIB) -luser32 -lgdi32
endif

ifeq ($(CC),cl)    # test for Visual C++
COMPILE_GRAPH_LIB = lib /nologo /out:$(GRAPH_LIB) $(GRAPH_OBJS)
LINK              = cl /nologo /MD -o $@ $< $(FTLIB)
COMMON_LINK       = $(LINK) $(COMMON_OBJ)
GRAPH_LINK        = $(COMMON_LINK) $(GRAPH_LIB) user32.lib gdi32.lib
endif

ifeq ($(CC),lcc)  # test for LCC-Win32
COMPILE_GRAPH_LIB = lcclib /out:$(subst /,\\,$(GRAPH_LIB)) $(subst /,\\,$(GRAPH_OBJS))
GRAPH_LINK        = $(subst /,\\,$(GRAPH_LIB)) user32.lib gdi32.lib
LINK_ROOT         = lcclnk -o $(subst /,\\,$@) $(subst /,\\,$<)
LINK              = $(LINK_ROOT) $(subst /,\\,$(FTLIB))
COMMON_LINK       = $(LINK_ROOT) $(subst /,\\,$(COMMON_OBJ)) $(subst /,\\,$(FTLIB))
GRAPH_LINK        = $(LINK_ROOT) $(subst /,\\,$(COMMON_OBJ)) $(subst /,\\,$(GRAPH_LIB)) $(subst /,\\,$(FTLIB))
endif
endif

