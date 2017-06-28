# Define required macros here

SHELL = /bin/sh

SRC = hash_to_file.c bitmap.c murmur3.c 
OBJS = $(src:.c=.o)

CFLAGS = -Wall -g
CC = gcc
INCLUDE = -I /usr/local/include/freetype2/ 
LIBS = -lfreetype

DPI = 72

hash:$(SRC)
	$(CC) $(CFLAGS) $(INCLUDE) -DDPI=$(DPI) -o $@  $(SRC) $(OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f *.o


