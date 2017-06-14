#include <stdio.h>
#include <ft2build.h>
#include "murmur3.h"            // MurmurHash3_x64_128 header file

#include FT_FREETYPE_H
#include FT_MODULE_H
#include FT_LCD_FILTER_H
#include FT_BITMAP_H

#define BITS_PER_PIXEL_MONO 1   // Constants for the Bitmap Header
#define BITS_PER_PIXEL_GRAY 8
#define BITS_PER_PIXEL_LCD 24

#define PLANES 1                // Constants for the Bitmap Header
#define COMPRESSION 0
#define X_PIXELS_PER_METER 0
#define Y_PIXELS_PER_METER 0
#define USED_COLORS 0
#define IMPORTANT_COLORS 0
//-------------------------------------------------------------------------------
#pragma pack(push,1)

typedef struct{                 // Bitmap FILE Header

    FT_UInt16 type;
    FT_UInt32 file_size;
    FT_UInt32 reserved;
    FT_UInt32 file_offset;

} BMP_FILE_HEADER;

typedef struct{                 // Bitmap INFO Header

    FT_UInt32 info_header_size;
    FT_UInt32 width;
    FT_UInt32 height;
    FT_UInt16 planes;
    FT_UInt16 bits_per_pixel;
    FT_UInt32 compression;
    FT_UInt32 image_size;
    FT_UInt32 y_pixels_per_meter;
    FT_UInt32 x_pixels_per_meter;
    FT_UInt32 used_colors;
    FT_UInt32 important_colors;

} BMP_INFO_HEADER;

typedef struct {                 // Bitmap Header  

    BMP_FILE_HEADER file_header;   
    BMP_INFO_HEADER info_header;

} HEADER;

//------------------------------------------------------------------------------
typedef struct {                // To store 32bit Hash
    FT_UInt32 hash[1];
}HASH_32;

typedef struct {                // To store 128bit Hash
    FT_UInt32 hash[4];
}HASH_128;

#pragma pack(pop)

//-------------------------------------------------------------------------------

HASH_32 * Generate_Hash_x86_32(FT_Bitmap * bitmap, HASH_32 * murmur);
HASH_128 * Generate_Hash_x86_128(FT_Bitmap * bitmap, HASH_128 * murmur);
HASH_128 * Generate_Hash_x64_128(FT_Bitmap * bitmap, HASH_128 * murmur);

//-------------------------------------------------------------------------------

int Get_Padding                     (FT_Bitmap * bitmap);          

int Get_Bits_Per_Pixel              ( unsigned char PIXEL_MODE );

void Write_Bitmap_Header            (FT_Bitmap * bitmap );

void Write_Bitmap_Data_MONO         (FT_Bitmap * bitmap);
void Write_Bitmap_Data_GRAY         (FT_Bitmap * bitmap);
void Write_Bitmap_Data_LCD_BGR      (FT_Bitmap * bitmap);
void Write_Bitmap_Data_LCD_RGB      (FT_Bitmap * bitmap);
void Write_Bitmap_Data_LCD_V_RGB    (FT_Bitmap * bitmap);
void Write_Bitmap_Data_LCD_V_BGR    (FT_Bitmap * bitmap);
