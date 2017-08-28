#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <ft2build.h>

#include "murmur3.h"            /* MurmurHash3_x64_128 header file */

#include <png.h>
#include <dlfcn.h>
#include <math.h>

#include FT_FREETYPE_H
#include FT_MODULE_H
#include FT_LCD_FILTER_H
#include FT_BITMAP_H

#define BITS_PER_PIXEL_RGBA 32
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {                /* To store 32bit Hash */
  FT_UInt32 hash;
}HASH_32;

typedef struct {                /* To store 128bit Hash */
  FT_UInt32 hash[4];
}HASH_128;

/* A 32-bit pixel */
typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
} PIXEL;

/* A picture. */ 
typedef struct  {
  PIXEL* pixels;
  size_t width;
  size_t height;
} IMAGE;

/* Render modes */
enum render_modes
  { MONO, AA, RGB, BGR, VRGB, VBGR };    
/*-----------------------------------------------------------------*/

HASH_32* Generate_Hash_x86_32(FT_Bitmap* bitmap, HASH_32* murmur);
HASH_128* Generate_Hash_x86_128(FT_Bitmap* bitmap, HASH_128* murmur);
HASH_128* Generate_Hash_x64_128(FT_Bitmap* bitmap, HASH_128* murmur);

int Compare_Hash(HASH_128* hash_b, HASH_128* hash_t);

/*-----------------------------------------------------------------*/
/* Returns the render_mode */
int Get_Render_Mode(const char* mode);
/* Returns a pointer to pixel */
/* at (x,y) co-ordinate */
PIXEL* Pixel_At (IMAGE * bitmap, int x, int y);  
/*Render mode string to render_mode code */
void Make_PNG (FT_Bitmap* bitmap,IMAGE* fruit, int i,int render_mode);
/* Image to file */
int Generate_PNG (IMAGE *bitmap, const char *path,int render_mode);   
/* Read PNG */
void Read_PNG(char *filename, IMAGE * after_effect);
/* Add effects using two PNG images and generate an image*/
int Add_effect(IMAGE* base, IMAGE* test, IMAGE* out, int Effect_ID);
/* Stitch 2 PNG files */
void Stitch(IMAGE* left, IMAGE* right, IMAGE* result);
/* Finding the first non-empty (non-white) column */
int First_Column(IMAGE* input);
/* Finding the first non-empty (non-white) row */
int First_Row(IMAGE* input);
/* Appening white columns with image alignment */
IMAGE* Append_Columns(IMAGE* small, IMAGE* big);
/* Appening white columns with image alignment */
IMAGE* Append_Rows(IMAGE* small, IMAGE* big);
/* calculating the Pixel Differences */
int Image_Diff( IMAGE* base, IMAGE* test);
/* Print the row in list-view webpage */
void Print_Row( FILE* fp, int index, char* name, int diff );
/* Print the table-headers in list-view webpage */
void Print_Head( FILE* fp );
