#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <ft2build.h>

#include "murmur3.h"            // MurmurHash3_x64_128 header file

#include <png.h>


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

typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
} PIXEL;

typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
} PIXEL_BGRA;

/* A picture. */
    
typedef struct  {
  PIXEL *pixels;
  size_t width;
  size_t height;
} IMAGE;
    
//------------------------------------------------------------------------------

HASH_32 * Generate_Hash_x86_32(FT_Bitmap * bitmap, HASH_32 * murmur);
HASH_128 * Generate_Hash_x86_128(FT_Bitmap * bitmap, HASH_128 * murmur);
HASH_128 * Generate_Hash_x64_128(FT_Bitmap * bitmap, HASH_128 * murmur);

//------------------------------------------------------------------------------

PIXEL * Pixel_At (IMAGE * bitmap, int x, int y);  // Returns a pointer to pixel
                                                  // at (x,y) co-ordinate
// buffer to image
void Make_PNG(FT_Bitmap* bitmap,char* name,int i,int render_mode);
// Image to file  
int Generate_PNG (IMAGE *bitmap, const char *path,int render_mode);  
// Read PNG
void Read_PNG(char *filename, IMAGE * after_effect);
// Add an effect using two PNG images
// Base Glyph = Gray {127,0,0,255}
// Differences = Red {255,0,0,255}
void Add_effect_1(IMAGE* base, IMAGE* test, IMAGE* out);
// Stitch two PNG files side by side
void Stitch(IMAGE* left, IMAGE* right, IMAGE* result);
