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
# We simply try to detect a `X11R6/bin', `X11R5/bin' or `X11/bin' in
# the current path.
#
ifneq ($(findstring X11R6$(SEP)bin,$(PATH)),)
  xversion := X11R6
else
  ifneq ($(findstring X11R5$(SEP)bin,$(PATH)),)
    xversion := X11R5
  else
    ifneq ($(findstring X11$(SEP)bin,$(PATH)),)
      xversion := X11
    endif
  endif
endif

ifdef xversion
  X11_PATH := $(subst ;, ,$(PATH)) $(subst :, ,$(PATH))
  X11_PATH := $(filter %$(xversion)$(SEP)bin,$(X11_PATH))
  X11_PATH := $(X11_PATH:%$(SEP)bin=%)
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
