#
# FreeType 2 build system -- top-level Makefile
#


# Project names
#
PROJECT := freetype
PROJECT_TITLE := FreeType

USE_MODULES := 1

# The variable TOP holds the path to the topmost directory in the project
# engine source hierarchy.  If it is not defined, default it to `.'.
#
ifndef TOP
  TOP := .
endif

include $(TOP)/builds/toplevel.mk

# EOF
