#****************************************************************************
#*                                                                          *
#*  FreeType modules sub-Makefile                                           *
#*                                                                          *
#*  Copyright 1996-1999 by                                                  *
#*  David Turner, Robert Wilhelm, and Werner Lemberg.                       *
#*                                                                          *
#*  This file is part of the FreeType project, and may only be used         *
#*  modified and distributed under the terms of the FreeType project        *
#*  license, LICENSE.TXT.  By continuing to use, modify, or distribute      *
#*  this file you indicate that you have read the license and               *
#*  understand and accept it fully.                                         *
#*                                                                          *
#*                                                                          *
#*                                                                          *
#*  DO NOT INVOKE THIS MAKEFILE DIRECTLY. IT IS MEANT TO BE INCLUDED BY     *
#*  OTHER MAKEFILES.                                                        *
#*                                                                          *
#*  This file is in charge of handling the generation of the modules list   *
#*  file, normally located in `config/ftmodule.h'                           *
#*                                                                          *
#****************************************************************************

.PHONY: make_module_list clean_module_list remake_module_list

# MODULE_LIST, as it name suggests, indicate where the modules list
# reside. For now, it is in $(BUILD)/ftmodule.h
#
ifndef FT_MODULE_LIST
FT_MODULE_LIST := $(TOP)$(SEP)config$(SEP)ftmodule.h
endif

# To build the modules list, we invoke the `make_module_list' target
#
#$(FT_MODULE_LIST): make_module_list

# Before the modules list file can be generated, we must remove the
# file in order to `clean' the list
#
clean_module_list:
	@-$(DELETE) $(subst $(SEP),$(HOSTSEP),$(FT_MODULE_LIST))
	@-echo Regenerating the font drivers list in $(FT_MODULE_LIST)

make_module_list: clean_module_list
	@echo -- done --

ifneq ($(findstring $(PLATFORM),dos win32 win16 os2),)
OPEN_MODULE  := @echo 
CLOSE_MODULE :=  >> $(FT_MODULE_LIST)
else
OPEN_MODULE  := @echo "
CLOSE_MODULE := " >> $(FT_MODULE_LIST)
endif

# OPEN_DRIVER & CLOSE_DRIVER are used to specify a given font driver
# in the `module.mk' rules file
#
OPEN_DRIVER  := $(OPEN_MODULE)FT_DRIVER(
CLOSE_DRIVER := )$(CLOSE_MODULE)

ECHO_DRIVER      := @echo "* driver: 
ECHO_DRIVER_DESC := (
ECHO_DRIVER_DONE := )"

# each `module.mk' in the `src' sub-dirs is used to add one rule to
# the target `make_module_list'.
#
include $(wildcard $(TOP)/src/*/module.mk)

