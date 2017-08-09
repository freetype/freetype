TOP_DIR := ../..
BUILD_DIR := $(TOP_DIR)/builds/unix

include $(TOP_DIR)/builds/unix/unix-def.mk

SRC_SPRITE = make_sprite.c bitmap.c murmur3.c
SRC_LIB = $(libdir)/libfreetype.a

OBJS = $(src:.c=.o)

CFLAGS = -Wall -g
CC = gcc
INCLUDE = -I $(includedir)/freetype2
LIBS = -lpng -lharfbuzz -lbz2 -ldl

DPI ?= 72
FT_TEST_RENDER_MODE ?= RGB

all: tests

tests: $(SRC_SPRITE) render_modes dpi
	$(CC) $(CFLAGS) $(INCLUDE) -DDPI=$(DPI) -DFT_TEST_RENDER_MODE=$(FT_TEST_RENDER_MODE) -o $@  $(SRC_SPRITE) $(SRC_LIB) $(OBJS) $(LIBS)

.PHONY: clean force
clean:
	-rm -f *.o

render_modes: force
	echo '$(FT_TEST_RENDER_MODE)' | cmp -s - $@ || echo '$(FT_TEST_RENDER_MODE)' > $@
	
dpi: force
	echo '$(DPI)' | cmp -s - $@ || echo '$(DPI)' > $@