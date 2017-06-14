#!/bin/bash
gcc example1.c bitmap.c murmur3.c -Wall -I /usr/local/include/freetype2/ -lfreetype -o example1 
gcc example2.c bitmap.c murmur3.c -Wall -I /usr/local/include/freetype2/ -lfreetype -o example2

