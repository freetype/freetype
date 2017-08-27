#include "bitmap.h"

/* Functions to generate MurmurHash3 values of the bitmap image */
/*data with a constant as the seed */

HASH_128 * Generate_Hash_x64_128( FT_Bitmap * bitmap, 
                                  HASH_128 * murmur)
{    
  int seed = 99; /* Dont change */

  MurmurHash3_x64_128(bitmap->buffer, 
                      (bitmap->pitch * bitmap->rows),
                      seed, 
                      murmur->hash);

  return murmur;
}
/*Not used*/
HASH_128 * Generate_Hash_x86_128( FT_Bitmap * bitmap,
                                  HASH_128 * murmur)
{    
  int seed = 99; /* Dont change */

  MurmurHash3_x86_128(bitmap->buffer,
                      (bitmap->pitch * bitmap->rows),
                      seed,
                      murmur->hash);

  return murmur;
}
/*Not used*/
HASH_32 * Generate_Hash_x86_32( FT_Bitmap * bitmap,
                                HASH_32 * murmur)
{    
  int seed = 99; /* Dont change */

  MurmurHash3_x86_32( bitmap->buffer,
                      (bitmap->pitch * bitmap->rows),
                      seed,
                      &murmur->hash);

  return murmur;
}

/* This function takes the render mode argument and */
/* returns the corresponding render_mode */
int Get_Render_Mode(const char* mode){
  /* Using -1 as the error code */
  int render_mode = -1;

  if ( strcmp(mode,"MONO") == 0 )
  {
    render_mode = 0;
  }else if ( strcmp(mode,"AA") == 0 )
  {
    render_mode = 1;
  }else if ( strcmp(mode,"RGB") == 0 )
  {
    render_mode = 2;
  }else if ( strcmp(mode,"BGR") == 0 )
  {
    render_mode = 3;
  }else if ( strcmp(mode,"VRGB") == 0 )
  {
    render_mode = 4;
  }else if ( strcmp(mode,"VBGR") == 0 )
  {
    render_mode = 5;
  }
  return render_mode;
}

/* This function takes in the IMAGE data and returns the pointer */
/* to the pixel at co-ordinates (x,y). This is used to access the */
/* pixel data */

PIXEL * Pixel_At (IMAGE * bitmap, int x, int y)
{
  return bitmap->pixels + bitmap->width * y + x;
}

/* Here we take the FT_Bitmap structure and make an IMAGE structure */
/* with pixel data from the FT_Bitmap buffer */

