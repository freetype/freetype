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

all: tests

tests: $(SRC_SPRITE)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@  $(SRC_SPRITE) $(SRC_LIB) $(OBJS) $(LIBS)

.PHONY: clean force
clean:
	-rm -f *.o
