# Define required macros here

SHELL = /bin/sh

SRC_HASH = hash_to_file.c bitmap.c murmur3.c  
SRC_PNG = make_png.c bitmap.c murmur3.c
OBJS = $(src:.c=.o)

CFLAGS = -Wall -g
CC = gcc
INCLUDE = -I /usr/local/include/freetype2/ 
LIBS = -lfreetype -lpng

DPI = 72

all: png hash

png:$(SRC_PNG)
	$(CC) $(CFLAGS) $(INCLUDE) -DDPI=$(DPI) -o $@  $(SRC_PNG) $(OBJS) $(LIBS)

hash:$(SRC_HASH)
	$(CC) $(CFLAGS) $(INCLUDE) -DDPI=$(DPI) -o $@  $(SRC_HASH) $(OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f *.o


