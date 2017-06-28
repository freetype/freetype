#include <stdio.h>
#include <ft2build.h>
#include "murmur3.h"            // MurmurHash3_x64_128 header file
#include <unistd.h>

#include FT_FREETYPE_H
#include FT_MODULE_H
#include FT_LCD_FILTER_H
#include FT_BITMAP_H

#define BITS_PER_PIXEL_RGBA 32

typedef struct {                // To store 32bit Hash
    FT_UInt32 hash[1];
}HASH_32;

typedef struct {                // To store 128bit Hash
    FT_UInt32 hash[4];
}HASH_128;

//-------------------------------------------------------------------------------

HASH_32 * Generate_Hash_x86_32(FT_Bitmap * bitmap, HASH_32 * murmur);
HASH_128 * Generate_Hash_x86_128(FT_Bitmap * bitmap, HASH_128 * murmur);
HASH_128 * Generate_Hash_x64_128(FT_Bitmap * bitmap, HASH_128 * murmur);

//-------------------------------------------------------------------------------
