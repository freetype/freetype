#
# demo.mk --- Makefile for GX driver demo program
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

GLIB_CFLAGS=`pkg-config --cflags glib-2.0 gobject-2.0`
GLIB_LIBS=`pkg-config --libs glib-2.0 gobject-2.0`
GTK_CFLAGS=`pkg-config --cflags gtk+-2.0`
GTK_LIBS=`pkg-config --libs gtk+-2.0`
POPT_LIBS=-lpopt
GCANVAS_CFLAGS=`pkg-config --cflags libgnomecanvas-2.0`
GCANVAS_LDFLAGS=`pkg-config --libs libgnomecanvas-2.0`

GUI_CFLAGS=$(GTK_CFLAGS) $(GLIB_CFLAGS)  $(GCANVAS_CFLAGS)
GUI_LIBS=$(GTK_LIBS) $(POPT_LIBS) $(GLIB_LIBS) $(GCANVAS_LDFLAGS)

BASE_CFLAGS=-c -O0 -g -I../../include -Wall -std=c99
BASE_LIBS=-Xlinker -rpath -Xlinker ../../objs/.libs -lz
########################################################################
GXOBJ=../../objs/.libs/libfreetype.so

DEMO=gxdemo
DEMOBIN=$(DEMO)
DEMOOBJ=$(DEMO).o
DEMOSRC=$(DEMO).c
DEMO_CFLAGS=$(BASE_CFLAGS) $(GUI_CFLAGS)
DEMO_LIBS=$(BASE_LIBS) $(GUI_LIBS)

FI=fi
FIBIN=$(FI)
FIOBJ=$(FI).o
FISRC=$(FI).c
FI_CFLAGS=$(BASE_CFLAGS)
FI_LIBS=$(BASE_LIBS)

########################################################################
all: $(DEMO) $(FI)
$(GXOBJ): *.c *.h
	(cd ../../; make)
clean:
	rm -f  $(DEMOBIN) $(DEMOOBJ) $(FIBIN) $(FIOBJ)  core.*

########################################################################
$(DEMOBIN):  $(GXOBJ) $(DEMOOBJ)
	$(CC) -o $(@) $(DEMO_LIBS) $^

$(DEMOOBJ): $(DEMOSRC) $(GXOBJ)
	$(CC) $(DEMO_CFLAGS) $(DEMOSRC)

########################################################################
$(FIBIN):  $(GXOBJ) $(FIOBJ)
	$(CC) -o $(@) $(FI_LIBS) $^

$(FIOBJ): $(FISRC) $(GXOBJ)
	$(CC) $(FI_CFLAGS) $(FISRC)

########################################################################
