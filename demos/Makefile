.PHONY: exes
all: exes

####################################################################
#
# TOP is the directory where the main FreeType source is found,
# as well as the 'config.mk' file
#

ifndef TOP
TOP := ..
endif

####################################################################
#
# Check that we have a working `config.mk' in the above directory.
# If not, issue a warning message, then stop there..
#
ifeq ($(wildcard $(TOP)/config.mk),)
no_config_mk := 1
endif

ifdef no_config_mk

exes:
	@echo Please compile the library before the demo programs !!

else

####################################################################
#
# Good, now include the `config.mk' in order to know how to build
# object files from sources, as well as other things (compiler flags)
#
include ../config.mk

####################################################################
#
# Detect DOS-like platforms, currently DOS, Win 3.1, Win32 & OS/2
#
#
ifneq ($(findstring $(PLATFORM),os2 win16 win32 dos),)
DOSLIKE := 1
endif


###################################################################
#
# Clean-up rules. Because the `del' command on DOS-like platforms
# cannot take a long list of arguments, we simply erase the directory
# contents..
#
.PHONY: clean  distclean

ifdef DOSLIKE
clean_demo:
	-del obj\*.$O
	-del src\*.bak

distclean_demo: clean_demo
	-del obj\*.*
	-del bin\*.exe
else
clean_demo:
	-$(DELETE) $(OBJ_)*.$O
	-$(DELETE) $(SRC_)*.bak graph$(SEP)*.bak
	-$(DELETE) $(SRC_)*~ graph$(SEP)*~


distclean_demo: clean_demo
	-$(DELETE) $(BIN_)*
endif

clean: clean_demo
distclean: distclean_demo

####################################################################
#
# Define a few important variables now
#
#
TOP_ := $(TOP)$(SEP)
SRC_ := $(TOP)$(SEP)src$(SEP)

BIN_ := bin$(SEP)
OBJ_ := obj$(SEP)

GRAPH_DIR := graph

SRC_DIR  := src
SRC_DIR_ := $(SRC_DIR)$(SEP)


FT_INCLUDES := $(BUILD) $(TOP_)include $(SRC_)base
TT_INCLUDES := $(SRC_)shared $(SRC_)truetype
T1_INCLUDES := $(SRC_)shared $(SRC_)type1

CFLAGS    = -c -O0 -g $(INCLUDES:%=$I%) -Wall -W -ansi
CC       := $(CC)
LINK     := $(CC)
FTLIB    := $(LIB_DIR)$(SEP)$(LIBRARY).$A


####################################################################
#
# Compute the executable suffix to use, and put it in `E'.
# It is ".exe" on DOS-ish platforms, and nothing otherwise
#
ifdef DOSLIKE
E := .exe
else
E :=
endif

###################################################################
#
# The list of demonstration programs to build.
#
EXES := ftlint t1dump ftview fttimer

ifneq ($(findstring $(PLATFORM),os2 unix),)
EXES += ttdebug
endif

###################################################################
#
# Include the rules needed to compile the graphics sub-system.
# This will also select which graphics driver to compile to the
# sub-system..
#
include $(GRAPH_DIR)/rules.mk

exes: $(EXES:%=$(BIN_)%$E)


INCLUDES := $(FT_INCLUDES)

####################################################################
#
# Rules for compiling object files for text-only demos
#

COMMON_OBJ := $(OBJ_)common.o
$(COMMON_OBJ): $(SRC_DIR_)common.c
	$(CC) $(CFLAGS) $T$@ $<

$(OBJ_)%.$O: $(SRC_DIR_)%.c $(FTLIB)
	$(CC) $(CFLAGS) $T$@ $<

$(OBJ_)ftlint.$O: $(SRC_DIR_)ftlint.c
	$(CC) $(CFLAGS) $T$@ $<

$(OBJ_)fttry.$O: $(SRC_DIR_)fttry.c
	$(CC) $(CFLAGS) $T$@ $<


$(OBJ_)ftview.$O: $(SRC_DIR_)ftview.c $(GRAPH_LIB)
	$(CC) $(CFLAGS) $(GRAPH_INCLUDES:%=$I%) $T$@ $<

$(OBJ_)fttimer.$O: $(SRC_DIR_)fttimer.c $(GRAPH_LIB)
	$(CC) $(CFLAGS) $(GRAPH_INCLUDES:%=$I%) $T$@ $<

#$(OBJ_)ftsbit.$O: $(SRC_DIR)/ftsbit.c $(GRAPH_LIB)
#	$(CC) $(CFLAGS) $T$@ $<


####################################################################
#
# Special rule to compile the `t1dump' program as it includes
# the Type1 source path
#
$(OBJ_)t1dump.$O: $(SRC_DIR)/t1dump.c
	$(CC) $(CFLAGS) $(T1_INCLUDES:%=$I%) $T$@ $<


####################################################################
#
# Special rule to compile the `ttdebug' program as it includes
# the TrueType source path and needs extra flags for correct keyboard
# handling on Unix
#

# POSIX TERMIOS: Do not define if you use OLD U*ix like 4.2BSD.
#
# detect a Unix system
ifeq ($(PLATFORM),unix)
EXTRAFLAGS = $DUNIX $DHAVE_POSIX_TERMIOS
endif

$(OBJ_)ttdebug.$O: $(SRC_DIR)/ttdebug.c
	$(CC) $(CFLAGS) $(TT_INCLUDES:%=$I%) $T$@ $< $(EXTRAFLAGS)


####################################################################
#
# Rules used to link the executables. Note that they could be
# over-ridden by system-specific things..
#

$(BIN_)ftlint$E: $(OBJ_)ftlint.$O $(FTLIB)
	$(LINK) $(LFLAGS) $T$@ $<  $(FTLIB) $(EFENCE)


$(BIN_)fttry$E: $(OBJ_)fttry.$O $(FTLIB)
	$(LINK) $(LFLAGS) $T$@ $<  $(FTLIB) $(EFENCE)


$(BIN_)ftsbit$E: $(OBJ_)ftsbit.$O $(FTLIB)
	$(LINK) $(LFLAGS) $T$@ $<  $(FTLIB) $(EFENCE)


$(BIN_)t1dump$E: $(OBJ_)t1dump.$O $(FTLIB)
	$(LINK)  $(LFLAGS) $T$@ $<  $(FTLIB) $(EFENCE)

$(BIN_)ttdebug$E: $(OBJ_)ttdebug.$O $(FTLIB)
	$(LINK) $(LFLAGS) $T$@ $< $(FTLIB) $(EFENCE)


$(BIN_)ftview$E: $(OBJ_)ftview.$O $(FTLIB) $(GRAPH_LIB) $(COMMON_OBJ)
	$(LINK) $(LFLAGS) $T$@ $< $(FTLIB) $(GRAPH_LINK) $(COMMON_OBJ) $(EFENCE)

$(BIN_)fttimer$E: $(OBJ_)fttimer.$O $(FTLIB) $(GRAPH_LIB) $(COMMON_OBJ)
	$(LINK) $(LFLAGS) $T$@ $< $(FTLIB) $(GRAPH_LINK) $(EFENCE)

endif


