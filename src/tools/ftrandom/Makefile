# TOP_DIR and OBJ_DIR should be set by the user to the right directories,
# if necessary.

TOP_DIR ?= ../../..
OBJ_DIR ?= $(TOP_DIR)/objs


SRC_DIR = $(TOP_DIR)/src/tools/ftrandom

CC = gcc
WFLAGS = -Wmissing-prototypes \
         -Wunused \
         -Wimplicit \
         -Wreturn-type \
         -Wparentheses \
         -pedantic \
         -Wformat \
         -Wchar-subscripts \
         -Wsequence-point
CFLAGS = $(WFLAGS) \
         -g \
         -I $(TOP_DIR)/include
LIBS = -lm \
       -L $(OBJ_DIR) \
       -lfreetype \
       -lz

all: ftrandom fttest

ftrandom: $(SRC_DIR)/ftrandom.c $(SRC_DIR)/fttest.c
	$(CC) -o $(OBJ_DIR)/ftrandom $(CFLAGS) $(SRC_DIR)/ftrandom.c $(LIBS)
fttest: $(SRC_DIR)/fttest.c
	$(CC) -o $(OBJ_DIR)/fttest -DSTANDALONE $(CFLAGS) $(SRC_DIR)/fttest.c $(LIBS)

# EOF
