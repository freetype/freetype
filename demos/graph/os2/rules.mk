#**************************************************************************
#*
#*  OS/2 specific rules file, used to compile the OS/2 graphics driver
#*  to the graphics subsystem
#*
#**************************************************************************

ifeq ($(PLATFORM),os2)

GR_OS2  := $(GRAPH_)os2
GR_OS2_ := $(GR_OS2)$(SEP)

# the GRAPH_LINK is expanded each time an executable is linked with the
# graphics library.
#
GRAPH_LINK     += $(GR_OS2_)gros2pm.def

# Add the OS/2 driver object file to the graphics library "graph.a"
#
GRAPH_OBJS += $(OBJ_)gros2pm.$O

DEVICES         += OS2_PM
DEVICE_INCLUDES += $(GR_OS2)

# the rule used to compile the graphics driver
#
$(OBJ_)gros2pm.$O: $(GR_OS2_)gros2pm.c $(GR_OS2_)gros2pm.h
	$(CC) $(CFLAGS) $(GRAPH_INCLUDES:%=$I%) $I$(GR_OS2) $T$@ $<

endif


