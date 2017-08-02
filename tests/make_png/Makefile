# Define required macros here

SHELL = /bin/sh

SRC_SPRITE = make_sprite.c bitmap.c murmur3.c

SRC_LIB = ../../objs/libfreetype.a
OBJS = $(src:.c=.o)

CFLAGS = -Wall -g
CC = gcc
INCLUDE = -I ../../include/
LIBS = -lpng -lharfbuzz -lbz2 -ldl

DPI = 72

all: sprite

sprite:$(SRC_SPRITE)
	$(CC) $(CFLAGS) $(INCLUDE) -DDPI=$(DPI) -o $@  $(SRC_SPRITE) $(SRC_LIB) $(OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f *.o




