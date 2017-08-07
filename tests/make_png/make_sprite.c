#include "bitmap.h"

int main(int argc, char const *argv[])
{
  if(argc != 6)
  {
    printf("Usage: ./sprite <base .so> <test .so> \
           <fnt_file> <pt_size> <render_mode>\n");

    printf("Values for render_mode    0 - monochrome\n");
    printf("                          1 - anti-aliased\n");
    printf("                          2 - lcd horizontal-RGB\n");
    printf("                          3 - lcd horizontal-BGR\n");
    printf("                          4 - lcd vertical-RGB\n");
    printf("                          5 - lcd vertical-BGR\n");

    return 0;
  }

/*******************************************************************/
  /* variables declaration */
  const char*      base_version; 
  const char*      test_version; 
  const char*      font_file;
  int              size;
  int              render_mode; 

  int              load_flag;  /* FT_LOAD_XXX */
  int              render_flag; /* FT_RENDER_MODE_XXX */
  int              target_flag; /* FT_LOAD_TARGET_XXX */

  base_version     = argv[1];
  test_version     = argv[2];
  
  font_file        = argv[3];
  size             = atoi(argv[4]);
  render_mode      = atoi(argv[5]);

  FT_Library       base_library;
  FT_Face          base_face;
  FT_GlyphSlot     base_slot;
  FT_Bitmap*       base_bitmap;
  FT_Bitmap        base_target;

  FT_Library       test_library;
  FT_Face          test_face;
  FT_GlyphSlot     test_slot;
  FT_Bitmap*       test_bitmap;
  FT_Bitmap        test_target;

  FT_Error         error;

  int alignment = 4;
  char * output_file_name = ( char * )calloc(50,sizeof(char));

  IMAGE* base_png        = (IMAGE*)malloc(sizeof(IMAGE));
  IMAGE* test_png        = (IMAGE*)malloc(sizeof(IMAGE));
  IMAGE* after_effect_1  = (IMAGE*)malloc(sizeof(IMAGE));
  IMAGE* after_effect_2  = (IMAGE*)malloc(sizeof(IMAGE));
  IMAGE* combi_effect_1  = (IMAGE*)malloc(sizeof(IMAGE));
  IMAGE* combi_effect_2  = (IMAGE*)malloc(sizeof(IMAGE));
  IMAGE* output          = (IMAGE*)malloc(sizeof(IMAGE));

  HASH_128 *  base_murmur = (HASH_128 *) malloc(sizeof(HASH_128)) ;
  HASH_128 *  test_murmur = (HASH_128 *) malloc(sizeof(HASH_128)) ;  

  int Is_Different;
  int pixel_diff, i;

  char glyph_name[50] = ".not-def";
/*******************************************************************/

  FT_Error ( *Base_Init_FreeType )( FT_Library* );

  FT_Error ( *Base_Library_SetLcdFilter )( FT_Library ,
                                           FT_LcdFilter );

  FT_Error ( *Base_New_Face )( FT_Library,
                               const char*,
                               FT_Long,
                               FT_Face* );

  FT_Error ( *Base_Set_Char_Size )( FT_Face, 
                                  FT_F26Dot6,
                                  FT_F26Dot6,
                                  FT_UInt, 
                                  FT_UInt );

  FT_Error ( *Base_Load_Glyph )( FT_Face,
                                 FT_UInt,
                                 FT_Int32 );

  FT_Error ( *Base_Render_Glyph )( FT_GlyphSlot,
                                   FT_Render_Mode );

  void     ( *Base_Bitmap_Init )( FT_Bitmap* );

  FT_Error ( *Base_Bitmap_Convert )( FT_Library,
                                     const FT_Bitmap*,
                                     FT_Bitmap*, 
                                     FT_Int);

  FT_Error ( *Base_Done_Face )( FT_Face );

  FT_Error ( *Base_Done_FreeType )( FT_Library );

/*******************************************************************/

  FT_Error ( *Test_Init_FreeType )( FT_Library* );

  FT_Error ( *Test_Library_SetLcdFilter )( FT_Library ,
                                           FT_LcdFilter );

  FT_Error ( *Test_New_Face )( FT_Library,
                               const char*,
                               FT_Long,
                               FT_Face* );

  FT_Error ( *Test_Set_Char_Size )( FT_Face, 
                                    FT_F26Dot6,
                                    FT_F26Dot6,
                                    FT_UInt, 
                                    FT_UInt );

  FT_Error ( *Test_Load_Glyph )( FT_Face,
                                 FT_UInt,
                                 FT_Int32 );

  FT_Error ( *Test_Render_Glyph )( FT_GlyphSlot,
                                   FT_Render_Mode );

  void     ( *Test_Bitmap_Init )( FT_Bitmap* );

  FT_Error ( *Test_Bitmap_Convert )( FT_Library,
                                     const FT_Bitmap*,
                                     FT_Bitmap*, 
                                     FT_Int);

  FT_Error ( *Test_Done_Face )( FT_Face );
  
  FT_Error ( *Test_Done_FreeType )( FT_Library );

/*******************************************************************/

  void* base_handle = dlopen( base_version, RTLD_LAZY );
  if (!base_handle) {
    fputs(dlerror(), stderr);
    exit(1);
  }

  void* test_handle = dlopen( test_version, RTLD_LAZY );
  if (!test_handle) {
    fputs(dlerror(), stderr);
    exit(1);
  }

  dlerror();

/*******************************************************************/

  *(void**)( & Base_Library_SetLcdFilter ) = dlsym(base_handle,
                                          "FT_Library_SetLcdFilter");
  *(void**)( & Base_Init_FreeType )   = dlsym(base_handle,
                                              "FT_Init_FreeType");
  *(void**)( & Base_New_Face )        = dlsym(base_handle,
                                              "FT_New_Face");
  *(void**)( & Base_Set_Char_Size )   = dlsym(base_handle,
                                              "FT_Set_Char_Size");
  *(void**)( & Base_Load_Glyph )      = dlsym(base_handle,
                                              "FT_Load_Glyph");
  *(void**)( & Base_Render_Glyph )    = dlsym(base_handle,
                                              "FT_Render_Glyph");
  *(void**)( & Base_Bitmap_Init )     = dlsym(base_handle,
                                              "FT_Bitmap_Init");
  *(void**)( & Base_Bitmap_Convert )  = dlsym(base_handle,
                                              "FT_Bitmap_Convert");
  *(void**)( & Base_Done_Face )       = dlsym(base_handle,
                                              "FT_Done_Face");
  *(void**)( & Base_Done_FreeType )   = dlsym(base_handle,
                                              "FT_Done_FreeType");

/*******************************************************************/

  *(void**)( & Test_Library_SetLcdFilter ) = dlsym(test_handle,
                                          "FT_Library_SetLcdFilter");
  *(void**)( & Test_Init_FreeType )   = dlsym(test_handle,
                                              "FT_Init_FreeType");
  *(void**)( & Test_New_Face )        = dlsym(test_handle,
                                              "FT_New_Face");
  *(void**)( & Test_Set_Char_Size )   = dlsym(test_handle,
                                              "FT_Set_Char_Size");
  *(void**)( & Test_Load_Glyph )      = dlsym(test_handle,
                                              "FT_Load_Glyph");
  *(void**)( & Test_Render_Glyph )    = dlsym(test_handle,
                                              "FT_Render_Glyph");
  *(void**)( & Test_Bitmap_Init )     = dlsym(test_handle,
                                              "FT_Bitmap_Init");
  *(void**)( & Test_Bitmap_Convert )  = dlsym(test_handle,
                                              "FT_Bitmap_Convert");
  *(void**)( & Test_Done_Face )       = dlsym(test_handle,
                                              "FT_Done_Face");
  *(void**)( & Test_Done_FreeType )   = dlsym(test_handle,
                                              "FT_Done_FreeType");

/*******************************************************************/

  switch ( render_mode ) {
  case 0: render_flag   = FT_RENDER_MODE_MONO;
          load_flag     = FT_LOAD_MONOCHROME;
          target_flag   = FT_LOAD_TARGET_MONO;
          break;

  case 1: render_flag   = FT_RENDER_MODE_NORMAL;
          load_flag     = FT_LOAD_DEFAULT;
          target_flag   = FT_LOAD_TARGET_NORMAL;
          break;

  case 2: render_flag   = FT_RENDER_MODE_LCD;
          load_flag     = FT_LOAD_DEFAULT;
          target_flag   = FT_LOAD_TARGET_LCD;
          break;

  case 3: render_flag   = FT_RENDER_MODE_LCD;
          load_flag     = FT_LOAD_DEFAULT;
          target_flag   = FT_LOAD_TARGET_LCD;
          break;

  case 4: render_flag   = FT_RENDER_MODE_LCD_V;
          load_flag     = FT_LOAD_DEFAULT;
          target_flag   = FT_LOAD_TARGET_LCD_V;
          break;

  case 5: render_flag   = FT_RENDER_MODE_LCD_V;
          load_flag     = FT_LOAD_DEFAULT;
          target_flag   = FT_LOAD_TARGET_LCD_V;
          break;

  default:render_flag   = FT_RENDER_MODE_NORMAL;
          load_flag     = FT_LOAD_DEFAULT;
          target_flag   = FT_LOAD_TARGET_NORMAL;
  }

  error = Base_Init_FreeType( &base_library );
  if(error){
    printf("Error while initialising library\n");
    exit(1);
  }
  error = Test_Init_FreeType( &test_library );
  if(error){
    printf("Error while initialising library\n");
    exit(1);
  }

  if (render_mode > 1 )
  {
    error = Base_Library_SetLcdFilter( base_library,
                                       FT_LCD_FILTER_DEFAULT );
    if(error){
      printf("Error while setting LCD filter\n");
      exit(1);
    }
    error = Test_Library_SetLcdFilter( test_library,
                                       FT_LCD_FILTER_DEFAULT );
    if(error){
      printf("Error while setting LCD filter\n");
      exit(1);
    }
  }

  error = Base_New_Face( base_library, 
                         font_file, 
                         0, 
                         &base_face );
  if(error){
    printf("Error loading new face\n");
    exit(1);
  }
  error = Test_New_Face( test_library, 
                         font_file, 
                         0, 
                         &test_face );
  if(error){
    printf("Error loading new face\n");
    exit(1);
  }

  error = Base_Set_Char_Size( base_face,
                              size * 64, 
                              0, 
                              DPI,
                              0 );
  if(error){
    printf("Error setting character size\n");
    exit(1);
  }
  error = Test_Set_Char_Size( test_face,
                              size * 64, 
                              0, 
                              DPI,
                              0 );
  if(error){
    printf("Error setting character size\n");
    exit(1);
  }

  base_slot = base_face->glyph;
  test_slot = test_face->glyph;

  /* If folders aren't there, create folders */
  struct stat st = {0};
  if (stat("./html/", &st) == -1) {
      mkdir("./html/", 0777);
  }

  if (stat("./html/images/", &st) == -1) {
      mkdir("./html/images/", 0777);
  }

  /* Initialising file pointer for the list-view*/
  FILE* fp = fopen("html/index.html","w");
 	
 	Print_Head(fp,base_face->family_name, base_face->style_name, size);

/* Need to write code to check the values in FT_Face and compare */
  for ( i = 0; i < base_face->num_glyphs; ++i)
  {
    error = Base_Load_Glyph( base_face,
                             i, 
                             load_flag | target_flag);
    if(error){
      printf("Error loading glyph\n");
      exit(1);
    }
    error = Test_Load_Glyph( test_face,
                             i, 
                             load_flag | target_flag);
    if(error){
      printf("Error loading glyph\n");
      exit(1);
    }

    error = Base_Render_Glyph( base_slot, 
                               render_flag);
    if(error){
      printf("Error rendering the glyph\n");
      exit(1);
    }
    error = Test_Render_Glyph( test_slot, 
                               render_flag);
    if(error){
      printf("Error rendering the glyph\n");
      exit(1);
    }

    base_bitmap = &base_slot->bitmap;
    test_bitmap = &test_slot->bitmap;

    if (base_bitmap->width == 0 || base_bitmap->rows == 0)
    {
      printf("Empty Glyph in glyph-index %d\n", i);
      continue;
    }
    /* Generating Hashes */
    base_murmur = Generate_Hash_x64_128(base_bitmap,base_murmur);
    test_murmur = Generate_Hash_x64_128(test_bitmap,test_murmur);

    Is_Different = Compare_Hash(base_murmur, test_murmur);
 	/* To convert 1-bit monochrome to 8-bit bitmap */
 	/* because we need 8-bit colour to generate images */
    Base_Bitmap_Init( &base_target );
    Test_Bitmap_Init( &test_target );

    error = Base_Bitmap_Convert(  base_library, 
                                  base_bitmap, 
                                  &base_target,
                                  alignment);
    if(error){
      printf("Error converting the bitmap\n");
      exit(1);
    }
    error = Test_Bitmap_Convert(  test_library, 
                                  test_bitmap, 
                                  &test_target,
                                  alignment);
    if(error){
      printf("Error converting the bitmap\n");
      exit(1);
    }

    if (Is_Different != 0) 
    {
      pixel_diff = 0; /* Difference metric*/
      if (render_mode == 0)
      { /* If monochrome, take the converted image*/
        Make_PNG( &base_target, base_png, i, render_mode );
        Make_PNG( &test_target, test_png, i, render_mode );

      }else{

        Make_PNG( base_bitmap, base_png, i, render_mode );
        Make_PNG( test_bitmap, test_png, i, render_mode );
      }
      /* Aligning images and appending rows */
      if (base_png->height < test_png->height)
      {
        base_png = Append_Rows(base_png, test_png);
      }else if (base_png->height > test_png->height)
      {
        test_png = Append_Rows(test_png, base_png);
      }
      /* Aligning images and appending columns */
      if (base_png->width < test_png->width)
      {
        base_png = Append_Columns(base_png, test_png);
      }else if (base_png->width > test_png->width)
      {
        test_png = Append_Columns(test_png, base_png);
      }
      /* Updating Difference metric */
      pixel_diff += Image_Diff( base_png, test_png);

      Add_effect( base_png, test_png, after_effect_1, 1);
      pixel_diff = Add_effect( base_png, test_png, after_effect_2,2);

      Stitch( base_png, test_png, combi_effect_1);
      Stitch( after_effect_1, after_effect_2, combi_effect_2);

      Stitch( combi_effect_1, combi_effect_2, output);

      if (FT_HAS_GLYPH_NAMES(base_face))
      {
        FT_Get_Glyph_Name(  base_face,
                            i,
                            glyph_name,
                            50 );
      }

      sprintf( output_file_name, "./html/images/%s.png", glyph_name );

      Generate_PNG ( output, output_file_name, render_mode );
      /* To print table row to HTML file */
      Print_Row(fp,i,glyph_name,pixel_diff );
    }
  }
 /* HTML footer */
  fprintf(fp,
        "</tbody>\n\
        </table>\n\
      </body>\n\
    </html>\n" );

  fclose(fp);

  error = Base_Done_Face(base_face);
  if(error){
    printf("Error freeing the face object");
    exit(1);
  }
  error = Test_Done_Face(test_face);
  if(error){
    printf("Error freeing the face object");
    exit(1);
  }
  
  error = Base_Done_FreeType(base_library);
  if(error){
    printf("Error freeing library");
    exit(1);
  }
  error = Test_Done_FreeType(test_library);
  if(error){
    printf("Error freeing library");
    exit(1);
  }

  dlclose(base_handle);
  dlclose(test_handle);

  return 0;
}
