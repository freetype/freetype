#**************************************************************************
#*
#*  FreeType demo utilities sub-Makefile
#*
#*  This Makefile is to be included by "freetype/demo/Makefile". Its
#*  purpose is to compile MiGS (the Minimalist Graphics Subsystem)
#*
#*  It is written for GNU Make. Other make utilities are not
#*  supported.. !!
#*
#*
#*  The following variables must be defined :
#*
#*  CFLAGS     : C flags to use when compiling the utilities. This
#*               must NOT include the '-c' flag used to specify a
#*               simple compilation.
#*
#*  IFLAG      : include path flag. This is typically "-I" but some
#*               compilers use a different convention..
#*
#*  LFLAG      : add link directory flag. Usually '-L' but could be
#*               different..
#*
#*  OBJ_DIR    : target location of the object files
#*
#*  UTIL_DIR   : location of the utilities sources. I.e. this
#*               directory (usually "freetype/demo/graph").
#*
#*
#*  It also defines the following variables
#*
#*  SIMPLE_UTILS : list of object files for the non-graphical utilities
#*
#*  GRAPH_UTILS  : all object files, including the graphics sub-system
#*
#*  GRAPH_FLAGS  : additional compile flags for graphical apps
#*  GRAPH_LINK   : additional link flags for graphical apps
#*
#**************************************************************************

##########################################################################
#
#
#
#
#

GRAPH_INCLUDES := $(TOP2_)graph
GRAPH_LIB      := $(OBJ_)graph.$A
#GRAPH_LINK     := $(GRAPH_LIB)

GRAPH_ := $(TOP2_)graph$(SEP)

GRAPH_H := $(GRAPH_)graph.h    \
           $(GRAPH_)grtypes.h  \
           $(GRAPH_)grobjs.h   \
           $(GRAPH_)grdevice.h \
           $(GRAPH_)grblit.h


GRAPH_OBJS := $(OBJ_)grblit.$O   \
              $(OBJ_)grobjs.$O   \
              $(OBJ_)grfont.$O   \
              $(OBJ_)grdevice.$O \
              $(OBJ_)grinit.$O


# Default value for COMPILE_GRAPH_LIB
# this value can be modified by the system-specific graphics drivers..
#
COMPILE_GRAPH_LIB = ar -r $@ $(GRAPH_OBJS)

# Add the rules used to detect and compile graphics driver depending
# on the current platform..
#
include $(wildcard $(TOP2)/graph/*/rules.mk)

#########################################################################
#
# Build the "graph" library from its objects. This should be changed
# in the future in order to support more systems. Probably something
# like a `config/<system>' hierarchy with a system-specific rules file
# to indicate how to make a library file, but for now, I'll stick to
# unix, Win32 and OS/2-gcc..
#
#
$(GRAPH_LIB): $(GRAPH_OBJS)
	$(COMPILE_GRAPH_LIB)


# pattern rule for normal sources
#
$(OBJ_)%.$O: $(GRAPH_)%.c $(GRAPH_H)
	$(CC) $(CFLAGS) $(GRAPH_INCLUDES:%=$I%) $T$@ $<


# a special rule is used for 'grinit.o' as it needs the definition
# of some macros like "-DDEVICE_X11" or "-DDEVICE_OS2_PM"
#
$(OBJ_)grinit.$O: $(GRAPH_)grinit.c $(GRAPH_H)
	$(CC) $(CFLAGS) $(GRAPH_INCLUDES:%=$I%)  \
                    $(DEVICE_INCLUDES:%=$I%) \
                    $(DEVICES:%=$DDEVICE_%) $T$@ $<

