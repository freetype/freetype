#**************************************************************************
#*
#*  X11-specific rules files, used to compile the X11 graphics driver
#*  when supported by the current platform
#*
#**************************************************************************

#########################################################################
#
# Try to detect an X11 setup.
#
# We try to detect the following directories (in that order) in the current
# path:
#
#   X11   (usually a symlink to the current release)
#   X11R6
#   X11R5
#
# If the variable X11_PATH is set (to specify unusual locations of X11), no
# other directory is searched.  More than one directory must be separated
# with spaces.  Example:
#
#   make X11_PATH="/usr/openwin /usr/local/X11R6"
#
ifndef X11_PATH
  ifneq ($(findstring X11$(SEP)bin,$(PATH)),)
    xversion := X11
  else
    ifneq ($(findstring X11R6$(SEP)bin,$(PATH)),)
      xversion := X11R6
    else
      ifneq ($(findstring X11R5$(SEP)bin,$(PATH)),)
        xversion := X11R5
      endif
    endif
  endif

  ifdef xversion
    X11_PATH := $(subst ;, ,$(PATH)) $(subst :, ,$(PATH))
    X11_PATH := $(filter %$(xversion)$(SEP)bin,$(X11_PATH))
    X11_PATH := $(X11_PATH:%$(SEP)bin=%)
  endif
endif

##########################################################################
#
# Update some variables to compile the X11 graphics module. Note that
# X11 is available on Unix, or on OS/2. However, it only compiles with
# gcc on the latter platform, which is why it is safe to use the flags
# `-L' and `-l' in GRAPH_LINK
#
ifneq ($(X11_PATH),)

  X11_INCLUDE    := $(X11_PATH:%=%$(SEP)include)
  X11_LIB        := $(X11_PATH:%=%$(SEP)lib)

  # the GRAPH_LINK variable is expanded each time an executable is linked
  # against the graphics library.
  #
  GRAPH_LINK     += $(X11_LIB:%=-L%) -lX11

  # Solaris needs a -lsocket in GRAPH_LINK ..
  #
  UNAME := $(shell uname)
  ifneq ($(findstring $(UNAME),SunOS Solaris),)
    GRAPH_LINK += -lsocket
  endif


  # add the X11 driver object file to the graphics library
  #
  GRAPH_OBJS += $(OBJ_)grx11.$O


  GR_X11  := $(GRAPH_)x11
  GR_X11_ := $(GR_X11)$(SEP)

  DEVICES         += X11
  DEVICE_INCLUDES += $(GR_X11)

  # the rule used to compile the X11 driver
  #
  $(OBJ_)grx11.$O: $(GR_X11_)grx11.c $(GR_X11_)grx11.h
	  $(CC) $(CFLAGS) $(GRAPH_INCLUDES:%=$I%) $I$(GR_X11) \
                $(X11_INCLUDE:%=$I%) $T$@ $<
endif

# EOF
