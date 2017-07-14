# Define required macros here

SHELL = /bin/sh

SRC_HASH = hash_to_file.c bitmap.c murmur3.c  
SRC_PNG = make_png.c bitmap.c murmur3.c
SRC_SPRITE = make_sprite.c bitmap.c murmur3.c

SRC_LIB = ../../objs/libfreetype.a
OBJS = $(src:.c=.o)

CFLAGS = -Wall -g
CC = gcc
INCLUDE = -I ../../include/
LIBS = -lpng -lharfbuzz -lbz2 -ldl

DPI = 72

all: png hash sprite

png:$(SRC_PNG)
	$(CC) $(CFLAGS) $(INCLUDE) -DDPI=$(DPI) -o $@  $(SRC_PNG) $(SRC_LIB) $(OBJS) $(LIBS)

sprite:$(SRC_PNG)
	$(CC) $(CFLAGS) $(INCLUDE) -DDPI=$(DPI) -o $@  $(SRC_SPRITE) $(SRC_LIB) $(OBJS) $(LIBS)

hash:$(SRC_HASH)
	$(CC) $(CFLAGS) $(INCLUDE) -DDPI=$(DPI) -o $@  $(SRC_HASH) $(SRC_LIB) $(OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f *.o



