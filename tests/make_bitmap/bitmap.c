#include "bitmap.h"

HASH_128 * Generate_Hash_x64_128(FT_Bitmap * bitmap, HASH_128 * murmur)
{    
    int seed = 99; // Dont change

    MurmurHash3_x64_128(bitmap->buffer, (bitmap->pitch * bitmap->rows), seed, murmur->hash);

    return murmur;
}

HASH_128 * Generate_Hash_x86_128(FT_Bitmap * bitmap, HASH_128 * murmur)
{    
    int seed = 99; // Dont change

    MurmurHash3_x86_128(bitmap->buffer, (bitmap->pitch * bitmap->rows), seed, murmur->hash);

    return murmur;
}

HASH_32 * Generate_Hash_x86_32(FT_Bitmap * bitmap, HASH_32 * murmur)
{    
    int seed = 99; // Dont change

    MurmurHash3_x86_32(bitmap->buffer, (bitmap->pitch * bitmap->rows), seed, murmur->hash);

    return murmur;
}

int Get_Padding( FT_Bitmap*	bitmap ){

	int rem;
	if (bitmap->pixel_mode == 6)
	{
		rem = ( 3 * bitmap->width ) % 4;
	}else{
		rem = ( bitmap->width ) % 4;
	}
	
	if (!rem )
    {
        return rem;
    }
        return  (4 - rem);
}

int Get_Bits_Per_Pixel ( unsigned char PIXEL_MODE) {
	if (PIXEL_MODE < 5)
	{
		return BITS_PER_PIXEL_GRAY;
	}else{
		return BITS_PER_PIXEL_LCD;
	}
}

void Write_Bitmap_Header (FT_Bitmap * bitmap ) { 
          
    FILE *fp = fopen("test.bmp","w");     // Bitmap File 
    HEADER *header  = ( HEADER* ) calloc( 1 , sizeof( HEADER ));

    unsigned char   pixel_mode = ( bitmap->pixel_mode );
    FT_UInt			image_width;
    FT_UInt			image_rows;
    FT_UInt			color_table_size = 0;

    switch(pixel_mode){

    	case 5 :	image_width = bitmap->width / 3; 		// LCD
    				image_rows 	= bitmap->rows;
    				break;

    	case 6 :	image_width = bitmap->width;			// LCD_V
    				image_rows 	= bitmap->rows / 3;
    				break;

    	default	:   image_width = bitmap->width;			// MONO and GRAY
    				image_rows 	= bitmap->rows;
    				color_table_size = 4* 256;
    				break;
    }

    FT_Int  image_size; 
    image_size = (image_rows * image_width );

    header->file_header.type                      = 0x4D42;       
    header->file_header.file_size                 = sizeof(HEADER) + color_table_size + image_size;
    header->file_header.file_offset               = sizeof(HEADER) + color_table_size;

    header->info_header.info_header_size          = sizeof(BMP_INFO_HEADER);
    header->info_header.width                     = image_width ;
    header->info_header.height                    = image_rows;
    header->info_header.planes                    = PLANES;
    header->info_header.bits_per_pixel            = Get_Bits_Per_Pixel( bitmap->pixel_mode );
    header->info_header.compression               = COMPRESSION;
    header->info_header.image_size                = image_size;
    header->info_header.y_pixels_per_meter        = Y_PIXELS_PER_METER ;
    header->info_header.x_pixels_per_meter        = X_PIXELS_PER_METER ;
    header->info_header.used_colors               = USED_COLORS;
    header->info_header.important_colors          = IMPORTANT_COLORS;

    fwrite (header, 1, sizeof(HEADER),fp);   
    free(header);
    fclose(fp);
}

void Write_Bitmap_Data_LCD_RGB( FT_Bitmap * bitmap ){

    char value;

    FILE *fp = fopen("test.bmp","a");     

    for (int i = bitmap->rows - 1; i >= 0  ; --i)
    {
        for (int j = 2; j < bitmap->width; j = j+3) 
        {
            value = 0xff - bitmap->buffer[( i * bitmap->pitch) + j];
            fwrite (&value, 1, 1,fp);

            value = 0xff - bitmap->buffer[( i * bitmap->pitch) + j - 1];
            fwrite (&value, 1, 1,fp);

            value = 0xff - bitmap->buffer[( i * bitmap->pitch) + j - 2];
            fwrite (&value, 1, 1,fp);
        }
        for (int k = 0; k < Get_Padding(bitmap); ++k)
        {
            value = 0xff;
            fwrite (&value, 1, 1,fp);           
        }
    }

    fclose(fp);
} 

