#include "bitmap.h"

HASH_128 * Generate_Hash_x64_128( FT_Bitmap * bitmap, 
                                  HASH_128 * murmur)
{    
  int seed = 99; // Dont change

  MurmurHash3_x64_128(bitmap->buffer, 
                      (bitmap->pitch * bitmap->rows),
                      seed, 
                      murmur->hash);

  return murmur;
}

HASH_128 * Generate_Hash_x86_128( FT_Bitmap * bitmap,
                                  HASH_128 * murmur)
{    
  int seed = 99; // Dont change

  MurmurHash3_x86_128(bitmap->buffer,
                      (bitmap->pitch * bitmap->rows),
                      seed,
                      murmur->hash);

  return murmur;
}

HASH_32 * Generate_Hash_x86_32( FT_Bitmap * bitmap,
                                HASH_32 * murmur)
{    
  int seed = 99; // Dont change

  MurmurHash3_x86_32( bitmap->buffer,
                      (bitmap->pitch * bitmap->rows),
                      seed,
                      murmur->hash);

  return murmur;
}

PIXEL * Pixel_At (IMAGE * bitmap, int x, int y)
{
  return bitmap->pixels + bitmap->width * y + x;
}
    
int Generate_PNG (IMAGE *bitmap,
                  const char *path,
                  int render_mode)
{
  FILE * fp;
  png_structp png_ptr       = NULL;
  png_infop info_ptr        = NULL;

  size_t x, y;
  png_byte ** row_pointers = NULL;

  int status = -1;

  int pixel_size = 4;
  int depth = 8;
  
  fp = fopen (path, "wb");
  if (! fp) {
    goto fopen_failed;
  }

  png_ptr = png_create_write_struct ( PNG_LIBPNG_VER_STRING,
                                      NULL,
                                      NULL,
                                      NULL);
  if (png_ptr == NULL) {
    goto png_create_write_struct_failed;
  }
  
  info_ptr = png_create_info_struct (png_ptr);
  if (info_ptr == NULL) {
    goto png_create_info_struct_failed;
  }

  if (setjmp (png_jmpbuf (png_ptr))) {
    goto png_failure;
  }

  png_set_IHDR (png_ptr,
                info_ptr,
                bitmap->width,
                bitmap->height,
                depth,
                PNG_COLOR_TYPE_RGBA,
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT);

  row_pointers = png_malloc ( png_ptr,
                              bitmap->height * sizeof (png_byte *));

  for (y = 0; y < bitmap->height; y++) {

    png_byte *row = png_malloc (png_ptr, 
                                sizeof (uint8_t) * bitmap->width * pixel_size);
    row_pointers[y] = row;

    for (x = 0; x < bitmap->width; x++) {

      PIXEL * pixel = Pixel_At (bitmap, x, y);

        if (render_mode == 3 || render_mode == 5)
        {
          *row++ = pixel->blue;
          *row++ = pixel->green;
          *row++ = pixel->red;
          *row++ = pixel->alpha;
          continue;
        }

        *row++ = pixel->red;
        *row++ = pixel->green;
        *row++ = pixel->blue;
        *row++ = pixel->alpha;
    }
  }    

  png_init_io ( png_ptr,
                fp);

  png_set_rows (png_ptr,
                info_ptr,
                row_pointers);

  png_write_png ( png_ptr,
                  info_ptr,
                  PNG_TRANSFORM_IDENTITY,
                  NULL);

  status = 0;
  
  for (y = 0; y < bitmap->height; y++) {
    png_free (png_ptr, row_pointers[y]);
  }
  png_free (png_ptr, row_pointers);
  
  png_failure:
  png_create_info_struct_failed:
    png_destroy_write_struct (&png_ptr, &info_ptr);
  png_create_write_struct_failed:
    fclose (fp);
  fopen_failed:
    return status;
}

