#
# demo.mk --- Makefile for OpenType driver demo program
#
# Copyright 2004 by Masatake YAMATO and RedHat K.K.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.
#
########################################################################

# Development of the code in demo.mk is support of
# Information-technology Promotion Agency, Japan.   

SHELL=/bin/bash

POPT_LIBS=-lpopt

CFLAGS=-c -O0 -g -I../../include -Wall -std=c99 
LIBS=-Xlinker -rpath -Xlinker ../../objs/.libs -lz $(POPT_LIBS)
########################################################################
OTOBJ=../../objs/.libs/libfreetype.so

DEMOBIN=otdemo
DEMOOBJ=otdemo.o
DEMOSRC=otdemo.c

########################################################################
all: $(DEMOBIN)
$(DEMOBIN):  $(OTOBJ) $(DEMOOBJ)
	$(CC) -o $(@) $(LIBS) $^ 

$(DEMOOBJ): $(DEMOSRC) $(OTOBJ)

$(OTOBJ): *.c *.h
	(cd ../../; make)
########################################################################
clean:
	rm -f  $(DEMOBIN) $(DEMOOBJ) core.*
