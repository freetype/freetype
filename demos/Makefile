all: exes

####################################################################
#
# TOP is the directory where the main FreeType source is found,
# as well as the 'config.mk' file
#
# TOP2 is the directory is the top of the demonstration
# programs directory
#

ifndef TOP
  TOP := ..
endif

ifndef TOP2
  TOP2 := $(TOP)/demos
endif

######################################################################
#
# MY_CONFIG_MK points to the current "config.mk" to use. It is
# defined by default as $(TOP)/config.mk
#
ifndef CONFIG_MK
  CONFIG_MK := $(TOP)/config.mk
endif

####################################################################
#
# Check that we have a working `config.mk' in the above directory.
# If not, issue a warning message, then stop there..
#
ifeq ($(wildcard $(CONFIG_MK)),)
  no_config_mk := 1
endif

ifdef no_config_mk
  exes:
	  @echo Please compile the library before the demo programs!
  clean distclean:
	  @echo "I need \`$(TOP)/config.mk' to do that!"
else

  ####################################################################
  #
  # Good, now include the `config.mk' in order to know how to build
  # object files from sources, as well as other things (compiler flags)
  #
  include $(CONFIG_MK)


  ####################################################################
  #
  # Define a few important variables now
  #
  TOP_  := $(TOP)$(SEP)
  TOP2_ := $(TOP2)$(SEP)
  SRC_  := $(TOP)$(SEP)src$(SEP)

  BIN_ := bin$(SEP)
  OBJ_ := obj$(SEP)

  GRAPH_DIR := graph

  ifeq ($(TOP),..)
    SRC_DIR := src
  else
    SRC_DIR := $(TOP2_)src
  endif

  SRC_DIR_ := $(SRC_DIR)$(SEP)

  FT_INCLUDES := $(BUILD) $(TOP_)config $(TOP_)include $(SRC_)base $(SRC_DIR)
  TT_INCLUDES := $(SRC_)shared $(SRC_)truetype
  T1_INCLUDES := $(SRC_)shared $(SRC_)type1

  COMPILE    = $(CC) $(CFLAGS) $(INCLUDES:%=$I%)
  FTLIB     := $(TOP_)$(LIB_DIR)$(SEP)$(LIBRARY).$A

  # the default commands used to link the executables. These can
  # be re-defined for platform-specific stuff..
  #
  LINK = $(CC) $T$@ $< $(FTLIB) $(EFENCE) $(LDFLAGS)
  COMMON_LINK = $(LINK) $(COMMON_OBJ)
  GRAPH_LINK  = $(COMMON_LINK) $(GRAPH_LIB)

  .PHONY: exes clean distclean

  ###################################################################
  #
  # Include the rules needed to compile the graphics sub-system.
  # This will also select which graphics driver to compile to the
  # sub-system..
  #
  include $(GRAPH_DIR)/rules.mk

  ####################################################################
  #
  # Detect DOS-like platforms, currently DOS, Win 3.1, Win32 & OS/2
  #
  ifneq ($(findstring $(PLATFORM),os2 win16 win32 dos),)
    DOSLIKE := 1
  endif


  ###################################################################
  #
  # Clean-up rules.  Because the `del' command on DOS-like platforms
  # cannot take a long list of arguments, we simply erase the directory
  # contents.
  #
  ifdef DOSLIKE

    clean_demo:
	    -del obj\*.$O 2> nul
	    -del $(subst /,\,$(TOP2))\src\*.bak 2> nul

    distclean_demo: clean_demo
	    -del obj\*.lib 2> nul
	    -del bin\*.exe 2> nul

  else

    clean_demo:
	    -$(DELETE) $(OBJ_)*.$O
	    -$(DELETE) $(SRC_)*.bak graph$(SEP)*.bak
	    -$(DELETE) $(SRC_)*~ graph$(SEP)*~

    distclean_demo: clean_demo
	    -$(DELETE) $(EXES:%=$(BIN_)%$E)
	    -$(DELETE) $(GRAPH_LIB)

  endif

  clean: clean_demo
  distclean: distclean_demo

  ####################################################################
  #
  # Compute the executable suffix to use, and put it in `E'.
  # It is ".exe" on DOS-ish platforms, and nothing otherwise.
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
  EXES := ftlint ftview fttimer compos ftstring

  ifneq ($(findstring $(PLATFORM),os2 unix win32),)
    EXES += ttdebug
  endif

  exes: $(EXES:%=$(BIN_)%$E)


  INCLUDES := $(FT_INCLUDES)

  ####################################################################
  #
  # Rules for compiling object files for text-only demos
  #
  COMMON_OBJ := $(OBJ_)common.$O
  $(COMMON_OBJ): $(SRC_DIR_)common.c
  ifdef DOSLIKE
	  $(COMPILE) $T$@ $< $DEXPAND_WILDCARDS 
  else
	  $(COMPILE) $T$@ $<
  endif


  $(OBJ_)%.$O: $(SRC_DIR_)%.c $(FTLIB)
	  $(COMPILE) $T$@ $<

  $(OBJ_)ftlint.$O: $(SRC_DIR_)ftlint.c
	  $(COMPILE) $T$@ $<

  $(OBJ_)compos.$O: $(SRC_DIR_)compos.c
	  $(COMPILE) $T$@ $<

  $(OBJ_)ftgrays2.$O: $(SRC_DIR_)ftgrays2.c
	  $(COMPILE) $T$@ $<

  $(OBJ_)ftrast.$O: $(SRC_DIR_)ftrast.c
	  $(COMPILE) $T$@ $<

  $(OBJ_)ftrast2.$O: $(SRC_DIR_)ftrast2.c
	  $(COMPILE) $T$@ $<

  $(OBJ_)fttry.$O: $(SRC_DIR_)fttry.c
	  $(COMPILE) $T$@ $<


  $(OBJ_)ftview.$O: $(SRC_DIR_)ftview.c $(GRAPH_LIB)
	  $(COMPILE) $(GRAPH_INCLUDES:%=$I%) $T$@ $<

  $(OBJ_)ftstring.$O: $(SRC_DIR_)ftstring.c $(GRAPH_LIB)
	  $(COMPILE) $(GRAPH_INCLUDES:%=$I%) $T$@ $<

  $(OBJ_)try.$O: $(SRC_DIR_)try.c $(GRAPH_LIB)
	  $(COMPILE) $(GRAPH_INCLUDES:%=$I%) $T$@ $<

  $(OBJ_)fttimer.$O: $(SRC_DIR_)fttimer.c $(GRAPH_LIB)
	  $(COMPILE) $(GRAPH_INCLUDES:%=$I%) $T$@ $<

  $(OBJ_)fttimer2.$O: $(SRC_DIR_)fttimer2.c $(GRAPH_LIB)
	  $(COMPILE) $(GRAPH_INCLUDES:%=$I%) $T$@ $<