void Make_PNG(FT_Bitmap* bitmap,char* name,int i,int render_mode){

  IMAGE fruit;
  int x;
  int y;

  unsigned char value;
  int p;

  switch(render_mode){

    case 0 :  fruit.width = bitmap->width;            // MONO and GRAY
              fruit.height  = bitmap->rows;

              fruit.pixels = calloc ( fruit.width * fruit.height,
                                      sizeof (PIXEL));

              for (y = 0; y < fruit.height; y++) {
                for (x = 0; x < fruit.width; x++) {

                  PIXEL * pixel = Pixel_At (& fruit, x, y);
                  p = (y * bitmap->pitch ) + x;

                  value = bitmap->buffer[p];
                  
                  if ( value != 0x00 ){
                    value = 0xff;
                  }else{
                    value = 0x00;
                  }

                  pixel->red = 255- value;
                  pixel->green = 255- value;
                  pixel->blue = 255- value;
                  pixel->alpha = 255;
                }
              }                    
              break;
    case 1 :  fruit.width = bitmap->width;            // MONO and GRAY
              fruit.height  = bitmap->rows;

              fruit.pixels = calloc ( fruit.width * fruit.height,
                                      sizeof (PIXEL));

              for (y = 0; y < fruit.height; y++) {
                for (x = 0; x < fruit.width; x++) {

                  PIXEL * pixel = Pixel_At (& fruit, x, y);
                  p = (y * bitmap->pitch ) + x;

                  value = bitmap->buffer[p];
                  
                  pixel->red = 255- value;
                  pixel->green = 255- value;
                  pixel->blue = 255- value;
                  pixel->alpha = 255;
                }
              }                    
              break;

    case 2 :
    case 3 :  fruit.width = bitmap->width / 3;        // LCD
              fruit.height  = bitmap->rows;

              fruit.pixels = calloc ( fruit.width * fruit.height, 
                                      sizeof (PIXEL));

              for (y = 0; y < fruit.height; y++) {
                for (x = 0; x < fruit.width; x++) {

                  PIXEL * pixel = Pixel_At (& fruit, x, y);
                  p = (y * bitmap->pitch ) + (x)*3;

                  value = bitmap->buffer[p];
                  pixel->red = 255- value;
                  p++;

                  value = bitmap->buffer[p];
                  pixel->green = 255- value;
                  p++;

                  value = bitmap->buffer[p];
                  pixel->blue = 255- value;

                  pixel->alpha = 255;
                }
              }
              break;

    case 4 :
    case 5 :  fruit.width = bitmap->width;            // LCD_V
              fruit.height  = bitmap->rows / 3;

              fruit.pixels = calloc ( fruit.width * fruit.height,
                                      sizeof (PIXEL));

              for (y = 0; y < fruit.height; y++) {
                for (x = 0; x < fruit.width; x++) {

                  PIXEL * pixel = Pixel_At (& fruit, x, y);
                  p = ((y*3) * bitmap->pitch ) + x;

                  value = bitmap->buffer[p];
                  pixel->red = 255- value;
                  p += bitmap->pitch;

                  value = bitmap->buffer[p];
                  pixel->green = 255- value;
                  p += bitmap->pitch;

                  value = bitmap->buffer[p];
                  pixel->blue = 255- value;

                  pixel->alpha = 255;
                }
              }
              break;

    default :   fruit.width = bitmap->width;           
                fruit.height  = bitmap->rows;
                break;
  }

  char * file_name = ( char * )calloc(150,sizeof(char));

  sprintf(file_name, "%s_%d", name, i);

  Generate_PNG (& fruit, file_name, render_mode);

  free (fruit.pixels);
}

void Read_PNG(char *filename, IMAGE * after_effect) {

  int width, height;
  png_bytep *row_pointers;

  FILE *fp = fopen(filename, "rb");

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png) abort();

  png_infop info = png_create_info_struct(png);
  if(!info) abort();

  if(setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  png_set_user_limits(png, 0x7fffffffL, 0x7fffffffL);

  png_read_info(png, info);

  width      = png_get_image_width(png, info);
  height     = png_get_image_height(png, info);

  after_effect->width = width;
  after_effect->height = height;

  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
  }

  png_read_image(png, row_pointers);

  after_effect->pixels = (PIXEL*)malloc(width * height * sizeof(PIXEL));

  for(int y = 0; y < height; y++) {

    png_bytep row = row_pointers[y];

    for(int x = 0; x < width; x++ ) {

      png_bytep px = &(row[x * 4]);

      PIXEL * pixel = Pixel_At ( after_effect, x, y);

      pixel->red = px[0];
      pixel->green = px[1];
      pixel->blue = px[2];
      pixel->alpha = px[3];
    }
  }

  fclose(fp);
}

void Add_effect_1(IMAGE* base, IMAGE* test, IMAGE* out){

  out->pixels = (PIXEL*)malloc(base->width * base->height * sizeof(PIXEL));

  PIXEL temp;

  for(int y = 0; y < base->height; y++) {
    for(int x = 0; x < base->width; x++ ) {

      PIXEL * pixel_base = Pixel_At ( base, x, y);
      PIXEL * pixel_test = Pixel_At ( test, x, y);
      PIXEL * pixel_out = Pixel_At ( out, x, y);

      if (pixel_base->red   == 255 &&
          pixel_base->green == 255 &&
          pixel_base->blue  == 255 &&
          pixel_base->alpha == 255 )
      {
        pixel_out->red    = 255;
        pixel_out->green  = 255;
        pixel_out->blue   = 255;
        pixel_out->alpha  = 255;
      }else{
        pixel_out->red    = 127;
        pixel_out->green  = 127;
        pixel_out->blue   = 127;
        pixel_out->alpha  = 255;
      }

      if (pixel_base->red   != pixel_test->red    ||
          pixel_base->green != pixel_test->green  ||
          pixel_base->blue  != pixel_test->blue   ||
          pixel_base->alpha != pixel_test->alpha  )
      {
        pixel_out->red    = 255;
        pixel_out->green  = 0;
        pixel_out->blue   = 0;
        pixel_out->alpha  = 255;
      }
    }
  }
}