void Write_Bitmap_Data_LCD_BGR( FT_Bitmap * bitmap ){

    char value;

    FILE *fp = fopen("test.bmp","a");     

    for (int i = bitmap->rows - 1; i >= 0  ; --i)
    {
        for (int j = 0; j < bitmap->width; j = j+3) 
        {
            value = 0xff - bitmap->buffer[( i * bitmap->pitch) + j];
            fwrite (&value, 1, 1,fp);

            value = 0xff - bitmap->buffer[( i * bitmap->pitch) + j + 1];
            fwrite (&value, 1, 1,fp);

            value = 0xff - bitmap->buffer[( i * bitmap->pitch) + j + 2];
            fwrite (&value, 1, 1,fp);
        }
        for (int k = 0; k < Get_Padding(bitmap); ++k)
        {
            value = 0xff;
            fwrite (&value, 1, 1,fp);           
        }
    }

    fclose(fp);
} 

void Write_Bitmap_Data_LCD_V_BGR (FT_Bitmap * bitmap) {            

    FILE *fp = fopen("test.bmp","a"); 

    int i,j,k,step;
    char value;
    step = bitmap->rows - 1;

    while ( step > 0 ){

        for (i = 0; i < bitmap->width; i++)                   
        {
            for (j = step - 2; j <= step; ++j) 
            {
                value = 0xff - bitmap->buffer[(j * bitmap->pitch) + i];
                fwrite (&value, 1, 1,fp);
            }
        }
        for (k = 0; k < Get_Padding(bitmap); ++k)
        {
            value = 0xff;
            fwrite (&value, 1, 1,fp);
        }
        step = step - 3;
    }

    fclose(fp);
} 

void Write_Bitmap_Data_LCD_V_RGB (FT_Bitmap * bitmap) {            

    FILE *fp = fopen("test.bmp","a"); 

    int i,j,k,step;
    char value;
    step = bitmap->rows - 1;

    while ( step > 0 ){

        for (i = 0; i < bitmap->width; i++)                   
        {
            for (j = step; j > step - 3; --j) 
            {
                value = 0xff - bitmap->buffer[(j * bitmap->pitch) + i];
                fwrite (&value, 1, 1,fp);
            }
        }
        for (k = 0; k < Get_Padding(bitmap); ++k)
        {
            value = 0xff;
            fwrite (&value, 1, 1,fp);
        }
        step = step - 3;
    }

    fclose(fp);
} 

void Write_Bitmap_Data_MONO (FT_Bitmap * bitmap) {            

    FILE *fp = fopen("test.bmp","a");

    int i,j;
    unsigned char value;
    for ( i = 0; i < 256; ++i)
    {
        value = i;
        fwrite (&value,1,1,fp);
        fwrite (&value,1,1,fp);
        fwrite (&value,1,1,fp);
        value = 0;
        fwrite (&value,1,1,fp);
    }

    for ( i = bitmap->rows - 1; i >= 0  ; --i)
    {
        for ( j = 0; j < bitmap->pitch; ++j)
        {
            if ( bitmap->buffer[( i * bitmap->pitch) + j] != 0x00 ){
                value = 0x00; // remember taking reverse
                fwrite (&value, 1, 1,fp);
            }else{
                value = 0xff;
                fwrite (&value, 1, 1,fp);
            }
        }
    }

    fclose(fp);
} 

void Write_Bitmap_Data_GRAY(FT_Bitmap * bitmap) {            

    FILE *fp = fopen("test.bmp","a");
    int i,j;

    unsigned char value;
    for ( i = 0; i < 256; ++i)
    {
        value = i;
        fwrite (&value,1,1,fp);
        fwrite (&value,1,1,fp);
        fwrite (&value,1,1,fp);
        value = 0;
        fwrite (&value,1,1,fp);
    }

    for ( j = bitmap->rows - 1; j >= 0; --j)
    {
        for ( i = 0; i < bitmap->pitch; ++i)
        {
            value = 255 - bitmap->buffer[j * bitmap->pitch + i];
            fwrite(&value,1,1,fp);
        }
    }

    fclose(fp);
}