void Make_PNG(FT_Bitmap* bitmap,IMAGE* fruit,int i,int render_mode){

  int x;
  int y;

  unsigned char value;
  int p;

  switch(render_mode){

    case 0 :  fruit->width = bitmap->width;           /* MONO */
              fruit->height  = bitmap->rows;
              /* Allocate Memory to the IMAGE structure as per the */
              /* dimensions of the FT_Bitmap image*/
              fruit->pixels = calloc ( fruit->width * fruit->height,
                                      sizeof (PIXEL));

              for (y = 0; y < fruit->height; y++) {
                for (x = 0; x < fruit->width; x++) {
                  /* Access pixel by co-ordinates */
                  PIXEL * pixel = Pixel_At ( fruit, x, y);
                  p = (y * bitmap->pitch ) + x;

                  value = bitmap->buffer[p];
                  /* If there is some colour, make it white */
                  /* else, make it black */
                  if ( value != 0x00 ){
                    value = 0xff;
                  }else{
                    value = 0x00;
                  }
                  /* Invert the colours to make the background white*/
                  /* and the character black */
                  pixel->red = 255- value;
                  pixel->green = 255- value;
                  pixel->blue = 255- value;
                  pixel->alpha = 255;
                }
              }                    
              break;
    case 1 :  fruit->width = bitmap->width;            /* GRAY */
              fruit->height  = bitmap->rows;

              fruit->pixels = calloc ( fruit->width * fruit->height,
                                      sizeof (PIXEL));

              for (y = 0; y < fruit->height; y++) {
                for (x = 0; x < fruit->width; x++) {

                  PIXEL * pixel = Pixel_At ( fruit, x, y);
                  p = (y * bitmap->pitch ) + x;

                  /* Access the image data from the buffer */
                  value = bitmap->buffer[p];
                  /* R=G=B for Grayscale images */
                  pixel->red = 255- value;
                  pixel->green = 255- value;
                  pixel->blue = 255- value;
                  pixel->alpha = 255;
                }
              }                    
              break;

/********************************************************************/
/* FT_Bitmap has 'width' three times the size of glyph in case of   */
/* LCD rendering i.e. three adjacent values in bitmap buffer row    */
/* correspond to one RGB triplet. Accessing the buffer accordingly  */
/* and filling the RGB values of the IMAGE structure                */
/********************************************************************/
    case 2 :
    case 3 :  fruit->width = bitmap->width / 3;        /* LCD */
              fruit->height  = bitmap->rows;

              fruit->pixels = calloc ( fruit->width * fruit->height, 
                                      sizeof (PIXEL));

              for (y = 0; y < fruit->height; y++) {
                for (x = 0; x < fruit->width; x++) {

                  PIXEL * pixel = Pixel_At ( fruit, x, y);
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
/********************************************************************/
/* FT_Bitmap has 'height' three times the size of glyph in case of  */
/* LCD_v rendering i.e. three adjacent values in bitmap buffer      */
/* column correspond to one RGB triplet. Accessing the buffer       */
/* accordingly and filling the RGB values of the IMAGE structure    */
/********************************************************************/
    case 4 :
    case 5 :  fruit->width = bitmap->width;            /* LCD_V */
              fruit->height  = bitmap->rows / 3;

              fruit->pixels = calloc ( fruit->width * fruit->height,
                                      sizeof (PIXEL));

              for (y = 0; y < fruit->height; y++) {
                for (x = 0; x < fruit->width; x++) {

                  PIXEL * pixel = Pixel_At ( fruit, x, y);
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

    default :   fruit->width = bitmap->width;           
                fruit->height  = bitmap->rows;
                break;
  }
}

/********************************************************************/
/* This function generates the PNG file taking the IMAGE structure  */
/* , path to the file and the render_mode. ( Using libpng )         */
/* 32-bit RGBA images are generated. Each channel is 8-bit with     */
/* alpha channel set to 255. Transparency can be set accordingly    */
/********************************************************************/
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

  int pixel_size = 4; /* Each pixel is 4-byte */
  int depth = 8; /* Each colour is 8-bit*/
  
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

    png_byte *row = 
    png_malloc(png_ptr, sizeof(uint8_t) * bitmap->width * pixel_size);
    row_pointers[y] = row;

    for (x = 0; x < bitmap->width; x++) {

      PIXEL * pixel = Pixel_At (bitmap, x, y);

        if (render_mode == 3 || render_mode == 5)
        { /* For BGRA images */
          *row++ = pixel->blue;
          *row++ = pixel->green;
          *row++ = pixel->red;
          *row++ = pixel->alpha;
          continue;
        }
        /* For RGBA images */
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
  
  printf("%s\n", path );
  for (y = 0; y < bitmap->height; y++) {
    png_free (png_ptr, row_pointers[y]);
  }
  png_free (png_ptr, row_pointers);
  
  free (bitmap->pixels);

  png_failure:
  png_create_info_struct_failed:
    png_destroy_write_struct (&png_ptr, &info_ptr);
  png_create_write_struct_failed:
    fclose (fp);
  fopen_failed:
    return status;
}

/********************************************************************/
/* Compare the MurmurHash3 values ( x64_128 bit implemented )       */
/* Returns 1 if they are different and 0 if identical.              */
/********************************************************************/
int Compare_Hash(HASH_128* hash_b, HASH_128* hash_t){
  if (hash_b->hash[0] != hash_t->hash[0] ||
      hash_b->hash[1] != hash_t->hash[1] ||
      hash_b->hash[2] != hash_t->hash[2] ||
      hash_b->hash[3] != hash_t->hash[3] )
  {
    return 1;
  }
  return 0;
}

/********************************************************************/
/* Returns the index of the first-column to have a coloured pixel.  */
/* 'Coloured' means the pixel isn't in the background               */
/* Background Colour RGB = ( 255,255,255 )                          */
/********************************************************************/
int First_Column(IMAGE* input){

  int x, y;

  for ( x = 0; x < input->width; ++x)
  {
    for ( y = 0; y < input->height; ++y)
    {
      PIXEL * pixel_result = Pixel_At ( input, x, y);

      if ( pixel_result->red    == 255 &&
           pixel_result->green  == 255 &&
           pixel_result->blue   == 255 &&
           pixel_result->alpha  == 255 )
      {
        continue;
      }else{
        return x;
      }
    }
  }
  return input->width;
}

/* Returns the index of the first-row to have a coloured pixel. */
int First_Row(IMAGE* input){

  int x, y;
  
  for ( y = 0; y < input->height; ++y)
  {
    for ( x = 0; x < input->width; ++x)
    {
      PIXEL * pixel_result = Pixel_At ( input, x, y);

      if ( pixel_result->red    == 255 &&
           pixel_result->green  == 255 &&
           pixel_result->blue   == 255 &&
           pixel_result->alpha  == 255 )
      {
        continue;
      }else{
        return y;
      }
    }
  }
  return input->height;
}

/********************************************************************/
/* The following two  functions takes two IMAGES with different     */
/* dimensions and aligns the two images based on the first pixel    */
/* that isn't in the background i.e. RGB != ( 255,255,255 ).        */
/* The first argument is the one which has smaller height/width.    */
/* Columns/Rows are appended to the smaller image to match the      */
/* dimensions.                                                      */
/********************************************************************/
IMAGE* Append_Columns(IMAGE* small, IMAGE* big){
  /* small->width < big->width */
  int x, y;

  IMAGE* result = (IMAGE*)malloc(sizeof(IMAGE));

  result->height = small->height;
  result->width = big->width; 
  result->pixels = 
  (PIXEL*) malloc(result->width * result->height * sizeof(PIXEL));

  int first_col = First_Column(big);

  for ( x = 0; x < first_col; ++x) /* Filling White columns */
  {
    for ( y = 0; y < result->height; ++y)
    {
      PIXEL * pixel_result = Pixel_At ( result, x, y);
      /* Filling White columns */
      pixel_result->red    = 255;
      pixel_result->green  = 255;
      pixel_result->blue   = 255;
      pixel_result->alpha  = 255; 
    }
  }

  for ( y = 0; y < result->height; ++y) 
  {
    for ( x = first_col; x < first_col + small->width; ++x)
    {
      PIXEL * pixel_small = Pixel_At ( small, (x - first_col), y);
      PIXEL * pixel_result = Pixel_At ( result, x, y);
      /* Putting the original IMAGE data */
      pixel_result->red    = pixel_small->red;
      pixel_result->green  = pixel_small->green;
      pixel_result->blue   = pixel_small->blue;
      pixel_result->alpha  = pixel_small->alpha; 
    }
  }

  for ( x = first_col + small->width; x < result->width; ++x)
  {
    for ( y = 0; y < result->height; ++y)
    {
      PIXEL * pixel_result = Pixel_At ( result, x, y);
      /* Filling White columns */
      pixel_result->red    = 255;
      pixel_result->green  = 255;
      pixel_result->blue   = 255;
      pixel_result->alpha  = 255; 
    }
  }

  return result;
}

IMAGE* Append_Rows(IMAGE* small, IMAGE* big){
  /* small->height < big->height */
  int x, y;

  IMAGE* result = (IMAGE*)malloc(sizeof(IMAGE));

  result->height = big->height;
  result->width = small->width; 
  result->pixels = 
  (PIXEL*) malloc(result->width * result->height * sizeof(PIXEL));

  int first_row = First_Row(big);

  for ( y = 0; y < first_row; ++y)
  {
    for ( x = 0; x < result->width; ++x) 
    {
      PIXEL * pixel_result = Pixel_At ( result, x, y);
      /* Filling White rows */
      pixel_result->red    = 255;
      pixel_result->green  = 255;
      pixel_result->blue   = 255;
      pixel_result->alpha  = 255; 
    }
  }

  for ( y = first_row; y < first_row + small->height; ++y)
  {
    for ( x = 0; x < result->width; ++x)
    { 
      PIXEL * pixel_small = Pixel_At ( small, x, y - first_row);
      PIXEL * pixel_result = Pixel_At ( result, x, y);
      /* Putting the original IMAGE data */
      pixel_result->red    = pixel_small->red;
      pixel_result->green  = pixel_small->green;
      pixel_result->blue   = pixel_small->blue;
      pixel_result->alpha  = pixel_small->alpha; 
    }
  }

  for ( y = first_row + small->height; y < result->height; ++y)
  {
    for ( x = 0; x < result->width; ++x)
    {
      PIXEL * pixel_result = Pixel_At ( result, x, y);
      /* Filling White rows */
      pixel_result->red    = 255;
      pixel_result->green  = 255;
      pixel_result->blue   = 255;
      pixel_result->alpha  = 255; 
    }
  }

  return result; 
}

/********************************************************************/
/* This fuction visually highlights the differences between the two */
/* images generated for the same glyph. There are two effects each  */
/* given an Effect_ID.                                              */
/* This function generates a new IMAGE structure after comparing and*/
/* adding the desired effect.                                       */
/*                                                                  */
/* Effect_ID =1 means that the differences in pixels are highlighted*/
/* and the pixels that are same are in GRAY (RGB = (128,128,128))   */
/*                                                                  */
/* Effect_ID =2 means that the differences in pixels are highlighted*/
/* over the 'base' version's rendered image                         */
/*                                                                  */
/* Highlighting is done in RED colour i.e. RGB=( 255, 0, 0) for     */
/* mono, grayscale and RGB-LCD displays and for BGR-LCD displays    */
/* is is done in BLUE colour i.e. RGB = ( 0, 0, 255).               */
/********************************************************************/
int Add_effect(IMAGE* base, IMAGE* test, IMAGE* out, int Effect_ID)
{ 
  int pixel_diff = 0;
  int x,y;
  /* new IMAGE */
  out->width = base->width;
  out->height = base->height;
  out->pixels = 
  (PIXEL*)malloc(base->width * base->height * sizeof(PIXEL));

  for( y = 0; y < base->height; y++) {
    for( x = 0; x < base->width; x++ ) {

      PIXEL * pixel_base = Pixel_At ( base, x, y);
      PIXEL * pixel_test = Pixel_At ( test, x, y);
      PIXEL * pixel_out = Pixel_At ( out, x, y);

      if (Effect_ID == 1)
      { /* If colour is white */
        if (pixel_base->red   == 255 &&
            pixel_base->green == 255 &&
            pixel_base->blue  == 255 &&
            pixel_base->alpha == 255 )
        {
          pixel_out->red    = 255; /* White*/
          pixel_out->green  = 255;
          pixel_out->blue   = 255;
          pixel_out->alpha  = 255;
        }else{
          pixel_out->red    = 127; /* Gray */
          pixel_out->green  = 127;
          pixel_out->blue   = 127;
          pixel_out->alpha  = 255;
        }
      }
      /* if colour is different*/
      if (pixel_base->red   != pixel_test->red    ||
          pixel_base->green != pixel_test->green  ||
          pixel_base->blue  != pixel_test->blue   ||
          pixel_base->alpha != pixel_test->alpha  )
      { /* Highlighting pixels */
        pixel_out->red    = 255; 
        pixel_out->green  = 0;
        pixel_out->blue   = 0;
        pixel_out->alpha  = 255;

        pixel_diff++;

      }else{
        if (Effect_ID == 2)
        {
          pixel_out->red    = pixel_base->red;
          pixel_out->green  = pixel_base->green;
          pixel_out->blue   = pixel_base->blue;
          pixel_out->alpha  = pixel_base->alpha;
        }
      }
    }
  }
  return pixel_diff;
}

/********************************************************************/
/* This function takes two IMAGE structures and stitches the images */
/* horizontally to make one IMAGE.                                  */
/* i.e. ( result->width =  left->width + right->width )             */
/* The first argument forms the left part of the image and the      */
/* second forms the right part.                                     */
/* This returns the pointer to the stitched image                   */
/********************************************************************/

void Stitch(IMAGE* left, IMAGE* right, IMAGE* result){

  int x, y;

  result->width = left->width + right->width;
  result->height = MAX(left->height, right->height); 

  result->pixels = 
  (PIXEL*)calloc(result->width * result->height, sizeof(PIXEL));

  for ( y = 0; y < left->height; ++y)
  {
    for ( x = 0; x < left->width; ++x)
    {
      PIXEL * pixel_left = Pixel_At ( left, x, y);
      PIXEL * pixel_result = Pixel_At ( result, x, y);
      /* Filling Left part of the image*/
      pixel_result->red    = pixel_left->red;
      pixel_result->green  = pixel_left->green;
      pixel_result->blue   = pixel_left->blue;
      pixel_result->alpha  = pixel_left->alpha; 
    }
  }

  for ( y = 0; y < right->height; ++y)
  {
    for ( x = left->width; x < result->width; ++x)
    {
      PIXEL * pixel_right = Pixel_At ( right, x - left->width, y);
      PIXEL * pixel_result = Pixel_At ( result, x, y);
      /* Filling right part of the image*/
      pixel_result->red    = pixel_right->red;
      pixel_result->green  = pixel_right->green;
      pixel_result->blue   = pixel_right->blue;
      pixel_result->alpha  = pixel_right->alpha; 
    }
  }
}

/* This prints table-headers to a HTML file for the list-view page */
void Print_Head( FILE* fp ){
  printf("   *** Generating Images ***   \n");
  fprintf(fp,
  "<html>\n\
    <head>\n\
      <title>\n\
        Glyph_Diff\n\
      </title>\n\
      <script src=\"../../../../../scripts/top.js\" type=\"text/javascript\"></script>\n\
      <link href=\"../../../../../styles/top.css\" rel=\"stylesheet\" type=\"text/css\" >\n\
    </head>\n\
    <body>\n\
    <table>\n\
      <colgroup>\n\
        <col span=\"3\" style=\"background-color:white\">\n\
        <col style=\"width:50%%\">\n\
      </colgroup>\n\
    <thead>\n\
      <tr>\n\
      <th onclick=\"sort_t(data,0,asc1);asc1*=-1;asc2=1;asc3=1;\">\n\
        <a href=\"#\">Index</a>\n\
      </th>\n\
      <th onclick=\"sort_t(data,1,asc2);asc2*=-1;asc3=1;asc1=1;\">\n\
        <a href=\"#\">Name</a>\n\
      </th>\n\
      <th onclick=\"sort_t(data,2,asc3);asc3*=-1;asc1=1;asc2=1;\">\n\
        <a href=\"#\">Diff</a>\n\
      </th>\n\
      <th>\n\
        Images\n\
      </th>\n\
      </tr>\n\
    </thead>\n\
    <tbody id=\"data\">\n" );
}

/* This prints a row to the HTML file for the list-view page. */
void Print_Row( FILE* fp, int index, char* name, int diff )
{
  fprintf(fp,
    "<tr>\n\
      <td>%04d</td>\n\
      <td>%s</td>\n\
      <td>%04d</td>\n\
      <td id=\"image_row\">\
        <img id=\"sprite\" src=\"images/%s.png\" onclick=\"frame_2_source(this)\">\
      </td>\n\
    </tr>\n", index, name, diff, name);
}

/* To calculate the Difference-Metric used in the list-view page */
int Image_Diff( IMAGE* base, IMAGE* test){

  int diff = 0;

  int max_width = MAX(base->width, test->width);
  int min_width = MIN(base->width, test->width);

  int max_height = MAX(base->height, test->height);
  int min_height = MIN(base->height, test->height);

  diff = (max_width - min_width) * max_height;
  diff += (max_height - min_height) * min_width;

  return diff;
}
/* For more information on the list-view page, go to README */