# $(OBJ_)ftsbit.$O: $(SRC_DIR)/ftsbit.c $(GRAPH_LIB)
#	 $(COMPILE) $T$@ $<


  ####################################################################
  #
  # Special rule to compile the `t1dump' program as it includes
  # the Type1 source path
  #
  $(OBJ_)t1dump.$O: $(SRC_DIR)/t1dump.c
	  $(COMPILE) $(T1_INCLUDES:%=$I%) $T$@ $<


  ####################################################################
  #
  # Special rule to compile the `ttdebug' program as it includes
  # the TrueType source path and needs extra flags for correct keyboard
  # handling on Unix

  # POSIX TERMIOS: Do not define if you use OLD U*ix like 4.2BSD.
  #
  # detect a Unix system
  ifeq ($(PLATFORM),unix)
    EXTRAFLAGS = $DUNIX $DHAVE_POSIX_TERMIOS
  endif

  $(OBJ_)ttdebug.$O: $(SRC_DIR)/ttdebug.c
	    $(COMPILE) $(TT_INCLUDES:%=$I%) $T$@ $< $(EXTRAFLAGS)


  ####################################################################
  #
  # Rules used to link the executables. Note that they could be
  # over-ridden by system-specific things..
  #
  $(BIN_)ftlint$E: $(OBJ_)ftlint.$O $(FTLIB) $(COMMON_OBJ)
	  $(COMMON_LINK)

  $(BIN_)compos$E: $(OBJ_)compos.$O $(FTLIB) $(COMMON_OBJ)
	  $(COMMON_LINK)

  $(BIN_)fttry$E: $(OBJ_)fttry.$O $(FTLIB)
	  $(LINK)

  $(BIN_)ftsbit$E: $(OBJ_)ftsbit.$O $(FTLIB)
	  $(LINK)

  $(BIN_)t1dump$E: $(OBJ_)t1dump.$O $(FTLIB)
	  $(LINK)

  $(BIN_)ttdebug$E: $(OBJ_)ttdebug.$O $(FTLIB)
	  $(LINK)


  $(BIN_)ftview$E: $(OBJ_)ftview.$O $(FTLIB) $(GRAPH_LIB) $(COMMON_OBJ) $(OBJ_)ftrast2.$O
	  $(GRAPH_LINK) $(OBJ_)ftrast2.$O

  $(BIN_)ftstring$E: $(OBJ_)ftstring.$O $(FTLIB) $(GRAPH_LIB) $(COMMON_OBJ)
	  $(GRAPH_LINK)


  $(BIN_)try$E: $(OBJ_)try.$O $(FTLIB) $(GRAPH_LIB) $(COMMON_OBJ) $(OBJ_)ftgrays2.$O
	  $(GRAPH_LINK) $(OBJ_)ftgrays2.$O

  $(BIN_)fttimer$E: $(OBJ_)fttimer.$O $(FTLIB) $(GRAPH_LIB) $(COMMON_OBJ)
	  $(GRAPH_LINK)

  $(BIN_)fttimer2$E: $(OBJ_)fttimer2.$O $(FTLIB) $(GRAPH_LIB) $(COMMON_OBJ) $(OBJ_)ftgrays2.$O
	  $(GRAPH_LINK) $(OBJ_)ftgrays2.$O


endif

# EOF
